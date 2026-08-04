/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC
 * Copyright 2020-2026 David Xanatos, xanasoft.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

//---------------------------------------------------------------------------
// About Sandboxie Dialog Box
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "MyApp.h"
#include "AboutDialog.h"

#include "SbieIni.h"
#include "common/my_version.h"
#include "apps/common/MyGdi.h"
#include "apps/common/RunBrowser.h"
#include "common/win32_ntddk.h"
#include "core/drv/api_defs.h"
#include "core/drv/verify.h"
#include <winhttp.h>
#include "common/json/JSON.h"


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CAboutDialog, CBaseDialog)

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


HBITMAP CAboutDialog::m_bitmap = NULL;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CAboutDialog::CAboutDialog(CWnd *pParentWnd)
    : CBaseDialog(pParentWnd, NULL)
{
    //
    // prepare image
    //

    if (! m_bitmap)
        m_bitmap = MyGdi_CreateFromResource(L"MASTHEADLOGO");

    //
    // display dialog
    //

    CString DialogTemplateName = L"ABOUT_DIALOG";
    SetDialogTemplate(DialogTemplateName);

    DoModal();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CAboutDialog::~CAboutDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CAboutDialog::OnInitDialog()
{
    CStatic *pic = (CStatic *)GetDlgItem(ID_ABOUT_LOGO);
    pic->SetBitmap(m_bitmap);

    CRect rc;
    GetClientRect(&rc);
    ULONG wDlg = rc.right - rc.left;

    pic->GetClientRect(&rc);
    ULONG wBtn = rc.right - rc.left;
    ULONG hBtn = rc.bottom - rc.top;

    pic->GetWindowRect(&rc);
    ScreenToClient((POINT *)&rc);
    rc.left = (wDlg - wBtn) / 2;
    pic->MoveWindow(rc.left, rc.top, wBtn, hBtn, TRUE);

    //
    //
    //

    pic->GetWindowRect(rc);
    ScreenToClient(rc);
    rc.left -= 5;
    rc.top -= 10;
    rc.right += 3 * 2;
    rc.bottom += 3 * 2;
    GetDlgItem(ID_ABOUT_FRAME)->MoveWindow(rc);

    CString text = CMyMsg(MSG_3601);
    SetWindowText(text);

    //
    //
    //

    const ULONG _bitness =
#ifdef _WIN64
                            64;
#else
                            32;
#endif _WIN64

    ULONG U_LRO, U_PDF;
    if (CMyApp::m_LayoutRTL) {
        U_LRO = 0x202D;   // Start of left-to-right override
        U_PDF = 0x202C;   // Pop directional formatting
    } else {
        U_LRO = L' ';
        U_PDF = L' ';
    }
    text.Format(L"%S %c(%d-bit)%c",
        MY_VERSION_STRING, U_LRO, _bitness, U_PDF);

    CString ver = CMyMsg(MSG_3302, text);
    GetDlgItem(ID_ABOUT_VERSION)->SetWindowText(ver);

    //
    //
    //

    text.Format(L"%S\r\n%S", MY_COPYRIGHT_STRING, MY_COPYRIGHT_STRING_OLD);
    GetDlgItem(ID_ABOUT_COPYRIGHT)->SetWindowText(text);

    __declspec(align(8)) SCertInfo CertInfo = { 0 };
    SbieApi_QueryDrvInfo(-1, &CertInfo, sizeof(CertInfo));
    if (CertInfo.active)
        GetDlgItem(ID_ABOUT_INFO)->SetWindowText(CMyMsg(MSG_7988));
    else if (CertInfo.expired) // expired && !active -> outdated
        GetDlgItem(ID_ABOUT_INFO)->SetWindowText(CMyMsg(MSG_7989));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));

    return TRUE;
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CAboutDialog::OnOK()
{
    EndDialog(0);
}


//---------------------------------------------------------------------------
// Serial Number Detection and Certificate Retrieval Helpers
//---------------------------------------------------------------------------


static BOOL IsSerialNumber(const WCHAR* text)
{
    if (!text)
        return FALSE;
    size_t len = wcslen(text);
    // Serial numbers start with "SBIE" and have at least 5 chars
    if (len < 5)
        return FALSE;
    return (_wcsnicmp(text, L"SBIE", 4) == 0);
}


static BOOL GetHWIDFromDriver(WCHAR* hwid, ULONG size)
{
    // Use info_class -2 to get HWID from driver
    LONG status = SbieApi_QueryDrvInfo((ULONG)-2, hwid, size);
    if (status != 0) {
        hwid[0] = L'\0';
        return FALSE;
    }
    return TRUE;
}


static CString ReadUpdateKeyFromCert()
{
    CString updateKey;

    WCHAR CertPath[MAX_PATH];
    SbieApi_GetHomePath(NULL, 0, CertPath, MAX_PATH);
    wcscat(CertPath, L"\\Certificate.dat");

    HANDLE hFile = CreateFile(CertPath, FILE_GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        char CertData[0x1000];
        DWORD bytesRead = 0;
        if (ReadFile(hFile, CertData, sizeof(CertData) - 1, &bytesRead, NULL)) {
            CertData[bytesRead] = 0;
            CString sCertData = CString(CertData);
            int pos = sCertData.Find(L"UPDATEKEY:");
            if (pos != -1) {
                pos += 10;
                int end = sCertData.Find(L"\n", pos);
                if (end == -1) end = sCertData.GetLength();
                updateKey = sCertData.Mid(pos, end - pos).Trim();
            }
        }
        CloseHandle(hFile);
    }

    return updateKey;
}


static BOOLEAN DownloadCertificateData(const WCHAR* Host, const WCHAR* Path, PSTR* pData, ULONG* pDataLength)
{
    BOOLEAN success = FALSE;

    PVOID SessionHandle = NULL;
    PVOID ConnectionHandle = NULL;
    PVOID RequestHandle = NULL;

    OSVERSIONINFOW osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOW);
    *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlGetVersion");
    if (RtlGetVersion == NULL || !NT_SUCCESS(RtlGetVersion(&osvi)))
        GetVersionExW(&osvi);

    {
        SessionHandle = WinHttpOpen(NULL,
            osvi.dwMajorVersion >= 8 ? WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY : WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (!SessionHandle)
            goto CleanupExit;

        if (osvi.dwMajorVersion >= 8) {
            ULONG Options = WINHTTP_DECOMPRESSION_FLAG_GZIP | WINHTTP_DECOMPRESSION_FLAG_DEFLATE;
            WinHttpSetOption(SessionHandle, WINHTTP_OPTION_DECOMPRESSION, &Options, sizeof(Options));
        }
    }

    {
        ConnectionHandle = WinHttpConnect(SessionHandle, Host, 443, 0); // ssl port
        if (!ConnectionHandle)
            goto CleanupExit;
    }

    {
        ULONG httpFlags = WINHTTP_FLAG_SECURE | WINHTTP_FLAG_REFRESH;
        RequestHandle = WinHttpOpenRequest(ConnectionHandle,
            NULL, Path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, httpFlags);

        if (!RequestHandle)
            goto CleanupExit;

        ULONG Options = WINHTTP_DISABLE_KEEP_ALIVE;
        WinHttpSetOption(RequestHandle, WINHTTP_OPTION_DISABLE_FEATURE, &Options, sizeof(Options));
    }

    if (!WinHttpSendRequest(RequestHandle, WINHTTP_NO_ADDITIONAL_HEADERS, 0, NULL, 0, 0, 0))
        goto CleanupExit;

    if (!WinHttpReceiveResponse(RequestHandle, NULL))
        goto CleanupExit;

    {
        ULONG allocatedLength;
        ULONG dataLength;
        ULONG returnLength;
        BYTE buffer[PAGE_SIZE];

        if (pData == NULL)
            goto CleanupExit;

        allocatedLength = sizeof(buffer);
        *pData = (PSTR)malloc(allocatedLength);
        dataLength = 0;

        while (WinHttpReadData(RequestHandle, buffer, PAGE_SIZE, &returnLength))
        {
            if (returnLength == 0)
                break;

            if (allocatedLength < dataLength + returnLength)
            {
                allocatedLength *= 2;
                *pData = (PSTR)realloc(*pData, allocatedLength);
            }

            memcpy(*pData + dataLength, buffer, returnLength);

            dataLength += returnLength;
        }

        if (allocatedLength < dataLength + 1)
        {
            allocatedLength++;
            *pData = (PSTR)realloc(*pData, allocatedLength);
        }

        // Ensure that the buffer is null-terminated.
        (*pData)[dataLength] = 0;

        if (pDataLength != NULL)
            *pDataLength = dataLength;
    }

    success = TRUE;

CleanupExit:
    if (RequestHandle)
        WinHttpCloseHandle(RequestHandle);
    if (ConnectionHandle)
        WinHttpCloseHandle(ConnectionHandle);
    if (SessionHandle)
        WinHttpCloseHandle(SessionHandle);

    return success;
}


static std::wstring GetJSONStringSafe(const JSONObject& root, const std::wstring& key)
{
    auto I = root.find(key);
    if (I == root.end() || !I->second->IsString())
        return L"";
    return I->second->AsString();
}


static BOOL GetCertificateFromServer(const CString& serial, CString& outCert, CString& outError)
{
    // Build query path
    CString path = L"/get_cert.php?SN=" + serial;

    // Check for node-locked serial (5th char is 'N')
    if (serial.GetLength() > 5 && towupper(serial[4]) == L'N') {
        WCHAR hwid[40] = { 0 };
        if (GetHWIDFromDriver(hwid, sizeof(hwid))) {
            path += L"&HwId=" + CString(hwid);
        }
    }

    // Check for renewal/upgrade serial (5th char is 'R' or 'U')
    if (serial.GetLength() > 5) {
        WCHAR type = towupper(serial[4]);
        if (type == L'R' || type == L'U') {
            CString updateKey = ReadUpdateKeyFromCert();
            if (updateKey.IsEmpty()) {
                outError = CMyMsg(type == L'U' ? MSG_7994 : MSG_7995);
                return FALSE;
            }
            path += L"&UpdateKey=" + updateKey;
        }
    }

    // Add hash key (same as Updater.cpp)
    CString Section, UserName;
    BOOL IsAdmin;
    CSbieIni::GetInstance().GetUser(Section, UserName, IsAdmin);
    DWORD Hash = wcstoul(Section.Mid(13), NULL, 16);

    ULONGLONG RandID = 0;
    SbieApi_Call(API_GET_SECURE_PARAM, 3, L"RandID", (ULONG_PTR)&RandID, sizeof(RandID));
    if (RandID == 0) {
        srand(GetTickCount());
        RandID = (ULONGLONG)(rand() & 0xFFFF) | ((ULONGLONG)(rand() & 0xFFFF) << 16) |
            ((ULONGLONG)(rand() & 0xFFFF) << 32) | ((ULONGLONG)(rand() & 0xFFFF) << 48);
        SbieApi_Call(API_SET_SECURE_PARAM, 3, L"RandID", (ULONG_PTR)&RandID, sizeof(RandID));
    }

    wchar_t hashKey[26];
    wsprintf(hashKey, L"%08X-%08X%08X", Hash, (DWORD)(RandID >> 32), (DWORD)RandID);
    path += L"&HashKey=" + CString(hashKey);

    // Make HTTP request
    char* data = NULL;
    if (!DownloadCertificateData(L"sandboxie-plus.com", path, &data, NULL)) {
        outError = CMyMsg(MSG_7997);
        return FALSE;
    }

    if (!data || !*data) {
        outError = CMyMsg(MSG_7998);
        if (data) free(data);
        return FALSE;
    }

    // Check for JSON error response
    if (data[0] == '{') {
        JSONValue* jsonObject = JSON::Parse(data);
        if (jsonObject && jsonObject->IsObject()) {
            JSONObject root = jsonObject->AsObject();
            std::wstring errorMsg = GetJSONStringSafe(root, L"errorMsg");
            if (!errorMsg.empty())
                outError = errorMsg.c_str();
            else
                outError = CMyMsg(MSG_7999);
            delete jsonObject;
        }
        else {
            outError = CMyMsg(MSG_7999);
        }
        free(data);
        return FALSE;
    }

    // Success - convert to wide string
    int len = MultiByteToWideChar(CP_UTF8, 0, data, -1, NULL, 0);
    WCHAR* wdata = new WCHAR[len];
    MultiByteToWideChar(CP_UTF8, 0, data, -1, wdata, len);
    outCert = wdata;
    delete[] wdata;
    free(data);

    return TRUE;
}


//---------------------------------------------------------------------------
// ApplyCertificate
//---------------------------------------------------------------------------


void ApplyCertificate()
{
    if (CMyApp::MsgBox(NULL, MSG_7990, MB_OKCANCEL) != IDOK)
        return;

    CString clipboardText;
    CString certificateText;
    BOOL bIsSerial = FALSE;

    // Read clipboard content
    if (OpenClipboard(nullptr)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData != nullptr) {
            WCHAR* pszText = static_cast<WCHAR*>(GlobalLock(hData));
            if (pszText != nullptr) {
                clipboardText = pszText;
                clipboardText.Trim();
                GlobalUnlock(hData);
            }
        }
        CloseClipboard();
    }

    if (clipboardText.IsEmpty()) {
        CMyApp::MsgBox(NULL, MSG_7991, MB_OK | MB_ICONERROR);
        return;
    }

    // Check if clipboard contains a serial number
    if (IsSerialNumber(clipboardText)) {
        bIsSerial = TRUE;

        // Check for renewal/upgrade without existing certificate
        if (clipboardText.GetLength() > 5) {
            WCHAR type = towupper(clipboardText[4]);
            if (type == L'U' || type == L'R') {
                CString existingUpdateKey = ReadUpdateKeyFromCert();
                if (existingUpdateKey.IsEmpty()) {
                    CMyApp::MsgBox(NULL, type == L'U' ? MSG_7994 : MSG_7995, MB_OK | MB_ICONERROR);
                    return;
                }
            }
        }

        // Retrieve certificate from server
        CString serverError;
        if (!GetCertificateFromServer(clipboardText, certificateText, serverError)) {
            CMyApp::MsgBox(NULL, CMyMsg(MSG_7996, serverError), MB_OK | MB_ICONERROR);
            return;
        }
    }
    else {
        // Use clipboard content as certificate directly
        certificateText = clipboardText;
    }

    // Write certificate to temp file
    WCHAR CertPath[MAX_PATH];
    GetTempPath(MAX_PATH, CertPath);
    wcscat(CertPath, L"Sbie+Certificate.dat");

    ULONG lenWritten = 0;
    HANDLE hFile = CreateFile(CertPath, FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        ULONG utf8_len = WideCharToMultiByte(CP_UTF8, 0, certificateText, certificateText.GetLength(), NULL, 0, NULL, NULL);
        char* text_utf8 = (char*)HeapAlloc(GetProcessHeap(), 0, utf8_len);
        if (text_utf8) {
            ULONG lenToWrite = WideCharToMultiByte(CP_UTF8, 0, certificateText, certificateText.GetLength(), text_utf8, utf8_len, NULL, NULL);
            if (!WriteFile(hFile, (void*)text_utf8, lenToWrite, &lenWritten, NULL))
                lenWritten = 0;
            HeapFree(GetProcessHeap(), 0, text_utf8);
        }
        CloseHandle(hFile);
    }

    if (lenWritten == 0) {
        CMyApp::MsgBox(NULL, MSG_7991, MB_OK | MB_ICONERROR);
        return;
    }

    WCHAR HomePath[MAX_PATH];
    SbieApi_GetHomePath(NULL, 0, HomePath, MAX_PATH);
    wcscat(HomePath, L"\\Certificate.dat");

    SHFILEOPSTRUCT SHFileOp;
    memset(&SHFileOp, 0, sizeof(SHFILEOPSTRUCT));
    SHFileOp.hwnd = NULL;
    SHFileOp.wFunc = FO_MOVE; // FO_DELETE;
	SHFileOp.pFrom = CertPath;
    SHFileOp.pTo = HomePath;
    SHFileOp.fFlags = NULL;    
    SHFileOperation(&SHFileOp);

    NTSTATUS status = SbieApi_Call(API_RELOAD_CONF, 2, -1, SBIE_CONF_FLAG_RELOAD_CERT);
    if (!NT_SUCCESS(status)) {
        CMyApp::MsgBox(NULL, MSG_7992, MB_OK | MB_ICONWARNING);
        return;
    }

    CMyApp::MsgBox(NULL, MSG_7993, MB_OK | MB_ICONINFORMATION);
}

