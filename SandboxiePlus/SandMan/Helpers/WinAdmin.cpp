#include "stdafx.h"
#include "WinAdmin.h"

#include <windows.h>
#include <codecvt>
#include <Shlwapi.h>

bool IsElevated()
{
    bool fRet = false;
    HANDLE hToken = NULL;
    if(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken))
	{
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if(GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof( Elevation ), &cbSize)) 
            fRet = Elevation.TokenIsElevated;
    }
    if(hToken) 
        CloseHandle(hToken);
    return fRet;
}

int RunElevated(const std::wstring& Params, bool bGetCode)
{
	// Launch itself as admin
	wchar_t szPath[MAX_PATH];
	if (!GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		return -104;
	return RunElevated(std::wstring(szPath), Params, bGetCode ? 10000 : 0);
}

int RunElevated(const std::wstring& binaryPath, const std::wstring& Params, quint32 uTimeOut)
{
	SHELLEXECUTEINFO sei = { sizeof(sei) };
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.lpVerb = L"runas";
	sei.lpFile = binaryPath.c_str();
	sei.lpParameters = Params.c_str();
	sei.hwnd = NULL;
	sei.nShow = SW_NORMAL;

	if (!ShellExecuteEx(&sei))
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_CANCELLED)
			return -102; // The user refused to allow privileges elevation.
		return -101;
	}
	else
	{
		DWORD ExitCode = 0;
		BOOL success = TRUE;
		if (uTimeOut)
		{
			WaitForSingleObject(sei.hProcess, uTimeOut);
			success = GetExitCodeProcess(sei.hProcess, &ExitCode);
		}
		CloseHandle(sei.hProcess);
		return success ? ExitCode : STATUS_PENDING;
	}
}

int RestartElevated(int &argc, char **argv)
{
	std::wstring Params;
	for (int i = 1; i < argc; i++)
	{
		if (i > 1)
			Params.append(L" ");
		Params.append(L"\"" + std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(argv[i]) + L"\"");
	}
	return RunElevated(Params);
}

bool IsAdminUser(bool OnlyFull)
{
	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), MAXIMUM_ALLOWED, &hToken))
		return false;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    BOOL bRet = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
	if (bRet) {
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &bRet))
			bRet = FALSE;
		FreeSid(AdministratorsGroup);
		if (!bRet || OnlyFull) {
			OSVERSIONINFO osvi;
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			if (GetVersionEx(&osvi) && osvi.dwMajorVersion >= 6) {
				ULONG elevationType, len;
				bRet = GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)TokenElevationType, &elevationType, sizeof(elevationType), &len);
				if (bRet && (elevationType != TokenElevationTypeFull && (OnlyFull || elevationType != TokenElevationTypeLimited)))
					bRet = FALSE;
			}
		}
	}

	CloseHandle(hToken);

    return !!bRet;
}

//////////////////////////////////////////////////////////////////////////////////
// AutoRun

#define APP_NAME L"SandboxiePlus"

#define AUTO_RUN_KEY_NAME APP_NAME L"_AutoRun"

bool IsAutorunEnabled()
{
	bool result = false;

	HKEY hkey = nullptr;
	if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hkey))
	{
		// First, determine the required buffer size, including NUL terminator (in bytes). RegGetValue() always adds
		// an extra NUL terminator to size, even if one already exists, in case the stored value doesn't have one.
		DWORD size {0};
		if (ERROR_SUCCESS == RegGetValue(hkey, nullptr, AUTO_RUN_KEY_NAME, RRF_RT_REG_SZ, nullptr, nullptr, &size))
		{
			// Then, allocate the buffer (in WCHARs) and retrieve the auto-run value. If successful, the size
			// variable will be set to the actual size, without the extra NUL terminator.
			auto buffer = std::make_unique< WCHAR[] >(size / sizeof(WCHAR));
			if (ERROR_SUCCESS == RegGetValue(hkey, nullptr, AUTO_RUN_KEY_NAME, RRF_RT_REG_SZ, nullptr, reinterpret_cast<LPBYTE>(buffer.get()), &size))
			{
				result = true; // todo: check path
			}
		}

		RegCloseKey (hkey);
	}

	return result;
}

bool AutorunEnable (bool is_enable)
{
	bool result = false;

	HKEY hkey = nullptr;
	if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hkey))
	{
		if (is_enable)
		{
			constexpr size_t MAX_PATH_EX = 32767; // Long file path max length, in characters
			auto szPath = std::make_unique< WCHAR[] >(MAX_PATH_EX);
			if (GetModuleFileName(NULL, szPath.get(), MAX_PATH_EX))
			{
				const std::wstring path = L"\"" + std::wstring(szPath.get()) + L"\" -autorun";

				result = (ERROR_SUCCESS == RegSetValueEx(hkey, AUTO_RUN_KEY_NAME, 0, REG_SZ, reinterpret_cast<const BYTE*>(path.c_str()), static_cast<DWORD>((path.length() + 1) * sizeof(WCHAR))));
			}
		}
		else
		{
			result = (ERROR_SUCCESS == RegDeleteValue (hkey, AUTO_RUN_KEY_NAME));
		}

		RegCloseKey (hkey);
	}

	return false;
}