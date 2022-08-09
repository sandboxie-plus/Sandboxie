#pragma once

#include <QObject>
#include "../MiscHelpers/Common/NetworkAccessManager.h"

#include "SbiePlusAPI.h"

class COnlineUpdater : public QObject
{
	Q_OBJECT
public:
	COnlineUpdater(QObject* parent);

	static COnlineUpdater* Instance();

	static void			Process();

	void				InstallUpdate();

	void				UpdateCert();

	void				CheckForUpdates(bool bManual = true);
	void				DownloadUpdates(const QString& DownloadUrl, bool bManual);

private slots:

	void				OnUpdateCheck();
	void				OnUpdateProgress(qint64 bytes, qint64 bytesTotal);
	void				OnUpdateDownload();

	void				OnCertCheck();

protected:

	CNetworkAccessManager*	m_RequestManager;
	CSbieProgressPtr	m_pUpdateProgress;
};