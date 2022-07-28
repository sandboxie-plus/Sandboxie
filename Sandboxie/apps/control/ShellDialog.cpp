/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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
// Shell Integration Dialog Box
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "ShellDialog.h"

#include "UserSettings.h"
#include "Boxes.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const CString _EnableLogonStart(L"EnableLogonStart");
static const CString _EnableAutoStart(L"EnableAutoStart");
static const CString _AddDesktopIcon(L"AddDesktopIcon");
static const CString _AddQuickLaunchIcon(L"AddQuickLaunchIcon");
static const CString _AddContextMenu(L"AddContextMenu");
static const CString _AddSendToMenu(L"AddSendToMenu");

extern const WCHAR *_ShortcutNotify;

static const CString _QuickLaunch(
    L"\\Microsoft\\Internet Explorer\\Quick Launch");

static const WCHAR *_classes = L"software\\classes\\";
static const WCHAR *_shell = L"\\shell";
static const WCHAR *_sandbox = L"\\" SANDBOX_VERB;
static const WCHAR *_command = L"\\command";

BOOL CShellDialog::m_SpawnedMenu = FALSE;


//---------------------------------------------------------------------------
// Message Map
//---------------------------------------------------------------------------


BEGIN_MESSAGE_MAP(CShellDialog, CBaseDialog)

    ON_COMMAND(ID_SHELL_SHORTCUTS,      OnShortcuts)
    ON_COMMAND(IDCANCEL,                OnCloseOrCancel)
    ON_WM_CLOSE()

END_MESSAGE_MAP()


//---------------------------------------------------------------------------
// Shell Exports
//---------------------------------------------------------------------------


#if _MSC_VER == 1200        // Visual C++ 6.0


#define SHGFP_TYPE_CURRENT 0

typedef HRESULT (*P_SHGetFolderPath)(
    HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);

P_SHGetFolderPath SHGetFolderPathW;


#endif


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CShellDialog::CShellDialog(CWnd *pParentWnd)
    : CBaseDialog(pParentWnd, L"SHELL_DIALOG")
{
    DoModal();

    Sync();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CShellDialog::~CShellDialog()
{
}


//---------------------------------------------------------------------------
// OnInitDialog
//---------------------------------------------------------------------------


BOOL CShellDialog::OnInitDialog()
{
    SetWindowText(CMyMsg(MSG_3685));

    GetDlgItem(ID_SHELL_GROUP_1)->SetWindowText(CMyMsg(MSG_3686));
    GetDlgItem(ID_SHELL_RUNLOGON)->SetWindowText(CMyMsg(MSG_3687));
    GetDlgItem(ID_SHELL_RUNBOXED)->SetWindowText(CMyMsg(MSG_3688));

    GetDlgItem(ID_SHELL_GROUP_2)->SetWindowText(CMyMsg(MSG_3689));
    GetDlgItem(ID_SHELL_DESKTOP)->SetWindowText(CMyMsg(MSG_3690));
    GetDlgItem(ID_SHELL_QUICKLAUNCH)->SetWindowText(CMyMsg(MSG_3691));
    GetDlgItem(ID_SHELL_LABEL_1)->SetWindowText(CMyMsg(MSG_3692));
    GetDlgItem(ID_SHELL_SHORTCUTS)->SetWindowText(CMyMsg(MSG_3693));

    GetDlgItem(ID_SHELL_GROUP_3)->SetWindowText(CMyMsg(MSG_3694));
    GetDlgItem(ID_SHELL_CONTEXT)->SetWindowText(CMyMsg(MSG_3695));
    GetDlgItem(ID_SHELL_SENDTO)->SetWindowText(CMyMsg(MSG_3696));

    GetDlgItem(IDOK)->SetWindowText(CMyMsg(MSG_3001));
    GetDlgItem(IDCANCEL)->SetWindowText(CMyMsg(MSG_3002));

    BOOL logonstart, autostart;
    BOOL desktop, quicklaunch;
    BOOL contextmenu, sendtomenu;

    CUserSettings &user = CUserSettings::GetInstance();
    user.GetBool(_EnableLogonStart,     logonstart,     TRUE);
    user.GetBool(_EnableAutoStart,      autostart,      TRUE);
    user.GetBool(_AddDesktopIcon,       desktop,        TRUE);
    user.GetBool(_AddQuickLaunchIcon,   quicklaunch,    TRUE);
    user.GetBool(_AddContextMenu,       contextmenu,    TRUE);
    user.GetBool(_AddSendToMenu,        sendtomenu,     TRUE);

    if (logonstart)
        ((CButton *)GetDlgItem(ID_SHELL_RUNLOGON))->SetCheck(BST_CHECKED);
    if (autostart)
        ((CButton *)GetDlgItem(ID_SHELL_RUNBOXED))->SetCheck(BST_CHECKED);
    if (desktop)
        ((CButton *)GetDlgItem(ID_SHELL_DESKTOP))->SetCheck(BST_CHECKED);
    if (quicklaunch)
        ((CButton *)GetDlgItem(ID_SHELL_QUICKLAUNCH))->SetCheck(BST_CHECKED);
    if (contextmenu)
        ((CButton *)GetDlgItem(ID_SHELL_CONTEXT))->SetCheck(BST_CHECKED);
    if (sendtomenu)
        ((CButton *)GetDlgItem(ID_SHELL_SENDTO))->SetCheck(BST_CHECKED);

    return TRUE;
}


//---------------------------------------------------------------------------
// OnOK
//---------------------------------------------------------------------------


void CShellDialog::OnOK()
{
    if (m_SpawnedMenu)
        return;

    BOOL logonstart, autostart;
    BOOL desktop, quicklaunch;
    BOOL contextmenu, sendtomenu;

    CButton *pButton;

    pButton = (CButton *)GetDlgItem(ID_SHELL_RUNLOGON);
    logonstart = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

    pButton = (CButton *)GetDlgItem(ID_SHELL_RUNBOXED);
    autostart = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

    pButton = (CButton *)GetDlgItem(ID_SHELL_DESKTOP);
    desktop = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

    pButton = (CButton *)GetDlgItem(ID_SHELL_QUICKLAUNCH);
    quicklaunch = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

    pButton = (CButton *)GetDlgItem(ID_SHELL_RUNLOGON);
    logonstart = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

    pButton = (CButton *)GetDlgItem(ID_SHELL_RUNBOXED);
    autostart = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

    pButton = (CButton *)GetDlgItem(ID_SHELL_CONTEXT);
    contextmenu = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

    pButton = (CButton *)GetDlgItem(ID_SHELL_SENDTO);
    sendtomenu = (pButton->GetCheck() == BST_CHECKED) ? TRUE : FALSE;

    //
    // record updated settings
    //

    CUserSettings &user = CUserSettings::GetInstance();
    user.SetBool(_EnableLogonStart,     logonstart);
    user.SetBool(_EnableAutoStart,      autostart);
    user.SetBool(_AddDesktopIcon,       desktop);
    user.SetBool(_AddQuickLaunchIcon,   quicklaunch);
    user.SetBool(_AddContextMenu,       contextmenu);
    user.SetBool(_AddSendToMenu,        sendtomenu);

    EndDialog(0);
}


//---------------------------------------------------------------------------
// OnShortcuts
//---------------------------------------------------------------------------


void CShellDialog::OnShortcuts()
{
    if (m_SpawnedMenu)
        return;

    m_SpawnedMenu = TRUE;

    CUserSettings &settings = CUserSettings::GetInstance();
    BOOL warn;
    settings.GetBool(_ShortcutNotify, warn, TRUE);
    if (warn) {
        int rv = CMyApp::MsgCheckBox(this, MSG_3697, 0, MB_OK);
        if (rv < 0)
            settings.SetBool(_ShortcutNotify, FALSE);
    }

    CString BoxName, LinkPath, IconPath, WorkDir;
    ULONG IconIndex;
    if (GetStartMenuShortcut(
                        BoxName, LinkPath, IconPath, IconIndex, WorkDir))
        CreateDesktopShortcut(
                        BoxName, LinkPath, IconPath, IconIndex, WorkDir);

    m_SpawnedMenu = FALSE;
}


//---------------------------------------------------------------------------
// OnCloseOrCancel
//---------------------------------------------------------------------------


void CShellDialog::OnCloseOrCancel()
{
    if (! m_SpawnedMenu) {
        const MSG *msg = GetCurrentMessage();
        if (msg->message == WM_COMMAND)
            CDialog::OnCancel();
        else if (msg->message == WM_CLOSE)
            CDialog::OnClose();
    }
}


//---------------------------------------------------------------------------
// Sync
//---------------------------------------------------------------------------


void CShellDialog::Sync(BOOL bUninstall)
{
    //
    // initialize Shell exports (Visual C++ 6.0)
    //

#if _MSC_VER == 1200

    if (! SHGetFolderPathW) {
        HMODULE shell = GetModuleHandle(L"shell32.dll");
        SHGetFolderPathW =
            (P_SHGetFolderPath)GetProcAddress(shell, "SHGetFolderPathW");
    }

#endif

    //
    // sync
    //

    SyncRunLogon(bUninstall);
    SyncBrowserIcon(_AddDesktopIcon,     CSIDL_DESKTOPDIRECTORY, CString(), bUninstall);
    SyncBrowserIcon(_AddQuickLaunchIcon, CSIDL_APPDATA,       _QuickLaunch, bUninstall);
    SyncContextMenu(bUninstall);
    SyncSendToMenu(bUninstall);
}


//---------------------------------------------------------------------------
// SyncRunLogon
//---------------------------------------------------------------------------


void CShellDialog::SyncRunLogon(BOOL bUninstall)
{
    static const WCHAR *_RunKey =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    static const WCHAR *_RunValue = SANDBOXIE_CONTROL;

    //
    // query ini setting and actual registry information
    //

    BOOL ini = FALSE;
    if(!bUninstall)
        CUserSettings::GetInstance().GetBool(_EnableLogonStart, ini, TRUE);

    HKEY hkey;
    LONG rc = RegOpenKeyEx(HKEY_CURRENT_USER, _RunKey, 0, KEY_READ, &hkey);
    if (rc != 0)
        return;

    ULONG type;
    WCHAR path[512];
    ULONG path_len = sizeof(path) - sizeof(WCHAR) * 4;
    rc = RegQueryValueEx(
        hkey, _RunValue, NULL, &type, (BYTE *)path, &path_len);
    RegCloseKey(hkey);

    BOOL reg = FALSE;
    if (rc == 0 && type == REG_SZ && path_len)
        reg = TRUE;

    //
    // sync according to ini setting
    //

    if (ini && reg)
        return;
    if ((! ini) && (! reg))
        return;

    rc = RegOpenKeyEx(HKEY_CURRENT_USER, _RunKey, 0, KEY_WRITE, &hkey);
    if (rc != 0)
        return;

    if (ini) {

        STARTUPINFO si;
        if (SbieDll_RunFromHome(SBIECTRL_EXE, NULL, &si, NULL)) {
            WCHAR *path2 = (WCHAR *)si.lpReserved;
            wcscpy(path, path2);
            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, path2);
        } else
            GetModuleFileName(NULL, path, sizeof(path) / sizeof(WCHAR) - 4);
        path_len = (wcslen(path) + 1) * sizeof(WCHAR);

        RegSetValueEx(hkey, _RunValue, NULL, REG_SZ, (BYTE *)path, path_len);

    } else {

        RegDeleteValue(hkey, _RunValue);
    }

    RegCloseKey(hkey);
}


//---------------------------------------------------------------------------
// SyncContextMenu
//---------------------------------------------------------------------------


void CShellDialog::SyncContextMenu(BOOL bUninstall)
{
    static const WCHAR *_star   = L"*";
    static const WCHAR *_folder = L"Folder";

    //
    // query ini setting and actual registry information
    //

    BOOL ini = FALSE;
    if(!bUninstall)
        CUserSettings::GetInstance().GetBool(_AddContextMenu, ini, TRUE);
    BOOL reg = FALSE;

    ULONG type;
    WCHAR path[512];
    ULONG path_len;

    HKEY hkey;
    LONG rc;

    //
    // open HKCU\Software\Classes\*\shell\sandbox\command.
    // if missing, then reg remains FALSE, and we may need to sync
    //

    wcscpy(path, _classes);
    wcscat(path, _star);
    wcscat(path, _shell);
    wcscat(path, _sandbox);
    wcscat(path, _command);

    rc = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_READ, &hkey);
    if (rc != 0)
        goto reg_done;

    path_len = sizeof(path) - sizeof(WCHAR) * 4;
    rc = RegQueryValueEx(hkey, NULL, NULL, &type, (BYTE *)path, &path_len);
    RegCloseKey(hkey);
    if (rc != 0)
        goto reg_done;

    //
    // open HKCU\Software\Classes\Folder\shell\sandbox\command.
    // if missing, then reg remains FALSE, and we may need to sync
    //

    wcscpy(path, _classes);
    wcscat(path, _folder);
    wcscat(path, _shell);
    wcscat(path, _sandbox);
    wcscat(path, _command);

    rc = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_READ, &hkey);
    if (rc != 0)
        goto reg_done;

    path_len = sizeof(path) - sizeof(WCHAR) * 4;
    rc = RegQueryValueEx(hkey, NULL, NULL, &type, (BYTE *)path, &path_len);
    RegCloseKey(hkey);
    if (rc != 0)
        goto reg_done;

    //
    // both shell keys were found so set reg TRUE
    //

    reg = TRUE;

reg_done:

    //
    // sync according to ini setting
    //

    if (ini && reg)
        return;
    if ((! ini) && (! reg))
        return;

    if (ini) {

        CreateAssoc(path, _star);
        CreateAssoc(path, _folder);

    } else {

        DeleteAssoc(path, _star);
        DeleteAssoc(path, _folder);
    }
}


//---------------------------------------------------------------------------
// CreateAssoc
//---------------------------------------------------------------------------


void CShellDialog::CreateAssoc(WCHAR *path, const WCHAR *classname)
{
    static const WCHAR *boxask = L"/box:__ask__ ";
    HKEY hkey;
    LONG rc;
    ULONG path_len;
    STARTUPINFO si;

    //
    // open/create class key:
    // HKCU\Software\Classes\{*|Folder}
    //

    wcscpy(path, _classes);
    wcscat(path, classname);

    rc = RegCreateKeyEx(HKEY_CURRENT_USER, path,
                        0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (rc != 0)
        return;

    RegCloseKey(hkey);

    //
    // open/create shell key:
    // HKCU\Software\Classes\{*|Folder}\shell
    //

    wcscat(path, _shell);

    rc = RegCreateKeyEx(HKEY_CURRENT_USER, path,
                        0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (rc != 0)
        return;

    RegCloseKey(hkey);

    //
    // open/create sandbox subkey:
    // HKCU\Software\Classes\{*|Folder}\shell\sandbox
    //
    // set title "Run Sandboxed" and Start.exe icon for shell key
    //

    wcscat(path, _sandbox);

    rc = RegCreateKeyEx(HKEY_CURRENT_USER, path,
                        0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (rc != 0)
        return;

    wcscpy(path, CMyMsg(MSG_3699));
    path_len = (wcslen(path) + 1) * sizeof(WCHAR);

    rc = RegSetValueEx(hkey, NULL, 0, REG_SZ, (BYTE *)path, path_len);

    if (rc == 0 && SbieDll_RunFromHome(START_EXE, NULL, &si, NULL)) {

        WCHAR *path2 = (WCHAR *)si.lpReserved;

        wcscpy(path, path2);
        path_len = (wcslen(path) + 1) * sizeof(WCHAR);

        rc = RegSetValueEx(hkey, L"Icon", 0, REG_SZ, (BYTE *)path, path_len);

        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, path2);
    }

    RegCloseKey(hkey);

    if (rc != 0)
        return;

    //
    // open/create command subkey:
    // HKCU\Software\Classes\{*|Folder}\shell\sandbox\command
    //

    wcscpy(path, _classes);
    wcscat(path, classname);
    wcscat(path, _shell);
    wcscat(path, _sandbox);
    wcscat(path, _command);

    rc = RegCreateKeyEx(HKEY_CURRENT_USER, path,
                        0, NULL, 0, KEY_WRITE, NULL, &hkey, NULL);

    if (rc != 0)
        return;

    //
    // prepare Start.exe command line
    //

    if (classname[0] == L'*') {

        //
        // for files:  Start.exe /box:__ask__ "%1" %*
        //

        wcscpy(path, boxask);
        wcscat(path, L"\"%1\" %*");

    } else {

        static const WCHAR *explorer = L"explorer.exe";

        //
        // for folders:  get shell program
        //

        HKEY hkeyWinlogon;
        static const WCHAR *pathToWinlogon =
            L"software\\microsoft\\windows nt\\currentversion\\winlogon";

        path[0] = L'\0';

        rc = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE, pathToWinlogon,
                0, KEY_READ, &hkeyWinlogon);

        if (rc == 0) {

            ULONG type;
            path_len = MAX_PATH * sizeof(WCHAR);
            memzero(path, path_len + sizeof(WCHAR));

            rc = RegQueryValueEx(
                    hkeyWinlogon, L"Shell", NULL,
                    &type, (BYTE *)path, &path_len);

            if (rc != 0 || (type != REG_SZ && type != REG_EXPAND_SZ))
                path[0] = L'\0';
            else if (_wcsicmp(path, explorer) == 0)
                path[0] = L'\0';

            RegCloseKey(hkeyWinlogon);
        }

        if (! path[0]) {

            //
            // if we don't know the shell program or if it is the
            // the default explorer.exe, then get a full path
            //

            path[0] = L'\"';
            GetWindowsDirectory(&path[1], MAX_PATH);
            path_len = wcslen(path);
            if (path_len >= 1 && path[path_len - 1] != L'\\') {
                path[path_len] = L'\\';
                ++path_len;
            }
            wcscpy(path + path_len, explorer);
            path_len = wcslen(path);
            path[path_len] = L'\"';
            path[path_len + 1] = L'\0';
        }

        //
        // command line:  Start.exe /box:__ask__ explorer.exe "%1"
        //

        wmemmove(path + wcslen(boxask), path, wcslen(path));
        wmemcpy(path, boxask, wcslen(boxask));
        wcscat(path, L" \"%1\"");
    }

    //
    // write Start.exe command line into command key
    //

    if (SbieDll_RunFromHome(START_EXE, path, &si, NULL)) {

        WCHAR *path2 = (WCHAR *)si.lpReserved;

        wcscpy(path, path2);
        path_len = (wcslen(path) + 1) * sizeof(WCHAR);

        RegSetValueEx(hkey, NULL, 0, REG_SZ, (BYTE *)path, path_len);

        HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, path2);
    }

    //
    // close command key
    //

    RegCloseKey(hkey);
}


//---------------------------------------------------------------------------
// DeleteAssoc
//---------------------------------------------------------------------------


void CShellDialog::DeleteAssoc(WCHAR *path, const WCHAR *classname)
{
    HKEY hkeyParent;
    LONG rc;

    //
    // delete command subkey:
    // HKCU\Software\Classes\{*|Folder}\shell\sandbox\command
    //

    wcscpy(path, _classes);
    wcscat(path, classname);
    wcscat(path, _shell);
    wcscat(path, _sandbox);

    rc = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_WRITE, &hkeyParent);
    if (rc != 0)
        return;
    rc = RegDeleteKey(hkeyParent, _command + 1);
    RegCloseKey(hkeyParent);
    if (rc != 0)
        return;

    //
    // delete sandbox subkey:
    // HKCU\Software\Classes\{*|Folder}\shell\sandbox
    //

    *wcsrchr(path, L'\\') = L'\0';

    rc = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_WRITE, &hkeyParent);
    if (rc != 0)
        return;
    rc = RegDeleteKey(hkeyParent, _sandbox + 1);
    RegCloseKey(hkeyParent);
}


//---------------------------------------------------------------------------
// SyncBrowserIcon
//---------------------------------------------------------------------------


void CShellDialog::SyncBrowserIcon(
    const CString &setting, int nFolder, const CString &subdir, BOOL bUninstall)
{
    //
    // query ini setting and actual filesystem information
    //

    BOOL ini = FALSE;
    if(!bUninstall)
        CUserSettings::GetInstance().GetBool(setting, ini, TRUE);

    WCHAR path[512];
    HRESULT hr = SHGetFolderPathW(
        NULL, nFolder, NULL, SHGFP_TYPE_CURRENT, path);
    if (hr != 0 || (! path[0]))
        return;
    if (path[wcslen(path) - 1] != L'\\')
        wcscat(path, L"\\");
    if (! subdir.IsEmpty()) {
        wcscat(path, subdir);
        wcscat(path, L"\\");
    }
    wcscat(path, CMyMsg(MSG_3698));

    BOOL fs = FALSE;
    if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
        fs = TRUE;

    //
    // sync according to ini setting
    //

    if (ini && fs)
        return;
    if ((! ini) && (! fs))
        return;

    if (ini) {

        STARTUPINFO si;
        if (SbieDll_RunFromHome(L"QuickLaunch.lnk", NULL, &si, NULL)) {
            WCHAR *path2 = (WCHAR *)si.lpReserved;
            CopyFile(path2, path, FALSE);
            HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, path2);
        }

    } else {

        DeleteFile(path);
    }
}


//---------------------------------------------------------------------------
// SyncSendToMenu
//---------------------------------------------------------------------------


void CShellDialog::SyncSendToMenu(BOOL bUninstall)
{
    if (CMyApp::m_WindowsVista) {
        SyncSendToMenuVista();
        return;
    }

    BOOL ini = FALSE;
    if(!bUninstall)
        CUserSettings::GetInstance().GetBool(_AddSendToMenu, ini, TRUE);

    WCHAR path[512];
    HRESULT hr = SHGetFolderPathW(
        NULL, CSIDL_SENDTO, NULL, SHGFP_TYPE_CURRENT, path);
    if (hr != 0 || (! path[0]))
        return;
    if (path[wcslen(path) - 1] != L'\\')
        wcscat(path, L"\\");
    wcscat(path, SANDBOXIE);

    //
    // check if the SendTo\Sandbox folder is there, and either
    // create it, or abort, depending on the user settings
    //

    HANDLE handle = CreateFile(
        path, FILE_GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        if (! ini)
            return;
        CreateDirectory(path, NULL);
    } else
        CloseHandle(handle);

    //
    // build our list of boxes
    //

    CStringList boxnames;
    if (ini) {
        CBoxes &boxes = CBoxes::GetInstance();
        for (int index = (int)boxes.GetSize() - 1; index >= 1; --index)
            boxnames.AddTail(boxes.GetBox(index).GetName());
    }

    //
    // go through sandbox shortcuts, delete either all shortcuts,
    // or only those that don't reference any current sandbox
    //

    wcscat(path, L"\\*");
    WIN32_FIND_DATA data;
    handle = FindFirstFile(path, &data);
    while (handle != INVALID_HANDLE_VALUE) {

        CString name(data.cFileName);
        if (name != L"." && name != L"..") {

            BOOL del = TRUE;

            int index = name.ReverseFind(L'.');
            if (index != -1) {
                name = name.Left(index);
                POSITION pos = boxnames.Find(name);
                if (pos) {
                    boxnames.RemoveAt(pos);
                    del = FALSE;
                }
            }

            if (del) {
                wcscpy(wcsrchr(path, L'\\') + 1, data.cFileName);
                DeleteFile(path);
            }
        }

        if (! FindNextFile(handle, &data)) {
            FindClose(handle);
            handle = INVALID_HANDLE_VALUE;
        }
    }

    //
    // remove directory and finish, if clearing all shortcuts
    //

    if (! ini) {

        *wcsrchr(path, L'\\') = L'\0';
        RemoveDirectory(path);
        return;
    }

    //
    // create shortcuts for any remaining sandboxes
    //

    while (! boxnames.IsEmpty()) {
        CString name = boxnames.RemoveHead();

        wcscpy(wcsrchr(path, L'\\') + 1, name);

        CreateShortcut(path, CString(), name, NULL, CString(), 0, CString());
    }
}


//---------------------------------------------------------------------------
// SyncSendToMenuVista
//---------------------------------------------------------------------------


void CShellDialog::SyncSendToMenuVista()
{
    BOOL ini;
    CUserSettings::GetInstance().GetBool(_AddSendToMenu, ini, TRUE);

    WCHAR path[512];
    HRESULT hr = SHGetFolderPathW(
        NULL, CSIDL_SENDTO, NULL, SHGFP_TYPE_CURRENT, path);
    if (hr != 0 || (! path[0]))
        return;
    if (path[wcslen(path) - 1] != L'\\')
        wcscat(path, L"\\");
    wcscat(path, L"*");

    CString Sandbox(SANDBOXIE L" - ");

    //
    // build our list of boxes
    //

    CStringList boxnames;
    if (ini) {
        CBoxes &boxes = CBoxes::GetInstance();
        for (int index = (int)boxes.GetSize() - 1; index >= 1; --index) {
            CString name = Sandbox + boxes.GetBox(index).GetName();
            boxnames.AddTail(name);
        }
    }

    //
    // go through sandbox shortcuts, delete either all shortcuts,
    // or only those that don't reference any current sandbox
    //

    WIN32_FIND_DATA data;
    HANDLE handle = FindFirstFile(path, &data);
    while (handle != INVALID_HANDLE_VALUE) {

        CString name(data.cFileName);
        if (name != L"." && name != L"..") {

            BOOL del = FALSE;
            if (name.Find(Sandbox) == 0)
                del = TRUE;

            int index = name.ReverseFind(L'.');
            if (index != -1) {
                name = name.Left(index);
                POSITION pos = boxnames.Find(name);
                if (pos) {
                    boxnames.RemoveAt(pos);
                    del = FALSE;
                }
            }

            if (del) {
                wcscpy(wcsrchr(path, L'\\') + 1, data.cFileName);
                DeleteFile(path);
            }
        }

        if (! FindNextFile(handle, &data)) {
            FindClose(handle);
            handle = INVALID_HANDLE_VALUE;
        }
    }

    //
    // remove directory and finish, if clearing all shortcuts
    //

    if (! ini) {

        *wcsrchr(path, L'\\') = L'\0';
        RemoveDirectory(path);
        return;
    }

    //
    // create shortcuts for any remaining sandboxes
    //

    while (! boxnames.IsEmpty()) {
        CString name = boxnames.RemoveHead();

        wcscpy(wcsrchr(path, L'\\') + 1, name);

        name = name.Mid(Sandbox.GetLength());

        CreateShortcut(path, CString(), name, NULL, CString(), 0, CString());
    }
}


//---------------------------------------------------------------------------
// GetStartMenuShortcut
//---------------------------------------------------------------------------


BOOL CShellDialog::GetStartMenuShortcut(
    CString &BoxName, CString &LinkPath,
    CString &IconPath, ULONG &IconIndex,
    CString &WorkDir)
{
    //
    // prepare start menu work area in common memory
    //

    WCHAR MapName[128];
    wsprintf(MapName, SANDBOXIE L"_StartMenu_WorkArea_%08X_%08X",
             GetCurrentProcessId(), GetTickCount());

    HANDLE hMapping = CreateFileMapping(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 8192, MapName);
    if (! hMapping)
        return FALSE;

    WCHAR *buf = (WCHAR *)MapViewOfFile(
        hMapping, FILE_MAP_WRITE, 0, 0, 8192);
    if (! buf) {
        CloseHandle(hMapping);
        return FALSE;
    }
    memzero(buf, 8192);

    //
    // call Sandboxie Start
    //

    CString str;
    str.Format(L"start_menu:%s", MapName);
    if (! LinkPath.IsEmpty())
        str += L":" + LinkPath;
    CMyApp::RunStartExe(str, BoxName, TRUE, TRUE);

    //
    // process results
    //

    buf[40] = L'\0';
    BoxName = &buf[0];

    buf[1022] = L'\0';
    LinkPath = &buf[64];

    if (buf[1024]) {
        IconPath = &buf[1024];
        IconIndex = *(ULONG *)(buf + 1020);
    }

    if (buf[2048])
        WorkDir = &buf[2048];
    else {
        int index = LinkPath.ReverseFind(L'\\');
        if (index > 1)
            WorkDir = LinkPath.Left(index);
    }

    UnmapViewOfFile(buf);
    CloseHandle(hMapping);

    if (BoxName.IsEmpty() || LinkPath.IsEmpty())
        return FALSE;
    return TRUE;
}


//---------------------------------------------------------------------------
// CreateDesktopShortcut
//---------------------------------------------------------------------------


void CShellDialog::CreateDesktopShortcut(
    const CString &BoxName, const CString &LinkPath,
    const CString &IconPath, ULONG IconIndex,
    const CString &WorkDir)
{
    WCHAR path[512];
    HRESULT hr = SHGetFolderPathW(
        NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, path);
    if (hr != 0 || (! path[0]))
        return;
    if (path[wcslen(path) - 1] != L'\\')
        wcscat(path, L"\\");

    CString LinkName;
    int index = LinkPath.ReverseFind(L'\\');
    if (index == -1)
        return;
    if (index == 2 && LinkPath.GetLength() == 3) // for a path like "C:\"
        LinkName = CMyMsg(MSG_3676, LinkPath.Left(1));
    else {
        LinkName = LinkPath.Mid(index + 1);
        index = LinkName.FindOneOf(L"\":;,*?.");
        if (index != -1)
            LinkName = LinkName.Left(index);
    }

    wcscat(path, L"[");
    wcscat(path, BoxName);
    wcscat(path, L"] ");
    wcscat(path, LinkName);

    //
    //
    //

    CreateShortcut(path, LinkName,
                   BoxName, LinkPath, IconPath, IconIndex, WorkDir);
}


//---------------------------------------------------------------------------
// CreateShortcut
//---------------------------------------------------------------------------


BOOL CShellDialog::CreateShortcut(
    const CString &LinkPath, const CString &LinkName,
    const CString &boxname, const WCHAR *pgmArgs,
    const CString &iconPath, int iconIndex, const CString &workdir,
    BOOL run_elevated)
{
    //
    // get location of Start.exe for shortcut creation
    //

    static WCHAR *StartExe = NULL;

    if (! StartExe) {

        STARTUPINFO si;
        if (SbieDll_RunFromHome(START_EXE, L"", &si, NULL)) {
            StartExe = ((WCHAR *)si.lpReserved) + 1;
            *wcschr(StartExe, L'\"') = L'\0';
        }
    }

    //
    //
    //

    IUnknown *pUnknown;
    HRESULT hr = CoCreateInstance(
        CLSID_ShellLink, NULL, CLSCTX_INPROC, IID_IUnknown,
        (void **)&pUnknown);
    if (FAILED(hr))
        return FALSE;

    IShellLink *pShellLink;
    hr = pUnknown->QueryInterface(IID_IShellLink, (void **)&pShellLink);
    if (SUCCEEDED(hr)) {

        CString StartArgs;
        if (run_elevated)
            StartArgs += L"/elevated ";

        StartArgs += L"/box:";
        StartArgs += boxname;
        if (pgmArgs) {
            StartArgs += " \"";
            StartArgs += pgmArgs;
            StartArgs += "\"";
        }

        pShellLink->SetPath(StartExe);
        pShellLink->SetArguments(StartArgs);
        if (! iconPath.IsEmpty())
            pShellLink->SetIconLocation(iconPath, iconIndex);
        if (! workdir.IsEmpty())
            pShellLink->SetWorkingDirectory(workdir);

        if (! LinkName.IsEmpty()) {
            CMyMsg desc(MSG_4491, LinkName, boxname);
            pShellLink->SetDescription(desc);
        }

        IPersistFile *pPersistFile;
        hr = pUnknown->QueryInterface(
                            IID_IPersistFile, (void **)&pPersistFile);
        if (SUCCEEDED(hr)) {

            CString link1 = LinkPath + L".lnk";
            pPersistFile->Save(link1, FALSE);

            pPersistFile->Release();
        }

        pShellLink->Release();
    }

    pUnknown->Release();
    return (SUCCEEDED(hr));
}


//---------------------------------------------------------------------------
// CreateShortcut
//---------------------------------------------------------------------------


void CShellDialog::CreateShortcut(CString BoxName, CString LinkPath)
{
    CString IconPath, WorkDir;
    ULONG IconIndex;
    if (GetStartMenuShortcut(
                        BoxName, LinkPath, IconPath, IconIndex, WorkDir))
        CreateDesktopShortcut(
                        BoxName, LinkPath, IconPath, IconIndex, WorkDir);
}
