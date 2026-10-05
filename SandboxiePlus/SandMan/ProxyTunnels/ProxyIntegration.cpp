// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
// See ../LICENSE for the SandMan license.
#include "stdafx.h"
#include "ProxyIntegration.h"
#include "ProxyWindow.h"
#include "SandMan.h"
#include "Helpers/WinAdmin.h"
#include "AddonManager.h"
#include "Windows/SettingsWindow.h"
#include <QCoreApplication>
#include <QDir>
#include <QSet>
#include <QUuid>

namespace
{
	const QString Guard = QStringLiteral("ProxyTunnelGuard");
	QString ProfileId(QString Binding)
	{
		Binding = Binding.section(',', -1).trimmed();
		if (!Binding.startsWith("SbieProxy-", Qt::CaseInsensitive)) return QString();
		const QUuid Id(Binding.mid(10));
		return Id.isNull() ? QString() : Id.toString(QUuid::WithoutBraces);
	}
	QString PolicyError(const CSandBoxPtr& Box)
	{
		if (Box->GetBool("NoSecurityIsolation", false, true, true) || Box->GetBool("NoSecurityFiltering", false, true, true))
			return CProxyWindow::tr("Security-isolation bypass settings are incompatible with managed tunnels.");
		const auto Bindings = Box->GetTextList("BindAdapter", true, false, true);
		if (Bindings.size() != 1 || Bindings.first().contains(',') || ProfileId(Bindings.first()).isEmpty()
			|| !Box->GetTextList("BindAdapterIP", true, false, true).isEmpty()
			|| !Box->GetTextList("NetworkUseProxy", true, false, true).isEmpty())
			return CProxyWindow::tr("The managed binding conflicts with another network binding or proxy setting.");
		if (!Box->GetTemplates().contains(Guard)) return CProxyWindow::tr("The managed tunnel guard template is missing.");
		const auto Strict = Box->GetTextList("StrictBindIP", true, false, true);
		for (const auto& Value : Strict) if (Value.compare("y", Qt::CaseInsensitive) != 0)
			return CProxyWindow::tr("Strict binding has an incompatible override.");
		if (Strict.isEmpty() || Box->GetTextList("NetworkDnsFilter", true, false, true) != QStringList{"*"})
			return CProxyWindow::tr("The DNS/binding guard is missing or overridden.");
		QStringList Rules = Box->GetTextList("NetworkAccess", true, false, true);
		QStringList Expected{"*,Block;Protocol=UDP", "*,Block;Protocol=ICMP", "*,Block;Port=53"};
		Rules.sort(); Expected.sort();
		if (Rules != Expected || !Box->GetTextList("ClosedIpcPath", true, false, true).contains("\\RPC Control\\DNSResolver"))
			return CProxyWindow::tr("The managed network restrictions changed. Review the sandbox settings.");
		return QString();
	}
}

CProxyIntegration::CProxyIntegration(CSandMan* Gui) : QObject(Gui), m_Gui(Gui), m_Manager(nullptr)
{
	SProxyHooks Hooks;
	Hooks.ActivationError = [this]() { return ActivationError(); };
	Hooks.Boxes = [this]() { return Boxes(); };
	Hooks.ModificationError = [this](const QString& Id) { return ModificationError(Id); };
	Hooks.Assign = [this](const QString& Box, const QString& Id, QString& Error) { return Assign(Box, Id, Error); };
	Hooks.DependencyDirectory = []() { return QDir(QCoreApplication::applicationDirPath()).filePath("Addons/ProxyTunnels"); };
	Hooks.InstallDependencies = [this](QString& Error) {
		auto Addons = m_Gui->GetAddonManager();
		Addons->GetAddons();
		if (!Addons->GetAddon("ProxyTunnels", CAddonManager::eAny)) {
			Error = CProxyWindow::tr("The ProxyTunnels package is not present in the official add-on catalog. No executable was downloaded. Maintainer packaging approval is pending.");
			return false;
		}
		const auto Status = Addons->TryInstallAddon("ProxyTunnels", m_Gui);
		if (Status.IsError()) { Error = CProxyWindow::tr("Dependency installation was cancelled or failed. See the existing add-on progress/error dialog."); return false; }
		return true;
	};
	Hooks.Log = [this](const QString& Message) { m_Gui->OnLogMessage(Message); };
	m_Manager = new CProxyManager(QDir(theConf->GetConfigDir()).filePath("ProxyProfiles.json"), std::move(Hooks), this);
}

CProxyIntegration::~CProxyIntegration()
{
	delete m_Window.data();
	delete m_Manager; // Stop and join owned workers before SandMan releases the API.
}

void CProxyIntegration::Show()
{
	if (!m_Window) m_Window = new CProxyWindow(m_Manager, m_Gui);
	m_Window->show(); m_Window->raise(); m_Window->activateWindow();
}

QString CProxyIntegration::ActivationError() const
{
#ifndef SBIE_PROXY_TUNNELS_LAB
	return CProxyWindow::tr("This review prototype cannot start tunnels or create sandbox bindings. A stopped sandbox may detach a local managed binding to recover its configuration. Windows routing, ownership, DNS, and fail-closed qualification are still pending.");
#else
	if (!theAPI || !theAPI->IsConnected()) return CProxyWindow::tr("Sandboxie is not connected.");
	if (!IsElevated()) return CProxyWindow::tr("Adapter setup requires an administrator. Restart SandMan using its existing elevation command.");
	if (!g_CertInfo.active || !g_CertInfo.opt_net) return CProxyWindow::tr("The existing advanced-network certificate requirement is not satisfied.");
	if (!m_Gui->IsWFPEnabled()) return CProxyWindow::tr("Windows Filtering Platform must already be enabled. The tunnel manager will not change this setting.");
	for (const auto& Box : theAPI->GetAllBoxes()) {
		bool Managed = false;
		for (const auto& Binding : Box->GetTextList("BindAdapter", true, false, true)) Managed |= !ProfileId(Binding).isEmpty();
		if (Managed) { const QString Error = PolicyError(Box); if (!Error.isEmpty()) return Error; }
	}
	return QString();
#endif
}

QList<SProxyBox> CProxyIntegration::Boxes() const
{
	QList<SProxyBox> Result;
	if (!theAPI || !theAPI->IsConnected()) return Result;
	for (const auto& Box : theAPI->GetAllBoxes()) {
		QSet<QString> Seen;
		const auto Bindings = Box->GetTextList("BindAdapter", true, false, true);
		for (const auto& Binding : Bindings) {
			const QString Id = ProfileId(Binding);
			if (Id.isEmpty() || Seen.contains(Id)) continue;
			Seen.insert(Id);
			QString Detail = PolicyError(Box);
			if (Detail.isEmpty()) Detail = CProxyWindow::tr("Saved binding; tunnel runtime is shown above.");
			Result.append({Box->GetName(), Id, Detail});
		}
		if (Seen.isEmpty()) Result.append({Box->GetName(), QString(), Bindings.isEmpty()
			? CProxyWindow::tr("No managed association.") : CProxyWindow::tr("An unmanaged adapter binding is configured.")});
	}
	return Result;
}

QString CProxyIntegration::ModificationError(const QString& Id) const
{
	if (!theAPI || !theAPI->IsConnected()) return CProxyWindow::tr("Connect to Sandboxie to check all profile references before changing this profile.");
	for (const auto& Box : theAPI->GetAllBoxes()) {
		for (const auto& Binding : Box->GetTextList("BindAdapter", true, false, true)) {
			if (ProfileId(Binding) == Id && Box->GetActiveProcessCount() != 0)
				return CProxyWindow::tr("Stop every process in the associated sandboxes before editing the profile.");
		}
	}
	return QString();
}

bool CProxyIntegration::Assign(const QString& BoxName, const QString& Id, QString& Error)
{
	if (!theAPI || !theAPI->IsConnected()) { Error = CProxyWindow::tr("Sandboxie is not connected."); return false; }
	if (!Id.isEmpty()) {
		Error = ActivationError();
		if (!Error.isEmpty()) return false;
	}
	const auto Box = theAPI->GetBoxByName(BoxName);
	if (!Box) { Error = CProxyWindow::tr("The sandbox no longer exists."); return false; }
	if (Box->GetActiveProcessCount() != 0) { Error = CProxyWindow::tr("Stop all sandbox processes before changing its network association."); return false; }
	const auto Bindings = Box->GetTextList("BindAdapter", true, false, true);
	const auto LocalBindings = Box->GetTextList("BindAdapter", false);
	if (Bindings != LocalBindings || Bindings.size() > 1 || (!Bindings.isEmpty() && ProfileId(Bindings.first()).isEmpty())) {
		Error = CProxyWindow::tr("Existing adapter bindings must be reviewed in sandbox options; they were not overwritten."); return false;
	}
	if (Id.isEmpty()) {
		if (!Bindings.isEmpty() && Box->DelValue("BindAdapter", Bindings.first()).IsError()) { Error = CProxyWindow::tr("Cannot remove the managed binding. Check Sandboxie configuration permissions."); return false; }
		if (!Box->GetTemplates().contains(Guard)) return true;
		if (!Box->GetTextList("Template", false).contains(Guard)) {
			Error = CProxyWindow::tr("The managed binding was removed, but the restrictive guard is inherited and was left unchanged."); return false;
		}
		if (Box->DelValue("Template", Guard).IsError()) { Error = CProxyWindow::tr("The binding was removed, but the restrictive guard remains. Review sandbox options."); return false; }
		return true;
	}
	if (Box->GetBool("NoSecurityIsolation", false, true, true) || Box->GetBool("NoSecurityFiltering", false, true, true)) {
		Error = CProxyWindow::tr("The sandbox disables security isolation or filtering."); return false;
	}
	if (!Box->GetTextList("BindAdapterIP", true, false, true).isEmpty() || !Box->GetTextList("NetworkUseProxy", true, false, true).isEmpty()) {
		Error = CProxyWindow::tr("Remove conflicting direct-IP bindings or built-in proxy rules before associating a managed tunnel."); return false;
	}
	if (!Box->GetTemplates().contains(Guard)) {
		if (!Box->GetTextList("NetworkAccess", true, false, true).isEmpty() || !Box->GetTextList("NetworkDnsFilter", true, false, true).isEmpty()
			|| !Box->GetTextList("StrictBindIP", true, false, true).isEmpty()) {
			Error = CProxyWindow::tr("Existing network restrictions or overrides need manual review; no rules were replaced."); return false;
		}
		if (Box->AppendText("Template", Guard).IsError()) { Error = CProxyWindow::tr("Cannot add the tunnel guard template. Check Sandboxie configuration permissions."); return false; }
	}
	// The guard is applied first. Failed writes leave restrictive settings, not a silent direct fallback.
	if (Box->SetText("BindAdapter", QStringLiteral("SbieProxy-") + Id).IsError()) {
		Error = CProxyWindow::tr("Cannot save the tunnel binding. The guard may remain active; review sandbox options."); return false;
	}
	Error = PolicyError(Box);
	return Error.isEmpty();
}
