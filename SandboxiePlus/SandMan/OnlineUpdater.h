#pragma once

#include <QObject>
#include "../MiscHelpers/Common/NetworkAccessManager.h"

#include "SbiePlusAPI.h"

#define UPDATE_INTERVAL (7 * 24 * 60 * 60)

class CGetUpdatesJob : public QObject
{
	Q_OBJECT

protected:
	friend class COnlineUpdater;

	CGetUpdatesJob(const QVariantMap& Params, QObject* parent = nullptr) : QObject(parent) { m_Params = Params; }
	virtual ~CGetUpdatesJob() {}

	QVariantMap	m_Params;

signals:
	void				UpdateData(const QVariantMap& Data, const QVariantMap& Params);
	void				Download(const QString& Path, const QVariantMap& Params);
};


class COnlineUpdater : public QObject
{
	Q_OBJECT
public:
	COnlineUpdater(QObject* parent);

	void				Process();

	void				GetUpdates(QObject* receiver, const char* member, const QVariantMap& Params = QVariantMap());
	void				DownloadFile(const QString& Url, QObject* receiver, const char* member, const QVariantMap& Params = QVariantMap());

	QVariantMap			GetUpdateData() { return m_UpdateData; }
	QDateTime			GetLastUpdateTime() { return m_LastUpdate; }

	void				UpdateCert(bool bWait = false);

	void				CheckForUpdates(bool bManual = false);

	bool				DownloadUpdate(const QVariantMap& Update, bool bAndApply = false);
	bool				ApplyUpdate(bool bSilent);

	bool				DownloadInstaller(const QVariantMap& Release, bool bAndRun = false);
	bool				RunInstaller(bool bSilent);

	static QString		MakeVersionStr(const QVariantMap& Data);
	static QString		ParseVersionStr(const QString& Str, int* pUpdate = NULL);
	static QString		GetCurrentVersion();
	static int			GetCurrentUpdate();
	static bool			IsVersionNewer(const QString& VersionStr);

	QString				GetUpdateDir(bool bCreate = false);

	static quint32		CurrentVersion();
	static quint32		VersionToInt(const QString& VersionStr);

private slots:
	void				OnUpdateCheck();

	void				OnFileDownload();

	void				OnDownloadProgress(qint64 bytes, qint64 bytesTotal);
	void				OnInstallerDownload(const QString& Path, const QVariantMap& Params);

	void				OnUpdateData(const QVariantMap& Data, const QVariantMap& Params);

	void				OnPrepareOutput();
	void				OnPrepareError();
	void				OnPrepareFinished(int exitCode, QProcess::ExitStatus exitStatus);

	void				OnCertCheck();

protected:
	bool				HandleUserMessage(const QVariantMap& Data);
	bool				HandleUpdate();

	enum EUpdateScope
	{
		eNone = 0,	// No files updated
		eMeta,		// Only Templates.ini and/or translations changed
		eCore,		// Core sandboxie Components Changed
		eFull		// Plus components changed
	};
	EUpdateScope		ScanUpdateFiles(const QVariantMap& Update);
	EUpdateScope		GetFileScope(const QString& Path);

	bool				AskDownload(const QVariantMap& Update);
	
	friend class CAddonManager;

	static SB_RESULT(int) RunUpdater(const QStringList& Params, bool bSilent, bool Wait = false);
	static bool			RunInstaller2(const QString& FilePath, bool bSilent);

	CNetworkAccessManager*	m_RequestManager;
	CSbieProgressPtr	m_pUpdateProgress;
	QMap<QNetworkReply*, CGetUpdatesJob*> m_JobQueue;

	QStringList			m_IgnoredUpdates;
	enum ECHeckMode
	{
		eInit = 0,
		eAuto,
		eManual,
		ePendingUpdate,
		ePendingInstall
	}					m_CheckMode;
	QVariantMap			m_UpdateData;
	QDateTime			m_LastUpdate;
	QProcess*			m_pUpdaterUtil;
};