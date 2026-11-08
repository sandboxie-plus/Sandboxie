#include "stdafx.h"
#include "McpTools.h"
#include "McpDocs.h"
#include "McpKnowledge.h"
#include "../SandMan.h"
// for g_CertInfo - the certificate state lives there
#include "../Windows/SettingsWindow.h"
#include "../SbiePlusAPI.h"
#include "../Helpers/IniHighlighter.h"
#include "McpRedact.h"
#include "../../QSbieAPI/SbieTrace.h"
#include <QJsonDocument>
#include <QRegularExpression>

namespace McpTools
{

static CMcpDocs* g_pDocs = NULL;

static CMcpKnowledge* g_pKnowledge = NULL;

void SetKnowledge(CMcpKnowledge* pKnowledge)
{
	g_pKnowledge = pKnowledge;
}

void SetDocs(CMcpDocs* pDocs)
{
	g_pDocs = pDocs;
}

//
// Response caps. Agent context is finite and a box section can be long, so
// every list is bounded and says so in the result rather than silently
// truncating - a model that cannot tell it got a partial answer will reason
// from it as if it were complete.
//
static const int kMaxBoxes = 200;
static const int kMaxSettings = 500;
static const int kMaxProcesses = 200;
#ifdef SBIE_HAS_DIAG_EVENTS
static const int kMaxDiagEvents = 200;
#endif
// A trace is effectively unbounded - a few seconds of a browser starting runs to
// tens of thousands of entries - so this is a page, not a limit on what exists.
static const int kMaxTraceRows = 300;
static const int kMaxLogRows = 200;

QJsonObject MakeTextResult(const QString& Text, const QJsonObject& Structured)
{
	QJsonObject Item;
	Item["type"] = "text";
	Item["text"] = Text;

	QJsonArray Content;
	Content.append(Item);

	QJsonObject Result;
	Result["content"] = Content;
	if (!Structured.isEmpty())
		Result["structuredContent"] = Structured;
	return Result;
}

void AppendNote(QJsonObject& Result, const QString& Note)
{
	QJsonObject Item;
	Item["type"] = "text";
	Item["text"] = Note;

	// Its own block rather than glued onto the end of the answer, so it
	// cannot be mistaken for part of what the tool reported.
	QJsonArray Content = Result.value("content").toArray();
	Content.append(Item);
	Result["content"] = Content;
}

QJsonObject MakeErrorResult(const QString& Text)
{
	QJsonObject Result = MakeTextResult(Text);
	Result["isError"] = true;
	return Result;
}

static QJsonObject QueryLimitSchema(const QString& QueryDescription)
{
	QJsonObject Query;
	Query["type"] = "string";
	Query["description"] = QueryDescription;

	QJsonObject Limit;
	Limit["type"] = "integer";
	Limit["description"] = QObject::tr("How many results at most. Defaults to 10.");

	QJsonObject Properties;
	Properties["query"] = Query;
	Properties["limit"] = Limit;

	QJsonArray Required;
	Required.append("query");

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	Schema["required"] = Required;
	return Schema;
}

static QJsonObject IdSchema()
{
	QJsonObject Id;
	Id["type"] = "string";
	Id["description"] = QObject::tr("The entry id, as returned by kb_search or kb_match.");

	QJsonObject Properties;
	Properties["id"] = Id;

	QJsonArray Required;
	Required.append("id");

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	Schema["required"] = Required;
	return Schema;
}

static QJsonObject BoxLimitSchema()
{
	QJsonObject Box;
	Box["type"] = "string";
	Box["description"] = QObject::tr("Name or alias of the sandbox to explain.");

	QJsonObject Limit;
	Limit["type"] = "integer";
	Limit["description"] = QObject::tr("How many candidates at most. Defaults to 5.");

	QJsonObject Properties;
	Properties["box"] = Box;
	Properties["limit"] = Limit;

	QJsonArray Required;
	Required.append("box");

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	Schema["required"] = Required;
	return Schema;
}

static QJsonObject NoArgsSchema()
{
	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = QJsonObject();
	return Schema;
}

static QJsonObject BoxNameSchema(const QString& Description)
{
	QJsonObject Name;
	Name["type"] = "string";
	Name["description"] = Description;

	QJsonObject Properties;
	Properties["box"] = Name;

	QJsonArray Required;
	Required.append("box");

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	Schema["required"] = Required;
	return Schema;
}

// One required string argument, optionally a second that is not required.
static QJsonObject StringArgSchema(const QString& Name, const QString& Description,
	const QString& Optional = QString(), const QString& OptionalDescription = QString())
{
	QJsonObject First;
	First["type"] = "string";
	First["description"] = Description;

	QJsonObject Properties;
	Properties[Name] = First;

	if (!Optional.isEmpty()) {
		QJsonObject Second;
		Second["type"] = "integer";
		Second["description"] = OptionalDescription;
		Properties[Optional] = Second;
	}

	QJsonArray Required;
	Required.append(Name);

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	Schema["required"] = Required;
	return Schema;
}

static QJsonObject LogQuerySchema()
{
	QJsonObject Box;
	Box["type"] = "string";
	Box["description"] = QObject::tr("Limit to one sandbox, by name or alias. Many entries carry no box.");

	QJsonObject Kind;
	Kind["type"] = "string";
	Kind["description"] = QObject::tr("One of message, diagnostic or notice. Omit for all three.");

	QJsonObject Contains;
	Contains["type"] = "string";
	Contains["description"] = QObject::tr("Only entries whose text contains this.");

	QJsonObject Limit;
	Limit["type"] = "integer";
	Limit["description"] = QObject::tr("Entries to return, newest first. Default 100, maximum 200.");

	QJsonObject Properties;
	Properties["box"] = Box;
	Properties["kind"] = Kind;
	Properties["contains"] = Contains;
	Properties["limit"] = Limit;

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	return Schema;
}

static QJsonObject TraceQuerySchema()
{
	QJsonObject Box;
	Box["type"] = "string";
	Box["description"] = QObject::tr("Limit to one sandbox, by name or alias. Omit for every sandbox.");

	QJsonObject Contains;
	Contains["type"] = "string";
	Contains["description"] = QObject::tr("Only rows whose path or message contains this text.");

	QJsonObject Status;
	Status["type"] = "string";
	Status["description"] = QObject::tr("Only rows with this status, for example Denied. Omit for all.");

	QJsonObject Limit;
	Limit["type"] = "integer";
	Limit["description"] = QObject::tr("Rows to return, default 100, maximum 300.");

	QJsonObject Offset;
	Offset["type"] = "integer";
	Offset["description"] = QObject::tr("Rows to skip, for paging. The result reports the total.");

	QJsonObject Properties;
	Properties["box"] = Box;
	Properties["contains"] = Contains;
	Properties["status"] = Status;
	Properties["limit"] = Limit;
	Properties["offset"] = Offset;

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	return Schema;
}

static QJsonObject Tool(const QString& Name, const QString& Title, const QString& Description, const QJsonObject& Schema)
{
	QJsonObject Out;
	Out["name"] = Name;
	Out["title"] = Title;
	Out["description"] = Description;
	Out["inputSchema"] = Schema;

	// Advertising read-only lets a client skip a confirmation it would otherwise
	// have to ask for. It is a hint, so it is only ever set where it is true.
	QJsonObject Annotations;
	Annotations["readOnlyHint"] = true;
	Out["annotations"] = Annotations;
	return Out;
}

QJsonArray Describe()
{
	QJsonArray Tools;

	Tools.append(Tool("sandboxie_version", QObject::tr("Sandboxie version"),
		QObject::tr("Version of Sandboxie-Plus and of the operating system it is running on. "
			"Call this first: fixes and setting names differ between versions, and a "
			"suggestion that does not apply to the installed build wastes a round trip."),
		NoArgsSchema()));

	Tools.append(Tool("box_list", QObject::tr("List sandboxes"),
		QObject::tr("Every configured sandbox with its type, whether it is enabled, how many "
			"processes are running in it, and whether it holds any content. Boxes often "
			"have an alias, which is what the user calls them - refer to a box the way "
			"displayName shows it, and pass either the name or the alias to other tools."),
		NoArgsSchema()));

	Tools.append(Tool("box_get_config", QObject::tr("Read sandbox configuration"),
		QObject::tr("The settings explicitly set on one sandbox, as name and value pairs. "
			"Values inherited from templates and global defaults are not included, so "
			"an absent setting means it is not set here, not that it has no effect."),
		BoxNameSchema(QObject::tr("Name or alias of the sandbox, as reported by box_list."))));

	Tools.append(Tool("box_processes", QObject::tr("What is running in a sandbox"),
		QObject::tr("The processes currently running in a sandbox, with their image paths, "
			"command lines and start times. Empty is a useful answer too: a program that "
			"exits immediately leaves nothing here, which points at a start-up failure "
			"rather than a running-program problem."),
		BoxNameSchema(QObject::tr("Name or alias of the sandbox."))));

#ifdef SBIE_HAS_DIAG_EVENTS
	Tools.append(Tool("box_diagnostics", QObject::tr("Sandbox diagnostic events"),
		QObject::tr("Diagnostic events Sandboxie has recorded for a sandbox - the same ones the "
			"troubleshooting wizard reports. These are the driver saying what it refused "
			"or had trouble with, so read them before guessing at a cause. Paths and "
			"account names are redacted."),
		BoxNameSchema(QObject::tr("Name or alias of the sandbox."))));
#endif

	Tools.append(Tool("log_query", QObject::tr("Search the message log"),
		QObject::tr("Everything Sandboxie-Plus has reported this session: driver and service "
			"messages, diagnostic events, and notices from SandMan itself. This is the "
			"same log the user is looking at, newest first. Unlike the resource trace it "
			"needs nothing switched on, so it is the first place to look. Paths and "
			"account names are redacted."),
		LogQuerySchema()));

	Tools.append(Tool("trace_query", QObject::tr("Search the resource trace"),
		QObject::tr("The resource access trace: what sandboxed programs tried to open and "
			"whether it was allowed. This is where a 'why will it not start' answer "
			"usually comes from - look for denied accesses just before the program "
			"exits.\n\n"
			"Requires tracing to be enabled in SandMan, and only covers what has been "
			"recorded since. If it is off, say so and ask the user to turn it on and "
			"reproduce the problem - do not speculate from an empty trace. Results are "
			"paged and redacted."),
		TraceQuerySchema()));

	Tools.append(Tool("kb_search", QObject::tr("Search known problems"),
		QObject::tr("Searches the troubleshooting knowledge base - curated entries mapping a symptom "
			"to a cause and a fix. This is a different thing from the documentation: the docs say what "
			"a setting does, these say why something is broken.\n\n"
			"Search by how the user described the problem, in their words. Returns ids; pass one to "
			"kb_get for the whole entry."),
		QueryLimitSchema(QObject::tr("What the user reports, in their own words."))));

	Tools.append(Tool("kb_get", QObject::tr("Read a known problem"),
		QObject::tr("The whole knowledge-base entry: what it looks like, why it happens, the settings "
			"that fix it, what the fix costs, and how to confirm it worked.\n\n"
			"Read the risk before proposing the fix. Every one of these loosens something, and the "
			"user is entitled to hear what, in your own words, when you ask them to approve it."),
		IdSchema()));

	Tools.append(Tool("kb_match", QObject::tr("What could explain this sandbox's problems"),
		QObject::tr("Scores the knowledge base against what this machine actually logged for a "
			"sandbox - the message codes seen, the resources that were blocked, and which settings are "
			"present - and returns the entries that explain some of it, best first.\n\n"
			"Each result carries matchedBecause, saying which observations it accounts for. Read that "
			"rather than trusting the order: a top-ranked entry that matched on one weak signal is a "
			"guess, and an entry that explains three separate observations is not. Nothing here is "
			"a verdict - it is a shortlist to check against the trace."),
		BoxLimitSchema()));

	Tools.append(Tool("docs_search", QObject::tr("Search the documentation"),
		QObject::tr("Finds documentation pages by title, path, heading or setting name. "
			"Returns paths to read with docs_get, not the page text. Coverage is "
			"partial: roughly two in five settings have a page, so finding nothing "
			"here does not mean the setting does not exist - check config_schema."),
		StringArgSchema("query", QObject::tr("A setting name, a feature name, or a few words."),
			"limit", QObject::tr("Maximum results, default 10."))));

	Tools.append(Tool("docs_get", QObject::tr("Read a documentation page"),
		QObject::tr("The full Markdown text of one page, addressed by the path that "
			"docs_search or docs_for_setting returned."),
		StringArgSchema("path", QObject::tr("Page path, for example Content/UseRamDisk.md."))));

	Tools.append(Tool("config_schema", QObject::tr("Look up a setting"),
		QObject::tr("What Sandboxie itself knows about a configuration setting: whether it "
			"exists, its category and syntax, which version added or removed it, and "
			"what supersedes it. This is the authority on whether a setting is real - "
			"the documentation covers only some of them. An unknown setting name is "
			"silently ignored by the driver rather than rejected, so check here before "
			"recommending one."),
		StringArgSchema("setting", QObject::tr("Setting name, for example ClosedFilePath."))));

	Tools.append(Tool("docs_for_setting", QObject::tr("Find the page for a setting"),
		QObject::tr("The reference page for one Sandboxie.ini setting, plus any other pages "
			"that discuss it. Distinguishes a setting that exists but is undocumented "
			"from one that does not exist at all - do not infer the latter from the "
			"former."),
		StringArgSchema("setting", QObject::tr("Setting name, for example ClosedFilePath."))));

	return Tools;
}

bool IsPublic(const QString& Name)
{
	// Knowledge-base entries are curated, shipped content, reviewed in a pull
	// request like the documentation. kb_match is the one that is not here:
	// it scores those entries against what this machine actually logged.
	if (Name == "kb_search" || Name == "kb_get")
		return true;

	// sandboxie_version is here on purpose rather than by oversight. It names
	// the build and the OS, which any local process can already determine,
	// and without it documentation answers are wrong in the way that matters:
	// settings appear, change and disappear between versions, so an agent
	// that cannot ask which build this is will confidently cite a setting
	// that does not exist here.
	return Name == "docs_search" || Name == "docs_get" || Name == "docs_for_setting"
		|| Name == "config_schema" || Name == "sandboxie_version";
}

bool Exists(const QString& Name)
{
	return Name == "sandboxie_version" || Name == "box_list" || Name == "box_get_config"
		|| Name == "config_schema"
		|| Name == "box_processes"
#ifdef SBIE_HAS_DIAG_EVENTS
		|| Name == "box_diagnostics"
#endif
		|| Name == "trace_query"
		|| Name == "log_query"
		|| Name == "docs_search" || Name == "docs_get" || Name == "docs_for_setting"
		|| Name == "kb_search" || Name == "kb_get" || Name == "kb_match";
}

//
// A box has a name and may have an alias, and the user almost always knows it
// by the alias - SandMan displays "Alias (Name)" by default. An agent handed
// only the name will repeat a GUID back at someone who calls their box
// "New_Box_1", so both travel together and either is accepted as input.
//
static QString BoxAlias(const CSandBoxPtr& pBox)
{
	return pBox.isNull() ? QString() : pBox->GetText("BoxAlias").trimmed();
}

static QString BoxDisplayName(const CSandBoxPtr& pBox)
{
	const QString Alias = BoxAlias(pBox);
	if (Alias.isEmpty() || Alias.compare(pBox->GetName(), Qt::CaseInsensitive) == 0)
		return pBox->GetName();
	return QString("%1 (%2)").arg(Alias).arg(pBox->GetName());
}

// By name first, then by alias. Name wins on a collision: it is the identifier
// the configuration is actually keyed on.
static CSandBoxPtr ResolveBox(const QString& NameOrAlias)
{
	CSandBoxPtr pBox = theAPI->GetBoxByName(NameOrAlias);
	if (!pBox.isNull())
		return pBox;

	const QString Wanted = NameOrAlias.trimmed();
	foreach(const CSandBoxPtr& pCandidate, theAPI->GetAllBoxes()) {
		if (BoxAlias(pCandidate).compare(Wanted, Qt::CaseInsensitive) == 0)
			return pCandidate;
	}
	return CSandBoxPtr();
}

static QString BoxTypeName(int Type)
{
	switch (Type)
	{
		case CSandBoxPlus::eHardenedPlus:	return "hardened-plus";
		case CSandBoxPlus::eHardened:		return "hardened";
		case CSandBoxPlus::eDefaultPlus:	return "default-plus";
		case CSandBoxPlus::eDefault:		return "default";
		case CSandBoxPlus::eAppBoxPlus:		return "appbox-plus";
		case CSandBoxPlus::eAppBox:			return "appbox";
		case CSandBoxPlus::eInsecure:		return "insecure";
		case CSandBoxPlus::eOpen:			return "open";
		case CSandBoxPlus::ePrivatePlus:	return "private-plus";
		case CSandBoxPlus::ePrivate:		return "private";
		default:							return "unknown";
	}
}

static QJsonObject CallVersion()
{
	QJsonObject Data;
	Data["sandboxie"] = theGUI ? theGUI->GetVersion() : QString();
	Data["kernel"] = QSysInfo::kernelVersion();
	Data["architecture"] = QSysInfo::currentCpuArchitecture();
	Data["product"] = QSysInfo::prettyProductName();

	// Which supporter-certificate options are in force. This belongs with the
	// version because it answers the same question - what will work on this
	// installation - and without it an agent recommends settings the user has
	// no licence for, which looks like a fix and does nothing.
	//
	// Nothing identifying is exposed: SCertInfo is a bitfield of states and
	// feature bits, with no owner, address or serial in it.
	QJsonObject Cert;
	Cert["active"] = (bool)g_CertInfo.active;
	Cert["expired"] = (bool)g_CertInfo.expired;
	Cert["outdatedForThisBuild"] = (bool)g_CertInfo.outdated;
	Cert["inGracePeriod"] = (bool)g_CertInfo.grace_period;
	Cert["type"] = (int)g_CertInfo.type;
	Cert["level"] = (int)g_CertInfo.level;

	QJsonObject Options;
	Options["isolatedDesktops"] = (bool)g_CertInfo.opt_desk;
	Options["advancedNetwork"] = (bool)g_CertInfo.opt_net;
	Options["boxEncryption"] = (bool)g_CertInfo.opt_enc;
	Options["enhancedSecurityBoxTypes"] = (bool)g_CertInfo.opt_sec;
	Cert["options"] = Options;
	Data["certificate"] = Cert;

	QString Text = QObject::tr("Sandboxie-Plus %1 on %2 (%3, kernel %4)")
		.arg(Data["sandboxie"].toString())
		.arg(Data["product"].toString())
		.arg(Data["architecture"].toString())
		.arg(Data["kernel"].toString());

	if (!g_CertInfo.active) {
		Text += QObject::tr("\n\nNo active supporter certificate. Settings whose Requirements name one "
			"will not take effect here - check with config_validate before recommending them.");
	}
	else {
		QStringList On;
		if (g_CertInfo.opt_desk) On.append(QObject::tr("isolated desktops"));
		if (g_CertInfo.opt_net)  On.append(QObject::tr("advanced network features"));
		if (g_CertInfo.opt_enc)  On.append(QObject::tr("box encryption and protection"));
		if (g_CertInfo.opt_sec)  On.append(QObject::tr("enhanced security box types"));

		Text += QObject::tr("\n\nSupporter certificate: active");
		if (g_CertInfo.expired)
			Text += QObject::tr(" but expired%1").arg(g_CertInfo.grace_period
				? QObject::tr(", inside the renewal grace period") : QString());
		if (g_CertInfo.outdated)
			Text += QObject::tr(", and no longer valid for this build");
		Text += On.isEmpty()
			? QObject::tr(". No optional features unlocked.")
			: QObject::tr(". Unlocked: %1.").arg(On.join(", "));
	}

	return MakeTextResult(Text, Data);
}

static QJsonObject CallBoxList()
{
	QJsonArray Boxes;
	QString Text;
	int Shown = 0;
	int Total = 0;

	foreach(const CSandBoxPtr& pBox, theAPI->GetAllBoxes())
	{
		Total++;
		if (Shown >= kMaxBoxes)
			continue;

		CSandBoxPlus* pBoxEx = qobject_cast<CSandBoxPlus*>(pBox.data());

		QJsonObject Entry;
		Entry["name"] = pBox->GetName();
		const QString Alias = BoxAlias(pBox);
		if (!Alias.isEmpty())
			Entry["alias"] = Alias;
		Entry["displayName"] = BoxDisplayName(pBox);
		Entry["enabled"] = pBox->IsEnabled();
		Entry["activeProcesses"] = pBox->GetActiveProcessCount();
		if (pBoxEx) {
			Entry["type"] = BoxTypeName(pBoxEx->GetType());
			Entry["empty"] = pBoxEx->IsEmptyCached();
			Entry["ramDisk"] = pBoxEx->UseRamDisk();
		}
		Boxes.append(Entry);

		Text += QString("%1  type=%2  enabled=%3  processes=%4\n")
			.arg(BoxDisplayName(pBox))
			.arg(pBoxEx ? BoxTypeName(pBoxEx->GetType()) : QString("unknown"))
			.arg(pBox->IsEnabled() ? "yes" : "no")
			.arg(pBox->GetActiveProcessCount());
		Shown++;
	}

	QJsonObject Data;
	Data["boxes"] = Boxes;
	Data["count"] = Shown;
	Data["total"] = Total;

	if (Total > Shown) {
		Data["truncated"] = true;
		Text += QObject::tr("\n(%1 of %2 sandboxes listed; the rest were omitted)\n").arg(Shown).arg(Total);
	}
	if (!Total)
		Text = QObject::tr("No sandboxes are configured.");

	return MakeTextResult(Text, Data);
}

static QJsonObject CallBoxGetConfig(const QJsonObject& Arguments)
{
	const QString BoxName = Arguments.value("box").toString();
	if (BoxName.isEmpty())
		return MakeErrorResult(QObject::tr("The 'box' argument is required."));

	CSandBoxPtr pBox = ResolveBox(BoxName);
	if (pBox.isNull())
		return MakeErrorResult(QObject::tr("No sandbox called '%1'. Either its name or its alias works; "
			"call box_list for both.").arg(BoxName));

	qint32 Status = 0;
	QList<CSbieIni::SbieIniValue> Settings = pBox->GetIniSection(&Status);

	QJsonArray Values;
	QString Text;
	int Shown = 0;

	foreach(const CSbieIni::SbieIniValue& Value, Settings)
	{
		if (Shown >= kMaxSettings)
			break;
		QJsonObject Entry;
		Entry["name"] = Value.Name;
		Entry["value"] = Value.Value;
		Values.append(Entry);
		Text += Value.Name + "=" + Value.Value + "\n";
		Shown++;
	}

	QJsonObject Data;
	Data["box"] = pBox->GetName();
	if (!BoxAlias(pBox).isEmpty())
		Data["alias"] = BoxAlias(pBox);
	Data["settings"] = Values;
	Data["count"] = Shown;
	Data["total"] = Settings.count();
	if (Settings.count() > Shown) {
		Data["truncated"] = true;
		Text += QObject::tr("\n(%1 of %2 settings listed; the rest were omitted)\n").arg(Shown).arg(Settings.count());
	}
	if (Settings.isEmpty())
		Text = QObject::tr("Sandbox '%1' has no settings of its own; it runs on templates and global defaults.").arg(pBox->GetName());

	return MakeTextResult(Text, Data);
}

static QJsonObject NoDocsResult()
{
	return MakeErrorResult(QObject::tr(
		"The offline documentation pack is not installed, so documentation lookups are "
		"unavailable on this machine. The settings themselves can still be inspected with "
		"box_get_config, and the online documentation is at https://sandboxie-plus.com/docs/"));
}

static QJsonObject CallDocsSearch(const QJsonObject& Arguments)
{
	if (!g_pDocs || !g_pDocs->IsOpen())
		return NoDocsResult();

	const QString Query = Arguments.value("query").toString();
	if (Query.trimmed().isEmpty())
		return MakeErrorResult(QObject::tr("The 'query' argument is required."));

	int Limit = Arguments.contains("limit") ? Arguments.value("limit").toInt() : 10;
	Limit = qBound(1, Limit, 50);

	const QJsonArray Hits = g_pDocs->Search(Query, Limit);

	QString Text;
	foreach(const QJsonValue& Value, Hits) {
		const QJsonObject Hit = Value.toObject();
		Text += Hit.value("path").toString() + "  -  " + Hit.value("title").toString();
		if (Hit.contains("documents"))
			Text += QObject::tr("   [reference page for %1]").arg(Hit.value("documents").toString());
		Text += "\n";
	}
	if (Hits.isEmpty())
		Text = QObject::tr("Nothing in the documentation matches '%1'. Note that only about two in five "
			"settings have a documentation page, so this is not evidence that a setting "
			"does not exist - check it with config_schema or box_get_config.").arg(Query);

	QJsonObject Data;
	Data["query"] = Query;
	Data["results"] = Hits;
	Data["count"] = Hits.count();
	return MakeTextResult(Text, Data);
}

static QJsonObject CallDocsGet(const QJsonObject& Arguments)
{
	if (!g_pDocs || !g_pDocs->IsOpen())
		return NoDocsResult();

	const QString Path = Arguments.value("path").toString();
	if (Path.trimmed().isEmpty())
		return MakeErrorResult(QObject::tr("The 'path' argument is required."));

	QString Error;
	const QString Page = g_pDocs->ReadPage(Path, &Error);
	if (!Error.isEmpty())
		return MakeErrorResult(QObject::tr("%1. Use docs_search to find a valid page path.").arg(Error));

	QJsonObject Data;
	Data["path"] = Path;
	Data["bytes"] = Page.toUtf8().size();
	return MakeTextResult(Page, Data);
}

static QJsonObject CallDocsForSetting(const QJsonObject& Arguments)
{
	if (!g_pDocs || !g_pDocs->IsOpen())
		return NoDocsResult();

	const QString Setting = Arguments.value("setting").toString();
	if (Setting.trimmed().isEmpty())
		return MakeErrorResult(QObject::tr("The 'setting' argument is required."));

	const QJsonObject Found = g_pDocs->ForSetting(Setting);

	QString Text;
	if (Found.contains("referencePage")) {
		Text = QObject::tr("%1 is documented on %2 (%3).")
			.arg(Found.value("setting").toString())
			.arg(Found.value("referencePage").toString())
			.arg(Found.value("title").toString());
		if (Found.contains("alsoMentionedIn")) {
			Text += QObject::tr("\nAlso discussed on:\n");
			foreach(const QJsonValue& Value, Found.value("alsoMentionedIn").toArray())
				Text += "  " + Value.toString() + "\n";
		}
	}
	else if (Found.contains("alsoMentionedIn")) {
		Text = QObject::tr("%1 has no page of its own, but is discussed on:\n").arg(Found.value("setting").toString());
		foreach(const QJsonValue& Value, Found.value("alsoMentionedIn").toArray())
			Text += "  " + Value.toString() + "\n";
	}
	else {
		// The distinction that matters: silence in the documentation says
		// nothing about whether the setting exists.
		Text = QObject::tr("'%1' has no documentation page. That does not mean it is not a real "
			"setting - most settings are undocumented. Verify it exists before telling "
			"the user it does not.").arg(Setting);
	}

	return MakeTextResult(Text, Found);
}

//
// The schema fields are kept tooltip-ready, which means HTML-escaped and
// occasionally carrying a tag. An agent reading "&lt;Program&gt;" learns the
// wrong syntax, so turn it back into text on the way out.
//
static QString PlainFromTooltip(const QString& Html)
{
	QString Text = Html;
	Text.replace(QRegularExpression("<br\\s*/?>", QRegularExpression::CaseInsensitiveOption), "\n");
	Text.remove(QRegularExpression("<[^>]*>"));
	// &amp; last, or an escaped entity would be decoded twice.
	Text.replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"")
		.replace("&#39;", "'").replace("&nbsp;", " ").replace("&amp;", "&");
	return Text.trimmed();
}

static QJsonObject CallConfigSchema(const QJsonObject& Arguments)
{
	const QString Setting = Arguments.value("setting").toString().trimmed();
	if (Setting.isEmpty())
		return MakeErrorResult(QObject::tr("The 'setting' argument is required."));

	if (!CIniHighlighter::EnsureSettingsLoaded())
		return MakeErrorResult(QObject::tr(
			"The settings reference (SbieSettings.ini) could not be loaded, so settings "
			"cannot be verified on this installation."));

	SettingInfo Info;
	QJsonObject Data;
	Data["setting"] = Setting;

	if (!CIniHighlighter::GetSettingInfo(Setting, &Info)) {
		Data["exists"] = false;
		return MakeTextResult(QObject::tr(
			"'%1' is not a known Sandboxie setting. Note that the driver ignores unknown "
			"settings silently instead of reporting an error, so a typo here would look "
			"like the setting simply had no effect.").arg(Setting), Data);
	}

	Data["exists"] = true;
	if (!Info.name.isEmpty())		Data["name"] = Info.name;
	if (!Info.category.isEmpty())	Data["category"] = Info.category;
	if (!Info.context.isEmpty())	Data["context"] = Info.context;
	if (!Info.syntax.isEmpty())		Data["syntax"] = PlainFromTooltip(Info.syntax);
	if (!Info.description.isEmpty())	Data["description"] = PlainFromTooltip(Info.description);
	if (!Info.requirements.isEmpty())	Data["requirements"] = Info.requirements;
	if (!Info.addedVersion.isEmpty())	Data["addedVersion"] = Info.addedVersion;
	if (!Info.removedVersion.isEmpty())	Data["removedVersion"] = Info.removedVersion;
	if (!Info.readdedVersion.isEmpty())	Data["readdedVersion"] = Info.readdedVersion;
	if (!Info.renamedVersion.isEmpty())	Data["renamedVersion"] = Info.renamedVersion;
	if (!Info.supersededBy.isEmpty())	Data["supersededBy"] = Info.supersededBy;

	// The cross-check that makes this worth calling: the schema says the setting
	// is real, and the docs either explain it or do not.
	if (g_pDocs && g_pDocs->IsOpen()) {
		const QJsonObject Found = g_pDocs->ForSetting(Info.name.isEmpty() ? Setting : Info.name);
		if (Found.contains("referencePage"))
			Data["documentationPage"] = Found.value("referencePage");
		else
			Data["documented"] = false;
	}

	QString Text = QObject::tr("%1 is a known setting.").arg(Data.value("name").toString(Setting));
	if (!Info.category.isEmpty())		Text += QObject::tr("\nCategory: %1").arg(Info.category);
	if (!Info.syntax.isEmpty())			Text += QObject::tr("\nSyntax: %1").arg(PlainFromTooltip(Info.syntax));
	if (!Info.requirements.isEmpty())	Text += QObject::tr("\nRequires: %1").arg(Info.requirements);
	if (!Info.addedVersion.isEmpty())	Text += QObject::tr("\nAdded in: %1").arg(Info.addedVersion);
	if (!Info.removedVersion.isEmpty())	Text += QObject::tr("\nRemoved in: %1").arg(Info.removedVersion);
	if (!Info.supersededBy.isEmpty())	Text += QObject::tr("\nSuperseded by: %1").arg(Info.supersededBy);
	if (!Info.description.isEmpty())	Text += QObject::tr("\n\n%1").arg(PlainFromTooltip(Info.description));
	if (Data.contains("documentationPage"))
		Text += QObject::tr("\n\nDocumentation: %1 (read it with docs_get)").arg(Data.value("documentationPage").toString());
	else if (Data.contains("documented"))
		Text += QObject::tr("\n\nThis setting has no documentation page; most do not.");

	return MakeTextResult(Text, Data);
}


//
// The diagnose tier reads what SandMan has already collected, never the driver
// directly. Two reasons and both matter: the driver hands each trace record out
// once, so a second reader would race the trace view and each would see half
// the story; and the diagnostic events are only assembled here in the first
// place. Nothing below consumes or clears anything, so the trace view still
// shows everything after an agent has looked.
//

static QJsonObject CallBoxProcesses(const QJsonObject& Arguments)
{
	const QString BoxName = Arguments.value("box").toString();
	if (BoxName.trimmed().isEmpty())
		return MakeErrorResult(QObject::tr("The 'box' argument is required."));

	CSandBoxPtr pBox = ResolveBox(BoxName);
	if (pBox.isNull())
		return MakeErrorResult(QObject::tr("No sandbox called '%1'. Call box_list for names and aliases.").arg(BoxName));

	QJsonArray Processes;
	QString Text;
	int Shown = 0;
	const QMap<quint32, CBoxedProcessPtr> List = pBox->GetProcessList();

	foreach(const CBoxedProcessPtr& pProcess, List) {
		if (pProcess.isNull() || pProcess->IsTerminated())
			continue;
		if (Shown >= kMaxProcesses)
			break;

		QJsonObject Entry;
		Entry["pid"] = (double)pProcess->GetProcessId();
		Entry["parentPid"] = (double)pProcess->GetParendPID();
		Entry["name"] = pProcess->GetProcessName();
		Entry["imagePath"] = McpRedact::Apply(pProcess->GetFileName());
		Entry["commandLine"] = McpRedact::Apply(pProcess->GetCommandLine());
		Entry["startedAt"] = pProcess->GetTimeStamp().toString(Qt::ISODate);
		Processes.append(Entry);

		Text += QString("%1  pid=%2  started=%3\n")
			.arg(pProcess->GetProcessName())
			.arg(pProcess->GetProcessId())
			.arg(pProcess->GetTimeStamp().toString("HH:mm:ss"));
		Shown++;
	}

	QJsonObject Data;
	Data["box"] = pBox->GetName();
	Data["processes"] = Processes;
	Data["count"] = Shown;

	if (!Shown) {
		// An empty list is a finding, not a dead end, and saying which way to
		// look next is cheaper than letting the model guess.
#ifdef SBIE_HAS_DIAG_EVENTS
		Text = QObject::tr("Nothing is running in '%1' right now. If a program was expected, it "
			"started and exited - look at box_diagnostics and trace_query rather than at "
			"the process list.").arg(BoxDisplayName(pBox));
#else
		Text = QObject::tr("Nothing is running in '%1' right now. If a program was expected, it "
			"started and exited - look at log_query and trace_query rather than at "
			"the process list.").arg(BoxDisplayName(pBox));
#endif
	}

	return MakeTextResult(Text, Data);
}

// Only where there is a per-box diagnostic event list to read; see
// SBIE_HAS_DIAG_EVENTS in SbiePlusAPI.h. Where there is not, the tool is not
// listed either, so an agent is never offered something that cannot answer.
#ifdef SBIE_HAS_DIAG_EVENTS
static QJsonObject CallBoxDiagnostics(const QJsonObject& Arguments)
{
	const QString BoxName = Arguments.value("box").toString();
	if (BoxName.trimmed().isEmpty())
		return MakeErrorResult(QObject::tr("The 'box' argument is required."));

	CSandBoxPtr pBox = ResolveBox(BoxName);
	if (pBox.isNull())
		return MakeErrorResult(QObject::tr("No sandbox called '%1'. Call box_list for names and aliases.").arg(BoxName));

	CSandBoxPlus* pBoxEx = qobject_cast<CSandBoxPlus*>(pBox.data());
	if (!pBoxEx)
		return MakeErrorResult(QObject::tr("Diagnostics are not available for '%1'.").arg(BoxName));

	// GetDiagEvents, and deliberately not the report helper beside it: that one
	// clears the events as it reads them, which would take them away from the
	// troubleshooting wizard and from the person watching.
	const QList<CSandBoxPlus::SDiagEvent> Events = pBoxEx->GetDiagEvents();

	QJsonArray Array;
	QString Text;
	int Shown = 0;
	foreach(const CSandBoxPlus::SDiagEvent& Event, Events) {
		if (Shown >= kMaxDiagEvents)
			break;

		QJsonObject Entry;
		Entry["eventId"] = (double)Event.EventId;
		if (!Event.ProcessName.isEmpty())
			Entry["process"] = Event.ProcessName;
		QJsonArray EventData;
		foreach(const QString& Value, McpRedact::Apply(Event.EventData))
			EventData.append(Value);
		Entry["data"] = EventData;
		Array.append(Entry);

		Text += QString::number(Event.EventId);
		if (!Event.ProcessName.isEmpty())
			Text += QString(" (%1)").arg(Event.ProcessName);
		Text += ": " + McpRedact::Apply(Event.EventData.mid(1).join("; ")) + "\n";
		Shown++;
	}

	QJsonObject Data;
	Data["box"] = pBox->GetName();
	Data["events"] = Array;
	Data["count"] = Shown;
	Data["total"] = Events.count();
	if (Events.count() > Shown)
		Data["truncated"] = true;

	if (Events.isEmpty()) {
		Text = QObject::tr("No diagnostic events have been recorded for '%1'. That is the normal "
			"state for a healthy sandbox, and it is not evidence that nothing went wrong - "
			"events are only recorded for the kinds of problem Sandboxie detects itself.")
			.arg(BoxDisplayName(pBox));
	}

	return MakeTextResult(Text, Data);
}
#endif

static QJsonObject CallTraceQuery(const QJsonObject& Arguments)
{
	if (!theAPI->IsMonitoring()) {
		// An empty trace and a disabled trace look identical from the outside,
		// and reasoning from the first when it is really the second is how a
		// confident wrong answer gets produced.
		return MakeErrorResult(QObject::tr(
			"Resource tracing is turned off, so there is no trace to search. Ask the user to "
			"enable it in SandMan and then reproduce the problem - the trace only records what "
			"happens while it is on. Do not draw conclusions from the absence of trace data."));
	}

	const QString BoxName = Arguments.value("box").toString().trimmed();
	const QString Contains = Arguments.value("contains").toString().trimmed();
	const QString Status = Arguments.value("status").toString().trimmed();
	int Limit = Arguments.contains("limit") ? Arguments.value("limit").toInt() : 100;
	Limit = qBound(1, Limit, kMaxTraceRows);
	const int Offset = qMax(0, Arguments.value("offset").toInt());

	const CSandBox* pWanted = NULL;
	if (!BoxName.isEmpty()) {
		CSandBoxPtr pBox = ResolveBox(BoxName);
		if (pBox.isNull())
			return MakeErrorResult(QObject::tr("No sandbox called '%1'. Call box_list for names and aliases.").arg(BoxName));
		pWanted = pBox.data();
	}

	// A reference to the list SandMan maintains. Read only: nothing is consumed
	// and nothing is cleared.
	const QVector<CTraceEntryPtr>& Trace = theAPI->GetTrace();

	QJsonArray Rows;
	QString Text;
	int Matched = 0;
	int Returned = 0;

	foreach(const CTraceEntryPtr& pEntry, Trace) {
		if (!pEntry.data())
			continue;
		if (pWanted && pEntry->GetBoxPtr() != pWanted)
			continue;

		const QString Name = pEntry->GetName();
		const QString Message = pEntry->GetMessage();
		const QString StatusStr = pEntry->GetStautsStr();

		if (!Status.isEmpty() && !StatusStr.contains(Status, Qt::CaseInsensitive))
			continue;
		if (!Contains.isEmpty() && !Name.contains(Contains, Qt::CaseInsensitive)
				&& !Message.contains(Contains, Qt::CaseInsensitive))
			continue;

		Matched++;
		// Counted before the page window, so the total is honest even when only
		// a slice comes back.
		if (Matched <= Offset || Returned >= Limit)
			continue;

		QJsonObject Row;
		Row["pid"] = (double)pEntry->GetProcessId();
		Row["process"] = pEntry->GetProcessName();
		Row["type"] = pEntry->GetTypeStr();
		if (!StatusStr.isEmpty())
			Row["status"] = StatusStr;
		Row["path"] = McpRedact::Apply(Name);
		if (!Message.isEmpty())
			Row["message"] = McpRedact::Apply(Message);
		Rows.append(Row);

		Text += QString("%1  %2  %3  %4\n")
			.arg(pEntry->GetProcessName())
			.arg(pEntry->GetTypeStr())
			.arg(StatusStr.isEmpty() ? QString("-") : StatusStr)
			.arg(McpRedact::Apply(Name));
		Returned++;
	}

	QJsonObject Data;
	Data["rows"] = Rows;
	Data["returned"] = Returned;
	Data["matched"] = Matched;
	Data["traceSize"] = Trace.count();
	Data["offset"] = Offset;
	Data["redacted"] = true;

	if (Matched > Offset + Returned) {
		Data["more"] = true;
		Data["nextOffset"] = Offset + Returned;
		Text += QObject::tr("\n(%1 of %2 matching rows; call again with offset=%3 for the next page)\n")
			.arg(Returned).arg(Matched).arg(Offset + Returned);
	}
	if (!Matched) {
		Text = QObject::tr("No trace rows match. The trace holds %1 entries in total - if that is 0, "
			"tracing has only just been enabled and the problem still has to be reproduced "
			"before there is anything to see.").arg(Trace.count());
	}

	return MakeTextResult(Text, Data);
}



//
// log_query reads the one list SandMan keeps of everything it has received.
// Not a copy and not a second subscription: the message view, the
// troubleshooting engine and this tool all read the same vector, so what an
// agent sees is exactly what the user sees in the log.
//
static QJsonObject CallLogQuery(const QJsonObject& Arguments)
{
	if (!theGUI)
		return MakeErrorResult(QObject::tr("The message log is not available."));

	const QString BoxName = Arguments.value("box").toString().trimmed();
	const QString Contains = Arguments.value("contains").toString().trimmed();
	const QString Kind = Arguments.value("kind").toString().trimmed();
	int Limit = Arguments.contains("limit") ? Arguments.value("limit").toInt() : 100;
	Limit = qBound(1, Limit, kMaxLogRows);

	QString WantedBox;
	if (!BoxName.isEmpty()) {
		CSandBoxPtr pBox = ResolveBox(BoxName);
		if (pBox.isNull())
			return MakeErrorResult(QObject::tr("No sandbox called '%1'. Call box_list for names and aliases.").arg(BoxName));
		WantedBox = pBox->GetName();
	}

	const QVector<CSandMan::SSbieMsg>& Log = theGUI->GetMessageLog();

	// Newest first, and the scan runs backwards so a long log costs only the
	// page that is actually returned.
	QJsonArray Rows;
	QString Text;
	int Matched = 0;

	for (int i = Log.count() - 1; i >= 0; i--) {
		const CSandMan::SSbieMsg& Msg = Log.at(i);

		QString KindStr;
		switch (Msg.Kind) {
			case CSandMan::SSbieMsg::eDiagnostic:	KindStr = "diagnostic"; break;
			case CSandMan::SSbieMsg::eNotice:		KindStr = "notice"; break;
			default:								KindStr = "message"; break;
		}
		if (!Kind.isEmpty() && KindStr.compare(Kind, Qt::CaseInsensitive) != 0)
			continue;
		if (!WantedBox.isEmpty() && Msg.BoxName.compare(WantedBox, Qt::CaseInsensitive) != 0)
			continue;

		// Notices carry their text in MsgData; messages are a code plus data
		// that only SandMan knows how to render.
		const QString Rendered = (Msg.Kind == CSandMan::SSbieMsg::eNotice)
			? Msg.MsgData.value(0)
			: theGUI->FormatSbieMessage(Msg.MsgCode, Msg.MsgData, Msg.ProcessName);
		const QString Clean = McpRedact::Apply(Rendered);

		if (!Contains.isEmpty() && !Clean.contains(Contains, Qt::CaseInsensitive))
			continue;

		Matched++;
		if (Rows.count() >= Limit)
			continue;

		QJsonObject Row;
		Row["at"] = Msg.TimeStamp.toString(Qt::ISODate);
		Row["kind"] = KindStr;
		if (Msg.MsgCode)
			Row["code"] = (double)(Msg.MsgCode & 0xFFFF);
		if (!Msg.ProcessName.isEmpty())
			Row["process"] = Msg.ProcessName;
		if (!Msg.BoxName.isEmpty())
			Row["box"] = Msg.BoxName;
		Row["text"] = Clean;
		Rows.append(Row);

		Text += Msg.TimeStamp.toString("HH:mm:ss") + "  " + KindStr + "  " + Clean + "\n";
	}

	QJsonObject Data;
	Data["rows"] = Rows;
	Data["returned"] = Rows.count();
	Data["matched"] = Matched;
	Data["logSize"] = Log.count();
	Data["redacted"] = true;
	if (Matched > Rows.count())
		Data["truncated"] = true;

	if (!Matched) {
		Text = QObject::tr("Nothing in the log matches. It holds %1 entries in total.").arg(Log.count());
	}
	else if (Matched > Rows.count()) {
		Text += QObject::tr("\n(newest %1 of %2 matching entries)\n").arg(Rows.count()).arg(Matched);
	}

	return MakeTextResult(Text, Data);
}


static QJsonObject NoKnowledgeResult()
{
	// Distinguishable from "nothing matched". An empty knowledge base is a
	// missing pack, and an agent told "no known causes" would reasonably
	// conclude the problem is novel when in fact nothing was consulted.
	return MakeTextResult(QObject::tr(
		"There is no troubleshooting knowledge base on this installation, so nothing could be "
		"consulted. This is not the same as finding no match: treat it as the lookup being "
		"unavailable, and fall back to the documentation and the trace."));
}

static QJsonObject CallKbSearch(const QJsonObject& Arguments)
{
	if (!g_pKnowledge || !g_pKnowledge->GetCount())
		return NoKnowledgeResult();

	const QString Query = Arguments.value("query").toString().trimmed();
	if (Query.isEmpty())
		return MakeErrorResult(QObject::tr("'query' is required."));

	const int Limit = Arguments.value("limit").toInt(10);
	const QJsonArray Hits = g_pKnowledge->Search(Query, Limit > 0 ? Limit : 10);

	if (Hits.isEmpty())
		return MakeTextResult(QObject::tr(
			"Nothing in the knowledge base matches \"%1\". It holds %2 entries, so this is a real "
			"absence rather than an empty database - the problem may simply not be a known one.")
			.arg(Query).arg(g_pKnowledge->GetCount()));

	QString Text;
	foreach(const QJsonValue& Value, Hits) {
		const QJsonObject Entry = Value.toObject();
		Text += QString("%1  %2\n").arg(Entry.value("id").toString(), -28)
			.arg(Entry.value("title").toString());
		if (!Entry.value("diagnosis").toString().isEmpty())
			Text += QString("    %1\n").arg(Entry.value("diagnosis").toString());
	}

	QJsonObject Data;
	Data["entries"] = Hits;
	Data["total"] = g_pKnowledge->GetCount();
	return MakeTextResult(Text, Data);
}

static QJsonObject CallKbGet(const QJsonObject& Arguments)
{
	if (!g_pKnowledge || !g_pKnowledge->GetCount())
		return NoKnowledgeResult();

	const QString Id = Arguments.value("id").toString().trimmed();
	if (Id.isEmpty())
		return MakeErrorResult(QObject::tr("'id' is required."));

	const QJsonObject Entry = g_pKnowledge->Get(Id);
	if (Entry.isEmpty())
		return MakeErrorResult(QObject::tr(
			"There is no knowledge-base entry called '%1'. Use kb_search to find one.").arg(Id));

	QString Text;
	Text += QObject::tr("%1 - %2\n").arg(Entry.value("id").toString())
		.arg(Entry.value("title").toString());
	if (!Entry.value("diagnosis").toString().isEmpty())
		Text += QObject::tr("\nWhy it happens:\n  %1\n").arg(Entry.value("diagnosis").toString());

	const QJsonArray Changes = Entry.value("iniChanges").toArray();
	if (!Changes.isEmpty()) {
		Text += QObject::tr("\nThe fix:\n");
		foreach(const QJsonValue& V, Changes)
			Text += QString("  %1\n").arg(V.toString());
	}
	if (!Entry.value("script").toString().isEmpty())
		Text += QObject::tr("\nA troubleshooting script also applies this: %1\n")
			.arg(Entry.value("script").toString());

	if (!Entry.value("risk").toString().isEmpty())
		Text += QObject::tr("\nWhat it costs:\n  %1\n").arg(Entry.value("risk").toString());
	if (!Entry.value("verify").toString().isEmpty())
		Text += QObject::tr("\nHow to confirm it worked:\n  %1\n").arg(Entry.value("verify").toString());
	if (!Entry.value("notes").toString().isEmpty())
		Text += QObject::tr("\n%1\n").arg(Entry.value("notes").toString());

	return MakeTextResult(Text, Entry);
}

//
// The one knowledge tool that reads the machine, which is why it waits for
// pairing while the other two do not.
//
static QJsonObject CallKbMatch(const QJsonObject& Arguments)
{
	if (!g_pKnowledge || !g_pKnowledge->GetCount())
		return NoKnowledgeResult();

	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return MakeErrorResult(QObject::tr("No sandbox called '%1'.")
			.arg(Arguments.value("box").toString()));

	// Observations come from the one list SandMan already keeps, not from a
	// second capture of our own - an event that exists in only one consumer
	// is an event the others cannot diagnose from.
	QList<int> Codes;
	QStringList Denied;
	foreach(const CSandMan::SSbieMsg& Msg, theGUI->GetMessageLog()) {
		if (!Msg.BoxName.isEmpty() && Msg.BoxName.compare(pBox->GetName(), Qt::CaseInsensitive) != 0)
			continue;
		if (Msg.MsgCode) {
			const int Code = (int)(Msg.MsgCode & 0xFFFF);
			if (!Codes.contains(Code))
				Codes.append(Code);
		}
		foreach(const QString& Data, Msg.MsgData)
			if (Data.contains(QChar(92)) && !Denied.contains(Data))
				Denied.append(Data);
	}

	// "Template=OpenCamera" style lines, which is what the ini_absent and
	// ini_present signals are written against.
	QStringList Settings;
	foreach(const CSbieIni::SbieIniValue& Value, pBox.objectCast<CSbieIni>()->GetIniSection())
		Settings.append(QString("%1=%2").arg(Value.Name).arg(Value.Value));

	const int Limit = Arguments.value("limit").toInt(5);
	const QJsonArray Hits = g_pKnowledge->Match(Codes, Denied, Settings, Limit > 0 ? Limit : 5);

	if (Hits.isEmpty())
		return MakeTextResult(QObject::tr(
			"Nothing in the knowledge base explains what %1 has logged. %2 entries were checked "
			"against %3 message code(s) and %4 blocked resource(s). Either the problem is not a "
			"known one, or nothing has been logged yet - if you have not reproduced the fault "
			"since Sandboxie-Plus started, there may be nothing to match against.")
			.arg(pBox->GetName()).arg(g_pKnowledge->GetCount()).arg(Codes.count()).arg(Denied.count()));

	QString Text = QObject::tr("Candidates for %1, best first. These explain some of what was "
		"logged; none of them is a verdict.\n\n").arg(pBox->GetName());
	foreach(const QJsonValue& Value, Hits) {
		const QJsonObject Entry = Value.toObject();
		Text += QObject::tr("%1  (score %2)\n").arg(Entry.value("id").toString())
			.arg(Entry.value("score").toInt());
		if (!Entry.value("diagnosis").toString().isEmpty())
			Text += QString("    %1\n").arg(Entry.value("diagnosis").toString());
		foreach(const QJsonValue& Why, Entry.value("matchedBecause").toArray())
			Text += QObject::tr("    matched: %1\n").arg(Why.toString());
		Text += QString("\n");
	}

	QJsonObject Data;
	Data["box"] = pBox->GetName();
	Data["candidates"] = Hits;
	Data["codesSeen"] = Codes.count();
	Data["blockedSeen"] = Denied.count();
	return MakeTextResult(Text, Data);
}

QJsonObject Call(const QString& Name, const QJsonObject& Arguments)
{
	// Neither the documentation nor the schema needs a driver connection, and
	// they are answered before the connection check: a user whose Sandboxie is
	// stopped is exactly who needs to look something up.
	if (Name == "config_schema")
		return CallConfigSchema(Arguments);
	if (Name == "kb_search")
		return CallKbSearch(Arguments);
	if (Name == "kb_get")
		return CallKbGet(Arguments);
	if (Name == "docs_search")
		return CallDocsSearch(Arguments);
	if (Name == "docs_get")
		return CallDocsGet(Arguments);
	if (Name == "docs_for_setting")
		return CallDocsForSetting(Arguments);

	if (!theAPI || !theAPI->IsConnected())
		return MakeErrorResult(QObject::tr("Sandboxie is not running or SandMan is not connected to the driver."));

	if (Name == "sandboxie_version")
		return CallVersion();
	if (Name == "box_list")
		return CallBoxList();
	if (Name == "box_get_config")
		return CallBoxGetConfig(Arguments);
	if (Name == "kb_match")
		return CallKbMatch(Arguments);
	if (Name == "box_processes")
		return CallBoxProcesses(Arguments);
#ifdef SBIE_HAS_DIAG_EVENTS
	if (Name == "box_diagnostics")
		return CallBoxDiagnostics(Arguments);
#endif
	if (Name == "trace_query")
		return CallTraceQuery(Arguments);
	if (Name == "log_query")
		return CallLogQuery(Arguments);

	// Exists() is checked by the caller, so reaching here is a bug rather than
	// bad input.
	return MakeErrorResult(QObject::tr("Tool '%1' is not implemented.").arg(Name));
}

}
