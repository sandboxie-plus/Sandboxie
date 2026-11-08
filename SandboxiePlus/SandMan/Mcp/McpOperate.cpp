#include "stdafx.h"
#include "McpOperate.h"
#include "McpConsent.h"
#include "McpSessionWindow.h"
#include "McpSession.h"
#include "McpTools.h"
#include "../SandMan.h"
#include "../SbiePlusAPI.h"
#include "../Helpers/IniHighlighter.h"
#include <QJsonDocument>

namespace McpOperate
{

static CMcpConsent* g_pConsent = NULL;

void SetConsent(CMcpConsent* pConsent)
{
	g_pConsent = pConsent;
}

// -------------------------------------------------------------------- helpers

static QString BoxAlias(const CSandBoxPtr& pBox)
{
	return pBox.isNull() ? QString() : pBox->GetText("BoxAlias").trimmed();
}

static QString BoxLabel(const CSandBoxPtr& pBox)
{
	const QString Alias = BoxAlias(pBox);
	if (Alias.isEmpty() || Alias.compare(pBox->GetName(), Qt::CaseInsensitive) == 0)
		return pBox->GetName();
	return QString("%1 (%2)").arg(Alias).arg(pBox->GetName());
}

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

// A change set as the agent supplies it: [{ setting, value }], where an absent
// or empty value means remove.
struct SProposed
{
	QString Setting;
	QString Value;
	bool	bRemove = false;
};

static bool ParseChanges(const QJsonObject& Arguments, QList<SProposed>* pOut, QString* pError)
{
	const QJsonArray Array = Arguments.value("changes").toArray();
	if (Array.isEmpty()) {
		*pError = QObject::tr("The 'changes' argument must be a non-empty array of "
			"{setting, value} objects.");
		return false;
	}
	if (Array.count() > 50) {
		// A change set nobody can read is a change set nobody can consent to.
		*pError = QObject::tr("That is %1 changes at once. Propose at most 50, so the user can "
			"actually read what they are agreeing to.").arg(Array.count());
		return false;
	}

	foreach(const QJsonValue& Value, Array) {
		const QJsonObject Item = Value.toObject();
		SProposed Change;
		Change.Setting = Item.value("setting").toString().trimmed();
		if (Change.Setting.isEmpty()) {
			*pError = QObject::tr("Every entry in 'changes' needs a 'setting'.");
			return false;
		}
		// A name with a newline in it could inject a second ini line.
		if (Change.Setting.contains(QChar('\n')) || Change.Setting.contains(QChar('\r'))
				|| Change.Setting.contains(QChar('='))) {
			*pError = QObject::tr("'%1' is not a usable setting name.").arg(Change.Setting);
			return false;
		}
		Change.Value = Item.value("value").toString();
		Change.bRemove = !Item.contains("value") || Change.Value.isEmpty();
		if (Change.Value.contains(QChar('\n')) || Change.Value.contains(QChar('\r'))) {
			*pError = QObject::tr("The value for '%1' contains a line break, which cannot be "
				"stored in Sandboxie.ini.").arg(Change.Setting);
			return false;
		}
		pOut->append(Change);
	}
	return true;
}

//
// Snapshot operations are asynchronous: they return a status of OP_ASYNC with a
// progress object, which is "started", not "failed". Treating it as an error
// reported a rollback that had in fact succeeded - and simply ignoring it would
// be the opposite lie, telling the agent a merge was done while it was still
// running. So wait for the real outcome.
//
// AddAsyncOp with bWait also puts the progress on screen, which a snapshot
// merge deserves: it is the one operation here that can take a while.
//
static SB_STATUS AwaitProgress(const SB_PROGRESS& Progress, const QString& What)
{
	if (Progress.GetStatus() != OP_ASYNC)
		return Progress;	// finished, or failed outright
	return theGUI->AddAsyncOp(Progress.GetValue(), true, What);
}

static QJsonObject NoConsentResult()
{
	return McpTools::MakeErrorResult(QObject::tr(
		"Changes cannot be confirmed right now, so nothing was changed."));
}

static QJsonObject BusyResult()
{
	return McpTools::MakeErrorResult(QObject::tr(
		"A confirmation is already waiting for the user. Nothing was changed - wait for them "
		"to answer the previous request before asking again."));
}

// --------------------------------------------------------------- descriptions

static QJsonObject Tool(const QString& Name, const QString& Title, const QString& Description,
	const QJsonObject& Schema, bool bDestructive = false)
{
	QJsonObject Out;
	Out["name"] = Name;
	Out["title"] = Title;
	Out["description"] = Description;
	Out["inputSchema"] = Schema;

	QJsonObject Annotations;
	Annotations["readOnlyHint"] = false;
	// Honest hints: a client may use these to decide how loudly to ask, and a
	// wrong one here would under-warn.
	if (bDestructive)
		Annotations["destructiveHint"] = true;
	Out["annotations"] = Annotations;
	return Out;
}

static QJsonObject BoxArg(const QString& Description)
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

static QJsonObject RunSchema()
{
	QJsonObject Box;
	Box["type"] = "string";
	Box["description"] = QObject::tr("Name or alias of the sandbox to run in.");

	QJsonObject Command;
	Command["type"] = "string";
	Command["description"] = QObject::tr(
		"The command line to start, exactly as it should run. Quote paths containing spaces. "
		"It has to be a single plain line - a command carrying line breaks is refused, because "
		"the user must be able to read what they are approving as one command.");

	QJsonObject WorkingDir;
	WorkingDir["type"] = "string";
	WorkingDir["description"] = QObject::tr("Working directory for the program. Optional.");

	QJsonObject Reason;
	Reason["type"] = "string";
	Reason["description"] = QObject::tr(
		"Why this needs to run, in one sentence. Shown to the user as your words, next to the "
		"command. A request with no reason given is a harder one to agree to.");

	QJsonObject Properties;
	Properties["box"] = Box;
	Properties["command"] = Command;
	Properties["working_dir"] = WorkingDir;
	Properties["reason"] = Reason;

	QJsonArray Required;
	Required.append("box");
	Required.append("command");

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	Schema["required"] = Required;
	return Schema;
}

static QJsonObject ChangesSchema(bool bWithRationale)
{
	QJsonObject Setting;
	Setting["type"] = "string";
	Setting["description"] = QObject::tr("The Sandboxie.ini setting name. Check it with config_schema first.");

	QJsonObject Value;
	Value["type"] = "string";
	Value["description"] = QObject::tr("The value to set. Omit or leave empty to remove the setting.");

	QJsonObject ItemProps;
	ItemProps["setting"] = Setting;
	ItemProps["value"] = Value;

	QJsonObject Item;
	Item["type"] = "object";
	Item["properties"] = ItemProps;

	QJsonObject Changes;
	Changes["type"] = "array";
	Changes["description"] = QObject::tr("The changes, at most 50.");
	Changes["items"] = Item;

	QJsonObject Box;
	Box["type"] = "string";
	Box["description"] = QObject::tr("Name or alias of the sandbox.");

	QJsonObject Properties;
	Properties["box"] = Box;
	Properties["changes"] = Changes;

	QJsonArray Required;
	Required.append("box");
	Required.append("changes");

	if (bWithRationale) {
		QJsonObject Rationale;
		Rationale["type"] = "string";
		Rationale["description"] = QObject::tr("Why these changes should help. Shown to the user "
			"verbatim when they are asked to approve, so write it for them and not for yourself.");
		Properties["rationale"] = Rationale;
		Required.append("rationale");
	}

	QJsonObject Schema;
	Schema["type"] = "object";
	Schema["properties"] = Properties;
	Schema["required"] = Required;
	return Schema;
}

QJsonArray Describe()
{
	QJsonArray Tools;

	Tools.append(Tool("config_validate", QObject::tr("Check a change set before proposing it"),
		QObject::tr("Checks proposed settings against what Sandboxie actually accepts, without "
			"changing anything and without asking the user. Always call this before "
			"box_apply_changes: an unknown setting name is ignored silently by the driver, "
			"so a typo produces a change that appears to work and does nothing."),
		ChangesSchema(false)));

	Tools.append(Tool("box_apply_changes", QObject::tr("Change sandbox settings"),
		QObject::tr("Proposes configuration changes to one sandbox. The user is shown a diff of "
			"current against proposed values and must approve it; nothing is written if "
			"they decline. Use this for a fix you are confident about, and for everyday "
			"requests such as adding a forced program.\n\n"
			"If you want to test a fix before asking the user to accept it, use "
			"shadow_begin instead - that lets you try it against a disposable copy of the "
			"configuration."),
		ChangesSchema(true)));

	Tools.append(Tool("shadow_begin", QObject::tr("Start testing a fix"),
		QObject::tr("Clones a sandbox configuration to a disposable copy that shares the real "
			"file set, so a fix can be tried without touching the sandbox. Returns the "
			"name of the copy; change it with shadow_set, then either shadow_commit to "
			"copy the differences back - which asks the user once - or shadow_discard to "
			"throw them away.\n\n"
			"Note what is shared: files written while testing land in the real sandbox and "
			"a discard does not undo them. Only configuration is disposable."),
		BoxArg(QObject::tr("Name or alias of the sandbox to test against."))));

	Tools.append(Tool("shadow_set", QObject::tr("Change a setting in the test copy"),
		QObject::tr("Applies changes to the disposable copy. No confirmation is needed because "
			"nothing real is affected yet."),
		ChangesSchema(false)));

	Tools.append(Tool("shadow_commit", QObject::tr("Keep the tested fix"),
		QObject::tr("Copies the differences from the test copy back onto the real sandbox, after "
			"showing the user what they are. This is the one confirmation on the "
			"troubleshooting path."),
		BoxArg(QObject::tr("Name or alias of the sandbox being tested."))));

	Tools.append(Tool("shadow_discard", QObject::tr("Abandon the tested fix"),
		QObject::tr("Deletes the test copy. Nothing to roll back, because the real sandbox was "
			"never changed. Call this when a hypothesis turns out to be wrong."),
		BoxArg(QObject::tr("Name or alias of the sandbox being tested."))));

	Tools.append(Tool("snapshot_create", QObject::tr("Take a rollback point"),
		QObject::tr("Takes a snapshot of a sandbox so file and registry changes can be undone "
			"later. Near-instant regardless of sandbox size, but it refuses while anything "
			"is running in the box - so take it before starting anything. Not offered for "
			"empty sandboxes or RAM-disk sandboxes, where it would be meaningless."),
		BoxArg(QObject::tr("Name or alias of the sandbox."))));

	Tools.append(Tool("snapshot_rollback", QObject::tr("Roll a sandbox back"),
		QObject::tr("Reverts a sandbox to a snapshot, discarding file and registry changes made "
			"since. Terminates whatever is running in the box first, because the operation "
			"requires it. This throws work away and cannot be undone."),
		BoxArg(QObject::tr("Name or alias of the sandbox.")), true));

	Tools.append(Tool("box_terminate", QObject::tr("Stop everything in a sandbox"),
		QObject::tr("Terminates all processes in a sandbox. Unsaved work in those programs is "
			"lost, which is why the user is asked."),
		BoxArg(QObject::tr("Name or alias of the sandbox.")), true));

	Tools.append(Tool("box_run", QObject::tr("Run a program inside a sandbox"),
		QObject::tr("Starts a command inside a sandbox, at the privileges Sandboxie-Plus itself "
			"has - never elevated. The user is shown the command line exactly as given and has to "
			"confirm it before anything runs, so write the command you actually mean: it is not "
			"summarised or rewritten.\n\n"
			"This is how a fix gets tested rather than described - start the program that was "
#ifdef SBIE_HAS_DIAG_EVENTS
			"failing, then read trace_query and box_diagnostics to see whether it still fails. "
#else
			"failing, then read trace_query to see whether it still fails. "
#endif
			"The call returns as soon as the program has started: it does not wait for it to exit "
			"and captures no output, so get the pid from box_processes.\n\n"
			"The settings in effect are the sandbox's CURRENT ones. A change staged with "
			"shadow_set is not in effect here - a test copy shares the real sandbox's file root, "
			"so a program started against it runs in the real sandbox with the real settings. To "
			"test a configuration change by running something, apply it first with "
			"shadow_commit, or write it directly with box_apply_changes."),
		RunSchema(), true));

	Tools.append(Tool("box_create", QObject::tr("Create a sandbox"),
		QObject::tr("Creates a new, empty sandbox with default settings. The user is asked first. "
			"Settings can then be applied with box_apply_changes."),
		BoxArg(QObject::tr("Name for the new sandbox. Letters, digits and underscores."))));

	return Tools;
}

bool Exists(const QString& Name)
{
	return Name == "config_validate" || Name == "box_apply_changes"
		|| Name == "shadow_begin" || Name == "shadow_set"
		|| Name == "shadow_commit" || Name == "shadow_discard"
		|| Name == "snapshot_create" || Name == "snapshot_rollback"
		|| Name == "box_terminate" || Name == "box_create" || Name == "box_run";
}

// ------------------------------------------------------------------ the tools

static QJsonObject CallConfigValidate(const QJsonObject& Arguments)
{
	QList<SProposed> Changes;
	QString Error;
	if (!ParseChanges(Arguments, &Changes, &Error))
		return McpTools::MakeErrorResult(Error);

	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.").arg(Arguments.value("box").toString()));

	const bool bSchema = CIniHighlighter::EnsureSettingsLoaded();

	QJsonArray Results;
	QString Text;
	int Unknown = 0;

	foreach(const SProposed& Change, Changes) {
		SettingInfo Info;
		const bool bKnown = bSchema && CIniHighlighter::GetSettingInfo(Change.Setting, &Info);
		if (!bKnown) Unknown++;

		QJsonObject Row;
		Row["setting"] = Change.Setting;
		Row["known"] = bKnown;
		Row["current"] = pBox->GetText(Change.Setting);
		if (bKnown) {
			if (!Info.removedVersion.isEmpty())
				Row["removedVersion"] = Info.removedVersion;
			if (!Info.supersededBy.isEmpty())
				Row["supersededBy"] = Info.supersededBy;
		}
		Results.append(Row);

		Text += Change.Setting + (bKnown ? QObject::tr("  ok") : QObject::tr("  NOT A KNOWN SETTING"));
		if (bKnown && !Info.supersededBy.isEmpty())
			Text += QObject::tr("  (superseded by %1)").arg(Info.supersededBy);
		if (bKnown && !Info.removedVersion.isEmpty())
			Text += QObject::tr("  (removed in %1)").arg(Info.removedVersion);
		Text += "\n";
	}

	QJsonObject Data;
	Data["box"] = pBox->GetName();
	Data["settings"] = Results;
	Data["unknown"] = Unknown;
	Data["schemaAvailable"] = bSchema;

	if (!bSchema)
		Text += QObject::tr("\nThe settings reference could not be loaded, so none of this was verified.\n");
	else if (Unknown)
		Text += QObject::tr("\n%1 of these are not settings Sandboxie recognises. Fix them before "
			"proposing the change - an unknown name is ignored silently rather than "
			"reported, so it would look like a fix that did nothing.\n").arg(Unknown);

	return McpTools::MakeTextResult(Text, Data);
}

// Shared by box_apply_changes and shadow_commit: builds the diff, asks, writes.
static QJsonObject ApplyToIni(const QSharedPointer<CSbieIni>& pTarget, const QString& Label,
	const QList<SProposed>& Changes, const QString& Rationale, const QString& Client, bool bAsk)
{
	const bool bSchema = CIniHighlighter::EnsureSettingsLoaded();

	QList<CMcpConsent::SChange> Diff;
	foreach(const SProposed& Change, Changes) {
		CMcpConsent::SChange Row;
		Row.Setting = Change.Setting;
		Row.Current = pTarget->GetText(Change.Setting);
		Row.Proposed = Change.bRemove ? QString() : Change.Value;
		Row.bKnown = !bSchema || CIniHighlighter::IsValidSetting(Change.Setting);
		Diff.append(Row);
	}

	if (bAsk) {
		if (!g_pConsent)
			return NoConsentResult();
		if (g_pConsent->IsPrompting())
			return BusyResult();
		if (!g_pConsent->AskChanges(Client, Label, Rationale, Diff)) {
			return McpTools::MakeErrorResult(QObject::tr(
				"The user declined, so nothing was changed. Do not ask again with the same "
				"change set - explain what you wanted to do and why, and let them decide."));
		}
	}

	int Written = 0;
	QStringList Failed;
	foreach(const SProposed& Change, Changes) {
		SB_STATUS Status = Change.bRemove
			? pTarget->DelValue(Change.Setting)
			: pTarget->SetText(Change.Setting, Change.Value);
		if (Status.IsError())
			Failed.append(Change.Setting);
		else
			Written++;
	}
	theAPI->CommitIniChanges();

	QJsonObject Data;
	Data["target"] = pTarget->GetName();
	Data["written"] = Written;
	if (!Failed.isEmpty()) {
		QJsonArray Array;
		foreach(const QString& Name, Failed) Array.append(Name);
		Data["failed"] = Array;
	}

	QString Text = QObject::tr("%1 of %2 changes written to %3.")
		.arg(Written).arg(Changes.count()).arg(Label);
	if (!Failed.isEmpty())
		Text += QObject::tr("\nThese could not be written: %1").arg(Failed.join(", "));

	QJsonObject Result = McpTools::MakeTextResult(Text, Data);
	if (!Failed.isEmpty())
		Result["isError"] = true;
	return Result;
}

static QJsonObject CallApplyChanges(const QJsonObject& Arguments, CMcpSession* pSession)
{
	QList<SProposed> Changes;
	QString Error;
	if (!ParseChanges(Arguments, &Changes, &Error))
		return McpTools::MakeErrorResult(Error);

	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.").arg(Arguments.value("box").toString()));

	return ApplyToIni(pBox.objectCast<CSbieIni>(), BoxLabel(pBox), Changes,
		Arguments.value("rationale").toString(), pSession->GetClientName(), true);
}

static QJsonObject CallShadowBegin(const QJsonObject& Arguments, CMcpSession* pSession)
{
	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.").arg(Arguments.value("box").toString()));

	const QString Existing = pSession->Shadows().GetShadowName(pBox->GetName());
	if (!Existing.isEmpty()) {
		QJsonObject Data;
		Data["shadow"] = Existing;
		Data["box"] = pBox->GetName();
		return McpTools::MakeTextResult(QObject::tr(
			"Already testing against %1. Use shadow_set to change it.").arg(Existing), Data);
	}

	QSharedPointer<CSbieIni> pShadow = pSession->Shadows().MakeShadow(pBox.objectCast<CSbieIni>());
	if (pShadow.isNull())
		return McpTools::MakeErrorResult(QObject::tr("The test copy could not be created."));

	QJsonObject Data;
	Data["shadow"] = pShadow->GetName();
	Data["box"] = pBox->GetName();

	return McpTools::MakeTextResult(QObject::tr(
		"Testing against %1, a disposable copy of the configuration of %2 that shares its real "
		"file set. Change it with shadow_set. Remember that files written while testing land in "
		"the real sandbox - only the configuration is disposable.")
		.arg(pShadow->GetName()).arg(BoxLabel(pBox)), Data);
}

static QJsonObject CallShadowSet(const QJsonObject& Arguments, CMcpSession* pSession)
{
	QList<SProposed> Changes;
	QString Error;
	if (!ParseChanges(Arguments, &Changes, &Error))
		return McpTools::MakeErrorResult(Error);

	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.").arg(Arguments.value("box").toString()));

	const QString ShadowName = pSession->Shadows().GetShadowName(pBox->GetName());
	if (ShadowName.isEmpty())
		return McpTools::MakeErrorResult(QObject::tr(
			"There is no test copy for '%1' yet. Call shadow_begin first.").arg(pBox->GetName()));

	CSandBoxPtr pShadow = theAPI->GetBoxByName(ShadowName);
	if (pShadow.isNull())
		return McpTools::MakeErrorResult(QObject::tr("The test copy %1 has gone.").arg(ShadowName));

	// No consent: the shadow is disposable and the real box is untouched until
	// shadow_commit, which is where the single confirmation lives.
	return ApplyToIni(pShadow.objectCast<CSbieIni>(), ShadowName, Changes, QString(),
		pSession->GetClientName(), false);
}

static QJsonObject CallShadowCommit(const QJsonObject& Arguments, CMcpSession* pSession)
{
	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.").arg(Arguments.value("box").toString()));

	const QString ShadowName = pSession->Shadows().GetShadowName(pBox->GetName());
	if (ShadowName.isEmpty())
		return McpTools::MakeErrorResult(QObject::tr(
			"There is no test copy for '%1'. Nothing to keep.").arg(pBox->GetName()));

	CSandBoxPtr pShadow = theAPI->GetBoxByName(ShadowName);
	if (pShadow.isNull())
		return McpTools::MakeErrorResult(QObject::tr("The test copy %1 has gone.").arg(ShadowName));

	// The diff is computed here rather than taken on trust from the agent: what
	// the user approves has to be what is actually different.
	//
	// Pairwise over both sections, the same way ApplyShadowChanges does it, and
	// deliberately NOT by comparing each shadow value against GetText. Plenty of
	// settings are multi-valued - a box carries several Template= lines - and a
	// single-value accessor returns only the first, so every repeat read as a
	// change. That turned a one-setting fix into a nine-change confirmation,
	// which is the one thing a consent dialog must never do.
	QList<CSbieIni::SbieIniValue> New = pShadow->GetIniSection();
	QList<CSbieIni::SbieIniValue> Old = pBox->GetIniSection();

	for (auto I = New.begin(); I != New.end();) {
		auto II = I++;
		bool bMatched = false;
		for (auto J = Old.begin(); J != Old.end(); ) {
			auto JJ = J++;
			if (II->Name == JJ->Name && II->Value == JJ->Value) {
				I = New.erase(II);
				Old.erase(JJ);
				bMatched = true;
				break;
			}
		}
		Q_UNUSED(bMatched);
	}

	// What the shadow carries for its own sake is not a change to anything.
	static const QStringList Internal = QStringList() << "FileRootPath" << "IsShadow" << "BoxAlias";

	QList<CMcpConsent::SChange> Diff;
	foreach(const CSbieIni::SbieIniValue& Value, New) {
		if (Internal.contains(Value.Name))
			continue;
		CMcpConsent::SChange Row;
		Row.Setting = Value.Name;
		// Whatever is left in Old under this name is what it replaces; for a
		// multi-valued setting this is an addition rather than a replacement.
		foreach(const CSbieIni::SbieIniValue& Was, Old) {
			if (Was.Name == Value.Name) { Row.Current = Was.Value; break; }
		}
		Row.Proposed = Value.Value;
		Row.bKnown = true;
		Diff.append(Row);
	}
	foreach(const CSbieIni::SbieIniValue& Value, Old) {
		if (Internal.contains(Value.Name))
			continue;
		// Present on the box, gone from the shadow: a removal, and it has to be
		// shown as one.
		bool bStillThere = false;
		foreach(const CMcpConsent::SChange& Row, Diff) {
			if (Row.Setting == Value.Name) { bStillThere = true; break; }
		}
		if (bStillThere)
			continue;
		CMcpConsent::SChange Row;
		Row.Setting = Value.Name;
		Row.Current = Value.Value;
		Row.Proposed = QString();
		Row.bKnown = true;
		Diff.append(Row);
	}

	if (Diff.isEmpty()) {
		pSession->Shadows().SetApplyShadow(pBox->GetName(), false);
		return McpTools::MakeTextResult(QObject::tr(
			"The test copy is identical to %1, so there is nothing to keep. Discarding it would "
			"have the same effect.").arg(BoxLabel(pBox)));
	}

	if (!g_pConsent)
		return NoConsentResult();
	if (g_pConsent->IsPrompting())
		return BusyResult();

	QString Rationale = Arguments.value("rationale").toString();
	if (Rationale.trimmed().isEmpty())
		Rationale = QObject::tr("(the agent gave no reason)");

	if (!g_pConsent->AskChanges(pSession->GetClientName(), BoxLabel(pBox), Rationale, Diff)) {
		return McpTools::MakeErrorResult(QObject::tr(
			"The user declined. The test copy is still there, and %1 is unchanged - call "
			"shadow_discard to clean up.").arg(BoxLabel(pBox)));
	}

	pSession->Shadows().SetApplyShadow(pBox->GetName(), true);
	if (!pSession->Shadows().ApplyShadowChanges())
		return McpTools::MakeErrorResult(QObject::tr("The changes could not be copied back."));

	QJsonObject Data;
	Data["box"] = pBox->GetName();
	Data["applied"] = Diff.count();
	return McpTools::MakeTextResult(QObject::tr("%1 change(s) applied to %2.")
		.arg(Diff.count()).arg(BoxLabel(pBox)), Data);
}

static QJsonObject CallShadowDiscard(const QJsonObject& Arguments, CMcpSession* pSession)
{
	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.").arg(Arguments.value("box").toString()));

	if (pSession->Shadows().GetShadowName(pBox->GetName()).isEmpty())
		return McpTools::MakeTextResult(QObject::tr("There was no test copy for '%1'.").arg(pBox->GetName()));

	// No confirmation: discarding changes nothing the user has, which is the
	// whole point of the shadow.
	pSession->Shadows().SetApplyShadow(pBox->GetName(), false);
	pSession->Shadows().DiscardAll();

	return McpTools::MakeTextResult(QObject::tr(
		"The test copy is gone and %1 was never changed. Note that any files the test wrote are "
		"still in the sandbox - discarding only undoes configuration.").arg(BoxLabel(pBox)));
}

static QJsonObject CallSnapshotCreate(const QJsonObject& Arguments, CMcpSession* pSession)
{
	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.").arg(Arguments.value("box").toString()));

	CSandBoxPlus* pBoxEx = qobject_cast<CSandBoxPlus*>(pBox.data());

	// The skip conditions, checked before bothering the user: a snapshot of
	// nothing protects nothing, and a RAM-disk sandbox is gone at reboot anyway.
	if (pBox->IsEmpty())
		return McpTools::MakeErrorResult(QObject::tr(
			"'%1' holds no content, so there is nothing to snapshot. Note that an encrypted or "
			"image-backed sandbox also looks empty while it is not mounted - if this one is, it "
			"has to be opened first.").arg(BoxLabel(pBox)));
	if (pBoxEx && pBoxEx->UseRamDisk())
		return McpTools::MakeErrorResult(QObject::tr(
			"'%1' is a RAM-disk sandbox: its contents are discarded when it is unmounted or the "
			"machine restarts, so a snapshot would protect nothing.").arg(BoxLabel(pBox)));
	if (pBox->GetActiveProcessCount() > 0)
		return McpTools::MakeErrorResult(QObject::tr(
			"Something is still running in '%1'. Snapshots require the sandbox to be idle - ask "
			"the user to close its programs, or use box_terminate, and then try again.").arg(BoxLabel(pBox)));

	if (!g_pConsent) return NoConsentResult();
	if (g_pConsent->IsPrompting()) return BusyResult();

	if (!g_pConsent->AskAction(pSession->GetClientName(), QObject::tr("Confirm snapshot"),
			QObject::tr("Take a snapshot of sandbox %1, so its files and registry can be rolled back "
				"later.\n\nThis is quick and does not copy the sandbox. It does mean the sandbox "
				"cannot shrink until the snapshot is merged or deleted.").arg(BoxLabel(pBox)),
			QObject::tr("Take Snapshot"))) {
		return McpTools::MakeErrorResult(QObject::tr("The user declined; no snapshot was taken."));
	}

	SB_STATUS Status = AwaitProgress(pBox->TakeSnapshot(QObject::tr("Before AI agent changes")),
		QObject::tr("Taking a snapshot of %1").arg(BoxLabel(pBox)));
	if (Status.IsError())
		return McpTools::MakeErrorResult(QObject::tr("The snapshot failed: %1")
			.arg(CSandMan::FormatError(Status)));

	QString Current;
	pBox->GetSnapshots(&Current);

	QJsonObject Data;
	Data["box"] = pBox->GetName();
	Data["snapshot"] = Current;
	return McpTools::MakeTextResult(QObject::tr(
		"Snapshot taken for %1. File and registry changes from here on can be undone with "
		"snapshot_rollback.").arg(BoxLabel(pBox)), Data);
}

static QJsonObject CallSnapshotRollback(const QJsonObject& Arguments, CMcpSession* pSession)
{
	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.").arg(Arguments.value("box").toString()));

	QString Current;
	const QMap<QString, SBoxSnapshot> Snapshots = pBox->GetSnapshots(&Current);
	if (Snapshots.isEmpty() || Current.isEmpty())
		return McpTools::MakeErrorResult(QObject::tr(
			"'%1' has no snapshot to roll back to.").arg(BoxLabel(pBox)));

	if (!g_pConsent) return NoConsentResult();
	if (g_pConsent->IsPrompting()) return BusyResult();

	const int Running = pBox->GetActiveProcessCount();
	QString Detail = QObject::tr("Roll sandbox %1 back to its snapshot.\n\n"
		"Every file and registry change made in the sandbox since the snapshot will be "
		"discarded. This cannot be undone.").arg(BoxLabel(pBox));
	if (Running > 0) {
		// Said plainly up front, because the rollback cannot proceed otherwise
		// and the user should not discover it as a side effect.
		Detail += QObject::tr("\n\n%1 program(s) are running in the sandbox and will be terminated "
			"first. Unsaved work in them will be lost.").arg(Running);
	}

	if (!g_pConsent->AskAction(pSession->GetClientName(), QObject::tr("Confirm rollback"),
			Detail, QObject::tr("Roll Back and Discard Changes"), CMcpConsent::eDestroys)) {
		return McpTools::MakeErrorResult(QObject::tr("The user declined; nothing was rolled back."));
	}

	if (Running > 0) {
		SB_STATUS Status = pBox->TerminateAll();
		if (Status.IsError())
			return McpTools::MakeErrorResult(QObject::tr(
				"The sandbox could not be stopped, so nothing was rolled back: %1")
				.arg(CSandMan::FormatError(Status)));
	}

	SB_STATUS Status = AwaitProgress(pBox->SelectSnapshot(Current),
		QObject::tr("Rolling %1 back").arg(BoxLabel(pBox)));
	if (Status.IsError())
		return McpTools::MakeErrorResult(QObject::tr("The rollback failed: %1")
			.arg(CSandMan::FormatError(Status)));

	QJsonObject Data;
	Data["box"] = pBox->GetName();
	Data["snapshot"] = Current;
	return McpTools::MakeTextResult(QObject::tr(
		"%1 was rolled back to its snapshot.").arg(BoxLabel(pBox)), Data);
}

static QJsonObject CallBoxTerminate(const QJsonObject& Arguments, CMcpSession* pSession)
{
	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.").arg(Arguments.value("box").toString()));

	const int Running = pBox->GetActiveProcessCount();
	if (!Running)
		return McpTools::MakeTextResult(QObject::tr("Nothing is running in %1.").arg(BoxLabel(pBox)));

	if (!g_pConsent) return NoConsentResult();
	if (g_pConsent->IsPrompting()) return BusyResult();

	if (!g_pConsent->AskAction(pSession->GetClientName(), QObject::tr("Confirm terminate"),
			QObject::tr("Terminate %1 program(s) running in sandbox %2.\n\nUnsaved work in them "
				"will be lost.").arg(Running).arg(BoxLabel(pBox)),
			QObject::tr("Terminate Them"), CMcpConsent::eDestroys)) {
		return McpTools::MakeErrorResult(QObject::tr("The user declined; nothing was terminated."));
	}

	SB_STATUS Status = pBox->TerminateAll();
	if (Status.IsError())
		return McpTools::MakeErrorResult(QObject::tr("Terminating failed: %1").arg(CSandMan::FormatError(Status)));

	return McpTools::MakeTextResult(QObject::tr("%1 program(s) in %2 were terminated.")
		.arg(Running).arg(BoxLabel(pBox)));
}

static QJsonObject CallBoxCreate(const QJsonObject& Arguments, CMcpSession* pSession)
{
	const QString Name = Arguments.value("box").toString().trimmed();
	if (Name.isEmpty())
		return McpTools::MakeErrorResult(QObject::tr("The 'box' argument is required."));

	SB_STATUS Valid = CSbieAPI::ValidateName(Name);
	if (Valid.IsError())
		return McpTools::MakeErrorResult(QObject::tr(
			"'%1' is not a usable sandbox name. Use letters, digits and underscores.").arg(Name));

	if (!theAPI->GetBoxByName(Name).isNull())
		return McpTools::MakeErrorResult(QObject::tr("A sandbox called '%1' already exists.").arg(Name));

	if (!g_pConsent) return NoConsentResult();
	if (g_pConsent->IsPrompting()) return BusyResult();

	if (!g_pConsent->AskAction(pSession->GetClientName(), QObject::tr("Confirm new sandbox"),
			QObject::tr("Create a new sandbox called %1, with default settings.").arg(Name),
			QObject::tr("Create It"), CMcpConsent::eReversible)) {
		return McpTools::MakeErrorResult(QObject::tr("The user declined; no sandbox was created."));
	}

	SB_STATUS Status = theAPI->CreateBox(Name);
	if (Status.IsError())
		return McpTools::MakeErrorResult(QObject::tr("The sandbox could not be created: %1")
			.arg(CSandMan::FormatError(Status)));

	QJsonObject Data;
	Data["box"] = Name;
	return McpTools::MakeTextResult(QObject::tr(
		"Sandbox %1 was created with default settings. Use box_apply_changes to configure it.").arg(Name), Data);
}

//
// Running a program is the one capability here whose risk is not bounded by
// Sandboxie. A sandbox contains what a program can reach, but the agent chose
// the program, so "it only runs sandboxed" is not the reassurance it sounds
// like. What makes it acceptable is that the user reads the command line
// itself, verbatim, and has to agree to it - so the checks below exist to keep
// that showing honest, not to vet the command:
//
//   * one line, no control characters. A command carrying newlines could add
//     its own reassuring paragraph to the text the user is reading, and
//     nothing legitimate needs them
//   * a length cap, because a command too long to display is a command the
//     user would be approving unseen
//   * never elevated. eStartDefault, with no argument that reaches anything
//     else - an agent that can ask for administrator rights can ask
//     repeatedly, and someone who has clicked Allow four times clicks a fifth
//
static QJsonObject CallBoxRun(const QJsonObject& Arguments, CMcpSession* pSession)
{
	const QString Command = Arguments.value("command").toString();
	const QString WorkingDir = Arguments.value("working_dir").toString().trimmed();
	const QString Reason = Arguments.value("reason").toString().trimmed();

	if (Command.trimmed().isEmpty())
		return McpTools::MakeErrorResult(QObject::tr("The 'command' argument is required."));

	const int kMaxCommand = 2000;
	if (Command.length() > kMaxCommand)
		return McpTools::MakeErrorResult(QObject::tr(
			"That command line is %1 characters, and the confirmation can only honestly show up "
			"to %2. Write a shorter command, or put the work in a script file and run that.")
			.arg(Command.length()).arg(kMaxCommand));

	foreach(const QChar& Char, Command) {
		if (Char.category() == QChar::Other_Control)
			return McpTools::MakeErrorResult(QObject::tr(
				"That command line contains a line break or a control character. It has to be a "
				"single plain line, because the user is shown it as one and has to be able to read "
				"exactly what they are approving."));
	}
	if (WorkingDir.contains(QChar(10)) || WorkingDir.contains(QChar(13)))
		return McpTools::MakeErrorResult(QObject::tr("The working directory has to be a single line."));

	CSandBoxPtr pBox = ResolveBox(Arguments.value("box").toString());
	if (pBox.isNull())
		return McpTools::MakeErrorResult(QObject::tr("No sandbox called '%1'.").arg(Arguments.value("box").toString()));

	// A test copy is not a place a program can run, and this is worth refusing
	// loudly rather than letting it look like it worked.
	//
	// A shadow is a clone of a box's CONFIGURATION whose FileRootPath points at
	// the original's file set - see CBoxShadows. The sandbox a process actually
	// joins follows that file root, not the section name: starting a program
	// against "<box>_Shadow" puts it in <box>, with <box>'s settings, and the
	// driver reports it as such. An earlier revision of this tool had a
	// use_test_copy argument that claimed otherwise; it was removed because the
	// confirmation named one sandbox while the program ran in another, which is
	// the one failure this whole consent path exists to prevent.
	if (pBox->GetBool("IsShadow", false)) {
		return McpTools::MakeErrorResult(QObject::tr(
			"'%1' is a test copy, which is a copy of a sandbox's configuration and not a "
			"sandbox a program can run in - it shares the real sandbox's files, so anything "
			"started against it would run in the real sandbox with the real settings. Apply the "
			"change with shadow_commit first, then run against the sandbox itself.")
			.arg(pBox->GetName()));
	}

	const QString RunIn = pBox->GetName();
	const QString Where = BoxLabel(pBox);

	if (!g_pConsent) return NoConsentResult();
	if (g_pConsent->IsPrompting()) return BusyResult();

	// The command goes in verbatim and on its own line. Anything that reformats
	// it here is a difference between what was approved and what runs.
	QString Detail = QObject::tr("Run this command inside %1:\n\n    %2").arg(Where).arg(Command);
	if (!WorkingDir.isEmpty())
		Detail += QObject::tr("\n\nWorking directory:\n    %1").arg(WorkingDir);
	if (!Reason.isEmpty())
		Detail += QObject::tr("\n\nIts reason, in its own words:\n%1").arg(Reason);
	Detail += QObject::tr("\n\nIt runs sandboxed and with your normal privileges, not as "
		"administrator - but the command above is the agent's choice, so read it before agreeing.");

	if (!g_pConsent->AskAction(pSession->GetClientName(), QObject::tr("Confirm running a program"),
			Detail, QObject::tr("Run It"), CMcpConsent::eExecutes)) {
		return McpTools::MakeErrorResult(QObject::tr("The user declined; nothing was started."));
	}

	// Routed through CSandMan rather than the API directly: it mounts an image
	// box that is not mounted yet, and handles the box-desktop modes.
	SB_RESULT(quint32) Result = theGUI->RunStart(RunIn, Command, CSbieAPI::eStartDefault, WorkingDir);
	if (Result.IsError())
		return McpTools::MakeErrorResult(QObject::tr("The program could not be started: %1")
			.arg(CSandMan::FormatError(Result)));

	QJsonObject Data;
	Data["box"] = RunIn;
	Data["command"] = Command;
	// Deliberately NOT called "pid". CSbieAPI::RunStart launches through
	// Sandboxie's own Start.exe, which starts the target and exits - so what
	// comes back is the launcher, and it is already dead by the time an agent
	// could use it. Handing that over as the program's pid would be a number
	// that looks usable and is not.
	Data["launcherPid"] = (double)Result.GetValue();

	return McpTools::MakeTextResult(QObject::tr(
		"Launched in %1. The pid is not reported because the program is started through "
		"Sandboxie's launcher, which exits immediately - its pid is not the program's. "
		"Call box_processes on %2 to get the real pid and to confirm it is still running, "
#ifdef SBIE_HAS_DIAG_EVENTS
		"and trace_query or box_diagnostics to see what it did. Nothing is waited for and no "
#else
		"and trace_query to see what it did. Nothing is waited for and no "
#endif
		"output is captured.").arg(Where).arg(RunIn), Data);
}

QJsonObject Call(const QString& Name, const QJsonObject& Arguments, CMcpSession* pSession)
{
	if (!theAPI || !theAPI->IsConnected())
		return McpTools::MakeErrorResult(QObject::tr(
			"Sandboxie is not running, so nothing can be changed."));

	if (Name == "config_validate")		return CallConfigValidate(Arguments);
	if (Name == "box_apply_changes")	return CallApplyChanges(Arguments, pSession);
	if (Name == "shadow_begin")			return CallShadowBegin(Arguments, pSession);
	if (Name == "shadow_set")			return CallShadowSet(Arguments, pSession);
	if (Name == "shadow_commit")		return CallShadowCommit(Arguments, pSession);
	if (Name == "shadow_discard")		return CallShadowDiscard(Arguments, pSession);
	if (Name == "snapshot_create")		return CallSnapshotCreate(Arguments, pSession);
	if (Name == "snapshot_rollback")	return CallSnapshotRollback(Arguments, pSession);
	if (Name == "box_terminate")		return CallBoxTerminate(Arguments, pSession);
	if (Name == "box_create")			return CallBoxCreate(Arguments, pSession);
	if (Name == "box_run")				return CallBoxRun(Arguments, pSession);

	return McpTools::MakeErrorResult(QObject::tr("Tool '%1' is not implemented.").arg(Name));
}

//
// A shadow is discarded by its owner going away, so a crash leaves the section
// behind. This runs at startup and covers the wizard too, which leaks them the
// same way - the mechanism predates this server.
//
int SweepOrphanShadows()
{
	if (!theAPI || !theAPI->IsConnected())
		return 0;

	int Removed = 0;
	foreach(const CSandBoxPtr& pBox, theAPI->GetAllBoxes()) {
		if (!pBox->GetBool("IsShadow", false))
			continue;
		// At startup nothing can legitimately own one: sessions and wizard
		// engines are both younger than this call.
		if (pBox->GetActiveProcessCount() > 0)
			pBox->TerminateAll();
		pBox->RemoveSection();
		Removed++;
	}
	if (Removed) {
		theAPI->CommitIniChanges();
		theAPI->ReloadBoxes(true);
	}
	return Removed;
}

}
