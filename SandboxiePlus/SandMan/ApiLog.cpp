#include "stdafx.h"
#include "ApiLog.h"

CApiLog::CApiLog(QObject* parent) : QThread(parent)
{
	m_pServer = NULL;
	
	start();
}

CApiLog::~CApiLog()
{
	quit();
	if (!wait(10 * 1000))
		terminate();
}

void CApiLog::run()
{
	qDebug() << "LogAPI server started";

	m_pServer = new CApiLogServer();

	QThread::run(); // run messge loop

	m_pServer->deleteLater();

	qDebug() << "LogAPI server stopped";
}

/////////////////////////////////////////////////////////////////////////////////////
//

CApiLogServer::CApiLogServer()
{
	m_pServer = new QLocalServer(this);
	m_pServer->setSocketOptions(QLocalServer::WorldAccessOption);
	if (!m_pServer->listen("LogAPI")) {
		qDebug() << "Not able to open Server Pipe";
		return;
	}

	connect(m_pServer, SIGNAL(newConnection()), this, SLOT(OnPipe()));

	m_pApiLog = qobject_cast<CApiLog*>(thread());
}

void CApiLogServer::OnPipe()
{
	QLocalSocket *pSocket = m_pServer->nextPendingConnection();
	connect(pSocket, SIGNAL(readyRead()), this, SLOT(OnData()));
	connect(pSocket, SIGNAL(disconnected()), this, SLOT(OnClose()));

	m_pClients.insert(pSocket, new SApiLog());
}

void CApiLogServer::OnData()
{
	QLocalSocket* pSocket = qobject_cast<QLocalSocket*>(sender());
	SApiLog* ApiLog = m_pClients.value(pSocket);
	if (!ApiLog)
		return;

	ApiLog->Buffer.append(pSocket->readAll());

	for (;;)
	{
		int endPos = ApiLog->Buffer.indexOf('\0');
		if (endPos == -1)
			break;

		emit m_pApiLog->ApiLogEntry(QString(ApiLog->Buffer.data()));
		ApiLog->Buffer.remove(0, endPos + 1);
	}
}

void CApiLogServer::OnClose()
{
	QLocalSocket* pSocket = qobject_cast<QLocalSocket*>(sender());
	delete m_pClients.take(pSocket);
	pSocket->deleteLater();
}