// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
// See ../LICENSE for the SandMan license.
#pragma once
#include "ProxyTunnel.h"
#include <QDateTime>
#include <QLockFile>
#include <QMap>
#include <QTimer>
#include <functional>
#include <memory>

struct SProxyBox
{
	QString Name;
	QString ProfileId;
	QString Detail;
};

// SandMan supplies policy/INI access; the controller never writes Sandboxie.ini directly.
struct SProxyHooks
{
	std::function<QString()> ActivationError;
	std::function<QList<SProxyBox>()> Boxes;
	std::function<bool(const QString&, const QString&, QString&)> Assign;
	std::function<QString(const QString&)> ModificationError;
	std::function<QString()> DependencyDirectory;
	std::function<bool(QString&)> InstallDependencies;
	std::function<void(const QString&)> Log;
};

struct SProxyRuntime
{
	int State = CProxyTunnel::Stopped;
	QString Detail;
	QString Egress;
	QDateTime CheckedAt;
};

class CProxyManager : public QObject
{
	Q_OBJECT
public:
	using TFactory = std::function<CProxyTunnel*(const SProxyProfile&, const QString&, QObject*)>;
	CProxyManager(const QString& Path, SProxyHooks Hooks, QObject* Parent = nullptr, TFactory Factory = TFactory());
	~CProxyManager() override;
	QList<SProxyProfile> Profiles() const { return m_Profiles; }
	QList<SProxyBox> Boxes() const;
	QStringList Users(const QString& Id) const;
	SProxyRuntime Runtime(const QString& Id) const { return m_Runtime.value(Id); }
	QString StorageError() const { return m_StorageError; }
	QString ActivationError() const;
	bool SaveProfile(const SProxyInput& Input, QString& Id, QString& Error);
	bool Import(const QString& Text, QList<SProxyImportError>& Errors, int& Added, QString& Error);
	bool Remove(const QString& Id, QString& Error);
	bool Assign(const QString& Box, const QString& Id, QString& Error);
	bool Start(const QString& Id, QString& Error);
	void Stop(const QString& Id);
	void StopAll();
	void Check(const QString& Id);
	bool InstallDependencies(QString& Error);
	bool HasWorker(const QString& Id) const { return m_Workers.contains(Id); }

signals:
	void Changed();

private:
	int Find(const QString& Id) const;
	bool Persist(const QList<SProxyProfile>& Profiles, QString& Error);
	void SetState(const QString& Id, int State, const QString& Detail);
	void Revalidate();
	QString m_Path;
	QString m_StorageError;
	std::unique_ptr<QLockFile> m_Lock;
	QList<SProxyProfile> m_Profiles;
	QMap<QString, SProxyRuntime> m_Runtime;
	QMap<QString, CProxyTunnel*> m_Workers;
	SProxyHooks m_Hooks;
	TFactory m_Factory;
	QTimer m_Timer;
};
