#pragma once

class CApiLogEntry : public QSharedData
{
public:
	CApiLogEntry(quint64 ProcessId, const QString& Message);

	quint64				GetProcessId() const { return m_ProcessId; }
	QDateTime			GetTimeStamp() const { return m_TimeStamp; }
	QString				GetMessage() const { return m_Message; }

	quint64				GetUID() const { return m_uid; }

protected:
	quint64 m_ProcessId;
	QDateTime m_TimeStamp;
	QString m_Message;

	quint64 m_uid;
};

typedef QSharedDataPointer<CApiLogEntry> CApiLogEntryPtr;

class CApiLog : public QThread
{
	Q_OBJECT
public:
	CApiLog(QObject* parent = 0);
	virtual ~CApiLog();

	virtual QList<CApiLogEntryPtr> GetApiLog() const { QReadLocker Lock(&m_ApiLogMutex); return m_ApiLogList; }
	virtual void		ClearApiLog() { QWriteLocker Lock(&m_ApiLogMutex); m_ApiLogList.clear(); }

protected:
	friend class CApiLogServer;

	virtual void		run();

	mutable QReadWriteLock	m_ApiLogMutex;
	QList<CApiLogEntryPtr>	m_ApiLogList;

	CApiLogServer*		m_pServer;
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
		SApiLog(quint64 pid) { ProcessId = pid; }

		QByteArray Buffer;
		quint64 ProcessId;
	};

	QLocalServer*       m_pServer;
	QMap<QLocalSocket*, SApiLog*> m_pClients;

	CApiLog*			m_pApiLog;
};