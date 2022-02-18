#include "stdafx.h"
#include "DebugHelpers.h"
#include "Common.h"
#include <QDebug>

#ifdef WIN32
#include <windows.h>
#endif

bool IsDebuggerAttached()
{
	bool isDebuggerPresent = false; // Note: on linux change edit the value in debgger to indicate precense
#ifdef WIN32
	if (IsDebuggerPresent())
		return true;
#endif
	return isDebuggerPresent;
}

void WaitForDebugger()
{
	while (!IsDebuggerAttached())
		QThread::msleep(500);
}


#if defined(_DEBUG) || defined(_TRACE)

#ifdef WIN32
bool g_assert_active = IsDebuggerPresent();
#else
bool g_assert_active = true;
#endif

void Assert(bool test)
{
	if(!test && g_assert_active)
		_CrtDbgBreak();
}

void CTracer::operator()(const QString &sLine) const
{
	qDebug() << sLine;
}

void CTracer::operator()(const wchar_t *sLine, ...) const
{
	const size_t bufferSize = 10241;
	wchar_t bufferline[bufferSize];

	va_list argptr;
	va_start(argptr, sLine);
#ifndef WIN32
	if (vswprintf_l(bufferline, bufferSize, sLine, argptr) == -1)
#else
	if (vswprintf(bufferline, bufferSize, sLine, argptr) == -1)
#endif
		bufferline[bufferSize - 1] = L'\0';
	va_end(argptr);

	QString sBufferLine = QString::fromWCharArray(bufferline);
	qDebug() << sBufferLine;
}

void CTracer::operator()(const char *sLine, ...) const
{
	const size_t bufferSize = 10241;
	char bufferline[bufferSize];

	va_list argptr;
	va_start(argptr, sLine);
	if (vsprintf(bufferline, sLine, argptr) == -1)
		bufferline[bufferSize - 1] = L'\0';
	va_end(argptr);

	QString sBufferLine = bufferline;
	qDebug() << sBufferLine;
}


////////////////////////////////////////////////////////////////////////////////////////////
// Tracers
////////////////////////////////////////////////////////////////////////////////////////////

CMemTracer memTracer;

void CMemTracer::DumpTrace()
{
	QMutexLocker Locker(&m_Locker);
	for(map<string,int>::iterator I = m_MemoryTrace.begin(); I != m_MemoryTrace.end(); I++)
	{

		map<string,int>::iterator J = m_MemoryTrace2.find(I->first);
		if(J != m_MemoryTrace2.end())
			TRACE(L"MEMORY TRACE: Object '%S' has %d (%d) instances.",I->first.c_str(),I->second, J->second);
		else
			TRACE(L"MEMORY TRACE: Object '%S' has %d instances.",I->first.c_str(),I->second);
	}
}

void CMemTracer::TraceAlloc(string Name)
{
	QMutexLocker Locker(&m_Locker);
	m_MemoryTrace[Name] += 1;
}

void CMemTracer::TraceFree(string Name)
{
	QMutexLocker Locker(&m_Locker);
	m_MemoryTrace[Name] -= 1;

	//if(m_MemoryTrace2[Name] > 0)
	if(m_MemoryTrace2.find(Name) != m_MemoryTrace2.end())
		m_MemoryTrace2[Name] -= 1;
}

void CMemTracer::TracePre(string Name)
{
	QMutexLocker Locker(&m_Locker);
	m_MemoryTrace2[Name] += 1;
}

/////////////////////////////////////////////////////////////////////////////////////
CCpuTracer cpuTracer;

void CCpuTracer::DumpTrace()
{
	QMutexLocker Locker(&m_Locker);
	for(map<string,SCycles>::iterator I = m_CpuUsageTrace.begin(); I != m_CpuUsageTrace.end(); I++)
		TRACE(L"CPU TRACE: Prozedure '%S' needed %f seconds.",I->first.c_str(),(double)I->second.Total/1000000.0);
}

void CCpuTracer::ResetTrace()
{
	QMutexLocker Locker(&m_Locker);
	m_CpuUsageTrace.clear();
}

void CCpuTracer::TraceStart(string Name)
{
	QMutexLocker Locker(&m_Locker);
	m_CpuUsageTrace[Name].Counting = GetCurCycle();
}

void CCpuTracer::TraceStop(string Name)
{
	QMutexLocker Locker(&m_Locker);
	m_CpuUsageTrace[Name].Total += GetCurCycle() - m_CpuUsageTrace[Name].Counting;
}


/////////////////////////////////////////////////////////////////////////////////////
CLockTracer lockTracer;

void CLockTracer::DumpTrace()
{
	QMutexLocker Locker(&m_Locker);
	for(map<string,SLocks>::iterator I = m_LockTrace.begin(); I != m_LockTrace.end(); I++)
		TRACE(L"LOCK TRACE: Lock '%S' has %d Locks for %f seconds.",I->first.c_str(),I->second.LockCount, (double)(GetCurCycle()/1000 - I->second.LockTime) / 1000);
}

void CLockTracer::TraceLock(string Name, int op)
{
	QMutexLocker Locker(&m_Locker);
	SLocks &Locks =	m_LockTrace[Name];
	if(Locks.LockCount == 0)
		Locks.LockTime = GetCurCycle()/1000;
	Locks.LockCount += op;
	if(Locks.LockCount == 0)
		Locks.LockTime = 0;
}
#endif


/////////////////////////////////////////////////////////////////////////////////////////////
//

#ifdef WIN32
#include <dbghelp.h>

typedef BOOL (__stdcall *tMDWD)(
  IN HANDLE hProcess,
  IN DWORD ProcessId,
  IN HANDLE hFile,
  IN MINIDUMP_TYPE DumpType,
  IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
  IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
  IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
  );


static tMDWD s_pMDWD;
static HMODULE s_hDbgHelpMod;
static MINIDUMP_TYPE s_dumpTyp = MiniDumpNormal; // MiniDumpWithDataSegs or MiniDumpWithFullMemory
static wchar_t s_szMiniDumpName[64];
static wchar_t s_szMiniDumpPath[MAX_PATH];

static LONG __stdcall MyCrashHandlerExceptionFilter(EXCEPTION_POINTERS* pEx)
{  
#ifdef _M_IX86
  if (pEx->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)  
  {
    // be sure that we have enought space...
    static char MyStack[1024*128];  
    // it assumes that DS and SS are the same!!! (this is the case for Win32)
    // change the stack only if the selectors are the same (this is the case for Win32)
    //__asm push offset MyStack[1024*128];
    //__asm pop esp;
    __asm mov eax,offset MyStack[1024*128];
    __asm mov esp,eax;
  }
#endif
  bool bSuccess = false;

  wchar_t szMiniDumpFileName[128];
  wsprintf(szMiniDumpFileName, L"%s %s.dmp", s_szMiniDumpName, QDateTime::currentDateTime().toString("dd.MM.yyyy hh-mm-ss,zzz").replace(QRegExp("[:*?<>|\"\\/]"), "_").toStdWString().c_str());
  
  /*wchar_t szMiniDumpPath[MAX_PATH] = { 0 };

  HANDLE hFile = CreateFile(szMiniDumpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE)
	GetCurrentDirectory(MAX_PATH, szMiniDumpPath);
  else
  {
	  GetTempPath(MAX_PATH, szMiniDumpPath);

	  wchar_t szMiniDumpFilePath[MAX_PATH] = { 0 };
	  wsprintf(szMiniDumpFilePath, L"%s\\%s.dmp", szMiniDumpPath, szMiniDumpFileName);
	  hFile = CreateFile(szMiniDumpFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  }*/

  wchar_t szMiniDumpPath[MAX_PATH];
  wcscpy(szMiniDumpPath, s_szMiniDumpPath);
  wcscat(szMiniDumpPath, L"\\");
  wcscat(szMiniDumpPath, szMiniDumpFileName);

  HANDLE hFile = CreateFile(szMiniDumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile != INVALID_HANDLE_VALUE)
  {
    MINIDUMP_EXCEPTION_INFORMATION stMDEI;
    stMDEI.ThreadId = GetCurrentThreadId();
    stMDEI.ExceptionPointers = pEx;
    stMDEI.ClientPointers = TRUE;
    // try to create an miniDump:
    if (s_pMDWD(GetCurrentProcess(), GetCurrentProcessId(), hFile, s_dumpTyp, &stMDEI, NULL, NULL))
    {
      bSuccess = true;
    }
    CloseHandle(hFile);
  }

  wchar_t szMiniDumpMessage[256];
  if (!bSuccess)
	wsprintf(szMiniDumpMessage, L"%s crashed!\r\nCrashdump creation failed.", s_szMiniDumpName);
  else
	wsprintf(szMiniDumpMessage, L"%s crashed!\r\nCrashdump saved to \"%s\".\r\nPlease report the crash and attach the file \"%s\".", s_szMiniDumpName, szMiniDumpPath, szMiniDumpFileName);
  MessageBox(NULL, szMiniDumpMessage, s_szMiniDumpName, MB_OK | MB_ICONERROR);

  // or return one of the following:
  // - EXCEPTION_CONTINUE_SEARCH
  // - EXCEPTION_CONTINUE_EXECUTION
  // - EXCEPTION_EXECUTE_HANDLER
  return EXCEPTION_CONTINUE_SEARCH;  // this will trigger the "normal" OS error-dialog
}

void InitMiniDumpWriter(const wchar_t* Name, const wchar_t* Path)
{
  if (s_hDbgHelpMod != NULL)
    return;

  ASSERT(wcslen(Name) < ARRSIZE(s_szMiniDumpName));
  wcscpy(s_szMiniDumpName, Name);
  ASSERT(wcslen(Path) < ARRSIZE(s_szMiniDumpPath));
  wcscpy(s_szMiniDumpPath, Path);

  // Initialize the member, so we do not load the dll after the exception has occured
  // which might be not possible anymore...
  s_hDbgHelpMod = LoadLibrary(L"dbghelp.dll");
  if (s_hDbgHelpMod != NULL)
    s_pMDWD = (tMDWD) GetProcAddress(s_hDbgHelpMod, "MiniDumpWriteDump");

  // Register Unhandled Exception-Filter:
  SetUnhandledExceptionFilter(MyCrashHandlerExceptionFilter);

  // Additional call "PreventSetUnhandledExceptionFilter"...
  // See also: "SetUnhandledExceptionFilter" and VC8 (and later)
  // http://blog.kalmbachnet.de/?postid=75
}


quint64 GetCurCycle()
{
	quint64 freq, now;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	quint64 dwNow = ((now * 1000000) / freq) & 0xffffffff;
	return dwNow; // returns time since system start in us
}


#elif !defined(__APPLE__)
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static wchar_t s_szMiniDumpName[64];

/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext {
    unsigned long     uc_flags;
    struct ucontext   *uc_link;
    stack_t           uc_stack;
    struct sigcontext uc_mcontext;
    sigset_t          uc_sigmask;
} sig_ucontext_t;

void crit_err_hdlr(int sig_num, siginfo_t * info, void * ucontext)
{
    void *             array[50];
    void *             caller_address;
    char **            messages;
    int                size, i;
    sig_ucontext_t *   uc;

    uc = (sig_ucontext_t *)ucontext;

    /* Get the address at the time the signal was raised */
#if defined(__i386__) // gcc specific
    caller_address = (void *) uc->uc_mcontext.eip; // EIP: x86 specific
#elif defined(__x86_64__) // gcc specific
    caller_address = (void *) uc->uc_mcontext.rip; // RIP: x86_64 specific
#else
#error Unsupported architecture. // TO-DO: Add support for other arch.
#endif

    fprintf(stderr, "signal %d (%s), address is %p from %p\n",
    sig_num, strsignal(sig_num), info->si_addr,
    (void *)caller_address);

    size = backtrace(array, 50);

    /* overwrite sigaction with caller's address */
    array[1] = caller_address;

    messages = backtrace_symbols(array, size);

    char szMiniDumpFileName[128];
    sprintf(szMiniDumpFileName, "%S_%s.log", s_szMiniDumpName, QDateTime::currentDateTime().toString("dd.MM.yyyy_hh-mm-ss,zzz").replace(QRegExp("[:*?<>|\"\\/]"), "_").toStdString().c_str());

    FILE* file = fopen(szMiniDumpFileName, "wb");

    /* skip first stack frame (points here) */
    for (i = 1; i < size && messages != NULL; ++i)
    {
        fprintf(stderr, "[bt]: (%d) %s\n", i, messages[i]);
        fprintf(file, "[bt]: (%d) %s\n", i, messages[i]);
    }

    fclose(file);

    free(messages);

    exit(EXIT_FAILURE);
}

void InstallSigAction(int sig)
{
    struct sigaction sigact;
    sigact.sa_sigaction = crit_err_hdlr;
    sigact.sa_flags = SA_RESTART | SA_SIGINFO;

    if (sigaction(sig, &sigact, (struct sigaction *)NULL) != 0)
    {
        fprintf(stderr, "error setting signal handler for %d (%s)\n", sig, strsignal(sig));
        exit(EXIT_FAILURE);
    }
}

void InitMiniDumpWriter(const wchar_t* Name, const wchar_t* Path)
{
    ASSERT(wcslen(Name) < ARRSIZE(s_szMiniDumpName));
    wcscpy(s_szMiniDumpName, Name);

    InstallSigAction(SIGABRT);
    InstallSigAction(SIGSEGV);
}

quint64 GetCurCycle()
{
    return GetCurTick()*1000; // ToDo
}

#else
void InitMiniDumpWriter(const wchar_t* Name, const wchar_t* Path)
{

}
#endif
