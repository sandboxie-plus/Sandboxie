#include "stdafx.h"
#include "McpConsent.h"
#include "../SandMan.h"
#include "../SbiePlusAPI.h"
#include "McpSessionWindow.h"

CMcpConsent::CMcpConsent(QObject* parent)
 : QObject(parent)
{
}

//
// Settings that give a sandbox reach it did not have. Getting this list
// wrong in the permissive direction is the whole risk of the "ask only for
// risky things" mode, so it is deliberately broad: anything whose name
// starts with Open grants access by construction, and the named ones below
// each remove a specific protection.
//
bool CMcpConsent::OpensAccess(const QString& Setting)
{
	const QString Name = Setting.trimmed();
	if (Name.startsWith("Open", Qt::CaseInsensitive))
		return true;

	static const char* const kWidens[] = {
		"NormalFilePath", "NormalKeyPath", "NormalIpcPath",
		"Template",				// a template is an arbitrary bundle of rules
		"NoSecurityIsolation", "NoSecurityFiltering", "OriginalToken",
		"UnrestrictedToken", "StripSystemPrivileges", "KeepTokenIntegrity",
		"AllowNetworkAccess", "BlockNetworkFiles", "ClosedFilePath",
		"ConfigLevel", "EditAdminOnly", "DropAdminRights", "FakeAdminRights",
		"ExposeBoxedSystem", "ProtectHostImages", "UseSecurityMode",
		"SysCallLockDown", "RestrictDevices",
		NULL
	};
	for (int i = 0; kWidens[i]; i++) {
		if (Name.compare(kWidens[i], Qt::CaseInsensitive) == 0)
			return true;
	}
	return false;
}

bool CMcpConsent::IsConfirmationRequired()
{
	// Sandboxie.ini, not the per-user file: a ceiling that the thing it
	// restrains can rewrite is not a ceiling.
	if (!theAPI || !theAPI->IsConnected())
		return false;
	return theAPI->GetGlobalSettings()->GetBool("AiAgentRequireConfirmation", false);
}

CMcpConsent::EMode CMcpConsent::EffectiveMode() const
{
	if (IsConfirmationRequired() && m_Mode > eAskAlways)
		return eAskAlways;
	return m_Mode;
}

QString CMcpConsent::ModeName(EMode Mode)
{
	switch (Mode) {
		case eReadOnly:		return tr("Look, do not touch");
		case eAskRisky:		return tr("Ask only for risky things");
		case eAskNever:		return tr("Do not ask, just tell me");
		default:			return tr("Ask before every change");
	}
}

QString CMcpConsent::ModeDescription(EMode Mode)
{
	switch (Mode) {
		case eReadOnly:
			return tr("The agent can read and diagnose, and every attempt to change "
				"anything is refused without asking you.");
		case eAskRisky:
			return tr("Ordinary setting changes happen without interrupting you. You are "
				"still asked before anything that cannot be undone, runs a program, sends "
				"something off this computer, or opens a sandbox up.");
		case eAskNever:
			return tr("The agent acts without asking. Everything it does is still listed "
				"here and written to the audit log - you are not asked, but you are told.");
		default:
			return tr("Nothing changes, starts or stops until you say so.");
	}
}

//
// Whether this particular request stops to ask.
//
static bool NeedsAsking(CMcpConsent::EMode Mode, CMcpConsent::ERisk Risk)
{
	switch (Mode) {
		case CMcpConsent::eAskNever:	return false;
		case CMcpConsent::eAskRisky:	return Risk != CMcpConsent::eReversible;
		default:						return true;
	}
}

bool CMcpConsent::Ask(const QString& Title, const QString& Body, const QString& ConfirmText,
	const QString& AuditWhat, ERisk Risk)
{
	CMcpSessionWindow* pWindow = theGUI->GetMcpWindow();
	const EMode Mode = EffectiveMode();

	if (Mode == eReadOnly) {
		pWindow->AddEntry(CMcpSessionWindow::eRefused,
			tr("refused, because the agent is set to look but not touch: %1").arg(AuditWhat));
		emit Decided(AuditWhat, false, tr("refused: the agent may not change anything"));
		return false;
	}

	if (!NeedsAsking(Mode, Risk)) {
		// Not asked, but told. The history entry and the audit record are the
		// part that does not depend on the mode - a user who turned the
		// questions off still gets to see what happened.
		pWindow->AddEntry(CMcpSessionWindow::eChange,
			tr("done without asking: %1").arg(AuditWhat), Body);
		emit Decided(AuditWhat, true, tr("allowed automatically (%1)").arg(ModeName(Mode)));
		return true;
	}

	// Both flags matter: this one covers a second tool call arriving while the
	// first is being answered, and the window one also covers the connection
	// question, which does not come through here. Either way the answer is to
	// refuse rather than queue - stacking approvals is how someone ends up
	// agreeing to the second one while reading the first.
	if (m_bPrompting || pWindow->IsPrompting()) {
		emit Decided(AuditWhat, false, tr("another confirmation was already open"));
		return false;
	}

	m_bPrompting = true;

	// The question goes into the session window, at the bottom, under the
	// history of what this agent has already done. AskPrompt blocks until it is
	// answered, which is what keeps the agent waiting on the user.
	const bool bAllowed = pWindow->AskPrompt(Title, Body, ConfirmText);

	m_bPrompting = false;

	pWindow->AddEntry(bAllowed ? CMcpSessionWindow::eAllowed : CMcpSessionWindow::eRefused,
		bAllowed ? tr("allowed: %1").arg(AuditWhat) : tr("refused: %1").arg(AuditWhat), Body);

	emit Decided(AuditWhat, bAllowed, bAllowed ? tr("allowed by the user") : tr("refused by the user"));
	return bAllowed;
}

bool CMcpConsent::AskChanges(const QString& Client, const QString& BoxLabel,
	const QString& Rationale, const QList<SChange>& Changes)
{
	const QString Who = Client.isEmpty() ? tr("An AI agent") : Client;

	QString Body = tr("%1 wants to change the configuration of sandbox %2.")
		.arg(Who).arg(BoxLabel);

	if (!Rationale.trimmed().isEmpty()) {
		// The reason is the agent speaking, and is labelled as such: it is an
		// argument for the change, not a description of it.
		Body += tr("\n\nIts reason, in its own words:\n%1").arg(Rationale.trimmed());
	}

	Body += tr("\n\nThese are the changes:\n");

	bool bAnyUnknown = false;
	foreach(const SChange& Change, Changes)
	{
		if (!Change.bKnown)
			bAnyUnknown = true;

		if (Change.Proposed.isEmpty())
			Body += tr("\n  remove  %1 = %2").arg(Change.Setting).arg(Change.Current);
		else if (Change.Current.isEmpty())
			Body += tr("\n  add     %1 = %2").arg(Change.Setting).arg(Change.Proposed);
		else
			Body += tr("\n  change  %1 = %2   (was %3)").arg(Change.Setting).arg(Change.Proposed).arg(Change.Current);

		if (!Change.bKnown)
			Body += tr("   <- not a known setting");
	}

	if (bAnyUnknown) {
		// Worth saying loudly: the driver ignores an unknown setting instead of
		// complaining, so this would look like a fix that silently did nothing.
		Body += tr("\n\nWarning: at least one of these is not a setting Sandboxie recognises. "
			"Unknown settings are ignored silently rather than reported as an error, so such "
			"a change would appear to work and have no effect.");
	}

	// A change that opens a sandbox up is the one a prompt injection would
	// want, so it is not in the same class as any other setting edit and keeps
	// asking even in the mode that otherwise does not.
	ERisk Risk = eReversible;
	foreach(const SChange& Change, Changes) {
		if (OpensAccess(Change.Setting)) { Risk = eOpensAccess; break; }
	}

	return Ask(tr("Confirm configuration change"), Body, tr("Apply These Changes"),
		tr("change %1 setting(s) on %2").arg(Changes.count()).arg(BoxLabel), Risk);
}

bool CMcpConsent::AskAction(const QString& Client, const QString& Title,
	const QString& Detail, const QString& ConfirmText, ERisk Risk)
{
	const QString Who = Client.isEmpty() ? tr("An AI agent") : Client;
	const QString Body = tr("%1 wants to do the following.\n\n%2").arg(Who).arg(Detail);
	return Ask(Title, Body, ConfirmText, Detail.simplified().left(120), Risk);
}
