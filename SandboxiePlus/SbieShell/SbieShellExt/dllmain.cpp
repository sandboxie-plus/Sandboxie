// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <wrl/module.h>
#include <wrl/implements.h>
#include <wrl/client.h>
#include <shobjidl_core.h>
#include <wil\resource.h>
#include <string>
#include <vector>
#include <sstream>
#include <shellapi.h>

using namespace Microsoft::WRL;

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
    WCHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        strValue = szBuffer;
    }
    return nError;
}

std::wstring g_ExploreSandboxed = L"Explore Sandboxed";
std::wstring g_OpenSandboxed = L"Open Sandboxed";

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            wchar_t path[MAX_PATH];
            GetModuleFileName(hModule, path, MAX_PATH);
            wchar_t* ptr = wcsrchr(path, L'\\');
            *ptr = L'\0';
            g_path = std::wstring(path);

            HKEY hKey;
            LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Xanasoft\\Sandboxie-Plus\\SbieShellExt\\Lang", 0, KEY_READ, &hKey);
            bool bExistsAndSuccess(lRes == ERROR_SUCCESS);
            bool bDoesNotExistsSpecifically(lRes == ERROR_FILE_NOT_FOUND);
            GetStringRegKey(hKey, L"Explore Sandboxed", g_ExploreSandboxed);
            GetStringRegKey(hKey, L"Open Sandboxed", g_OpenSandboxed);
            CloseHandle(hKey);

            break;
        }
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
        auto title = wil::make_cotaskmem_string_nothrow(Title());
        RETURN_IF_NULL_ALLOC(title);
        *name = title.release();
        return S_OK;
    }
    IFACEMETHODIMP GetIcon(_In_opt_ IShellItemArray*, _Outptr_result_nullonfailure_ PWSTR* icon)// { *icon = nullptr; return E_NOTIMPL; }
    {
        std::wstring bpPath = g_path + L"\\SandMan.exe,-0";
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


        if (selection)
        {
            DWORD fileCount = 0;
            RETURN_IF_FAILED(selection->GetCount(&fileCount));
            for (DWORD i = 0; i < fileCount; i++)
            {
                IShellItem* shellItem = nullptr;
                selection->GetItemAt(i, &shellItem);
                LPWSTR itemName = nullptr;
                shellItem->GetDisplayName(SIGDN_FILESYSPATH, &itemName);
                if (itemName)
                {
                    std::wstring file = g_path + L"\\SandMan.exe";

                    std::wstring params = L"/box:__ask__";
                    if(GetCommand() == eExplore)
                        params += L" C:\\WINDOWS\\explorer.exe";
                    params += L" \"";
                    params += itemName;
                    params += L"\"";

                    SHELLEXECUTEINFO shExecInfo = { sizeof(SHELLEXECUTEINFO) };
                    shExecInfo.hwnd = nullptr;
                    shExecInfo.lpVerb = L"open";
                    shExecInfo.lpFile = file.c_str();
                    shExecInfo.lpParameters = params.c_str();
                    shExecInfo.nShow = SW_NORMAL;
                    ShellExecuteEx(&shExecInfo);

                    CoTaskMemFree(itemName);
                }
            }
        }

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
    return Module<ModuleType::InProc>::GetModule().GetActivationFactory(activatableClassId, factory);
}

STDAPI DllCanUnloadNow()
{
    return Module<InProc>::GetModule().GetObjectCount() == 0 ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _COM_Outptr_ void** instance)
{
    return Module<InProc>::GetModule().GetClassObject(rclsid, riid, instance);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////7
//

#include <winrt/Windows.Management.Deployment.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.h>

#pragma comment(lib, "windowsapp.lib")

int RegisterSparsePackage(const std::wstring& sparseExtPath, const std::wstring& sparsePackagePath)
{
    winrt::Windows::Management::Deployment::PackageManager manager;
    winrt::Windows::Management::Deployment::AddPackageOptions options;
    winrt::Windows::Foundation::Uri externalUri(sparseExtPath.c_str());
    winrt::Windows::Foundation::Uri packageUri(sparsePackagePath.c_str());
    options.ExternalLocationUri(externalUri);
    auto deploymentOperation = manager.AddPackageByUriAsync(packageUri, options);

    auto deployResult = deploymentOperation.get();

    if (!SUCCEEDED(deployResult.ExtendedErrorCode()))
    {
        // Deployment failed
        std::wstring error = L"AddPackageByUriAsync failed (Errorcode: ";
        error += std::to_wstring(deployResult.ExtendedErrorCode());
        error += L"):\n";
        error += deployResult.ErrorText();

        return -1;
    }
    return 0;
}

int UnregisterSparsePackage(const std::wstring& sparsePackageName)
{

    winrt::Windows::Management::Deployment::PackageManager manager;
    winrt::Windows::Foundation::Collections::IIterable<winrt::Windows::ApplicationModel::Package> packages;
    try
    {
        packages = manager.FindPackagesForUser(L"");
    }
    catch (winrt::hresult_error const& ex)
    {
        std::wstring error = L"FindPackagesForUser failed (Errorcode: ";
        error += std::to_wstring(ex.code().value);
        error += L"):\n";
        error += ex.message();
        return -1;
    }

    for (const auto& package : packages)
    {
        if (package.Id().Name() != sparsePackageName)
            continue;

        winrt::hstring fullName = package.Id().FullName();
        auto deploymentOperation = manager.RemovePackageAsync(fullName, winrt::Windows::Management::Deployment::RemovalOptions::None);
        auto deployResult = deploymentOperation.get();
        if (SUCCEEDED(deployResult.ExtendedErrorCode()))
            break;

        // Undeployment failed
        std::wstring error = L"RemovePackageAsync failed (Errorcode: ";
        error += std::to_wstring(deployResult.ExtendedErrorCode());
        error += L"):\n";
        error += deployResult.ErrorText();

        return -1;
    }
    return 0;
}

extern "C" __declspec(dllexport) int RegisterPackage()
{
    std::wstring sparseExtPath = g_path;
    std::wstring sparsePackagePath = g_path + L"\\SbieShellPkg.msix";
    return RegisterSparsePackage(sparseExtPath, sparsePackagePath);
}

extern "C" __declspec(dllexport) int RemovePackage()
{
    std::wstring sparsePackageName = L"SandboxieShell";
    return UnregisterSparsePackage(sparsePackageName);
}
