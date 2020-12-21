#include "stdafx.h"
#include "NetworkAccessManager.h"
#include "Common.h"

CNetworkAccessManager::CNetworkAccessManager(int TimeOut, QObject* parent)
:QNetworkAccessManager(parent)
{
	m_TimeOut = TimeOut;
	connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedRequest(QNetworkReply*))); 
#ifndef QT_NO_OPENSSL
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(sslErrors(QNetworkReply*, const QList<QSslError>&)));
#endif

	m_uTimerID = startTimer(1000);
}

CNetworkAccessManager::~CNetworkAccessManager()
{
	killTimer(m_uTimerID);
}

void CNetworkAccessManager::timerEvent(QTimerEvent* pEvent)
{
	if (pEvent->timerId() != m_uTimerID)
		return;

	foreach(QNetworkReply *pReply, m_Requests.keys())
	{
		if(m_Requests[pReply] < GetCurTick())
			pReply->abort();
	}
}

void CNetworkAccessManager::SetTimeOut(QNetworkReply *pReply)
{
	m_Requests[pReply] = GetCurTick() + m_TimeOut;
}

void CNetworkAccessManager::StopTimeOut(QNetworkReply *pReply)
{
	m_Requests.remove(pReply);
}

void CNetworkAccessManager::Abort(QNetworkReply *pReply)
{
	pReply->abort();
	StopTimeOut(pReply);
	pReply->deleteLater();
}

void CNetworkAccessManager::AbortAll()
{
	foreach(QNetworkReply *pReply, m_Requests.keys())
		Abort(pReply);
}

QNetworkReply* CNetworkAccessManager::createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData )
{
	QNetworkReply* pReply = QNetworkAccessManager::createRequest(op, req, outgoingData);
	connect(pReply, SIGNAL(downloadProgress (qint64, qint64)), this, SLOT(OnData(qint64, qint64)));
	connect(pReply, SIGNAL(uploadProgress (qint64, qint64)), this, SLOT(OnData(qint64, qint64)));
	SetTimeOut(pReply);
	return pReply;
}

void CNetworkAccessManager::finishedRequest(QNetworkReply *pReply)
{
	StopTimeOut(pReply);
}

void CNetworkAccessManager::OnData(qint64 bytesSent, qint64 bytesTotal)
{
	// Reset TimeOut, as long as data are being transferred its not a timeout
	SetTimeOut((QNetworkReply*)sender());
}

#ifndef QT_NO_OPENSSL
void CNetworkAccessManager::sslErrors(QNetworkReply *pReply, const QList<QSslError> &error)
{
	//pReply->ignoreSslErrors();
}
#endif
