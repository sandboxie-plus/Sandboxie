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
// Boxes
//---------------------------------------------------------------------------


#ifndef _MY_BOX_H
#define _MY_BOX_H


#include "BoxProc.h"
#include "BoxFile.h"


class CBox
{
    static const CString _empty;

    CString m_name;
    CBoxProc *m_BoxProc;
    CBoxFile *m_BoxFile;

    BOOL m_expandedView;
    BOOL m_TemporarilyDisableQuickRecover;

public:

    CBox(const CString &name);
    ~CBox();

    const CString &GetName() const;

    CBoxProc &GetBoxProc();
    CBoxFile &GetBoxFile();

    BOOL IsExpandedView() const;
    void SetExpandedView(BOOL view);

    //
    // Settings
    //

    void SetDefaultSettings();

    static void SetEnabled(const CString &BoxName);

    BOOL GetProcessSetting(WCHAR type, const CString &image);
    BOOL SetProcessSetting(WCHAR type, const CString &image, BOOL newval);
    void GetProcessList(WCHAR type, CStringList &list,
                        BOOL withBrackets = FALSE);

    void GetProcessGroupsInBox(CStringList &names);

    void GetProcessGroupName(
        const CString &GroupName,
        CString &LocalGroupName, CString &GlobalGroupName);
    CString GetProcessGroup(const CString &GroupName);
    void SetProcessGroup(const CString &GroupName, const CString &value);

    CString GetInternetProcess(BOOL *pNotify = NULL);
    BOOL SetInternetProcess(const CString &program, BOOL *pNotify = NULL);

    CString GetStartRunProcess(BOOL *pNotify = NULL);
    BOOL SetStartRunProcess(const CString &program, BOOL *pNotify = NULL);

    BOOL AddOrRemoveQuickRecoveryFolder(const CString &folder, BOOL add);
    void GetQuickRecoveryFolders(CStringList &list,
                                 BOOL withBrackets = FALSE);

    BOOL AddOrRemoveAutoRecoveryIgnores(const CString &item, BOOL add);
    void GetAutoRecoveryIgnores(CStringList &list,
                                BOOL withBrackets = FALSE);

    BOOL GetImmediateRecoveryState() const;
    void SetImmediateRecoveryState(BOOL enabled, BOOL temporarily);

    BOOL GetAutoDelete() const;
    void SetAutoDelete(BOOL enabled);

    BOOL GetNeverDelete() const;
    void SetNeverDelete(BOOL enabled);

    CString GetDeleteCommand() const;
    void SetDeleteCommand(const CString &cmd) const;

    ULONG GetSetCopyLimit(BOOL set, ULONG newval = 0);
    BOOL  GetSetCopySilent(BOOL set, BOOL newval = FALSE);

    UCHAR GetBoxNameTitle() const;
    BOOL SetBoxNameTitle(UCHAR enabled);
    BOOL GetBorder(COLORREF *color, BOOL *title);
    BOOL SetBorder(BOOL enabled, COLORREF color, BOOL title);

    BOOL GetSetBlock(WCHAR type, BOOL set, BOOL newval = FALSE);

    void GetXxxPathList(ULONG type, CStringList &list,
                        BOOL withBrackets = FALSE);
    void GetXxxPathListForPgm(
        ULONG type, CStringList &list, const CString &pgm,
        BOOL withBrackets = FALSE);
    void SetXxxPathListForPgm(
        ULONG type, const CStringList &list, const CString &pgm);

    BOOL GetSetDropAdminRights(BOOL set, BOOL newval = FALSE);
    BOOL GetSetBlockNetworkFiles(BOOL set, BOOL newval = FALSE);
    BOOL GetSetAllowSpoolerPrintToFile(BOOL set, BOOL newval = FALSE);

    static void GetSetUserAccounts(
        const CString &BoxName, BOOL set, CStringList &list);
    void GetSetUserAccounts(BOOL set, CStringList &list);

    BOOL IsTemplateEnabled(const WCHAR *TemplateName);
    BOOL EnableTemplate(const WCHAR *TemplateName, BOOL enable);

    BOOL IsTemplateRejected(const WCHAR *TemplateName);
    BOOL RejectTemplate(const WCHAR *TemplateName, BOOL reject);

    //
    // Pubic Variables
    //

    static const CString _Enabled;

    static const CString _ProcessGroup;
    static const CString _InternetAccessGroup;
    static const CString _StartRunAccessGroup;

    static const CString _AutoRecoverIgnore;
    static const CString _ForceFolder;
    static const CString _ForceProcess;
    static const CString _ForceUser;
    static const CString _LeaderProcess;
    static const CString _LingerProcess;
    static const CString _RecoverFolder;

    static const CString _ClosedFilePath;
    static const CString _ClosedIpcPath;
    static const CString _ClosedKeyPath;
    static const CString _OpenClsid;
    static const CString _OpenFilePath;
    static const CString _OpenIpcPath;
    static const CString _OpenKeyPath;
    static const CString _OpenPipePath;
    static const CString _OpenWinClass;
    static const CString _ReadFilePath;
    static const CString _ReadKeyPath;
    static const CString _WriteFilePath;
    static const CString _WriteKeyPath;

};


#endif // _MY_BOX_H
