#pragma once
#include <QObject>
#include "../QSbieAPI/SbieStatus.h"


class CAddon : public QObject
{
public:
	CAddon(const QVariantMap& Data) : Data(Data) 
	{
		Id = Data["id"].toString();
	}

	QString Id;
	QVariantMap Data;
	CSbieProgressPtr pProgress;

	QVariant GetSpecificEntry(const QString& Name, QString* pName = NULL);
};

typedef QSharedPointer<CAddon> CAddonPtr;

class CAddonInfo
{
public:
	CAddonInfo(const QVariantMap& data, bool installed) : Data(data), Installed(installed) { 
		Id = Data["id"].toString();
	};

	QString Id;
	QVariantMap Data;
	bool Installed;
	QString UpdateVersion;

	QString GetLocalizedEntry(const QString& Name);
};

typedef QSharedPointer<CAddonInfo> CAddonInfoPtr;

class CAddonManager : public QObject
{
	Q_OBJECT
public:
	CAddonManager(QObject* parent = NULL);

	bool LoadAddons();

	void UpdateAddonsWhenNotCached();
	void UpdateAddons();

	QList<CAddonInfoPtr> GetAddons();

	enum EState {
		eAny = 0,
		eInstalled,
		eNotINstalled
	};

	CAddonPtr GetAddon(const QString& Id, EState State = eAny);
	
	SB_PROGRESS	TryInstallAddon(const QString& Id, QWidget* pParent, const QString& Prompt = QString());
	SB_PROGRESS InstallAddon(const QString& Id);
	SB_PROGRESS	TryRemoveAddon(const QString& Id, QWidget* pParent);
	SB_PROGRESS RemoveAddon(const QString& Id);
	
	static QString GetAppArch();
	static QString GetSysArch();
	static QString GetFramework();

signals:
	void DataUpdated();
	void AddonInstalled();

private slots:
	void OnUpdateData(const QVariantMap& Data, const QVariantMap& Params);

protected:

	static void	RunUpdaterAsync(CAddonPtr pAddon, const QStringList& Params);

	QList<CAddonPtr> m_Installed;
	QList<CAddonPtr> m_KnownAddons;
};

