#include "stdafx.h"
#include "McpReport.h"
#include "McpConsent.h"
#include "McpSession.h"
#include "McpSessionWindow.h"
#include "McpTools.h"
#include "McpRedact.h"
#include "../SandMan.h"
#include "../SbiePlusAPI.h"
#include "../Boxes/BoxReport.h"
#include <QJsonDocument>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QFileDialog>
#include <QTextStream>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkReply>

namespace McpReport
{

static CMcpConsent* g_pConsent = NULL;

void SetConsent(CMcpConsent* pConsent)
{
	g_pConsent = pConsent;
}

// The envelope's shape is a contract with whatever mines the corpus later, so
// it carries its own version and changes to it are deliberate.
static const int kEnvelopeVersion = 1;

// GitHub rejects a URL much past this, and a truncated issue body is better
// than a dead link - but the agent has to be told it was cut.
static const int kMaxIssueUrl = 7000;

// -------------------------------------------------------------------- helpers

static QJsonObject NoConsentResult()
{
	return McpTools::MakeErrorResult(QObject::tr(
		"Reporting is unavailable because there is no way to ask the user, and nothing is sent "
		"without being asked."));
}

static QJsonObject BusyResult()
{
	return McpTools::MakeErrorResult(QObject::tr(
		"A confirmation is already waiting for the user. Nothing was sent - wait for them to "
		"answer the previous request before asking again."));
}

static QStringList StringList(const QJsonValue& Value)
{
	QStringList Out;
	foreach(const QJsonValue& V, Value.toArray()) {
		const QString S = V.toString().trimmed();
		if (!S.isEmpty())
			Out.append(S);
	}
	return Out;
}

static CSandBoxPtr ResolveBox(const QString& NameOrAlias)
{
	if (NameOrAlias.trimmed().isEmpty())
		return CSandBoxPtr();
	CSandBoxPtr pBox = theAPI->GetBoxByName(NameOrAlias);
	if (!pBox.isNull())
		return pBox;
	foreach(const CSandBoxPtr& pCandidate, theAPI->GetAllBoxes()) {
		if (pCandidate->GetText("BoxAlias").trimmed().compare(NameOrAlias, Qt::CaseInsensitive) == 0)
			return pCandidate;
	}
	return CSandBoxPtr();
}

//
// A stable slug for the box preset. Deliberately not the display name: the
// fingerprint is a clustering key, and a key that moves when someone changes
// a translation groups the same fault into two piles.
//
static QString BoxTypeSlug(const CSandBoxPtr& pBox)
{
	auto pBoxEx = pBox.objectCast<CSandBoxPlus>();
	if (pBoxEx.isNull())
		return "unknown";
	switch (pBoxEx->GetType())
	{
		case CSandBoxPlus::eHardenedPlus:	return "hardened_plus";
		case CSandBoxPlus::eHardened:		return "hardened";
		case CSandBoxPlus::eDefaultPlus:	return "default_plus";
		case CSandBoxPlus::eDefault:		return "default";
		case CSandBoxPlus::eAppBoxPlus:		return "appbox_plus";
		case CSandBoxPlus::eAppBox:			return "appbox";
		case CSandBoxPlus::eInsecure:		return "insecure";
		case CSandBoxPlus::eOpen:			return "open";
		case CSandBoxPlus::ePrivatePlus:	return "private_plus";
		case CSandBoxPlus::ePrivate:		return "private";
		default:							return "unknown";
	}
}

static bool IsKnownOutcome(const QString& Outcome)
{
	return Outcome == "fixed" || Outcome == "partly_fixed" || Outcome == "not_fixed";
}

static QString Bullets(const QStringList& Lines, const QString& Marker = QString("  - "))
{
	QString Out;
	foreach(const QString& Line, Lines)
		Out += Marker + McpRedact::Apply(Line) + "\n";
	return Out;
}

// ------------------------------------------------------------- the two halves

//
// Machine-captured. Facts, gathered here rather than taken from the agent -
// an agent that supplied its own "observed" section could write anything into
// the half a maintainer is meant to trust.
//
static QString MachineHalf(const CSandBoxPtr& pBox, QJsonObject* pFingerprint)
{
	QString Out;
	Out += "================ MACHINE-CAPTURED ================\n";
	Out += "Gathered by Sandboxie-Plus itself. Not written by the agent.\n\n";

	Out += McpRedact::Apply(CBoxReport::AssembleDiagInfo()).trimmed() + "\n";

	QJsonObject Finger;
	Finger["sandboxiePlusVersion"] = CSandMan::GetVersion();
	Finger["osKernel"] = QSysInfo::kernelVersion();
	Finger["arch"] = QSysInfo::currentCpuArchitecture();

	if (!pBox.isNull())
	{
		Out += "\nSandbox configuration (as set on the box; inherited values are not listed):\n";
		QStringList Templates;
		int Shown = 0;
		foreach(const CSbieIni::SbieIniValue& Value, pBox.objectCast<CSbieIni>()->GetIniSection())
		{
			if (Value.Name.compare("Template", Qt::CaseInsensitive) == 0) {
				Templates.append(Value.Value);
				continue;
			}
			if (++Shown > 200)
				continue;
			Out += QString("  %1=%2\n").arg(Value.Name).arg(McpRedact::Apply(Value.Value));
		}
		if (!Templates.isEmpty())
			Out += QString("  Templates: %1\n").arg(Templates.join(", "));

		Finger["boxType"] = BoxTypeSlug(pBox);
		Finger["templates"] = QJsonArray::fromStringList(Templates);

		const QString Diag = CBoxReport::AssembleDiagData(pBox->GetName()).trimmed();
		if (!Diag.isEmpty() && !Diag.endsWith(":"))
			Out += "\n" + McpRedact::Apply(Diag) + "\n";
	}

	// The SBIE message codes seen this session. These are the join key that
	// makes two reports of the same fault comparable, so they go in the
	// fingerprint as codes rather than as rendered prose.
	QSet<int> Codes;
	foreach(const CSandMan::SSbieMsg& Msg, theGUI->GetMessageLog()) {
		if (Msg.MsgCode)
			Codes.insert((int)(Msg.MsgCode & 0xFFFF));
	}
	QList<int> CodeList = Codes.values();
	std::sort(CodeList.begin(), CodeList.end());
	QJsonArray CodeArray;
	foreach(int Code, CodeList)
		CodeArray.append(Code);
	Finger["sbieCodes"] = CodeArray;

	if (!CodeList.isEmpty()) {
		QStringList AsText;
		foreach(int Code, CodeList)
			AsText.append(QString("SBIE%1").arg(Code, 4, 10, QChar('0')));
		Out += QString("\nMessage codes seen this session: %1\n").arg(AsText.join(", "));
	}

	if (pFingerprint)
		*pFingerprint = Finger;
	return Out;
}

//
// Agent-written. Labelled loudly, because this is the half that can be
// confidently wrong, and a maintainer reading it should never have to guess
// which sentences are evidence.
//
static QString AgentHalf(const QJsonObject& Arguments, const QString& Client)
{
	const QString Outcome = Arguments.value("outcome").toString();
	const QStringList Steps = StringList(Arguments.value("steps"));
	const QStringList Changes = StringList(Arguments.value("changes"));
	const QStringList Discarded = StringList(Arguments.value("discarded"));

	QString Out;
	Out += "\n================ WRITTEN BY THE AI AGENT ================\n";
	Out += QString("Produced by: %1\n").arg(Client.isEmpty() ? QObject::tr("an unidentified agent") : Client);
	Out += "This half is model output. It has not been verified by Sandboxie-Plus and may be wrong.\n\n";

	Out += QString("Outcome: %1\n").arg(Outcome);

	const QString Narrative = Arguments.value("narrative").toString().trimmed();
	if (!Narrative.isEmpty())
		Out += "\nWhat it says happened:\n" + McpRedact::Apply(Narrative) + "\n";

	if (!Steps.isEmpty())
		Out += "\nSteps it says it took:\n" + Bullets(Steps);

	if (!Changes.isEmpty())
		Out += "\nChanges it says it applied:\n" + Bullets(Changes);

	const QString Verification = Arguments.value("verification").toString().trimmed();
	if (!Verification.isEmpty())
		Out += "\nHow it says it verified the result:\n" + McpRedact::Apply(Verification) + "\n";

	if (!Discarded.isEmpty()) {
		// Worth asking for and worth printing: the hypotheses ruled out are what
		// make a knowledge-base entry's signals discriminating rather than
		// merely plausible.
		Out += "\nHypotheses it considered and ruled out:\n" + Bullets(Discarded);
	}

	return Out;
}

//
// A fixed outcome already knows the symptom, the change and the verification -
// that is a knowledge-base entry's front matter, field for field. Emitting it
// here turns triage into "review and merge" rather than "re-derive from a bug
// report".
//
static QString DraftKbEntry(const QJsonObject& Arguments, const QJsonObject& Fingerprint)
{
	QString Out;
	Out += "\n================ DRAFT KNOWLEDGE-BASE ENTRY (a proposal, not a contribution) ================\n";
	Out += "Reviewed by a human before it could ever ship in a pack.\n\n";
	Out += "---\n";
	Out += QString("title: %1\n").arg(McpRedact::Apply(Arguments.value("title").toString().trimmed()));

	const QJsonArray Codes = Fingerprint.value("sbieCodes").toArray();
	if (!Codes.isEmpty()) {
		QStringList AsText;
		foreach(const QJsonValue& V, Codes)
			AsText.append(QString("SBIE%1").arg(V.toInt(), 4, 10, QChar('0')));
		Out += QString("signals:\n  messages: [%1]\n").arg(AsText.join(", "));
	}

	const QStringList Changes = StringList(Arguments.value("changes"));
	if (!Changes.isEmpty()) {
		Out += "ini_changes:\n";
		foreach(const QString& Change, Changes)
			Out += QString("  - %1\n").arg(McpRedact::Apply(Change));
	}
	Out += "---\n";

	const QString Narrative = Arguments.value("narrative").toString().trimmed();
	if (!Narrative.isEmpty())
		Out += "\n" + McpRedact::Apply(Narrative) + "\n";
	return Out;
}

// --------------------------------------------------------------- descriptions

static QJsonObject Tool(const QString& Name, const QString& Title, const QString& Description,
	const QJsonObject& Schema, bool bReadOnly)
{
	QJsonObject Out;
	Out["name"] = Name;
	Out["title"] = Title;
	Out["description"] = Description;
	Out["inputSchema"] = Schema;

	QJsonObject Annotations;
	Annotations["readOnlyHint"] = bReadOnly;
	if (!bReadOnly)
		Annotations["openWorldHint"] = true;	// it leaves the machine
	Out["annotations"] = Annotations;
	return Out;
}

static QJsonObject Str(const QString& Description)
{
	QJsonObject Out;
	Out["type"] = "string";
	Out["description"] = Description;
	return Out;
}

static QJsonObject StrArray(const QString& Description)
{
	QJsonObject Items;
	Items["type"] = "string";

	QJsonObject Out;
	Out["type"] = "array";
	Out["items"] = Items;
	Out["description"] = Description;
	return Out;
}

static QJsonObject ComposeSchema()
{
	QJsonObject Outcome = Str(QObject::tr(
		"One of: fixed, partly_fixed, not_fixed. Report a success as readily as a failure - a run "
		"that found a cause and applied a working fix is the material the knowledge base is made "
		"of, and the project currently never sees one."));
	QJsonArray Allowed;
	Allowed.append("fixed");
	Allowed.append("partly_fixed");
	Allowed.append("not_fixed");
	Outcome["enum"] = Allowed;

	QJsonObject Properties;
	Properties["box"] = Str(QObject::tr("Name or alias of the sandbox the run was about. Optional."));
	Properties["outcome"] = Outcome;
	Properties["title"] = Str(QObject::tr("One line naming the problem, as an issue title would."));
	Properties["narrative"] = Str(QObject::tr(
		"What you found and why you think it was the cause. Written as your own account - it is "
		"labelled as model output in the report and a maintainer will read it as such."));
	Properties["steps"] = StrArray(QObject::tr("What you did, one step per entry."));
	Properties["changes"] = StrArray(QObject::tr(
		"The settings you changed, as 'Setting=Value' lines. These become the ini_changes block of "
		"the draft knowledge-base entry."));
	Properties["verification"] = Str(QObject::tr(
		"How you established the result - what you ran, what you read afterwards. A fix nobody "
		"verified is a guess."));
	Properties["discarded"] = StrArray(QObject::tr(
		"Hypotheses you ruled out, and what ruled them out. Please fill this in: what a cause is "
		"NOT is what makes a knowledge-base entry discriminate instead of merely sounding right."));
	Properties["confirmed_by_user"] = QJsonObject{ { "type", "boolean" }, { "description", QObject::tr(
		"Set this only when the user has told you whether their problem is gone - not when you believe it is. Ask them: 'is it working now?' is a question worth one turn. Until this is set, Sandboxie-Plus asks the user itself before the report is sent, and the report says plainly that the outcome is your assessment rather than theirs.") } };
	Properties["attach_ini"] = QJsonObject{ { "type", "boolean" }, { "description", QObject::tr(
		"Ask to attach the whole Sandboxie.ini. Off unless the problem is about configuration the "
		"box section does not show; it carries every sandbox the user has.") } };
	Properties["attach_trace"] = QJsonObject{ { "type", "boolean" }, { "description", QObject::tr(
		"Ask to attach the resource trace as well as the message log. Large, and detailed about "
		"everything running on the machine.") } };

	QJsonArray Required;
	Required.append("outcome");
	Required.append("title");
	Required.append("narrative");

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	Schema["required"] = Required;
	return Schema;
}

static QJsonObject IssueSchema()
{
	QJsonObject Properties;
	Properties["title"] = Str(QObject::tr("Issue title."));
	Properties["body"] = Str(QObject::tr(
		"Issue body, Markdown. Leave empty to use the composed report, which is the better choice: "
		"it is the text the user has already read."));
	Properties["labels"] = StrArray(QObject::tr("Label names to suggest. The user can change them."));

	QJsonArray Required;
	Required.append("title");

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	Schema["required"] = Required;
	return Schema;
}

static QJsonObject ExportSchema()
{
	QJsonObject Properties;
	Properties["box"] = Str(QObject::tr("Name or alias of the sandbox the workaround is for."));
	Properties["changes"] = StrArray(QObject::tr("The settings, as 'Setting=Value' lines."));
	Properties["notes"] = Str(QObject::tr("What the workaround does and what it costs, in plain words."));

	QJsonArray Required;
	Required.append("box");
	Required.append("changes");

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	Schema["required"] = Required;
	return Schema;
}

QJsonArray Describe()
{
	QJsonArray Tools;

	Tools.append(Tool("report_compose", QObject::tr("Write up what happened"),
		QObject::tr("Builds a report from your account of the run plus the facts Sandboxie-Plus "
			"captures itself, and shows you the result. Nothing is sent - this only composes.\n\n"
			"The two halves stay separate and labelled: what the machine observed, and what you "
			"wrote. Do not try to blend them; a maintainer has to be able to tell which sentences "
			"are evidence.\n\n"
			"Call this on a success too. A failure report tells the project what is still broken; "
			"a success report tells it what the fix was, and that is the one it never gets.\n\n"
			"Call it early and again as you learn more, rather than once at the end. Whatever you composed last is what Sandboxie-Plus offers the user when they close the agent window, and they may close it at any moment - the console going away does not wait for you to finish. An early rough write-up that gets sent is worth more than a perfect one that was never composed.\n\n"
			"You cannot send it yourself, by design. When you are done, tell the user to press "
			"Finish - the green tick at the bottom of the Sandboxie-Plus agent window - which opens "
			"your report for them to read, correct and send."),
		ComposeSchema(), true));

	// report_submit is deliberately not here any more. Sending is the user's,
	// from Sandboxie-Plus's own window, by pressing Finish.
	//
	// It was a tool, and it worked, and it still got the flow wrong: the agent
	// submitted, which put a confirmation on screen, and then the window
	// offered the same report again when the session ended. Being asked twice
	// about one report is worse than either prompt alone - the second one
	// teaches that the first did not count.
	//
	// Exists() still knows the name, so an agent that reaches for it is told
	// what to do instead rather than getting "unknown tool".

	Tools.append(Tool("issue_prefill", QObject::tr("Open a prefilled GitHub issue"),
		QObject::tr("Opens GitHub's new-issue form in the user's browser with the title and body "
			"filled in. It submits nothing - the user reads it on the site and decides.\n\n"
			"Prefer report_submit. This one requires the user to have a GitHub account and to be "
			"signed in, which many are not; report_submit reaches the project's own tracker with no "
			"account at all. Use this only when the user has said they want to discuss it on GitHub, "
			"or when they ask for it by name."),
		IssueSchema(), false));

	Tools.append(Tool("workaround_export", QObject::tr("Save a workaround others can apply"),
		QObject::tr("Writes a file holding the settings as an INI snippet plus the equivalent "
			"SbieIni.exe commands, so the fix can be shared or reapplied. Local only - nothing "
			"leaves the machine. The user picks where it goes."),
		ExportSchema(), false));

	return Tools;
}

bool Exists(const QString& Name)
{
	return Name == "report_compose" || Name == "report_submit"
		|| Name == "issue_prefill" || Name == "workaround_export";
}

// ------------------------------------------------------------------ the tools

static QJsonObject CallCompose(const QJsonObject& Arguments, CMcpSession* pSession)
{
	const QString Outcome = Arguments.value("outcome").toString().trimmed();
	if (!IsKnownOutcome(Outcome))
		return McpTools::MakeErrorResult(QObject::tr(
			"'outcome' must be one of: fixed, partly_fixed, not_fixed."));

	if (Arguments.value("title").toString().trimmed().isEmpty())
		return McpTools::MakeErrorResult(QObject::tr("'title' is required - one line naming the problem."));
	if (Arguments.value("narrative").toString().trimmed().isEmpty())
		return McpTools::MakeErrorResult(QObject::tr(
			"'narrative' is required. A report with no account of what you found is not worth a "
			"maintainer's time."));

	const CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (!Arguments.value("box").toString().trimmed().isEmpty() && pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.")
			.arg(Arguments.value("box").toString()));

	QJsonObject Fingerprint;
	QString Text;
	Text += QString("Sandboxie-Plus troubleshooting report\n");
	Text += QString("Title: %1\n").arg(McpRedact::Apply(Arguments.value("title").toString().trimmed()));
	// Who says so, every time. A maintainer reading "fixed" needs to know
	// whether that is the user's verdict or the agent's opinion of its own
	// work, and those are not the same claim.
	const bool bConfirmed = Arguments.value("confirmed_by_user").toBool();
	Text += QString("Outcome: %1 (%2)\n\n").arg(Outcome)
		.arg(bConfirmed ? "confirmed by the user" : "the agent's assessment, not confirmed by the user");

	Text += MachineHalf(pBox, &Fingerprint);
	Text += AgentHalf(Arguments, pSession->GetClientName());

	if (Outcome == "fixed")
		Text += DraftKbEntry(Arguments, Fingerprint);

	// Flat and enumerable beats complete: the corpus exists to answer "how many
	// machines saw this", and that has to be a query rather than a reading
	// exercise.
	QJsonObject Envelope;
	Envelope["envelopeVersion"] = kEnvelopeVersion;
	Envelope["outcome"] = Outcome;
	Envelope["outcomeConfirmed"] = bConfirmed;
	Envelope["title"] = McpRedact::Apply(Arguments.value("title").toString().trimmed());
	Envelope["fingerprint"] = Fingerprint;
	Envelope["changes"] = QJsonArray::fromStringList(McpRedact::Apply(StringList(Arguments.value("changes"))));
	Envelope["discarded"] = QJsonArray::fromStringList(McpRedact::Apply(StringList(Arguments.value("discarded"))));
	Envelope["verification"] = McpRedact::Apply(Arguments.value("verification").toString().trimmed());
	Envelope["agent"] = pSession->GetClientName();
	if (!pBox.isNull())
		Envelope["box"] = pBox->GetName();

	Text += "\n================ WHAT WAS REMOVED ================\n";
	foreach(const QString& Line, McpRedact::Describe())
		Text += "  " + Line + "\n";

	// What goes with it is listed in the submission prompt, not here - this
	// text is the report, and the attachments are part of the decision to send
	// rather than part of the document.

	// Kept twice over, and deliberately. The session holds it for the agent,
	// which may go on to submit it itself; the window holds it for the user,
	// who gets the offer even if the agent never comes back - its console being
	// closed mid-run is the ordinary case, not the exception.
	pSession->SetDraftReport(Text, Envelope,
		Arguments.value("attach_ini").toBool(), Arguments.value("attach_trace").toBool());
	pSession->SetOutcomeConfirmed(bConfirmed);

	if (theGUI)
		theGUI->GetMcpWindow()->SetPendingReport(pSession->GetClientName(), Text,
			pSession->DraftAttachesIni(), pSession->DraftAttachesTrace(), bConfirmed);

	QJsonObject Data;
	Data["envelope"] = Envelope;
	Data["characters"] = Text.length();

	return McpTools::MakeTextResult(QObject::tr(
		"Composed, and nothing has been sent. The user has not seen it yet either - call "
		"report_submit to put it in front of them, or issue_prefill to open it as a GitHub issue "
		"instead. This is the full text:\n\n%1").arg(Text), Data);
}

static QJsonObject CallSubmit(const QJsonObject& Arguments, CMcpSession* pSession)
{
	Q_UNUSED(Arguments);

	// Not a failure, and not something to work around: it is the one step of
	// this that belongs to the person whose machine it is.
	if (!pSession->HasDraftReport())
		return McpTools::MakeErrorResult(QObject::tr(
			"Nothing has been written up yet, and sending is not yours to do in any case. Call "
			"report_compose first."));

	return McpTools::MakeTextResult(QObject::tr(
		"Sending is the user's to do, not yours - this tool no longer submits anything.\n\n"
		"Your write-up is already with Sandboxie-Plus. Tell the user to press Finish, the green "
		"tick at the bottom of the Sandboxie-Plus agent window: that opens the report for them to "
		"read, lets them correct anything, and sends it if they agree. Say that sending it is "
		"worth doing and why, once, and then leave the decision with them.\n\n"
		"Keep composing if you learn more - they are offered whatever you wrote last."));
}


static QJsonObject CallIssuePrefill(const QJsonObject& Arguments, CMcpSession* pSession)
{
	const QString Title = Arguments.value("title").toString().trimmed();
	if (Title.isEmpty())
		return McpTools::MakeErrorResult(QObject::tr("'title' is required."));

	QString Body = Arguments.value("body").toString().trimmed();
	if (Body.isEmpty())
		Body = pSession->GetDraftReport();
	if (Body.isEmpty())
		return McpTools::MakeErrorResult(QObject::tr(
			"There is nothing to put in the issue. Supply 'body', or call report_compose first."));

	Body = McpRedact::Apply(Body);

	bool bTruncated = false;
	QUrl Url("https://github.com/sandboxie-plus/Sandboxie/issues/new");
	QUrlQuery Query;
	Query.addQueryItem("title", McpRedact::Apply(Title));
	const QStringList Labels = StringList(Arguments.value("labels"));
	if (!Labels.isEmpty())
		Query.addQueryItem("labels", Labels.join(","));

	Query.addQueryItem("body", Body);
	Url.setQuery(Query);
	if (Url.toString().length() > kMaxIssueUrl) {
		// Truncate the body rather than open a link the browser will reject,
		// and say so in the body itself so nobody files a half issue unknowingly.
		const int Over = Url.toString().length() - kMaxIssueUrl;
		Body = Body.left(qMax(0, Body.length() - Over - 200))
			+ "\n\n[... truncated: the full report did not fit in a URL. "
			  "Ask Sandboxie-Plus to submit the complete report instead, or paste it here.]";
		Query.removeQueryItem("body");
		Query.addQueryItem("body", Body);
		Url.setQuery(Query);
		bTruncated = true;
	}

	if (!g_pConsent) return NoConsentResult();
	if (g_pConsent->IsPrompting()) return BusyResult();

	if (!g_pConsent->AskAction(pSession->GetClientName(), QObject::tr("Confirm opening a GitHub issue"),
			QObject::tr("Open the Sandboxie-Plus issue form in your browser, filled in with this:\n\n"
				"Title: %1\n\n%2\n\n"
				"Nothing is submitted by opening it - you still have to press the button on the "
				"site, and you can edit it there first.").arg(Title).arg(Body),
			QObject::tr("Open the Browser"), CMcpConsent::eLeavesMachine)) {
		return McpTools::MakeErrorResult(QObject::tr("The user declined; no issue was opened."));
	}

	if (!QDesktopServices::openUrl(Url))
		return McpTools::MakeErrorResult(QObject::tr("The browser could not be opened."));

	return McpTools::MakeTextResult(bTruncated
		? QObject::tr("The issue form is open in the browser, but the body did not fit in a URL "
			"and was truncated. Nothing has been submitted; the user decides on the site.")
		: QObject::tr("The issue form is open in the browser, prefilled. Nothing has been "
			"submitted; the user decides on the site."));
}

static QJsonObject CallWorkaroundExport(const QJsonObject& Arguments, CMcpSession* pSession)
{
	const CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.")
			.arg(Arguments.value("box").toString()));

	const QStringList Changes = StringList(Arguments.value("changes"));
	if (Changes.isEmpty())
		return McpTools::MakeErrorResult(QObject::tr("'changes' is required - there is no workaround without it."));

	const QString BoxName = pBox->GetName();
	QString Out;
	Out += QString("; Sandboxie-Plus workaround for %1\n").arg(BoxName);
	const QString Notes = Arguments.value("notes").toString().trimmed();
	if (!Notes.isEmpty()) {
		foreach(const QString& Line, McpRedact::Apply(Notes).split('\n'))
			Out += "; " + Line + "\n";
	}
	Out += QString("; Written by %1. Check it before applying it to your own sandbox.\n")
		.arg(pSession->GetClientName().isEmpty() ? QObject::tr("an AI agent") : pSession->GetClientName());

	Out += QString("\n[%1]\n").arg(BoxName);
	foreach(const QString& Change, Changes)
		Out += McpRedact::Apply(Change) + "\n";

	Out += "\n; The same thing from a command line:\n";
	foreach(const QString& Change, Changes) {
		const int Eq = Change.indexOf('=');
		if (Eq <= 0)
			continue;
		Out += QString("; SbieIni.exe append %1 %2 \"%3\"\n")
			.arg(BoxName).arg(Change.left(Eq).trimmed()).arg(McpRedact::Apply(Change.mid(Eq + 1).trimmed()));
	}

	// The file dialog is the consent here: the user chooses whether this is
	// written at all and where it lands, which is a more meaningful answer than
	// a yes/no box about a file they have not named.
	const QString Path = QFileDialog::getSaveFileName(theGUI,
		QObject::tr("Save the workaround"),
		QString("%1-workaround.ini").arg(BoxName),
		QObject::tr("Sandboxie configuration (*.ini);;All files (*.*)"));
	if (Path.isEmpty())
		return McpTools::MakeErrorResult(QObject::tr("The user cancelled; nothing was written."));

	QFile File(Path);
	if (!File.open(QFile::WriteOnly | QFile::Truncate | QFile::Text))
		return McpTools::MakeErrorResult(QObject::tr("That file could not be written: %1")
			.arg(File.errorString()));
	File.write(Out.toUtf8());
	File.close();

	QJsonObject Data;
	Data["path"] = Path;
	Data["settings"] = Changes.count();
	return McpTools::MakeTextResult(QObject::tr(
		"Written to %1 - %2 setting(s), as an INI section plus the equivalent SbieIni.exe "
		"commands.").arg(Path).arg(Changes.count()), Data);
}

QJsonObject Call(const QString& Name, const QJsonObject& Arguments, CMcpSession* pSession)
{
	if (!theAPI || !theAPI->IsConnected())
		return McpTools::MakeErrorResult(QObject::tr("Sandboxie is not running."));

	if (Name == "report_compose")		return CallCompose(Arguments, pSession);
	if (Name == "report_submit")		return CallSubmit(Arguments, pSession);
	if (Name == "issue_prefill")		return CallIssuePrefill(Arguments, pSession);
	if (Name == "workaround_export")	return CallWorkaroundExport(Arguments, pSession);

	return McpTools::MakeErrorResult(QObject::tr("Tool '%1' is not implemented.").arg(Name));
}

}
