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
// Getting Started Wizard
//---------------------------------------------------------------------------


#ifndef _MY_GETTINGSTARTEDWIZARD_H
#define _MY_GETTINGSTARTEDWIZARD_H


#include "MyWizard.h"
#include "FlashingButton.h"
#include "FileListCtrl.h"
#include "AnimatedBitmap.h"


class CGettingStartedPage : public CMyWizardPage
{

protected:

    DECLARE_MESSAGE_MAP()

    union {

        struct {

            CAnimatedBitmap *m_animation;
            CBitmap *m_legend_bitmap;

        } page2;

        struct {

            HICON m_runsandbox_icon;
            CWnd *m_runsandbox_pwnd;

        } page3;

        struct {

            CWnd *m_shadow_wnd;
            bool m_open_tray;
            bool m_tray_opened;

        } page5;

    } u;

protected:

    virtual BOOL OnInitDialog();
    void OnInitDialog_Page1();
    void OnInitDialog_Page2();
    void OnInitDialog_Page3();
    void OnInitDialog_Page4();
    void OnInitDialog_Page5();
    void OnInitDialog_Page6();

    virtual BOOL OnKillActive();

    virtual void OnCancel();

    afx_msg void OnImageFrame_Page2(NMHDR *pNMHDR, LRESULT *pResult);

    afx_msg void OnShowDesktop_Page3();

    afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);

    afx_msg void OnTimer(UINT_PTR nIDEvent);

    afx_msg void OnImmediateRecover_Page4();

    afx_msg void OnShowBalloon_Page5();

    afx_msg void OnHideBalloon_Page5();

    afx_msg void OnDeleteContents_Page5();

    static HWND FindTrayButton(HWND hwnd, LPARAM lParam);

public:

    CGettingStartedPage(int page_num);
    ~CGettingStartedPage();

};


class CGettingStartedWizard : public CMyWizard
{

    friend class CGettingStartedPage;

protected:

    CFlashingButton m_next_button;

protected:

    void FlashNextButton(bool enable);

public:

    CGettingStartedWizard(CWnd *pParentWnd);

};


#endif // _MY_GETTINGSTARTEDWIZARD_H
