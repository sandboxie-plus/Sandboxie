#pragma once


class CApiLog : public QThread
{
	Q_OBJECT
public:
	CApiLog(QObject* parent = 0);
	virtual ~CApiLog();

signals:
	void				ApiLogEntry(const QString& Message);

protected:
	virtual void		run();

	class CApiLogServer*m_pServer;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class CApiLogServer : public QObject
{
	Q_OBJECT
protected:
	CApiLogServer();

public slots:
	void				OnPipe();
	void				OnData();
	void				OnClose();

protected:
	friend class CApiLog;

	struct SApiLog
	{
		QByteArray Buffer;
	};

	QLocalServer*       m_pServer;
	QMap<QLocalSocket*, SApiLog*> m_pClients;

	CApiLog*			m_pApiLog;
};