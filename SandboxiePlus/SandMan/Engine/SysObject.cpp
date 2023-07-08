#include "stdafx.h"
#include "SysObject.h"
#include "../SandMan.h"
#include "../QSbieAPI/Sandboxie/SbieTemplates.h"

#include "..\..\MiscHelpers\Common\Common.h"
#include "..\..\MiscHelpers\Common\OtherFunctions.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;
#include <Windows.h>
//#include <Winternl.h>

#include "..\..\Sandboxie\common\win32_ntddk.h"

#define MAX_KEY_NAME 255
#define MAX_VALUE_NAME 16383
#define MAX_VALUE_DATA 1024000


JSysObject::JSysObject(CBoxEngine* pEngine) 
 : m_pEngine(pEngine) 
{
}
    
void JSysObject::log(const QString& line) 
{
    m_pEngine->AppendLog(line);
}

void JSysObject::sleep(qint64 ms)
{
    for (qint64 i = 0; i < ms && m_pEngine->TestRunning(); i += 10)
        QThread::msleep(10);
}


// FS
HANDLE openFile(const QString& Path, bool bDir, bool bWrite = false)
{
    NTSTATUS status = STATUS_SUCCESS;
	IO_STATUS_BLOCK Iosb;

    std::wstring path = Path.toStdWString();
    if (path.substr(0, 1) != L"\\") // not nt path
        path = L"\\??\\" + path; // dos path
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING uni;

    RtlInitUnicodeString(&uni, path.c_str());
    InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    HANDLE handle = NULL;
    if (bWrite)
        NtCreateFile(&handle, GENERIC_ALL | SYNCHRONIZE, &objattrs, &Iosb, NULL, 0, FILE_SHARE_READ, FILE_OPEN_IF, (bDir ? FILE_DIRECTORY_FILE : 0) | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    else
        NtOpenFile(&handle, GENERIC_READ | SYNCHRONIZE, &objattrs, &Iosb, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, (bDir ? FILE_DIRECTORY_FILE : 0) | FILE_SYNCHRONOUS_IO_NONALERT);
	
    return handle;
}

QJSValue JSysObject::listDir(const QString& Path, const QStringList& filter, bool bSubDirs)
{
    QVariantList entries;

    HANDLE handle = openFile(Path, false);
    if (handle && handle != INVALID_HANDLE_VALUE) {

        PFILE_BOTH_DIRECTORY_INFORMATION Info = (PFILE_BOTH_DIRECTORY_INFORMATION)malloc(PAGE_SIZE);

	    for (NTSTATUS status = STATUS_SUCCESS ; status == STATUS_SUCCESS; )
	    {
		    HANDLE Event;
		    NtCreateEvent(&Event, GENERIC_ALL, 0, NotificationEvent, FALSE);
	        IO_STATUS_BLOCK Iosb;
		    status = NtQueryDirectoryFile(handle, Event, 0, 0, &Iosb, Info, PAGE_SIZE, FileBothDirectoryInformation, TRUE, NULL, FALSE);
		    if (status == STATUS_PENDING){
			    NtWaitForSingleObject(Event, TRUE, 0);
			    status = Iosb.Status;
		    }
		    NtClose(Event);

		    if (!NT_SUCCESS(status)) {
			    if(status == STATUS_NO_MORE_FILES)
				    status = STATUS_SUCCESS;
			    break;
		    }

            QString FileName = QString::fromWCharArray(Info->FileName, Info->FileNameLength / sizeof(wchar_t));
            if (FileName == "." || FileName == "..")
                continue;

            QVariantMap entry;
            entry["isDir"] = (Info->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
            entry["name"] = FileName;
            entry["path"] = Path + (Path.right(1) != "\\" ? "\\" : "") + FileName;
            entries.append(entry);
	    }

		free(Info);

        CloseHandle(handle);
    }

    return m_pEngine->m_pEngine->toScriptValue(entries);
}

QJSValue JSysObject::mkDir(const QString& Path)
{
    HANDLE handle = openFile(Path, true, true);
    if (handle && handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        return true;
    }
    return false;
}

QJSValue JSysObject::remove(const QString& Path)
{
    std::wstring path = Path.toStdWString();
    if (path.substr(0, 1) != L"\\") // not nt path
        path = L"\\??\\" + path; // dos path
    OBJECT_ATTRIBUTES objattrs;
    UNICODE_STRING uni;

    RtlInitUnicodeString(&uni, path.c_str());
    InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

    return NT_SUCCESS(NtDeleteFile(&objattrs));
}

QJSValue JSysObject::exists(const QString& Path)
{
    HANDLE handle = openFile(Path, false);
    if (handle && handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        return true;
    }
    return false;
}

QJSValue JSysObject::readFile(const QString& Path, quint64 pos, quint64 length)
{
    HANDLE handle = openFile(Path, false);
    if (handle && handle != INVALID_HANDLE_VALUE) {
        QByteArray Data;
        
        IO_STATUS_BLOCK Iosb;
        if (length == -1) {
            FILE_STANDARD_INFORMATION standardInfo;
            if (NT_SUCCESS(NtQueryInformationFile(handle, &Iosb, &standardInfo, sizeof(standardInfo), FileStandardInformation))) {
                length = standardInfo.EndOfFile.QuadPart;
            }
        }

        Data.resize(length);
        LARGE_INTEGER offset;
        offset.QuadPart = pos;
        NtReadFile(handle, NULL, NULL, NULL, &Iosb, Data.data(), length, &offset, NULL);

        CloseHandle(handle);
        return m_pEngine->m_pEngine->toScriptValue(Data);

    }
    return QJSValue::NullValue;
}

QJSValue JSysObject::writeFile(const QString& Path, const QByteArray& Data, quint64 pos)
{
    HANDLE handle = openFile(Path, false, true);
    if (handle && handle != INVALID_HANDLE_VALUE) {

        IO_STATUS_BLOCK Iosb;
        if (pos == -1) { // trim
            FILE_END_OF_FILE_INFORMATION endOfFileInfo;
            endOfFileInfo.EndOfFile.QuadPart = 0;
            NtSetInformationFile(handle, &Iosb, &endOfFileInfo, sizeof(endOfFileInfo), FileEndOfFileInformation);
            pos = 0;
        }

        LARGE_INTEGER offset;
        offset.QuadPart = pos;
        NtWriteFile(handle, NULL, NULL, NULL, &Iosb, (PVOID)Data.data(), Data.size(), &offset, NULL);
        
        CloseHandle(handle);
        return true;
    }
    return false;
}

QJSValue JSysObject::getFileInfo(const QString& Path)
{
    QVariantMap data;
    HANDLE handle = openFile(Path, false);
    if (handle && handle != INVALID_HANDLE_VALUE) {
        IO_STATUS_BLOCK Iosb;
        FILE_STANDARD_INFORMATION standardInfo;
        if (NT_SUCCESS(NtQueryInformationFile(handle, &Iosb, &standardInfo, sizeof(standardInfo), FileStandardInformation))) {
            data["path"] = Path;
            data["isDir"] = standardInfo.Directory;
            data["size"] = standardInfo.EndOfFile.QuadPart;
        }
        CloseHandle(handle);
    }
    return m_pEngine->m_pEngine->toScriptValue(data);
}


// REG
HANDLE openRegKey(const QString& Key, bool bWrite = false)
{
    QString Path = Key;
    if (Path.left(1) == "\\")
        Path.remove(0, 1);
    StrPair RootPath = Split2(Path, "\\");

    HANDLE handle = NULL;
    if (RootPath.first.left(5).compare("HKEY_", Qt::CaseInsensitive) == 0)
    {
        HKEY hRoot = NULL;
        if (RootPath.first == "HKEY_CLASSES_ROOT") hRoot = HKEY_CLASSES_ROOT;
        else if (RootPath.first == "HKEY_CURRENT_USER") hRoot = HKEY_CURRENT_USER;
        else if (RootPath.first == "HKEY_LOCAL_MACHINE") hRoot = HKEY_LOCAL_MACHINE;
        else if (RootPath.first == "HKEY_USERS") hRoot = HKEY_USERS;
        else if (RootPath.first == "HKEY_PERFORMANCE_DATA") hRoot = HKEY_PERFORMANCE_DATA;
        else if (RootPath.first == "HKEY_CURRENT_CONFIG") hRoot = HKEY_CURRENT_CONFIG;

        if (bWrite)
            RegCreateKeyEx(hRoot, RootPath.second.toStdWString().c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, (PHKEY)&handle, NULL);
        else
            RegOpenKeyEx(hRoot, RootPath.second.toStdWString().c_str(), REG_OPTION_NON_VOLATILE, KEY_READ, (PHKEY)&handle);
    }
    else
    {
        std::wstring key = Key.toStdWString();
        OBJECT_ATTRIBUTES objattrs;
        UNICODE_STRING uni;

        RtlInitUnicodeString(&uni, key.c_str());
        InitializeObjectAttributes(&objattrs, &uni, OBJ_CASE_INSENSITIVE, NULL, NULL);

        if (bWrite) {
            ULONG disposition;
            NtCreateKey(&handle, KEY_ALL_ACCESS, &objattrs, 0, NULL, 0, &disposition);
        } else
            NtOpenKey(&handle, KEY_READ, &objattrs);
    }
    return handle;
}

QJSValue JSysObject::listRegKey(const QString& Key)
{
    QVariantList entries;
    HANDLE handle = openRegKey(Key);
    if (handle && handle != INVALID_HANDLE_VALUE) 
    {
        WCHAR szSubKeyName[MAX_KEY_NAME];
        DWORD dwSubKeyNameSize;
        FILETIME ftLastWriteTime;

        for (DWORD dwIndex = 0; ; dwIndex++)
        {
            dwSubKeyNameSize = ARRAYSIZE(szSubKeyName);
            if (RegEnumKeyEx((HKEY)handle, dwIndex, szSubKeyName, &dwSubKeyNameSize, NULL, NULL, NULL, &ftLastWriteTime) != ERROR_SUCCESS)
                break;

            QVariantMap entry;
            entry["type"] = "REG_KEY";
            entry["name"] = QString::fromWCharArray(szSubKeyName);
            entries.append(entry);
        }

        WCHAR szValueName[MAX_VALUE_NAME];
        DWORD dwValueNameSize;
        DWORD dwType;
        std::vector<BYTE> Buff;
        Buff.reserve(MAX_VALUE_DATA);
        BYTE* lpData = Buff.data();
        DWORD dwDataSize;

        for (DWORD dwIndex = 0; ; dwIndex++)
        {
            dwValueNameSize = MAX_VALUE_NAME;
            dwDataSize = MAX_VALUE_DATA;
            if (RegEnumValue((HKEY)handle, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, lpData, &dwDataSize) != ERROR_SUCCESS)
                break;

            QVariantMap entry;
            entry["name"] = QString::fromWCharArray(szValueName);
            switch (dwType)
            {
            case REG_NONE:              entry["type"] = "REG_NONE"; break;
            case REG_SZ:                entry["type"] = "REG_SZ";           entry["value"] = QString::fromStdWString((wchar_t*)lpData); break;
            case REG_EXPAND_SZ:         entry["type"] = "REG_EXPAND_SZ";    entry["value"] = QString::fromStdWString((wchar_t*)lpData); break;
            case REG_BINARY:            entry["type"] = "REG_BINARY";       entry["value"] = QByteArray((char*)lpData, dwDataSize); break;
            case REG_DWORD:             entry["type"] = "REG_DWORD";        entry["value"] = *(quint32*)lpData; break;
            //case REG_DWORD_BIG_ENDIAN: break;
            case REG_MULTI_SZ:          {entry["type"] = "REG_MULTI_SZ";
                                        QStringList List;
                                        for (wchar_t* pStr = (wchar_t*)lpData; *pStr; pStr += wcslen(pStr) + 1)
                                            List.append(QString::fromStdWString((wchar_t*)pStr));
                                        entry["value"] = List;
                                        break; }
            case REG_QWORD:             entry["type"] = "REG_QWORD";        entry["value"] = *(quint64*)lpData;
            //case REG_LINK: break;
            //case REG_RESOURCE_LIST: break;
            //case REG_FULL_RESOURCE_DESCRIPTOR: break;
            default:                    entry["type"] = "REG_UNKNOWN";
            }
            entries.append(entry);
        }
        CloseHandle(handle);
    }
    return m_pEngine->m_pEngine->toScriptValue(entries);
}

QJSValue JSysObject::setRegValue(const QString& Key, const QString& Name, const QVariant& Value, const QString& Type)
{
    bool bRet = false;
    HANDLE handle = openRegKey(Key, true);
    if (handle && handle != INVALID_HANDLE_VALUE)
    {
        DWORD dwType = REG_NONE;
        std::vector<BYTE> Buff;
        Buff.reserve(MAX_VALUE_DATA);
        BYTE* lpData = Buff.data();
        DWORD dwDataSize = 0;

        if (Type.isEmpty()) {
            switch (Value.type()) {
            case QVariant::String:      dwType = REG_SZ; break;
            case QVariant::StringList:  dwType = REG_MULTI_SZ; break;
            case QVariant::ByteArray:   dwType = REG_BINARY; break;
            case QVariant::Int:
            case QVariant::UInt:        dwType = REG_DWORD; break;
            case QVariant::Double:      // lets cast double to QDWORD
            case QVariant::ULongLong:
            case QVariant::LongLong:    dwType = REG_QWORD; break;
            }
        }
        else if (Type.compare("REG_NONE", Qt::CaseInsensitive) == 0)
            dwType = REG_NONE;
        else if (Type.compare("REG_SZ", Qt::CaseInsensitive) == 0)
            dwType = REG_SZ;
        else if (Type.compare("REG_EXPAND_SZ", Qt::CaseInsensitive) == 0)
            dwType = REG_EXPAND_SZ;
        else if (Type.compare("REG_BINARY", Qt::CaseInsensitive) == 0)
            dwType = REG_BINARY;
        else if (Type.compare("REG_DWORD", Qt::CaseInsensitive) == 0)
            dwType = REG_DWORD;
        else if (Type.compare("REG_MULTI_SZ", Qt::CaseInsensitive) == 0)
            dwType = REG_MULTI_SZ;
        else if (Type.compare("REG_QWORD", Qt::CaseInsensitive) == 0)
            dwType = REG_QWORD;

        switch (dwType)
        {
        case REG_SZ:                
        case REG_EXPAND_SZ:         {QString str = Value.toString(); str.toWCharArray((wchar_t*)lpData); dwDataSize = (str.size() + 1) * sizeof(wchar_t); break; }
        case REG_BINARY:            {QByteArray arr = Value.toByteArray(); memcpy(lpData, arr.data(), arr.size()); dwDataSize = arr.size(); break; }
        case REG_DWORD:             *(quint32*)lpData = Value.toInt(); dwDataSize = sizeof(qint32); break;
        //case REG_DWORD_BIG_ENDIAN: break;
        case REG_MULTI_SZ:          {BYTE* ptr = lpData;
                                    foreach(const QString& str, Value.toStringList()){
                                        str.toWCharArray((wchar_t*)ptr);
                                        DWORD len = (str.size() + 1) * sizeof(wchar_t);
                                        ptr += len;
                                        dwDataSize += len;
                                    }
                                    *(wchar_t*)ptr = 0;
                                    dwDataSize += sizeof(wchar_t);
                                    break; }
        case REG_QWORD:             *(quint32*)lpData = Value.toLongLong(); dwDataSize = sizeof(qint64); break;
        //case REG_LINK: break;
        //case REG_RESOURCE_LIST: break;
        //case REG_FULL_RESOURCE_DESCRIPTOR: break;
        }

        bRet = RegSetKeyValue((HKEY)handle, L"", Name.toStdWString().c_str(), dwType, lpData, dwDataSize);
        CloseHandle(handle);
    }
    return bRet;
}

QJSValue JSysObject::getRegValue(const QString& Key, const QString& Name)
{
    QVariant value;
    HANDLE handle = openRegKey(Key, true);
    if (handle && handle != INVALID_HANDLE_VALUE)
    {
        DWORD dwType;
        std::vector<BYTE> Buff;
        Buff.reserve(MAX_VALUE_DATA);
        BYTE* lpData = Buff.data();
        DWORD dwDataSize = MAX_VALUE_DATA;

        if (RegQueryValueEx((HKEY)handle, Name.toStdWString().c_str(), 0, &dwType, lpData, &dwDataSize)) {
            switch (dwType)
            {
            case REG_SZ:                value = QString::fromStdWString((wchar_t*)lpData); break;
            case REG_EXPAND_SZ:         value = QString::fromStdWString((wchar_t*)lpData); break;
            case REG_BINARY:            value = QByteArray((char*)lpData, dwDataSize); break;
            case REG_DWORD:             value = *(quint32*)lpData; break;
            //case REG_DWORD_BIG_ENDIAN: break;
            case REG_MULTI_SZ:          {QStringList List;
                                        for (wchar_t* pStr = (wchar_t*)lpData; *pStr; pStr += wcslen(pStr) + 1)
                                            List.append(QString::fromStdWString((wchar_t*)pStr));
                                        value = List;
                                        break; }
            case REG_QWORD:             value = *(quint64*)lpData;
            //case REG_LINK: break;
            //case REG_RESOURCE_LIST: break;
            //case REG_FULL_RESOURCE_DESCRIPTOR: break;
            }
        }
        CloseHandle(handle);
    }
    return m_pEngine->m_pEngine->toScriptValue(value);
}

QJSValue JSysObject::removeRegKey(const QString& Key)
{
    HANDLE handle = openRegKey(Key, true);
    if (handle && handle != INVALID_HANDLE_VALUE)
    {
        NtDeleteKey(handle);
        CloseHandle(handle);
    }
    return QJSValue();
}

QJSValue JSysObject::removeRegValue(const QString& Key, const QString& Name)
{
    HANDLE handle = openRegKey(Key, true);
    if (handle && handle != INVALID_HANDLE_VALUE)
    {
        RegDeleteValue((HKEY)handle, Name.toStdWString().c_str());
        CloseHandle(handle);
    }
    return QJSValue();
}


// SYS
QJSValue JSysObject::execute(const QString& Path, const QVariant& Arguments, const QVariantMap& Options)
{
    QVariantMap out;

    std::wstring dir = Options["workingDirectory"].toString().toStdWString();
	std::wstring path = Path.toStdWString();
    std::wstring params = Arguments.toString().toStdWString();
    if (params.empty()) {
        QStringList Args = Arguments.toStringList();
        if(!Args.isEmpty())
            params = ("\"" + Args.join("\" \"") + "\"").toStdWString();
    }
    int nShow = Options["hide"].toBool() ? SW_HIDE : SW_SHOWNORMAL;

    HANDLE hProcess = NULL;
    HANDLE stdoutReadHandle = NULL;
    HANDLE stdoutWriteHandle = NULL;

    if (Options["elevate"].toBool())
    {
        SHELLEXECUTEINFO shex;
        memset(&shex, 0, sizeof(SHELLEXECUTEINFO));
        shex.cbSize = sizeof(SHELLEXECUTEINFO);
        shex.fMask = SEE_MASK_NOCLOSEPROCESS;
        if(nShow == SW_HIDE) 
            shex.fMask |= SEE_MASK_FLAG_NO_UI;
        shex.hwnd = NULL;
        shex.lpFile = path.c_str();
        shex.lpParameters = params.c_str();
        shex.lpDirectory = dir.empty() ? NULL : dir.c_str();
        shex.nShow = nShow;
        shex.lpVerb = L"runas";

        if (ShellExecuteEx(&shex)) {
            hProcess = shex.hProcess;
        }
    }
    else
    {
	    SECURITY_ATTRIBUTES saAttr;
	    // Set the bInheritHandle flag so pipe handles are inherited.
	    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	    saAttr.bInheritHandle = TRUE;
	    saAttr.lpSecurityDescriptor = nullptr;
	    // Create a pipe for the child process's STDOUT.
        if (!CreatePipe(&stdoutReadHandle, &stdoutWriteHandle, &saAttr, 0))
            return QJSValue::UndefinedValue; // should not happen
        SetHandleInformation(stdoutReadHandle, HANDLE_FLAG_INHERIT, 0);

        std::wstring command = L"\"" + path + L"\" " + params;

	    STARTUPINFOW si = { 0 };
	    si.cb = sizeof(si);
	    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	    si.hStdOutput = stdoutWriteHandle;
	    si.hStdError = stdoutWriteHandle;
	    si.wShowWindow = nShow;

	    PROCESS_INFORMATION pi = { 0 };
        if (CreateProcessW(NULL, (LPWSTR)command.c_str(), NULL, NULL, TRUE, 0, NULL, dir.empty() ? NULL : dir.c_str(), &si, &pi)) {
            CloseHandle(pi.hThread);
            hProcess = pi.hProcess;
        }
    }

    if (hProcess) 
    {
        QString Output;
        DWORD exitCode;
        for (;;)
        {
            // Check if the process is alive.
            GetExitCodeProcess(hProcess, &exitCode);
            if (exitCode != STILL_ACTIVE)
                break;
            // keep UI responsive
            QCoreApplication::processEvents();

            // Check if there is anything in the pipe, if there is a pipe.
            DWORD dataSize;
            if (stdoutReadHandle && PeekNamedPipe(stdoutReadHandle, nullptr, 0, nullptr, &dataSize, nullptr) && dataSize > 0)
            {
                // Read the data out of the pipe.
                CHAR buffer[4096] = { 0 };
                if (ReadFile(stdoutReadHandle, buffer, sizeof(buffer) - 1, &dataSize, nullptr))
                    Output.append(QByteArray(buffer, dataSize));
            }
        }

        CloseHandle(hProcess);

        out["exitCode"] = (quint32)exitCode;
        if (!Output.isEmpty()) out["output"] = Output;
    } 
    else
        out["error"] = (quint32)GetLastError();

    if(stdoutReadHandle) CloseHandle(stdoutReadHandle);
	if(stdoutWriteHandle) CloseHandle(stdoutWriteHandle);

    return m_pEngine->m_pEngine->toScriptValue(out);
}

QJSValue JSysObject::expand(const QString& name)
{
    return QProcessEnvironment::systemEnvironment().value(name);
}

// OS
QVariantMap JSysObject::GetOSVersion()
{
    // todo improve this

    RTL_OSVERSIONINFOEXW versionInfo;
    memset(&versionInfo, 0, sizeof(RTL_OSVERSIONINFOEXW));
    versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
	NTSTATUS(WINAPI *RtlGetVersion)(PRTL_OSVERSIONINFOEXW);
	*(void**)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetVersion");
	if (RtlGetVersion != NULL) 
        RtlGetVersion(&versionInfo);
	else
        GetVersionExW((LPOSVERSIONINFOW)&versionInfo); // since windows 10 this one is lying
    RtlGetVersion(&versionInfo);

    if (versionInfo.dwMajorVersion == 10 && versionInfo.dwBuildNumber > 22000)
        versionInfo.dwMajorVersion = 11;

    QVariantMap ver;
    ver["major"] = (quint32)versionInfo.dwMajorVersion;
    ver["minor"] = (quint32)versionInfo.dwMinorVersion;
    ver["build"] = (quint32)versionInfo.dwBuildNumber;
    ver["platform"] = (quint32)versionInfo.dwPlatformId;
    return ver;
}

QJSValue JSysObject::version()
{
    return m_pEngine->m_pEngine->toScriptValue(GetOSVersion());
}

QJSValue JSysObject::language()
{
    return theGUI->m_Language;
}

QJSValue JSysObject::enumUpdates()
{
    return m_pEngine->m_pEngine->toScriptValue(theGUI->GetCompat()->GetUpdates());
}


// 
void JSysObject::resetData()
{
    theGUI->GetCompat()->Reset();
}

QJSValue JSysObject::enumClasses()
{
    return m_pEngine->m_pEngine->toScriptValue(theGUI->GetCompat()->GetClasses());
}

QJSValue JSysObject::enumServices()
{
    return m_pEngine->m_pEngine->toScriptValue(theGUI->GetCompat()->GetServices());
}

QJSValue JSysObject::enumProducts()
{
    return m_pEngine->m_pEngine->toScriptValue(theGUI->GetCompat()->GetProducts());
}

QJSValue JSysObject::enumObjects()
{
    return m_pEngine->m_pEngine->toScriptValue(theGUI->GetCompat()->GetObjects());
}


QJSValue JSysObject::expandPath(const QString& path)
{
    QString Path = path;
	Path.replace("%SbieHome%", theAPI->GetSbiePath(), Qt::CaseInsensitive);
	Path.replace("%PlusData%", theConf->GetConfigDir(), Qt::CaseInsensitive);

    return theGUI->GetCompat()->ExpandPath(Path);
}

QJSValue JSysObject::checkFile(const QString& value)
{
    return theGUI->GetCompat()->CheckFile(value);
}

QJSValue JSysObject::checkRegKey(const QString& value)
{
    return theGUI->GetCompat()->CheckRegistryKey(value);
}

QJSValue JSysObject::checkClasses(const QString& value)
{
    return theGUI->GetCompat()->CheckClasses(value);
}

QJSValue JSysObject::checkServices(const QString& value)
{
    return theGUI->GetCompat()->CheckServices(value);
}

QJSValue JSysObject::checkProducts(const QString& value)
{
    return theGUI->GetCompat()->CheckProducts(value);
}

QJSValue JSysObject::checkObjects(const QString& value)
{
    return theGUI->GetCompat()->CheckObjects(value);
}

QJSValue JSysObject::checkUpdates(const QString& value)
{
    return theGUI->GetCompat()->CheckUpdates(value);
}

