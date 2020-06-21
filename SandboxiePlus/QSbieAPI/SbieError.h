#pragma once

#define ERROR_OK (1)
#define OP_ASYNC (2)

class CSbieError
{
public:
	CSbieError()
	{
		m = NULL;
	}
	CSbieError(const QString& Error, long Status = 0xC0000001 /*STATUS_UNSUCCESSFUL*/) : CSbieError()
	{
		SFlexError* p = new SFlexError();
		p->Error = Error;
		p->Status = Status;
		Attach(p);
	}
	CSbieError(long Status) : CSbieError(QObject::tr("Error Code: %1").arg(Status), Status)
	{
	}
	CSbieError(const CSbieError& other) : CSbieError()
	{
		if(other.m != NULL)
			Attach((SFlexError*)other.m);
	}
	/*virtual*/ ~CSbieError()
	{
		Detach();
	}

	CSbieError& operator=(const CSbieError& Array)
	{
		Attach(Array.m); 
		return *this; 
	}

	__inline bool IsError() const { return m != NULL; }
	__inline long GetStatus() const { return m ? m->Status : 0; }
	__inline QString GetText() const { return m ? m->Error: ""; }

	operator bool() const				{return !IsError();}

private:
	struct SFlexError
	{
		QString Error;
		long Status;

		mutable atomic<int> aRefCnt;
	} *m;

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

typedef CSbieError SB_STATUS;
#define SB_OK CSbieError()
#define SB_ERR CSbieError


/*
template <class T>
class CSbieResult : public CSbieError
{
public:
	CSbieResult(const T& value = T()) : CSbieError()
	{
		v = value;
	}
	CSbieResult(const CSbieError& other) : CSbieResult()
	{
		if (other.m != NULL)
			Attach((SFlexError*)other.m);
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
*/