#pragma once

#include "../mischelpers_global.h"

#include <QNetworkAccessManager>

class MISCHELPERS_EXPORT CNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
	CNetworkAccessManager(int TimeOut, QObject* parent = NULL);
	~CNetworkAccessManager();

	void				Abort(QNetworkReply* pReply);
	void				AbortAll();

private slots:
	void				finishedRequest(QNetworkReply *pReply);
	void				OnData(qint64 bytesSent, qint64 bytesTotal);
#ifndef QT_NO_OPENSSL
	void				sslErrors(QNetworkReply *pReply, const QList<QSslError> &error);
#endif

protected:
	void				timerEvent(QTimerEvent* pEvent);
	int					m_uTimerID;

	QNetworkReply*		createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0 );

	void				SetTimeOut(QNetworkReply *pReply);
	void				StopTimeOut(QNetworkReply *pReply);

	int					m_TimeOut;
	QMap<QNetworkReply*, quint64>	m_Requests;
};
