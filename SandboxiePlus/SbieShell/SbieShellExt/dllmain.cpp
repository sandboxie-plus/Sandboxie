// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <wrl/module.h>
#include <wrl/implements.h>
#include <wrl/client.h>
#include <shobjidl_core.h>
#include <wil\resource.h>
#include <cstdarg>
#include <exception>
#include <string>
#include <vector>
#include <sstream>
#include <shellapi.h>

using namespace Microsoft::WRL;

namespace
{
// Set SBIE_SHELL_EXT_DEBUG to any non-empty value to enable diagnostics.
constexpr wchar_t kDebugEnvironmentVariable[] = L"SBIE_SHELL_EXT_DEBUG";
constexpr wchar_t kDebugLogFileName[] = L"SbieShellExt.log";
constexpr LONGLONG kMaxDebugLogSize = 1024 * 1024;

bool IsDebugLoggingEnabled()
{
    wchar_t value[2] = {};
    return GetEnvironmentVariableW(kDebugEnvironmentVariable, value, _countof(value)) != 0;
}

void DebugLog(const wchar_t* format, ...) noexcept
{
    if (!IsDebugLoggingEnabled())
        return;

    try
    {
        wchar_t message[2048] = {};
        va_list args;
        va_start(args, format);
        _vsnwprintf_s(message, _countof(message), _TRUNCATE, format, args);
        va_end(args);

        SYSTEMTIME time = {};
        GetLocalTime(&time);

        wchar_t line[2304] = {};
        _snwprintf_s(line, _countof(line), _TRUNCATE,
            L"[%04u-%02u-%02u %02u:%02u:%02u.%03u pid=%lu tid=%lu] %ls\r\n",
            time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond,
            time.wMilliseconds, GetCurrentProcessId(), GetCurrentThreadId(), message);

        OutputDebugStringW(line);

        wchar_t tempPath[MAX_PATH] = {};
        DWORD tempPathLength = GetTempPathW(_countof(tempPath), tempPath);
        if (tempPathLength == 0 || tempPathLength >= _countof(tempPath))
            return;

        std::wstring logPath(tempPath, tempPathLength);
        logPath += kDebugLogFileName;

        wil::unique_hfile logFile(CreateFileW(logPath.c_str(), GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr));
        if (!logFile)
            return;

        int utf8Length = WideCharToMultiByte(CP_UTF8, 0, line, -1, nullptr, 0, nullptr, nullptr);
        if (utf8Length > 1)
        {
            std::string utf8(static_cast<size_t>(utf8Length), '\0');
            int converted = WideCharToMultiByte(CP_UTF8, 0, line, -1, &utf8[0], utf8Length, nullptr, nullptr);
            if (converted > 1)
            {
                LARGE_INTEGER logSize = {};
                if (!GetFileSizeEx(logFile.get(), &logSize))
                    return;

                if (logSize.QuadPart + converted - 1 > kMaxDebugLogSize)
                {
                    LARGE_INTEGER beginning = {};
                    if (!SetFilePointerEx(logFile.get(), beginning, nullptr, FILE_BEGIN) ||
                        !SetEndOfFile(logFile.get()))
                        return;
                }

                LARGE_INTEGER end = {};
                if (!SetFilePointerEx(logFile.get(), end, nullptr, FILE_END))
                    return;

                DWORD bytesWritten = 0;
                WriteFile(logFile.get(), utf8.data(), static_cast<DWORD>(converted - 1), &bytesWritten, nullptr);
            }
        }
    }
    catch (...)
    {
    }
}

std::wstring GuidToString(REFGUID guid)
{
    try
    {
        wchar_t text[64] = {};
        if (StringFromGUID2(guid, text, _countof(text)) == 0)
            return L"<invalid>";
        return text;
    }
    catch (...)
    {
        return L"<invalid>";
    }
}
}

std::wstring g_path;
LONG GetDWORDRegKey(HKEY hKey, const std::wstring& strValueName, DWORD& nValue)
{
    DWORD dwBufferSize(sizeof(DWORD));
    DWORD nResult(0);
    LONG nError = ::RegQueryValueExW(hKey,
        strValueName.c_str(),
        0,
        NULL,
        reinterpret_cast<LPBYTE>(&nResult),
        &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        nValue = nResult;
    }
    return nError;
}

LONG GetStringRegKey(HKEY hKey, const std::wstring& strValueName, std::wstring& strValue)
{
    if (hKey == nullptr)
        return ERROR_INVALID_HANDLE;

    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    DWORD type = REG_NONE;
    ULONG nError;
    nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, &type, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        if (type != REG_SZ || dwBufferSize < sizeof(WCHAR) ||
            dwBufferSize > sizeof(szBuffer) || dwBufferSize % sizeof(WCHAR) != 0)
            return ERROR_INVALID_DATA;

        const WCHAR* terminator = wmemchr(szBuffer, L'\0', dwBufferSize / sizeof(WCHAR));
        if (!terminator)
            return ERROR_INVALID_DATA;

        strValue.assign(szBuffer, static_cast<size_t>(terminator - szBuffer));
    }
    return nError;
}

std::wstring g_ExploreSandboxed = L"Explore Sandboxed";
std::wstring g_OpenSandboxed = L"Open Sandboxed";
HMODULE g_module = nullptr;
INIT_ONCE g_initializeOnce = INIT_ONCE_STATIC_INIT;
DWORD g_initializationError = ERROR_SUCCESS;

BOOL CALLBACK InitializeShellExtension(PINIT_ONCE, PVOID, PVOID*) noexcept try
{
    wchar_t path[MAX_PATH] = {};
    DWORD pathLength = GetModuleFileNameW(g_module, path, _countof(path));
    if (pathLength == 0 || pathLength >= _countof(path))
    {
        g_initializationError = pathLength == 0 ? GetLastError() : ERROR_INSUFFICIENT_BUFFER;
        DebugLog(L"InitializeShellExtension: GetModuleFileNameW failed length=%lu error=%lu",
            pathLength, g_initializationError);
        return FALSE;
    }

    wchar_t* ptr = wcsrchr(path, L'\\');
    if (!ptr)
    {
        g_initializationError = ERROR_BAD_PATHNAME;
        DebugLog(L"InitializeShellExtension: module path has no directory path=%ls", path);
        return FALSE;
    }

    *ptr = L'\0';
    g_path = path;
    DebugLog(L"InitializeShellExtension: moduleDirectory=%ls", g_path.c_str());

    HKEY hKey = nullptr;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
        L"SOFTWARE\\Xanasoft\\Sandboxie-Plus\\SbieShellExt\\Lang", 0, KEY_READ, &hKey);
    DebugLog(L"InitializeShellExtension: RegOpenKeyExW Lang result=%ld", result);
    if (result == ERROR_SUCCESS)
    {
        LONG exploreResult = GetStringRegKey(hKey, L"Explore Sandboxed", g_ExploreSandboxed);
        LONG openResult = GetStringRegKey(hKey, L"Open Sandboxed", g_OpenSandboxed);
        DebugLog(L"InitializeShellExtension: language values exploreResult=%ld openResult=%ld explore=%ls open=%ls",
            exploreResult, openResult, g_ExploreSandboxed.c_str(), g_OpenSandboxed.c_str());
        RegCloseKey(hKey);
    }

    return TRUE;
}
catch (...)
{
    g_initializationError = ERROR_UNHANDLED_EXCEPTION;
    DebugLog(L"InitializeShellExtension: unexpected exception");
    return FALSE;
}

bool EnsureShellExtensionInitialized()
{
    return InitOnceExecuteOnce(&g_initializeOnce, InitializeShellExtension, nullptr, nullptr) != FALSE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            g_module = hModule;
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

class TestExplorerCommandBase : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IExplorerCommand, IObjectWithSite>
{
public:
    virtual const wchar_t* Title() = 0;
    virtual const EXPCMDFLAGS Flags() { return ECF_DEFAULT; }
    virtual const EXPCMDSTATE State(_In_opt_ IShellItemArray* selection) { return ECS_ENABLED; }

    // IExplorerCommand
    IFACEMETHODIMP GetTitle(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* name)
    {
        *name = nullptr;
        DebugLog(L"GetTitle: title=%ls", Title());
        auto title = wil::make_cotaskmem_string_nothrow(Title());
        RETURN_IF_NULL_ALLOC(title);
        *name = title.release();
        return S_OK;
    }
    IFACEMETHODIMP GetIcon(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* icon)// { *icon = nullptr; return E_NOTIMPL; }
    {
        std::wstring bpPath = g_path + L"\\SandMan.exe,-0";
        DebugLog(L"GetIcon: path=%ls", bpPath.c_str());
        auto iconPath = wil::make_cotaskmem_string_nothrow(bpPath.c_str());
        RETURN_IF_NULL_ALLOC(iconPath);
        *icon = iconPath.release();
        return S_OK;
    }
    IFACEMETHODIMP GetToolTip(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* infoTip) { *infoTip = nullptr; return E_NOTIMPL; }
    IFACEMETHODIMP GetCanonicalName(_Out_ GUID* guidCommandName) { *guidCommandName = GUID_NULL;  return S_OK; }
    IFACEMETHODIMP GetState(_In_opt_ IShellItemArray* selection, _In_ BOOL okToBeSlow, _Out_ EXPCMDSTATE* cmdState)
    {
        *cmdState = State(selection);
        DebugLog(L"GetState: title=%ls okToBeSlow=%d state=0x%08lX", Title(), okToBeSlow,
            static_cast<unsigned long>(*cmdState));
        return S_OK;
    }
    enum ECommand {
        eExplore,
        eOpen,
    };
    virtual ECommand GetCommand() = 0;
    IFACEMETHODIMP Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*) noexcept try
    {
        /*HWND parent = nullptr;
        if (m_site)
        {
            ComPtr<IOleWindow> oleWindow;
            RETURN_IF_FAILED(m_site.As(&oleWindow));
            RETURN_IF_FAILED(oleWindow->GetWindow(&parent));
        }

        std::wostringstream title;
        title << Title();

        if (selection)
        {
            DWORD count;
            RETURN_IF_FAILED(selection->GetCount(&count));
            title << L" (" << count << L" selected items)";
        }
        else
        {
            title << L"(no selected items)";
        }

        MessageBox(parent, title.str().c_str(), L"TestCommand", MB_OK);*/

        DebugLog(L"Invoke: title=%ls command=%ls selection=%p", Title(),
            GetCommand() == eExplore ? L"Explore" : L"Open", selection);

        if (selection)
        {
            DWORD fileCount = 0;
            HRESULT result = selection->GetCount(&fileCount);
            if (FAILED(result))
            {
                DebugLog(L"Invoke: IShellItemArray::GetCount failed hr=0x%08lX",
                    static_cast<unsigned long>(result));
                return result;
            }
            DebugLog(L"Invoke: selectionCount=%lu", fileCount);
            for (DWORD i = 0; i < fileCount; i++)
            {
                ComPtr<IShellItem> shellItem;
                result = selection->GetItemAt(i, &shellItem);
                if (FAILED(result))
                {
                    DebugLog(L"Invoke: GetItemAt index=%lu failed hr=0x%08lX", i,
                        static_cast<unsigned long>(result));
                    return result;
                }

                wil::unique_cotaskmem_string itemName;
                result = shellItem->GetDisplayName(SIGDN_FILESYSPATH, &itemName);
                if (FAILED(result))
                {
                    DebugLog(L"Invoke: GetDisplayName index=%lu failed hr=0x%08lX", i,
                        static_cast<unsigned long>(result));
                    return result;
                }
                if (itemName)
                {
                    std::wstring file = g_path + L"\\SandMan.exe";

                    std::wstring params = L"/box:__ask__";
                    if(GetCommand() == eExplore)
                        params += L" C:\\WINDOWS\\explorer.exe";
                    params += L" \"";
                    params += itemName.get();
                    params += L"\"";

                    // The extension runs in a surrogate process, so set the selected item's directory explicitly.
                    std::vector<wchar_t> drive(_MAX_DRIVE), dir(_MAX_DIR);
                    _wsplitpath_s(itemName.get(), drive.data(), drive.size(), dir.data(), dir.size(), nullptr, 0, nullptr, 0);
                    std::wstring currentDirectory(drive.data());
                    currentDirectory += dir.data();
                    DebugLog(L"Invoke: index=%lu item=%ls currentDirectory=%ls params=%ls", i,
                        itemName.get(), currentDirectory.c_str(), params.c_str());

                    SHELLEXECUTEINFO shExecInfo = { sizeof(SHELLEXECUTEINFO) };
                    shExecInfo.hwnd = nullptr;
                    shExecInfo.lpVerb = L"open";
                    shExecInfo.lpFile = file.c_str();
                    shExecInfo.lpParameters = params.c_str();
                    shExecInfo.lpDirectory = currentDirectory.c_str();
                    shExecInfo.nShow = SW_NORMAL;
                    BOOL executeResult = ShellExecuteExW(&shExecInfo);
                    DebugLog(L"Invoke: ShellExecuteExW index=%lu result=%d error=%lu", i,
                        executeResult, executeResult ? ERROR_SUCCESS : GetLastError());
                }
                else
                    DebugLog(L"Invoke: GetDisplayName index=%lu returned no path", i);
            }
        }

        return S_OK;
    }
    CATCH_RETURN();

    IFACEMETHODIMP GetFlags(_Out_ EXPCMDFLAGS* flags) { *flags = Flags(); return S_OK; }
    IFACEMETHODIMP EnumSubCommands(_COM_Outptr_ IEnumExplorerCommand** enumCommands) { *enumCommands = nullptr; return E_NOTIMPL; }

    // IObjectWithSite
    IFACEMETHODIMP SetSite(_In_ IUnknown* site) noexcept
    {
        DebugLog(L"SetSite: site=%p", site);
        m_site = site;
        return S_OK;
    }
    IFACEMETHODIMP GetSite(_In_ REFIID riid, _COM_Outptr_ void** site) noexcept { return m_site.CopyTo(riid, site); }

protected:
    ComPtr<IUnknown> m_site;
};

class __declspec(uuid("EA3E972D-62C7-4309-8F15-883263041E99")) ExploreCommandHandler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return g_ExploreSandboxed.c_str(); }
    ECommand GetCommand() { return eExplore; }
};

class __declspec(uuid("3FD2D9EE-DAF9-404A-9B7E-13B2DCD63950")) OpenCommandHandler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return g_OpenSandboxed.c_str(); }
    ECommand GetCommand() { return eOpen; }
};

CoCreatableClass(ExploreCommandHandler)
CoCreatableClass(OpenCommandHandler)

CoCreatableClassWrlCreatorMapInclude(ExploreCommandHandler)
CoCreatableClassWrlCreatorMapInclude(OpenCommandHandler)

/*
class TestExplorerCommandBase : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IExplorerCommand, IObjectWithSite>
{
public:
    virtual const wchar_t* Title() = 0;
    virtual const EXPCMDFLAGS Flags() { return ECF_DEFAULT; }
    virtual const EXPCMDSTATE State(_In_opt_ IShellItemArray* selection) { return ECS_ENABLED; }

    // IExplorerCommand
    IFACEMETHODIMP GetTitle(_In_opt_ IShellItemArray* items, _Outptr_result_nullonfailure_ PWSTR* name)
    {
        *name = nullptr;
        auto title = wil::make_cotaskmem_string_nothrow(Title());
        RETURN_IF_NULL_ALLOC(title);
        *name = title.release();
        return S_OK;
    }
    IFACEMETHODIMP GetIcon(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* icon) { *icon = nullptr; return E_NOTIMPL; }
    IFACEMETHODIMP GetToolTip(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* infoTip) { *infoTip = nullptr; return E_NOTIMPL; }
    IFACEMETHODIMP GetCanonicalName(_Out_ GUID* guidCommandName) { *guidCommandName = GUID_NULL;  return S_OK; }
    IFACEMETHODIMP GetState(_In_opt_ IShellItemArray* selection, _In_ BOOL okToBeSlow, _Out_ EXPCMDSTATE* cmdState)
    {
        *cmdState = State(selection);
        return S_OK;
    }
    IFACEMETHODIMP Invoke(_In_opt_ IShellItemArray* selection, _In_opt_ IBindCtx*) noexcept try
    {
        HWND parent = nullptr;
        if (m_site)
        {
            ComPtr<IOleWindow> oleWindow;
            RETURN_IF_FAILED(m_site.As(&oleWindow));
            RETURN_IF_FAILED(oleWindow->GetWindow(&parent));
        }

        std::wostringstream title;
        title << Title();

        if (selection)
        {
            DWORD count;
            RETURN_IF_FAILED(selection->GetCount(&count));
            title << L" (" << count << L" selected items)";
        }
        else
        {
            title << L"(no selected items)";
        }

        MessageBox(parent, title.str().c_str(), L"TestCommand", MB_OK);
        return S_OK;
    }
    CATCH_RETURN();

    IFACEMETHODIMP GetFlags(_Out_ EXPCMDFLAGS* flags) { *flags = Flags(); return S_OK; }
    IFACEMETHODIMP EnumSubCommands(_COM_Outptr_ IEnumExplorerCommand** enumCommands) { *enumCommands = nullptr; return E_NOTIMPL; }

    // IObjectWithSite
    IFACEMETHODIMP SetSite(_In_ IUnknown* site) noexcept { m_site = site; return S_OK; }
    IFACEMETHODIMP GetSite(_In_ REFIID riid, _COM_Outptr_ void** site) noexcept { return m_site.CopyTo(riid, site); }

protected:
    ComPtr<IUnknown> m_site;
};

class __declspec(uuid("3282E233-C5D3-4533-9B25-44B8AAAFACFA")) TestExplorerCommandHandler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return L"ShellDemo Command1"; }
    const EXPCMDSTATE State(_In_opt_ IShellItemArray* selection) override { return ECS_DISABLED; }
};

class __declspec(uuid("817CF159-A4B5-41C8-8E8D-0E23A6605395")) TestExplorerCommand2Handler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return L"ShellDemo ExplorerCommand2"; }
};

class SubExplorerCommandHandler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return L"SubCommand"; }
};

class CheckedSubExplorerCommandHandler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return L"CheckedSubCommand"; }
    const EXPCMDSTATE State(_In_opt_ IShellItemArray* selection) override { return ECS_CHECKBOX | ECS_CHECKED; }
};

class RadioCheckedSubExplorerCommandHandler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return L"RadioCheckedSubCommand"; }
    const EXPCMDSTATE State(_In_opt_ IShellItemArray* selection) override { return ECS_CHECKBOX | ECS_RADIOCHECK; }
};

class HiddenSubExplorerCommandHandler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return L"HiddenSubCommand"; }
    const EXPCMDSTATE State(_In_opt_ IShellItemArray* selection) override { return ECS_HIDDEN; }
};

class EnumCommands : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IEnumExplorerCommand>
{
public:
    EnumCommands()
    {
        m_commands.push_back(Make<SubExplorerCommandHandler>());
        m_commands.push_back(Make<CheckedSubExplorerCommandHandler>());
        m_commands.push_back(Make<RadioCheckedSubExplorerCommandHandler>());
        m_commands.push_back(Make<HiddenSubExplorerCommandHandler>());
        m_current = m_commands.cbegin();
    }

    // IEnumExplorerCommand
    IFACEMETHODIMP Next(ULONG celt, __out_ecount_part(celt, *pceltFetched) IExplorerCommand** apUICommand, __out_opt ULONG* pceltFetched)
    {
        ULONG fetched{ 0 };
        wil::assign_to_opt_param(pceltFetched, 0ul);

        for (ULONG i = 0; (i < celt) && (m_current != m_commands.cend()); i++)
        {
            m_current->CopyTo(&apUICommand[0]);
            m_current++;
            fetched++;
        }

        wil::assign_to_opt_param(pceltFetched, fetched);
        return (fetched == celt) ? S_OK : S_FALSE;
    }

    IFACEMETHODIMP Skip(ULONG celt) { return E_NOTIMPL; }
    IFACEMETHODIMP Reset()
    {
        m_current = m_commands.cbegin();
        return S_OK;
    }
    IFACEMETHODIMP Clone(__deref_out IEnumExplorerCommand** ppenum) { *ppenum = nullptr; return E_NOTIMPL; }

private:
    std::vector<ComPtr<IExplorerCommand>> m_commands;
    std::vector<ComPtr<IExplorerCommand>>::const_iterator m_current;
};

class __declspec(uuid("1476525B-BBC2-4D04-B175-7E7D72F3DFF8")) TestExplorerCommand3Handler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return L"ShellDemo CommandWithSubCommands"; }
    const EXPCMDFLAGS Flags() override { return ECF_HASSUBCOMMANDS; }

    IFACEMETHODIMP EnumSubCommands(_COM_Outptr_ IEnumExplorerCommand** enumCommands)
    {
        *enumCommands = nullptr;
        auto e = Make<EnumCommands>();
        return e->QueryInterface(IID_PPV_ARGS(enumCommands));
    }
};

class __declspec(uuid("30DEEDF6-63EA-4042-A7D8-0A9E1B17BB99")) TestExplorerCommand4Handler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return L"ShellDemo Command4"; }
};

class __declspec(uuid("50419A05-F966-47BA-B22B-299A95492348")) TestExplorerHiddenCommandHandler final : public TestExplorerCommandBase
{
public:
    const wchar_t* Title() override { return L"ShellDemo HiddenCommand"; }
    const EXPCMDSTATE State(_In_opt_ IShellItemArray* selection) override { return ECS_HIDDEN; }
};

CoCreatableClass(TestExplorerCommandHandler)
CoCreatableClass(TestExplorerCommand2Handler)
CoCreatableClass(TestExplorerCommand3Handler)
CoCreatableClass(TestExplorerCommand4Handler)
CoCreatableClass(TestExplorerHiddenCommandHandler)

CoCreatableClassWrlCreatorMapInclude(TestExplorerCommandHandler)
CoCreatableClassWrlCreatorMapInclude(TestExplorerCommand2Handler)
CoCreatableClassWrlCreatorMapInclude(TestExplorerCommand3Handler)
CoCreatableClassWrlCreatorMapInclude(TestExplorerCommand4Handler)
CoCreatableClassWrlCreatorMapInclude(TestExplorerHiddenCommandHandler)
*/


STDAPI DllGetActivationFactory(_In_ HSTRING activatableClassId, _COM_Outptr_ IActivationFactory** factory)
{
    if (!EnsureShellExtensionInitialized())
        return HRESULT_FROM_WIN32(g_initializationError);

    HRESULT result = Module<ModuleType::InProc>::GetModule().GetActivationFactory(activatableClassId, factory);
    DebugLog(L"DllGetActivationFactory: result=0x%08lX", static_cast<unsigned long>(result));
    return result;
}

STDAPI DllCanUnloadNow()
{
    return Module<InProc>::GetModule().GetObjectCount() == 0 ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _COM_Outptr_ void** instance)
{
    if (!EnsureShellExtensionInitialized())
        return HRESULT_FROM_WIN32(g_initializationError);

    HRESULT result = Module<InProc>::GetModule().GetClassObject(rclsid, riid, instance);
    std::wstring classId = GuidToString(rclsid);
    std::wstring interfaceId = GuidToString(riid);
    DebugLog(L"DllGetClassObject: clsid=%ls riid=%ls result=0x%08lX", classId.c_str(),
        interfaceId.c_str(), static_cast<unsigned long>(result));
    return result;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////7
//

#include <winrt/Windows.Management.Deployment.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.h>

#pragma comment(lib, "windowsapp.lib")
#pragma comment(lib, "ole32.lib")

int RegisterSparsePackage(const std::wstring& sparseExtPath, const std::wstring& sparsePackagePath)
{
    DebugLog(L"RegisterSparsePackage: begin externalPath=%ls packagePath=%ls", sparseExtPath.c_str(),
        sparsePackagePath.c_str());

    DWORD externalAttributes = GetFileAttributesW(sparseExtPath.c_str());
    DWORD externalError = externalAttributes == INVALID_FILE_ATTRIBUTES ? GetLastError() : ERROR_SUCCESS;
    DWORD packageAttributes = GetFileAttributesW(sparsePackagePath.c_str());
    DWORD packageError = packageAttributes == INVALID_FILE_ATTRIBUTES ? GetLastError() : ERROR_SUCCESS;
    DebugLog(L"RegisterSparsePackage: externalAttributes=0x%08lX externalError=%lu packageAttributes=0x%08lX packageError=%lu",
        externalAttributes, externalError, packageAttributes, packageError);

    try
    {
        winrt::Windows::Management::Deployment::PackageManager manager;
        winrt::Windows::Management::Deployment::AddPackageOptions options;
        winrt::Windows::Foundation::Uri externalUri(sparseExtPath.c_str());
        winrt::Windows::Foundation::Uri packageUri(sparsePackagePath.c_str());
        options.ExternalLocationUri(externalUri);
        auto deploymentOperation = manager.AddPackageByUriAsync(packageUri, options);

        auto deployResult = deploymentOperation.get();
        long extendedError = static_cast<long>(deployResult.ExtendedErrorCode());
        std::wstring errorText = deployResult.ErrorText().c_str();
        DebugLog(L"RegisterSparsePackage: deployment result=0x%08lX errorText=%ls",
            static_cast<unsigned long>(extendedError), errorText.c_str());

        if (!SUCCEEDED(extendedError))
        {
            // Deployment failed
            std::wstring error = L"AddPackageByUriAsync failed (Errorcode: ";
            error += std::to_wstring(extendedError);
            error += L"):\n";
            error += errorText;

            return -1;
        }
        return 0;
    }
    catch (const winrt::hresult_error& ex)
    {
        DebugLog(L"RegisterSparsePackage: hresult_error code=0x%08lX message=%ls",
            static_cast<unsigned long>(ex.code().value), ex.message().c_str());
        return -1;
    }
    catch (const std::exception&)
    {
        DebugLog(L"RegisterSparsePackage: std::exception");
        return -1;
    }
    catch (...)
    {
        DebugLog(L"RegisterSparsePackage: unknown exception");
        return -1;
    }
}

int UnregisterSparsePackage(const std::wstring& sparsePackageName)
{
    DebugLog(L"UnregisterSparsePackage: begin packageName=%ls", sparsePackageName.c_str());

    try
    {
        winrt::Windows::Management::Deployment::PackageManager manager;
        winrt::Windows::Foundation::Collections::IIterable<winrt::Windows::ApplicationModel::Package> packages;
        packages = manager.FindPackagesForUser(L"");

        bool found = false;
        for (const auto& package : packages)
        {
            auto packageId = package.Id();
            if (packageId.Name() != sparsePackageName)
                continue;

            found = true;
            winrt::hstring fullName = packageId.FullName();
            DebugLog(L"UnregisterSparsePackage: removing fullName=%ls", fullName.c_str());
            auto deploymentOperation = manager.RemovePackageAsync(fullName, winrt::Windows::Management::Deployment::RemovalOptions::None);
            auto deployResult = deploymentOperation.get();
            long extendedError = static_cast<long>(deployResult.ExtendedErrorCode());
            std::wstring errorText = deployResult.ErrorText().c_str();
            DebugLog(L"UnregisterSparsePackage: deployment result=0x%08lX errorText=%ls",
                static_cast<unsigned long>(extendedError), errorText.c_str());
            if (SUCCEEDED(extendedError))
                break;

            // Undeployment failed
            std::wstring error = L"RemovePackageAsync failed (Errorcode: ";
            error += std::to_wstring(extendedError);
            error += L"):\n";
            error += errorText;

            return -1;
        }

        DebugLog(L"UnregisterSparsePackage: packageFound=%d", found);
        return 0;
    }
    catch (winrt::hresult_error const& ex)
    {
        DebugLog(L"UnregisterSparsePackage: hresult_error code=0x%08lX message=%ls",
            static_cast<unsigned long>(ex.code().value), ex.message().c_str());
        return -1;
    }
    catch (const std::exception&)
    {
        DebugLog(L"UnregisterSparsePackage: std::exception");
        return -1;
    }
    catch (...)
    {
        DebugLog(L"UnregisterSparsePackage: unknown exception");
        return -1;
    }
}

extern "C" __declspec(dllexport) int RegisterPackage()
{
    if (!EnsureShellExtensionInitialized())
        return static_cast<int>(HRESULT_FROM_WIN32(g_initializationError));

    std::wstring sparseExtPath = g_path;
    std::wstring sparsePackagePath = g_path + L"\\SbieShellPkg.msix";
    int result = RegisterSparsePackage(sparseExtPath, sparsePackagePath);
    DebugLog(L"RegisterPackage: result=%d", result);
    return result;
}

extern "C" __declspec(dllexport) int RemovePackage()
{
    std::wstring sparsePackageName = L"SandboxieShell";
    int result = UnregisterSparsePackage(sparsePackageName);
    DebugLog(L"RemovePackage: result=%d", result);
    return result;
}

extern "C" __declspec(dllexport) void CALLBACK RegisterPackageRundll(HWND, HINSTANCE, LPSTR, int)
{
    ExitProcess(static_cast<UINT>(RegisterPackage()));
}

extern "C" __declspec(dllexport) void CALLBACK RemovePackageRundll(HWND, HINSTANCE, LPSTR, int)
{
    ExitProcess(static_cast<UINT>(RemovePackage()));
}
