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
	wchar_t szPath[MAX_PATH];
	if (!GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
		return -3;
	return RunElevated(std::wstring(szPath), Params, bGetCode);
}

int RunElevated(const std::wstring& binaryPath, const std::wstring& Params, bool bGetCode)
{
	// Launch itself as admin
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
			return -2; // The user refused to allow privileges elevation.
	}
	else
	{
		if (bGetCode)
		{
			WaitForSingleObject(sei.hProcess, 10000);
			DWORD ExitCode = -4;
			BOOL success = GetExitCodeProcess(sei.hProcess, &ExitCode);
			CloseHandle(sei.hProcess);
			return success ? ExitCode : -4;
		}
		return 0;
	}
	return -1;
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
	if (RegOpenKeyEx (HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hkey) == ERROR_SUCCESS)
	{
		WCHAR buffer[MAX_PATH] = {0};
		DWORD size = _countof (buffer);

		if (RegQueryValueEx (hkey, AUTO_RUN_KEY_NAME, nullptr, nullptr, (LPBYTE)buffer, &size) == ERROR_SUCCESS)
		{
			result = true; // todo: check path
		}

		RegCloseKey (hkey);
	}

	return result;
}

bool AutorunEnable (bool is_enable)
{
	bool result = false;

	HKEY hkey = nullptr;
	if (RegOpenKeyEx (HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hkey) == ERROR_SUCCESS)
	{
		if (is_enable)
		{
			wchar_t szPath[MAX_PATH];
			if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
			{
				std::wstring path = L"\"" + std::wstring(szPath) + L"\" -autorun";

				result = (RegSetValueEx(hkey, AUTO_RUN_KEY_NAME, 0, REG_SZ, (LPBYTE)path.c_str(), DWORD((path.length() + 1) * sizeof(WCHAR))) == ERROR_SUCCESS);
			}
		}
		else
		{
			result = (RegDeleteValue (hkey, AUTO_RUN_KEY_NAME) == ERROR_SUCCESS);
		}

		RegCloseKey (hkey);
	}

	return false;
}