#include <windows.h>
//#include <appmodel.h>

#include <winrt/Windows.Management.Deployment.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.h>

//#pragma comment(lib, "windowsapp.lib")

HMODULE g_combase = NULL;

extern "C"
{
    int32_t __stdcall WINRT_GetRestrictedErrorInfo(void** info) noexcept {
        typedef int32_t(__stdcall* FUNC)(void** info);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "GetRestrictedErrorInfo");
        return Func(info);
    }
    int32_t __stdcall WINRT_RoGetActivationFactory(void* classId, winrt::guid const& iid, void** factory) noexcept {
        typedef int32_t(__stdcall* FUNC)(void* classId, winrt::guid const& iid, void** factory);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "RoGetActivationFactory");
        return Func(classId, iid, factory);
    }
    int32_t __stdcall WINRT_RoOriginateLanguageException(int32_t error, void* message, void* exception) noexcept {
        typedef int32_t(__stdcall* FUNC)(int32_t error, void* message, void* exception);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "RoOriginateLanguageException");
        return Func(error, message, exception);
    }
    int32_t __stdcall WINRT_SetRestrictedErrorInfo(void* info) noexcept {
        typedef int32_t(__stdcall* FUNC)(void* info);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "SetRestrictedErrorInfo");
        return Func(info);
    }

    int32_t __stdcall WINRT_WindowsCreateString(wchar_t const* sourceString, uint32_t length, void** string) noexcept {
        typedef int32_t(__stdcall* FUNC)(wchar_t const* sourceString, uint32_t length, void** string);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "WindowsCreateString");
        return Func(sourceString, length, string);
    }
    int32_t __stdcall WINRT_WindowsCreateStringReference(wchar_t const* sourceString, uint32_t length, void* hstringHeader, void** string) noexcept {
        typedef int32_t(__stdcall* FUNC)(wchar_t const* sourceString, uint32_t length, void* hstringHeader, void** string);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "WindowsCreateStringReference");
        return Func(sourceString, length, hstringHeader, string);
    }
    int32_t __stdcall WINRT_WindowsDeleteString(void* string) noexcept {
        typedef int32_t(__stdcall* FUNC)(void* string);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "WindowsDeleteString");
        return Func(string);
    }
    int32_t __stdcall WINRT_WindowsPreallocateStringBuffer(uint32_t length, wchar_t** charBuffer, void** bufferHandle) noexcept {
        typedef int32_t(__stdcall* FUNC)(uint32_t length, wchar_t** charBuffer, void** bufferHandle);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "WindowsPreallocateStringBuffer");
        return Func(length, charBuffer, bufferHandle);
    }
    int32_t __stdcall WINRT_WindowsDeleteStringBuffer(void* bufferHandle) noexcept {
        typedef int32_t(__stdcall* FUNC)(void* bufferHandle);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "WindowsDeleteStringBuffer");
        return Func(bufferHandle);
    }
    int32_t __stdcall WINRT_WindowsPromoteStringBuffer(void* bufferHandle, void** string) noexcept {
        typedef int32_t(__stdcall* FUNC)(void* bufferHandle, void** string);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "WindowsPromoteStringBuffer");
        return Func(bufferHandle, string);
    }
    wchar_t const* __stdcall WINRT_WindowsGetStringRawBuffer(void* string, uint32_t* length) noexcept {
        typedef wchar_t const* (__stdcall* FUNC)(void* string, uint32_t* length);
        static FUNC Func = NULL;
        if (Func == NULL) Func = (FUNC)GetProcAddress(g_combase, "WindowsGetStringRawBuffer");
        return Func(string, length);
    }
}

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
        std::wstring error   = L"AddPackageByUriAsync failed (Errorcode: ";
        error += std::to_wstring(deployResult.ExtendedErrorCode());
        error += L"):\n";
        error += deployResult.ErrorText();

        return -1;
    }
    return 0;
}

int UnregisterSparsePackage(const std::wstring & sparsePackageName)
{

    winrt::Windows::Management::Deployment::PackageManager manager;
    winrt::Windows::Foundation::Collections::IIterable<winrt::Windows::ApplicationModel::Package> packages;
    try
    {
        packages = manager.FindPackagesForUser(L"");
    }
    catch (winrt::hresult_error const& ex)
    {
        std::wstring error   = L"FindPackagesForUser failed (Errorcode: ";
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
        std::wstring error   = L"RemovePackageAsync failed (Errorcode: ";
        error += std::to_wstring(deployResult.ExtendedErrorCode());
        error += L"):\n";
        error += deployResult.ErrorText();

        return -1;
    }   
    return 0;
}

int wmain(int argc, wchar_t* argv[])
{
    /*g_combase = LoadLibraryW(L"combase.dll");

    if (argc < 2)
    {
        //wchar_t PackageName[MAX_PATH];
        //UINT32 Length = MAX_PATH;
        //NTSTATUS status = GetCurrentPackageFullName(&Length, PackageName);
        //if (status >= 0)
        //    printf("%S", PackageName);
        return 0;
    }

    if (wcsicmp(argv[1], L"-install") == 0)
    {
        wchar_t path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        wchar_t* ptr = wcsrchr(path, L'\\');
        *ptr = L'\0';
        std::wstring sparseExtPath = path;
        wcscat(path, L"\\SbieShellPkg.msix");
        std::wstring sparsePackagePath = path;
        return RegisterSparsePackage(sparseExtPath, sparsePackagePath);
    }

    else if (wcsicmp(argv[1], L"-uninstall") == 0)
    {
        std::wstring sparsePackageName = L"SandboxieShell";
        return UnregisterSparsePackage(sparsePackageName);
    }*/

    HMODULE ext = LoadLibraryW(L"SbieShellExt.dll");

    typedef int (*FUNC)();
    if (wcsicmp(argv[1], L"-install") == 0)
    {
        FUNC func = (FUNC)GetProcAddress(ext, "RegisterPackage");
        func();
    }
    else if (wcsicmp(argv[1], L"-uninstall") == 0)
    {
        FUNC func = (FUNC)GetProcAddress(ext, "RemovePackage");
        func();
    }

    return 0;
}