#include "stdafx.h"

#include <QAbstractEventDispatcher>
#include <QSettings>

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "..\..\..\Sandboxie\common\win32_ntddk.h"

#include "DbgHelper.h"


#include <dbghelp.h>

typedef BOOL(WINAPI* P_SymFromAddr)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol);
typedef BOOL(WINAPI* P_SymGetModuleInfoW64)(HANDLE hProcess, DWORD64 qwAddr, PIMAGEHLP_MODULEW64 ModuleInfo);
typedef DWORD(WINAPI* P_SymSetOptions)(DWORD SymOptions);
typedef DWORD(WINAPI* P_SymGetOptions)();
typedef BOOL(WINAPI* P_SymInitialize)(HANDLE hProcess, PCSTR UserSearchPath, BOOL fInvadeProcess);
typedef BOOL(WINAPI* P_SymCleanup)(HANDLE ProcessHandle);
typedef BOOL(WINAPI* P_SymSetSearchPathW)(HANDLE hProcess, PCWSTR SearchPath);
typedef BOOL(WINAPI* P_SymRegisterCallbackW64)(HANDLE hProcess, PSYMBOL_REGISTERED_CALLBACK64 CallbackFunction, ULONG64 UserContext);


static P_SymFromAddr                __sys_SymFromAddr               = NULL;
static P_SymGetModuleInfoW64        __sys_SymGetModuleInfoW64       = NULL;
static P_SymSetOptions              __sys_SymSetOptions             = NULL;
static P_SymGetOptions              __sys_SymGetOptions             = NULL;
static P_SymSetSearchPathW          __sys_SymSetSearchPathW         = NULL;
static P_SymInitialize              __sys_SymInitialize             = NULL;
static P_SymCleanup                 __sys_SymCleanup                = NULL;
static P_SymRegisterCallbackW64     __sys_SymRegisterCallbackW64    = NULL;

CSymbolProvider* g_SymbolProvider = NULL;

CSymbolProvider::CSymbolProvider()
{
    //m_uTimerID = startTimer(1000);

	// start thread
	m_bRunning = true;
	start();
}

CSymbolProvider::~CSymbolProvider()
{
    //killTimer(m_uTimerID);

	m_bRunning = false;
	//quit();
	if (!wait(10 * 1000))
		terminate();

	// cleanup unfinished tasks
	while (!m_JobQueue.isEmpty()) {
		m_JobQueue.takeFirst()->deleteLater();
	}

    g_SymbolProvider = NULL;
}

//void CSymbolProvider::timerEvent(QTimerEvent* pEvent)
//{
//    if (pEvent->timerId() != m_uTimerID)
//        return;
//}

void CSymbolProvider::run()
{
	quint64 LastCleanUp = 0;

	while (m_bRunning)
	{
		quint64 OldTime = GetTickCount64() - 3000; // cleanup everything older than 3 sec
		if (LastCleanUp < OldTime) 
        {
			QMutexLocker Lock(&m_SymLock);
            for(auto I = m_Workers.begin(); I != m_Workers.end(); )
            {
                if (GetTickCount64() - I->last > 2000) {
                    __sys_SymCleanup((HANDLE)I->handle);
                    if ((I->handle & 1) == 0)
                        CloseHandle((HANDLE)I->handle);
                    I = m_Workers.erase(I);
                }
                else
                    I++;
            }

			LastCleanUp = GetTickCount64();
		}

		QMutexLocker Locker(&m_JobMutex);
		if (m_JobQueue.isEmpty()) {
			Locker.unlock();
			QThread::msleep(250);
			continue;
		}
		CSymbolProviderJob* pJob = m_JobQueue.takeFirst();
		Locker.unlock();

        QString Name = Resolve(pJob->m_ProcessId, pJob->m_Address);
        emit pJob->SymbolResolved(pJob->m_Address, Name);

		pJob->deleteLater();
	}
}

extern "C" BOOL CALLBACK SymbolCallbackFunction(HANDLE ProcessHandle, ULONG ActionCode, ULONG64 CallbackData, ULONG64 UserContext);

QString CSymbolProvider::Resolve(quint64 pid, quint64 Address)
{
    QMutexLocker Lock(&m_SymLock);

    SWorker& Worker = m_Workers[pid];
    if (Worker.handle == 0) 
    {
        Worker.pProvider = this;

        static ACCESS_MASK accesses[] =
        {
            //STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0xfff, // pre-Vista full access
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_DUP_HANDLE,
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            MAXIMUM_ALLOWED
        };
        for (ULONG i = 0; i < sizeof(accesses) / sizeof(ACCESS_MASK); i++) {
            Worker.handle = (quint64)OpenProcess(accesses[i], FALSE, pid);
            if (Worker.handle != (quint64)INVALID_HANDLE_VALUE)
                break;
        }

        static QAtomicInt FakeHandle = 1; // real handles are divisible by 4
        if (Worker.handle == (quint64)INVALID_HANDLE_VALUE)
            Worker.handle = FakeHandle.fetchAndAddAcquire(4);

        __sys_SymInitialize((HANDLE)Worker.handle, NULL, TRUE);
        __sys_SymRegisterCallbackW64((HANDLE)Worker.handle, SymbolCallbackFunction, (ULONG64)&Worker);
        __sys_SymSetSearchPathW((HANDLE)Worker.handle, m_SymPath.toStdWString().c_str());
    }
    Worker.last = GetTickCount64();

    QString Symbol;

    DWORD64 displacement;
    UCHAR buffer[sizeof(SYMBOL_INFO) + sizeof(TCHAR) + (MAX_SYM_NAME - 1)] = { 0 };
    SYMBOL_INFO* symbolInfo = (SYMBOL_INFO*)buffer;
    symbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbolInfo->MaxNameLen = MAX_SYM_NAME;
    if (__sys_SymFromAddr((HANDLE)Worker.handle, Address, &displacement, symbolInfo))
    {
        symbolInfo->Name[symbolInfo->NameLen] = 0;

        Symbol = QString(symbolInfo->Name);
        if (displacement != 0)
            Symbol.append(QString("+0x%1").arg(displacement, 0, 16));

        IMAGEHLP_MODULEW64 ModuleInfo;
        ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);
        if (__sys_SymGetModuleInfoW64((HANDLE)Worker.handle, symbolInfo->ModBase ? symbolInfo->ModBase : symbolInfo->Address, &ModuleInfo))
            Symbol.prepend(QString::fromWCharArray(ModuleInfo.ModuleName) + "!");
    }
    else
    {
        // Then this happens, probably symsrv.dll is missing

        IMAGEHLP_MODULEW64 ModuleInfo;
        ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);
        if (__sys_SymGetModuleInfoW64((HANDLE)Worker.handle, Address, &ModuleInfo))
            Symbol.prepend(QString::fromWCharArray(ModuleInfo.ModuleName) + "+" + QString("0x%1").arg(Address - ModuleInfo.BaseOfImage, 0, 16));
    }

    return Symbol;
}

void CSymbolProvider::ResolveAsync(quint64 pid, quint64 Address, QObject* receiver, const char* member)
{
    CSymbolProvider* This = CSymbolProvider::Instance();
    if (!This)
        return;

    if (!QAbstractEventDispatcher::instance(QThread::currentThread())) {
        qWarning("CSymbolProvider::ResolveAsync() called with no event dispatcher");
        return;
    }

	CSymbolProviderJob* pJob = new CSymbolProviderJob(pid, Address); 
	pJob->moveToThread(This);
	QObject::connect(pJob, SIGNAL(SymbolResolved(quint64, const QString&)), receiver, member, Qt::QueuedConnection);

	QMutexLocker Locker(&This->m_JobMutex);
	This->m_JobQueue.append(pJob);
}

extern "C" BOOL CALLBACK SymbolCallbackFunction(HANDLE ProcessHandle, ULONG ActionCode, ULONG64 CallbackData, ULONG64 UserContext)
{
    CSymbolProvider::SWorker* pWorker = (CSymbolProvider::SWorker*)UserContext;

    switch (ActionCode)
    {
    case CBA_DEFERRED_SYMBOL_LOAD_START:
        {
            /*PIMAGEHLP_DEFERRED_SYMBOL_LOADW64 callbackData = (PIMAGEHLP_DEFERRED_SYMBOL_LOADW64)CallbackData;
            
            IMAGEHLP_MODULEW64 ModuleInfo;
            ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);
            if (__sys_SymGetModuleInfoW64(ProcessHandle, callbackData->BaseOfImage, &ModuleInfo))
            {
                HANDLE fileHandle;
                UNICODE_STRING fileName;
                OBJECT_ATTRIBUTES objectAttributes;
                IO_STATUS_BLOCK ioStatusBlock;

                std::wstring FileName = L"\\??\\" + std::wstring(ModuleInfo.ImageName);
                fileName.Buffer = (WCHAR*)FileName.c_str();
                fileName.Length = fileName.MaximumLength = FileName.size() * sizeof(WCHAR);

                InitializeObjectAttributes(
                    &objectAttributes,
                    &fileName,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL
                    );

                if (NT_SUCCESS(NtCreateFile(
                    &fileHandle,
                    FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                    &objectAttributes,
                    &ioStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                    FILE_OPEN,
                    FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, 
                    NULL, 
                    0
                    )))
                {
                    callbackData->FileName[0] = UNICODE_NULL;
                    callbackData->hFile = fileHandle;

                    return TRUE;
                }
            }*/
        }
        break;
    case CBA_DEFERRED_SYMBOL_LOAD_COMPLETE:
        {
            /*PIMAGEHLP_DEFERRED_SYMBOL_LOADW64 callbackData = (PIMAGEHLP_DEFERRED_SYMBOL_LOADW64)CallbackData;

            if (callbackData->hFile)
            {
                NtClose(callbackData->hFile);
                callbackData->hFile = NULL;
            }
            return TRUE;*/
        }
        break;
    case CBA_READ_MEMORY:
        {
            PIMAGEHLP_CBA_READ_MEMORY callbackData = (PIMAGEHLP_CBA_READ_MEMORY)CallbackData;

            /*if ((pWorker->handle & 1) == 0)
            {
                if (NT_SUCCESS(NtReadVirtualMemory(
                    ProcessHandle,
                    (PVOID)callbackData->addr,
                    callbackData->buf,
                    (SIZE_T)callbackData->bytes,
                    (PSIZE_T)callbackData->bytesread
                    )))
                {
                    return TRUE;
                }
            }*/
        }
        break;
    case CBA_DEFERRED_SYMBOL_LOAD_CANCEL:
        {
            //if (pWorker->last == 0) // terminating
            //    return TRUE;
        }
        break;
    case CBA_XML_LOG:
        {
            PWSTR callbackData = (PWSTR)CallbackData;
            QString data = QString::fromWCharArray(callbackData);
            //qDebug() << data;

            QVariantMap result;
            QXmlStreamReader xmlReader(data);
            while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                QXmlStreamReader::TokenType token = xmlReader.readNext();
                if (token == QXmlStreamReader::StartElement) {
                    QString elementName = xmlReader.name().toString();
                    QVariantMap attributes;
                    QXmlStreamAttributes xmlAttributes = xmlReader.attributes();
                    for (const auto& attribute : xmlAttributes) {
                        attributes.insert(attribute.name().toString(), attribute.value().toString());
                    }

                    result.insert(elementName, attributes);
                }
            }

            QString Message;
            if (!result.value("Activity").toMap()["details"].toString().isEmpty())
                Message = result.value("Activity").toMap()["details"].toString();
            //if (!result.value("Log").toMap()["message"].toString().isEmpty())
            //    Message = result.value("Log").toMap()["message"].toString();
            if (!result.value("Progress").toMap()["percent"].toString().isEmpty())
                Message = pWorker->LastMessage + QString(" (%1%)").arg(result.value("Progress").toMap()["percent"].toString());
            else if (!Message.isEmpty())
                pWorker->LastMessage = Message;
            if(!Message.isEmpty() || result.isEmpty())
                emit pWorker->pProvider->StatusChanged(Message);

            break;
        }
        break;
    }

    return FALSE;
}

bool MyBeginInitOnce(QAtomicInt& InitOnce)
{
    if (InitOnce == 1)
        return false;
    else
    {
        if (InitOnce.testAndSetAcquire(0, 2))
            return true;

        while (InitOnce == 2)
            Sleep(10);

        return false;
    }
}

CSymbolProvider* CSymbolProvider::Instance()
{
    static QAtomicInt InitOnce = 0;

    if (MyBeginInitOnce(InitOnce))
    {
/*#ifdef _WIN64
        QSettings settings("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows Kits\\Installed Roots", QSettings::NativeFormat);
#else
        QSettings settings("HKEY_LOCAL_MACHINE\\Software\\Wow6432Node\\Microsoft\\Windows Kits\\Installed Roots", QSettings::NativeFormat);
#endif
        QString KitsRoot = settings.value("KitsRoot10").toString(); // Windows 10 SDK
        if(KitsRoot.isEmpty())
            KitsRoot = settings.value("KitsRoot81").toString(); // Windows 8.1 SDK
        if(KitsRoot.isEmpty())
            KitsRoot = settings.value("KitsRoot").toString(); // Windows 8 SDK
#if defined(_M_AMD64)
        KitsRoot.append("\\Debuggers\\x64\\");
#elif defined(_M_ARM64)
        KitsRoot.append("\\Debuggers\\arm64\\");
#else
        KitsRoot.append("\\Debuggers\\x86\\");
#endif

        HMODULE DbgCoreMod = LoadLibrary((KitsRoot + "dbgcore.dll").toStdWString().c_str());
        HMODULE DbgHelpMod = LoadLibrary((KitsRoot + "dbghelp.dll").toStdWString().c_str());
        HMODULE SymSrvMod  = LoadLibrary((KitsRoot + "symsrv.dll").toStdWString().c_str());*/

        HMODULE DbgHelpMod = LoadLibrary(L"dbghelp.dll");

        __sys_SymFromAddr = (P_SymFromAddr)GetProcAddress(DbgHelpMod, "SymFromAddr");
        __sys_SymGetModuleInfoW64 = (P_SymGetModuleInfoW64)GetProcAddress(DbgHelpMod, "SymGetModuleInfoW64");
        __sys_SymSetOptions = (P_SymSetOptions)GetProcAddress(DbgHelpMod, "SymSetOptions");
        __sys_SymGetOptions = (P_SymGetOptions)GetProcAddress(DbgHelpMod, "SymGetOptions");
        __sys_SymSetSearchPathW = (P_SymSetSearchPathW)GetProcAddress(DbgHelpMod, "SymSetSearchPathW");
        __sys_SymInitialize = (P_SymInitialize)GetProcAddress(DbgHelpMod, "SymInitialize");
        __sys_SymCleanup = (P_SymCleanup)GetProcAddress(DbgHelpMod, "SymCleanup");
        __sys_SymRegisterCallbackW64 = (P_SymRegisterCallbackW64)GetProcAddress(DbgHelpMod, "SymRegisterCallbackW64");

        if (!__sys_SymSetOptions) {
            if (DbgHelpMod)
                FreeLibrary(DbgHelpMod);
            InitOnce = 0;
            return NULL;
        }

        __sys_SymSetOptions(
            __sys_SymGetOptions() | SYMOPT_UNDNAME |
                SYMOPT_AUTO_PUBLICS | SYMOPT_CASE_INSENSITIVE | SYMOPT_DEFERRED_LOADS |
                SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_INCLUDE_32BIT_MODULES |
                SYMOPT_LOAD_LINES | SYMOPT_OMAP_FIND_NEAREST | SYMOPT_UNDNAME // | SYMOPT_DEBUG
        );

        g_SymbolProvider = new CSymbolProvider();

        InitOnce = 1;
    }
    
    return g_SymbolProvider;
}