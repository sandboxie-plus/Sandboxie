// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
// See ../LICENSE for the SandMan license.
#ifdef PROXY_TUNNELS_STANDALONE
#include <QtCore>
#else
#include "stdafx.h"
#endif
#include "ProxyManager.h"

CProxyManager::CProxyManager(const QString& Path, SProxyHooks Hooks, QObject* Parent, TFactory Factory)
	: QObject(Parent), m_Path(Path), m_Hooks(std::move(Hooks)), m_Factory(std::move(Factory))
{
	m_Lock.reset(new QLockFile(Path + QStringLiteral(".lock")));
	m_Lock->setStaleLockTime(0);
	if (!m_Lock->tryLock(0)) m_StorageError = tr("Another manager owns this profile store, or the store is not writable.");
	QString Error;
	if (!ProxyProfiles::Load(Path, m_Profiles, Error)) m_StorageError = Error;
	if (!m_Factory) m_Factory = [](const SProxyProfile& Profile, const QString& Directory, QObject* Parent) {
		return new CProxyTunnel(Profile, Directory, Parent);
	};
	connect(&m_Timer, &QTimer::timeout, this, &CProxyManager::Revalidate);
	m_Timer.start(1000);
}

CProxyManager::~CProxyManager()
{
	m_Timer.stop();
	const auto Workers = m_Workers;
	for (auto Worker : Workers) { Worker->disconnect(this); Worker->RequestStop(); }
	for (auto Worker : Workers) { Worker->wait(); delete Worker; }
	m_Workers.clear();
}

int CProxyManager::Find(const QString& Id) const
{
	for (int i = 0; i < m_Profiles.size(); ++i) if (m_Profiles[i].Id == Id) return i;
	return -1;
}

QList<SProxyBox> CProxyManager::Boxes() const { return m_Hooks.Boxes ? m_Hooks.Boxes() : QList<SProxyBox>(); }

QStringList CProxyManager::Users(const QString& Id) const
{
	QStringList Names;
	for (const SProxyBox& Box : Boxes()) if (Box.ProfileId == Id) Names.append(Box.Name);
	return Names;
}

QString CProxyManager::ActivationError() const
{
	if (!m_StorageError.isEmpty()) return m_StorageError;
	return m_Hooks.ActivationError ? m_Hooks.ActivationError() : tr("No sandbox policy integration is available.");
}

bool CProxyManager::Persist(const QList<SProxyProfile>& Profiles, QString& Error)
{
	if (!m_StorageError.isEmpty()) { Error = m_StorageError; return false; }
	if (!ProxyProfiles::Save(m_Path, Profiles, Error)) return false;
	m_Profiles = Profiles;
	emit Changed();
	return true;
}

bool CProxyManager::SaveProfile(const SProxyInput& Input, QString& Id, QString& Error)
{
	QList<SProxyProfile> Updated = m_Profiles;
	int Index = Find(Id);
	if (!Id.isEmpty() && Index < 0) { Error = tr("The profile no longer exists."); return false; }
	if (m_Workers.contains(Id)) { Error = tr("Stop the tunnel before editing this profile."); return false; }
	if (!Id.isEmpty() && m_Hooks.ModificationError) {
		Error = m_Hooks.ModificationError(Id);
		if (!Error.isEmpty()) return false;
	}
	SProxyProfile Profile = Index < 0 ? SProxyProfile() : Updated[Index];
	if (!ProxyProfiles::Protect(Input, Profile, Error)) return false;
	if (Index < 0) Updated.append(Profile); else Updated[Index] = Profile;
	if (!Persist(Updated, Error)) return false;
	Id = Profile.Id;
	return true;
}

bool CProxyManager::Import(const QString& Text, QList<SProxyImportError>& Errors, int& Added, QString& Error)
{
	Added = 0;
	const auto Inputs = ProxyProfiles::ParseList(Text, Errors);
	if (Inputs.isEmpty()) { Error = tr("No valid proxy entries were found."); return false; }
	QList<SProxyProfile> Updated = m_Profiles;
	for (const auto& Input : Inputs) {
		SProxyProfile Profile;
		if (!ProxyProfiles::Protect(Input, Profile, Error)) return false;
		Updated.append(Profile);
	}
	if (!Persist(Updated, Error)) return false;
	Added = Inputs.size();
	return true;
}

bool CProxyManager::Remove(const QString& Id, QString& Error)
{
	const int Index = Find(Id);
	if (Index < 0) { Error = tr("The profile no longer exists."); return false; }
	if (m_Workers.contains(Id)) { Error = tr("Stop the tunnel before removing this profile."); return false; }
	if (!Users(Id).isEmpty()) { Error = tr("Detach all sandbox references before removing this profile."); return false; }
	if (m_Hooks.ModificationError) {
		Error = m_Hooks.ModificationError(Id);
		if (!Error.isEmpty()) return false;
	}
	QList<SProxyProfile> Updated = m_Profiles;
	Updated.removeAt(Index);
	if (!Persist(Updated, Error)) return false;
	m_Runtime.remove(Id);
	return true;
}

bool CProxyManager::Assign(const QString& Box, const QString& Id, QString& Error)
{
	if (!Id.isEmpty()) {
		Error = ActivationError();
		if (!Error.isEmpty()) return false;
	}
	if (!Id.isEmpty() && Find(Id) < 0) { Error = tr("The profile no longer exists."); return false; }
	if (!m_Hooks.Assign) { Error = tr("Sandbox association is unavailable."); return false; }
	if (!m_Hooks.Assign(Box, Id, Error)) return false;
	emit Changed();
	return true;
}

void CProxyManager::SetState(const QString& Id, int State, const QString& Detail)
{
	auto& Runtime = m_Runtime[Id];
	Runtime.State = State;
	Runtime.Detail = Detail;
	if (State == CProxyTunnel::Starting || State == CProxyTunnel::Stopped || State == CProxyTunnel::Failed) {
		Runtime.Egress.clear();
		Runtime.CheckedAt = QDateTime();
	}
	if (m_Hooks.Log) m_Hooks.Log(tr("Proxy tunnel [%1]: %2").arg(Id, Detail));
	emit Changed();
}

bool CProxyManager::Start(const QString& Id, QString& Error)
{
	Error = ActivationError();
	if (!Error.isEmpty()) return false;
	const int Index = Find(Id);
	if (Index < 0) { Error = tr("The profile no longer exists."); return false; }
	if (m_Workers.contains(Id)) { Error = tr("This tunnel is already starting, running, or stopping."); return false; }
	if (m_Workers.size() >= 16) { Error = tr("This prototype supports at most 16 simultaneous tunnels."); return false; }
	const QString Directory = m_Hooks.DependencyDirectory ? m_Hooks.DependencyDirectory() : QString();
	CProxyTunnel* Worker = m_Factory(m_Profiles[Index], Directory, this);
	if (!Worker) { Error = tr("Cannot allocate the tunnel worker."); return false; }
	m_Workers.insert(Id, Worker);
	SetState(Id, CProxyTunnel::Starting, tr("Starting the owned tunnel worker."));
	connect(Worker, &CProxyTunnel::StateChanged, this, [this, Worker, Id](const QString&, int State, const QString& Detail) {
		if (m_Workers.value(Id) != Worker) return;
		// Do not turn a cancelled startup back into a green status.
		if (m_Runtime.value(Id).State == CProxyTunnel::Stopping && State != CProxyTunnel::Stopped && State != CProxyTunnel::Failed) return;
		SetState(Id, State, Detail);
	});
	connect(Worker, &CProxyTunnel::EgressChecked, this, [this, Worker, Id](const QString&, const QString& Address) {
		if (m_Workers.value(Id) != Worker || m_Runtime.value(Id).State == CProxyTunnel::Stopping) return;
		m_Runtime[Id].Egress = Address;
		m_Runtime[Id].CheckedAt = QDateTime::currentDateTimeUtc();
		emit Changed();
	});
	connect(Worker, &QThread::finished, this, [this, Worker, Id]() {
		if (m_Workers.value(Id) != Worker) return;
		m_Workers.remove(Id);
		if (m_Runtime.value(Id).State != CProxyTunnel::Failed)
			SetState(Id, CProxyTunnel::Stopped, tr("Tunnel stopped. Sandbox associations were retained."));
		Worker->deleteLater();
		emit Changed();
	});
	Worker->start();
	return true;
}

void CProxyManager::Stop(const QString& Id)
{
	if (auto Worker = m_Workers.value(Id, nullptr)) {
		SetState(Id, CProxyTunnel::Stopping, tr("Stopping the owned worker; sandbox bindings remain configured."));
		Worker->RequestStop();
	}
}

void CProxyManager::StopAll() { const auto Ids = m_Workers.keys(); for (const QString& Id : Ids) Stop(Id); }
void CProxyManager::Check(const QString& Id) { if (auto Worker = m_Workers.value(Id, nullptr)) Worker->RequestCheck(); }

void CProxyManager::Revalidate()
{
	if (m_Workers.isEmpty()) return;
	const QString Error = ActivationError();
	if (Error.isEmpty()) return;
	for (auto It = m_Workers.begin(); It != m_Workers.end(); ++It) {
		if (m_Runtime.value(It.key()).State == CProxyTunnel::Stopping) continue;
		SetState(It.key(), CProxyTunnel::Stopping, Error);
		It.value()->RequestStop();
	}
}

bool CProxyManager::InstallDependencies(QString& Error)
{
	if (!m_Hooks.InstallDependencies) { Error = tr("No dependency installer is configured."); return false; }
	return m_Hooks.InstallDependencies(Error);
}
