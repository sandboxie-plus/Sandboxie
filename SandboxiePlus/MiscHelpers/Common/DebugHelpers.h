#pragma once

//#define _TRACE
#include "../mischelpers_global.h"

MISCHELPERS_EXPORT bool IsDebuggerAttached();

MISCHELPERS_EXPORT void WaitForDebugger();

#if defined(_DEBUG) || defined(_TRACE)


MISCHELPERS_EXPORT void Assert(bool test);

 #define TRACE				CTracer()
 #define ASSERT(x)			Assert(x)
 #define VERIFY(f)          ASSERT(f)
 #define DEBUG_ONLY(f)      (f)

MISCHELPERS_EXPORT extern bool g_assert_active;

class MISCHELPERS_EXPORT CTracer
{
public:
	void operator()(const QString &sLine) const;
	void operator()(const wchar_t *sLine, ...) const;
	void operator()(const char *sLine, ...) const;
};

////////////////////////////////////////////////////////////////////////////////////////////
// Tracers
////////////////////////////////////////////////////////////////////////////////////////////

class MISCHELPERS_EXPORT CMemTracer
{
public:
	//CMemTracer();
	//~CMemTracer();

	void	DumpTrace();

	void	TraceAlloc(string Name);
	void	TraceFree(string Name);
	void	TracePre(string Name);
	

private:
	QMutex				m_Locker;
	map<string,int>		m_MemoryTrace;
	map<string,int>		m_MemoryTrace2;
};

MISCHELPERS_EXPORT extern CMemTracer memTracer;

 #define TRACE_ALLOC(x)		memTracer.TraceAlloc(x);
 #define TRACE_FREE(x)		memTracer.TraceFree(x);
 #define TRACE_PRE_FREE(x)	memTracer.TracePre(x);
 #define TRACE_MEMORY		memTracer.DumpTrace();

class MISCHELPERS_EXPORT CCpuTracer
{
public:
	//CCpuTracer();
	//~CCpuTracer();

	void	DumpTrace();
	void	ResetTrace();

	void	TraceStart(string Name);
	void	TraceStop(string Name);

private:
	struct SCycles
	{
		SCycles()
		{
			Counting = 0;
			Total = 0;
		}
		quint64	Counting;
		quint64	Total;
	};
	QMutex					m_Locker;
	map<string,SCycles>		m_CpuUsageTrace;
};

MISCHELPERS_EXPORT extern CCpuTracer cpuTracer;

 #define TRACE_RESET	cpuTracer.ResetTrace();
 #define TRACE_START(x)	cpuTracer.TraceStart(x);
 #define TRACE_STOP(x)	cpuTracer.TraceStop(x);
 #define TRACE_CPU		cpuTracer.DumpTrace();


class MISCHELPERS_EXPORT CLockTracer
{
public:
	//CLockTracer();
	//~CLockTracer();

	void	DumpTrace();

	void	TraceLock(string Name, int op);

private:
	QMutex					m_Locker;
	struct SLocks
	{
		SLocks()
		{
			LockTime = 0;
			LockCount = 0;
		}
		time_t	LockTime;
		int		LockCount;
	};
	map<string,SLocks>		m_LockTrace;
};

MISCHELPERS_EXPORT extern CLockTracer lockTracer;

 #define TRACE_LOCK(x)	lockTracer.TraceLock(x,1);
 #define TRACE_UNLOCK(x)lockTracer.TraceLock(x,-1);
 #define TRACE_LOCKING	lockTracer.DumpTrace();

class MISCHELPERS_EXPORT CLockerTrace
{
public:
	CLockerTrace(string Name)
	{
		m_Name = Name;
		lockTracer.TraceLock(m_Name,1);
	}
	~CLockerTrace()
	{
		lockTracer.TraceLock(m_Name,-1);
	}

private:
	string	m_Name;
};

 #define TRACE_LOCKER(x)CLockerTrace LockTrace(x)



#else

//#define X_ASSERT(x)  if(!(x)) throw "Assertion failed in: " __FUNCTION__ "; File:" __FILE__ "; Line:" STR(__LINE__) ";";

 #define TRACE
 #define ASSERT(x)
 #define VERIFY(f)          (f)
 #define DEBUG_ONLY(f)

 #define TRACE_ALLOC(x)
 #define TRACE_FREE(x)
 #define TRACE_MEMORY

 #define TRACE_RESET
 #define TRACE_START(x)
 #define TRACE_STOP(x)
 #define TRACE_CPUUSAGE
 #define TRACE_CPU

 #define TRACE_LOCK(x)
 #define TRACE_UNLOCK(x)
 #define TRACE_LOCKING
 #define TRACE_LOCKER(x)
#endif

MISCHELPERS_EXPORT void InitMiniDumpWriter(const wchar_t* Name);
MISCHELPERS_EXPORT quint64 GetCurCycle();