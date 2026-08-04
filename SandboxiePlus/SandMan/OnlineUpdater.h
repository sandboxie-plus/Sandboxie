#pragma once

#include <QObject>
#include "../MiscHelpers/Common/NetworkAccessManager.h"

#include "SbiePlusAPI.h"

#define UPDATE_INTERVAL (7 * 24 * 60 * 60)

class CUpdatesJob : public QObject
{
	Q_OBJECT

protected:
	friend class COnlineUpdater;

	CUpdatesJob(const QVariantMap& Params, QObject* parent = nullptr) : QObject(parent) 
	{
		m_Params = Params; 
		m_pProgress = CSbieProgressPtr(new CSbieProgress());
	}
	virtual ~CUpdatesJob() {}

	virtual void Finish(QNetworkReply* pReply) = 0;

	QVariantMap			m_Params;
	CSbieProgressPtr	m_pProgress;

private slots:
	void OnDownloadProgress(qint64 bytes, qint64 bytesTotal)
	{
		if (bytesTotal != 0 && !m_pProgress.isNull())
			m_pProgress->Progress(100 * bytes / bytesTotal);
	}
};

class CGetUpdatesJob : public CUpdatesJob
{
	Q_OBJECT

protected:
	friend class COnlineUpdater;

	CGetUpdatesJob(const QVariantMap& Params, QObject* parent = nullptr) : CUpdatesJob(Params, parent) {}

	virtual void Finish(QNetworkReply* pReply);

signals:
	void				UpdateData(const QVariantMap& Data, const QVariantMap& Params);
};

class CGetFileJob : public CUpdatesJob
{
	Q_OBJECT

protected:
	friend class COnlineUpdater;

	CGetFileJob(const QVariantMap& Params, QObject* parent = nullptr) : CUpdatesJob(Params, parent) {}

	virtual void Finish(QNetworkReply* pReply);

signals:
	void				Download(const QString& Path, const QVariantMap& Params);
};

class CGetCertJob : public CUpdatesJob
{
	Q_OBJECT

protected:
	friend class COnlineUpdater;

	CGetCertJob(const QVariantMap& Params, QObject* parent = nullptr) : CUpdatesJob(Params, parent) {}

	virtual void Finish(QNetworkReply* pReply);

signals:
	void				Certificate(const QByteArray& Certificate, const QVariantMap& Params);
};

class COnlineUpdater : public QObject
{
	Q_OBJECT
public:
	COnlineUpdater(QObject* parent);

	SB_PROGRESS			GetUpdates(QObject* receiver, const char* member, const QVariantMap& Params = QVariantMap());
	SB_PROGRESS			DownloadFile(const QString& Url, QObject* receiver, const char* member, const QVariantMap& Params = QVariantMap());
	SB_PROGRESS			GetSupportCert(const QString& Serial, QObject* receiver, const char* member, const QVariantMap& Params = QVariantMap());

	static bool			IsLockRequired();

	static SB_RESULT(int) RunUpdater(const QStringList& Params, bool bSilent, bool Wait = false);

	void				Process();

	QVariantMap			GetUpdateData() { return m_UpdateData; }
	QDateTime			GetLastUpdateTime() { return m_LastUpdate; }

	void				CheckForUpdates(bool bManual = false);

	enum EUpdateScope
	{
		eNone = 0,	// No files updated
		eTmpl,		// Only Templates.ini
		eMeta,		// Only Templates.ini and/or translations changed
		eCore,		// Core sandboxie Components Changed
		eFull		// Plus components changed
	};
	bool				DownloadUpdate(const QVariantMap& Update, EUpdateScope Scope, bool bAndApply = false);
	bool				ApplyUpdate(EUpdateScope Scope, bool bSilent);

	bool				DownloadInstaller(const QVariantMap& Release, bool bAndRun = false);
	bool				RunInstaller(bool bSilent);

	void				UpdateTemplates();

	static QString		MakeVersionStr(const QVariantMap& Data);
	static QString		ParseVersionStr(const QString& Str, int* pUpdate = NULL);
	static QString		GetCurrentVersion();
	static int			GetCurrentUpdate();
	static bool			IsVersionNewer(const QString& VersionStr);

	static QString		GetUpdateDir(bool bCreate = false);

	static quint32		CurrentVersion();
	static quint32		VersionToInt(const QString& VersionStr);

	static quint64		GetRandID();

	static QDateTime	GetLastUpdateDate();

private slots:
	void				OnRequestFinished();

	void				OnInstallerDownload(const QString& Path, const QVariantMap& Params);

	void				OnUpdateData(const QVariantMap& Data, const QVariantMap& Params);
	void				OnUpdateDataTmpl(const QVariantMap& Data, const QVariantMap& Params);

	void				OnPrepareOutput();
	void				OnPrepareError();
	void				OnPrepareFinished(int exitCode, QProcess::ExitStatus exitStatus);

protected:

	void				StartJob(CUpdatesJob* pJob, const QUrl& Url);

	void				LoadState();

	bool				HandleUserMessage(const QVariantMap& Data);
	bool				HandleUpdate();

	QString				GetOnNewUpdateOption() const;
	QString				GetOnNewReleaseOption() const;
	bool				ShowCertWarningIfNeeded();

	EUpdateScope		ScanUpdateFiles(const QVariantMap& Update);
	EUpdateScope		GetFileScope(const QString& Path);

	bool				AskDownload(const QVariantMap& Update, bool bAuto);

	static bool			RunInstaller2(const QString& FilePath, bool bSilent);

	CNetworkAccessManager*	m_RequestManager;
	QMap<QNetworkReply*, CUpdatesJob*> m_JobQueue;

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
	CSbieProgressPtr	m_pUpdateProgress;
};