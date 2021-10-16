#pragma once

#define ERROR_UNDEFINED (1)
#define ERROR_CONFIRM (2)
#define ERROR_INTERNAL (3)
#define ERROR_PARAMS (4)

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CFlexError
{
public:
	CFlexError() 
	{
		m = NULL;
	}
	CFlexError(const QString& Error, long Status = ERROR_UNDEFINED) : CFlexError() 
	{
		Attach(MkError(Error, Status));
	}
	CFlexError(long Status) : CFlexError(QObject::tr("Error"), Status) 
	{
	}
	CFlexError(const CFlexError& other) : CFlexError() 
	{
		if(other.m != NULL)
			Attach((SFlexError*)other.m);
	}
	~CFlexError() 
	{
		Detach();
	}

	CFlexError& operator=(const CFlexError& Array) 
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

	SFlexError* MkError(const QString& Error, long Status)
	{
		SFlexError* p = new SFlexError();
		p->Error = Error;
		p->Status = Status;
		return p;
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

typedef CFlexError STATUS;
#define OK CFlexError()
#define ERR CFlexError