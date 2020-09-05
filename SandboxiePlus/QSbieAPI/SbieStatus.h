#pragma once

#define ERROR_OK (1)
#define OP_ASYNC (2)
#define OP_CONFIRM (3)
#define OP_CANCELED (4)

class CSbieStatus
{
public:
	CSbieStatus()
	{
		m = NULL;
	}
	CSbieStatus(const QString& Error, long Status = 0xC0000001 /*STATUS_UNSUCCESSFUL*/) : CSbieStatus()
	{
		SFlexError* p = new SFlexError();
		p->Error = Error;
		p->Status = Status;
		Attach(p);
	}
	CSbieStatus(long Status) : CSbieStatus(QObject::tr("Error Code: %1").arg(Status), Status)
	{
	}
	CSbieStatus(const CSbieStatus& other) : CSbieStatus()
	{
		if(other.m != NULL)
			Attach((SFlexError*)other.m);
	}
	~CSbieStatus()
	{
		Detach();
	}

	CSbieStatus& operator=(const CSbieStatus& other)
	{
		Attach(&other);
		return *this; 
	}

	__inline bool IsError() const { return m != NULL; }
	__inline long GetStatus() const { return m ? m->Status : 0; }
	__inline QString GetText() const { return m ? m->Error: ""; }

	operator bool() const				{return !IsError();}

protected:
	struct SFlexError
	{
		QString Error;
		long Status;

		mutable atomic<int> aRefCnt;
	} *m;

	void Attach(const CSbieStatus* p)
	{
		Attach(p->m);
	}

	void Attach(SFlexError* p)
	{
		Detach();

		if (p != NULL)
		{
			p->aRefCnt.fetch_add(1);
			m = p;
		}
	}

	void Detach()
	{
		if (m != NULL)
		{
			if (m->aRefCnt.fetch_sub(1) == 1)
				delete m;
			m = NULL;
		}
	}
};

typedef CSbieStatus SB_STATUS;
#define SB_OK SB_STATUS()
#define SB_ERR SB_STATUS

template <class T>
class CSbieResult : public CSbieStatus
{
public:
	CSbieResult(const T& value = T()) : CSbieStatus()
	{
		v = value;
	}
	CSbieResult(long Status, const T& value = T()) : CSbieStatus(Status)
	{
		v = value;
	}
	CSbieResult(const CSbieStatus& other) : CSbieResult()
	{
		Attach(&other);
	}
	CSbieResult(const CSbieResult& other) : CSbieResult(other)
	{
		v = other.v;
	}

	__inline T GetValue() const { return v; }

private:
	T v;
};

#define SB_RESULT(x) CSbieResult<x>
#define SB_RETURN(x,y) CSbieResult<x>(y)


#include "qsbieapi_global.h"


class QSBIEAPI_EXPORT CSbieProgress : public QObject
{
	Q_OBJECT
public:
	CSbieProgress() : m_Status(OP_ASYNC), m_Canceled(false) {}

	void Cancel() { m_Canceled = true; }
	bool IsCancel() { return m_Canceled; }

	void ShowMessage(const QString& text) { emit Message(text);
#ifdef _DEBUG
		QThread::sleep(3);
#endif
	}
	void Finish(SB_STATUS status) { m_Status = m_Canceled ? SB_ERR(OP_CANCELED) : status; emit Finished(); }

	long GetStatus() { return m_Status.GetStatus(); }
	bool IsFinished() { return GetStatus() != OP_ASYNC; }

signals:
	//void Progress(int procent);
	void Message(const QString& text);
	void Finished();

protected:
	volatile bool m_Canceled;
	SB_STATUS m_Status;
};

typedef QSharedPointer<CSbieProgress> CSbieProgressPtr;

#define SB_PROGRESS CSbieResult<CSbieProgressPtr>