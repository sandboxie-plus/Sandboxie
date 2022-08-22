#pragma once

#include <QObject>
#include "../MiscHelpers/Common/NetworkAccessManager.h"

#include "SbiePlusAPI.h"

class CGetUpdatesJob : public QObject
{
	Q_OBJECT

protected:
	friend class COnlineUpdater;

	CGetUpdatesJob(const QVariantMap& Params, QObject* parent = nullptr) : QObject(parent) { m_Params = Params; }
	virtual ~CGetUpdatesJob() {}

	QVariantMap	m_Params;

signals:
	void		UpdateData(const QVariantMap& Data, const QVariantMap& Params);
};


class COnlineUpdater : public QObject
{
	Q_OBJECT
public:
	COnlineUpdater(QObject* parent);

	static COnlineUpdater* Instance();

	static void			Process();

	void				GetUpdates(QObject* receiver, const char* member, const QVariantMap& Params = QVariantMap());

	void				DownloadUpdate();
	void				InstallUpdate();

	void				UpdateCert();

	void				CheckPendingUpdate();
	void				CheckForUpdates(bool bManual = true, bool bDownload = false);
	void				DownloadUpdates(const QString& DownloadUrl, bool bManual);

private slots:
	
	void				OnUpdateData(const QVariantMap& Data, const QVariantMap& Params);

	void				OnUpdateCheck();
	void				OnUpdateProgress(qint64 bytes, qint64 bytesTotal);
	void				OnUpdateDownload();

	void				OnCertCheck();

protected:
	bool				IsVersionNewer(const QString& VersionStr);

	CNetworkAccessManager*	m_RequestManager;
	CSbieProgressPtr	m_pUpdateProgress;
	QMap<QNetworkReply*, CGetUpdatesJob*> m_JobQueue;
};