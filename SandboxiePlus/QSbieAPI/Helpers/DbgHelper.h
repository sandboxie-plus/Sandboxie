#pragma once

#include "../qsbieapi_global.h"

#include <QQueue>
class CSymbolProviderJob;

class QSBIEAPI_EXPORT CSymbolProvider: public QThread
{
	Q_OBJECT

	CSymbolProvider();
public:
	static CSymbolProvider* Instance();
	~CSymbolProvider();

	QString				Resolve(quint64 pid, quint64 Address);
	static void			ResolveAsync(quint64 pid, quint64 Address, QObject* receiver, const char* member);

	void				SetSymPath(const QString& Path) { QMutexLocker Lock(&m_SymLock); m_SymPath = Path; }

	struct SWorker
	{
		SWorker() : last(-1), handle(0) {}
		quint64 last;
		quint64 handle;
		CSymbolProvider* pProvider;
		QString LastMessage;
	};

signals:
	void				StatusChanged(const QString& Message);

protected:
	//void				timerEvent(QTimerEvent* pEvent);
	//int					m_uTimerID;

	virtual void		run();
	bool				m_bRunning;

	mutable QMutex				m_JobMutex;
	QQueue<CSymbolProviderJob*>	m_JobQueue;

	QMutex				m_SymLock;
	QHash<quint64, SWorker> m_Workers;
	QString				m_SymPath;
};


class CSymbolProviderJob : public QObject
{
	Q_OBJECT

protected:
	friend class CSymbolProvider;

	CSymbolProviderJob(quint64 ProcessId, quint64 Address, QObject *parent = nullptr) : QObject(parent) { m_ProcessId = ProcessId;  m_Address = Address; }
	virtual ~CSymbolProviderJob() {}

	quint64			m_ProcessId;
	quint64			m_Address;

signals:
	void		SymbolResolved(quint64 Address, const QString& Name);
};