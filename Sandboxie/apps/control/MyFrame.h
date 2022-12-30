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
// Frame
//---------------------------------------------------------------------------


#ifndef _MY_FRAME_H
#define _MY_FRAME_H


#include "MenuXP.h"


#include "ProcListCtrl.h"
#include "FileListCtrl.h"
#include "FinderDialog.h"
#include "MessageDialog.h"
#include "MonitorDialog.h"


class CMyFrame : public CFrameWnd
{
    DECLARE_MESSAGE_MAP()
    DECLARE_MENUXP()

    CMenu *m_SandboxMenu;
    //CImageList m_imglist;
    //int m_imgRunWeb, m_imgRunAny;
    //int m_imgTerminate, m_imgRecover, m_imgExplore;
    //int m_imgRename, m_imgRemove;

    CMenu *m_pTraySandboxMenu;
    CMenu *m_pTrayMenu;
    HICON m_hIconFull, m_hIconEmpty;
    HICON m_hIconFullDfp, m_hIconEmptyDfp;
    HICON m_hIconDelete;
    HICON m_hIconPtr;
    BOOL m_TrayActive, m_TrayCommand;
    CPoint m_TrayPoint;

    BOOL m_hidden;
    //BOOL m_ShowWelcome;
    BOOL m_ReSyncShortcuts;
    BOOL m_AutoRunSoftCompat;
    BOOL m_AlwaysOnTop;

    UINT m_view, m_view_old;
    CProcListCtrl m_proclist;
    CFileListCtrl m_filelist;
    CMessageDialog *m_msgdlg;
    CFinderDialog *m_finder;
    CMonitorDialog *m_mondlg;

    CStringList m_DeleteList;

    static BOOL m_inTimer;
    static BOOL m_destroyed;

    CMyListCtrl *GetMyListCtrl();

    bool AdjustSizePosition(int &left, int &top, int &width, int &height);
    void InitMenus(void);

    void SaveSettings();

    void PostNcDestroy();

    void CheckShouldDelete(CBox &box);
    void RunExplorer(CBox &box, BOOL sandboxed);
    BOOL TerminateProcesses(CBox &box, BOOL warn);
    void RemoveOrRename(
            CBox &box, BOOL remove, BOOL rename, bool *ReloadBoxes);
    void InitSandboxMenu(CMenu *pMenu, int posChild, int posInsert);
    void InitSandboxMenu1(void *_box_order, CMenu *ModelMenu,
                          CMenu *SandboxMenu, int posInsert);
    void InitSandboxMenu2(CMenu *model, CMenu *child, UINT BaseId);

    void RefreshTrayIcon(BOOL ForceDeleteIcon = FALSE);

    afx_msg void OnCmdExit();
    afx_msg void OnCmdShowWindow();
    afx_msg void OnCmdDisableForce();
    afx_msg void OnCmdResourceMonitor();
    afx_msg void OnCmdTermAll();
    afx_msg void OnCmdViewContext();
    afx_msg void OnCmdViewProcess();
    afx_msg void OnCmdViewFiles();
    afx_msg void OnCmdViewRecoveryLog();
    afx_msg void OnCmdViewTopmost();
    afx_msg void OnCmdSandboxReveal();
    afx_msg void OnCmdSandboxSetFolder();
    afx_msg void OnCmdSandboxSetLayout();
    afx_msg void OnCmdSandboxCreateNew();
    afx_msg void OnCmdSandbox(UINT nID);
    afx_msg void OnCmdFinderOpen();
    afx_msg void OnCmdFinderClose();
    afx_msg void OnCmdConfAlert();
    afx_msg void OnCmdConfShell();
    afx_msg void OnCmdThirdParty();
    afx_msg void OnCmdConfHiddenMsg();
    afx_msg void OnCmdConfShowHideTips();
    afx_msg void OnCmdConfTrace();
    afx_msg void OnCmdConfLock();
    afx_msg void OnCmdConfEdit();
    afx_msg void OnCmdConfReload();
    afx_msg void OnCmdHelpSupport();
    afx_msg void OnCmdHelpContribution();
    afx_msg void OnCmdHelpTopics();
    afx_msg void OnCmdHelpTutorial();
    afx_msg void OnCmdHelpForum();
    afx_msg void OnCmdHelpUpdate();
    afx_msg void OnCmdHelpUpgrade();
    afx_msg void OnCmdHelpMigrate();
    afx_msg void OnCmdHelpAbout();
	//afx_msg LRESULT OnUpdateResult(WPARAM wParam, LPARAM lParam);
    afx_msg void OnCmdTerminateProcess();

    afx_msg void OnUpdViewMenu(CCmdUI *pCmdUI);
    afx_msg void OnUpdDisableForce(CCmdUI *pCmdUI);
    afx_msg void OnEnterIdle(UINT nWhy, CWnd *pWho);

    afx_msg void OnInitMenu(CMenu *pMenu);
    afx_msg void OnContextMenu(CWnd *pWnd, CPoint pt);
    afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);

    afx_msg void OnDestroy();
    afx_msg BOOL OnQueryEndSession();
    afx_msg BOOL OnEraseBkgnd(CDC *pDC);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnSizing(UINT nSide, LPRECT lpRect);
    afx_msg void OnClose();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnSetFocus(CWnd *pOldWnd);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnWindowPosChanged(WINDOWPOS *lpwndpos);

    afx_msg void OnDropFiles(HDROP hDrop);

    afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);

    static UINT AFX_CDECL OnCmdConfEditThread(LPVOID parm);

public:

    CMyFrame(BOOL ForceVisible, BOOL ForceSync);
    ~CMyFrame();

    static CWnd *m_GettingStartedWindow;
};


#endif // _MY_FRAME_H
