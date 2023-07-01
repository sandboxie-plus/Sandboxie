#pragma once
#include <QObject>
#include "../QSbieAPI/SbieStatus.h"


class CAddon : public QObject
{
public:
	CAddon(const QVariantMap& Data) : Installed(false), Data(Data) 
	{
		Id = Data["id"].toString();
	}
	QString Id;
	QVariantMap Data;
	bool Installed;
	CSbieProgressPtr pProgress;

	QVariant GetSpecificEntry(const QString& Name, QString* pName = NULL);
	QString GetLocalizedEntry(const QString& Name);
};

typedef QSharedPointer<CAddon> CAddonPtr;

class CAddonManager : public QObject
{
	Q_OBJECT
public:
	CAddonManager(QObject* parent = NULL);

	bool LoadAddons();

	void UpdateAddons();

	QList<CAddonPtr> GetAddons();

	CAddonPtr GetAddon(const QString& Id);
	bool HasAddon(const QString& Id);
	
	SB_PROGRESS	TryInstallAddon(const QString& Id, QWidget* pParent, const QString& Prompt = QString());
	SB_PROGRESS InstallAddon(const QString& Id);
	SB_PROGRESS	TryRemoveAddon(const QString& Id, QWidget* pParent);
	SB_PROGRESS RemoveAddon(const QString& Id);

signals:
	void DataUpdated();
	void AddonInstalled();

private slots:
	void OnUpdateData(const QVariantMap& Data, const QVariantMap& Params);
	void OnAddonDownloaded(const QString& Path, const QVariantMap& Params);

protected:
	static bool CheckAddon(const CAddonPtr& pAddon);

	static void CleanupPath(const QString& Path);

	static void	InstallAddonAsync(const QString& FilePath, CAddonPtr pAddon);
	static void	RemoveAddonAsync(CAddonPtr pAddon);

	static QString ExpandPath(QString Path);

	QList<CAddonPtr> m_Addons;
};

