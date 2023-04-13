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
// Border Guard
//---------------------------------------------------------------------------


#include "stdafx.h"
#include "resource.h"
#include "BorderGuard.h"

#include "Boxes.h"
#include "common/my_version.h"


//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------


struct BoxBorderParms {

    WCHAR boxname[BOXNAME_COUNT];
    COLORREF color;
    BOOL title;
    int width;
};


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


CBorderGuard *CBorderGuard::m_instance = NULL;

void *CBorderGuard::m_DwmIsCompositionEnabled = NULL;
void *CBorderGuard::m_DwmGetWindowAttribute   = NULL;


//---------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------


CBorderGuard::CBorderGuard()
{
    m_instance = this;

    m_windows_8 = false;
    HMODULE dwmapi = LoadLibrary(L"dwmapi.dll");
    if (dwmapi) {
        m_DwmIsCompositionEnabled =
            GetProcAddress(dwmapi, "DwmIsCompositionEnabled");
        if (m_DwmIsCompositionEnabled) {
            m_DwmGetWindowAttribute =
                GetProcAddress(dwmapi, "DwmGetWindowAttribute");
            if (! m_DwmGetWindowAttribute)
                m_DwmIsCompositionEnabled = NULL;
        }

        if (GetProcAddress(dwmapi, "DwmRenderGesture"))
            m_windows_8 = true;
    }

    m_timer_id = NULL;

    m_active_hwnd = NULL;
    m_active_pid = 0;

    m_border_hwnd = NULL;
    m_border_brush = NULL;
    m_border_brush_color = RGB(0,0,0);
    m_border_visible = FALSE;

    m_thumb_width  = GetSystemMetrics(SM_CXHTHUMB);
    m_thumb_height = GetSystemMetrics(SM_CYVTHUMB);

    static const WCHAR *WindowClassName = SANDBOXIE_CONTROL L"BorderWindow";

    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
    wc.lpfnWndProc = ::DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = AfxGetInstanceHandle();
    wc.hIcon = ::LoadIcon(wc.hInstance, L"AAAPPICON");
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WindowClassName;
    wc.hIconSm = NULL;

    ATOM atom = RegisterClassEx(&wc);
    if (! atom)
        return;

    m_border_hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE
                      | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        (LPCWSTR)atom, WindowClassName,
        WS_POPUP | WS_CLIPSIBLINGS,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, NULL, NULL);
    if (! m_border_hwnd)
        return;

    if (m_border_hwnd) {

        SetLayeredWindowAttributes(m_border_hwnd, 0, 192, LWA_ALPHA);

        ::ShowWindow(m_border_hwnd, SW_HIDE);
    }

    RefreshConf();
}


//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------


CBorderGuard::~CBorderGuard()
{
    if (m_border_hwnd) {
        DestroyWindow(m_border_hwnd);
        m_border_hwnd = NULL;
    }
}


//---------------------------------------------------------------------------
// MyTimerProc
//---------------------------------------------------------------------------


void CBorderGuard::MyTimerProc(
    HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    m_instance->Refresh();
}


//---------------------------------------------------------------------------
// Refresh
//---------------------------------------------------------------------------


void CBorderGuard::Refresh()
{
    if ((! m_border_hwnd) || (! m_timer_id))
        return;

    //
    // get current foreground window,
    // and its associated process id and box name
    //

    HWND hwnd;
    ULONG style;
    ULONG pid = NULL;
    WCHAR boxname[BOXNAME_COUNT];
    boxname[0] = L'\0';

    hwnd = GetForegroundWindow();
    if (hwnd) {

        style = GetWindowLong(hwnd, GWL_STYLE);
        if (style & WS_VISIBLE) {

            GetWindowThreadProcessId(hwnd, &pid);
            const CBoxes &boxes = CBoxes::GetInstance();
            const CBox &box = boxes.GetBoxByProcessId(pid);
            wcscpy(boxname, box.GetName());
        }
    }

    //
    // foreground process is sandboxed
    //

    if (boxname[0]) {

        //
        // get the bounding rectangle for the foreground window
        //

        RECT rect;

        bool doGetWindowRect = true;
        if (m_DwmIsCompositionEnabled) {

            typedef HRESULT (*P_DwmIsCompositionEnabled)(BOOL *enabled);
            P_DwmIsCompositionEnabled pDwmIsCompositionEnabled =
                (P_DwmIsCompositionEnabled)m_DwmIsCompositionEnabled;

            BOOL dwmEnabled = FALSE;
            HRESULT hr = pDwmIsCompositionEnabled(&dwmEnabled);
            if (SUCCEEDED(hr) && dwmEnabled) {

                typedef HRESULT (*P_DwmGetWindowAttribute)(
                    HWND hwnd, DWORD dwAttribute,
                    void *pvAttribute, DWORD cbAttribute);
                P_DwmGetWindowAttribute pDwmGetWindowAttribute =
                    (P_DwmGetWindowAttribute)m_DwmGetWindowAttribute;

                const ULONG DWMWA_EXTENDED_FRAME_BOUNDS = 9;
                hr = pDwmGetWindowAttribute(
                    hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT));
                if (SUCCEEDED(hr))
                    doGetWindowRect = false;
            }
        }

        if (doGetWindowRect)
            GetWindowRect(hwnd, &rect);

        //
        // check if the border has to be refreshed
        //

        BOOL have_cursor = FALSE;
        POINT cursor;

        BOOL refresh = TRUE;
        if (pid == m_active_pid && hwnd == m_active_hwnd) {
            if (memcmp(&rect, &m_active_rect, sizeof(RECT)) == 0) {

                //
                // window rect is same as last recorded window rect
                // but if we track title area for border then also
                // check the cursor is still in the title area
                //

                refresh = FALSE;
                if (m_title_mode) {
                    int title_mode = -1;
                    have_cursor = GetCursorPos(&cursor);
                    if (have_cursor && cursor.x >= m_title_rect.left
                                    && cursor.x <= m_title_rect.right
                                    && cursor.y >= m_title_rect.top
                                    && cursor.y <= m_title_rect.bottom)
                        title_mode = +1;
                    if (title_mode != m_title_mode)
                        refresh = TRUE;
                }

            } else {
                if (! m_fast_timer_start_ticks)
                    m_timer_id = SetTimer(NULL, m_timer_id, 10, MyTimerProc);
                m_fast_timer_start_ticks = GetTickCount();
            }
        }

        //
        // show the border if necessary
        //

        if (refresh) {

            if (m_border_visible)
                ::ShowWindow(m_border_hwnd, SW_HIDE);
            m_border_visible = FALSE;

            m_active_hwnd = hwnd;
            m_active_pid = pid;
            memcpy(&m_active_rect, &rect, sizeof(RECT));
            m_title_mode = 0;

            RefreshBorder(hwnd, style, &rect, boxname);
        }

    //
    // foreground process is not sandboxed:  hide border
    //

    } else {

        if (m_border_visible) {

            ::ShowWindow(m_border_hwnd, SW_HIDE);
            m_border_visible = FALSE;
        }

        m_active_hwnd = NULL;
        m_active_pid = 0;
    }

    //
    // reprogram the timer if necessary
    //

    if (m_fast_timer_start_ticks) {
        if (GetTickCount() - m_fast_timer_start_ticks >= 1000) {
            m_fast_timer_start_ticks = 0;
            m_timer_id = SetTimer(NULL, m_timer_id, 100, MyTimerProc);
        }
    }
}


//---------------------------------------------------------------------------
// RefreshBorder
//---------------------------------------------------------------------------


void CBorderGuard::RefreshBorder(
    HWND hwnd, ULONG style, RECT *rect, const WCHAR *boxname)
{
    //
    // ignore very small windows
    //

    if (rect->right - rect->left <= 2 || rect->bottom - rect->top <= 2)
            return;

    //
    //
    //

    BoxBorderParms *boxparm = NULL;

    ULONG count = (ULONG)m_boxes.GetSize();
    ULONG i = 0;
    for (i = 0; i < count; ++i) {
        boxparm = (BoxBorderParms *)m_boxes.GetAt(i);
        if (wcscmp(boxparm->boxname, boxname) == 0)
            break;
    }
    if (i == count)
        return;

    //
    // get information about the screen containing the window.
    // don't draw the color border if the window is full screen
    // and also has no caption bar (i.e. a full screen app)
    //

    HMONITOR hmonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
    if (! hmonitor)
        return;

    MONITORINFO monitor;
    memzero(&monitor, sizeof(MONITORINFO));
    monitor.cbSize = sizeof(MONITORINFO);
    if (! GetMonitorInfo(hmonitor, &monitor))
        return;

    const RECT *desktop = &monitor.rcMonitor;

    if (rect->left  <= desktop->left  && rect->top    <= desktop->top    &&
        rect->right >= desktop->right && rect->bottom >= desktop->bottom &&
        (style & WS_CAPTION) != WS_CAPTION) {

        return;
    }

    //
    // display the border only if the cursor is on the title bar area,
    // when the corresponding border mode is enabled
    //

    if (boxparm->title) {

        TITLEBARINFO tbinfo;
        tbinfo.cbSize = sizeof(TITLEBARINFO);
        GetTitleBarInfo(hwnd, &tbinfo);
        memcpy(&m_title_rect, &tbinfo.rcTitleBar, sizeof(RECT));

#define t m_title_rect
#define w (*rect)

        if (t.left   < w.left   || t.left   > w.right   ||
            t.top    < w.top    || t.top    > w.bottom  ||
            t.right  < w.left   || t.right  > w.right   ||
            t.bottom < w.top    || t.bottom > w.bottom  ||
            t.right - t.left <= m_thumb_width           ||
            t.bottom - t.top <= m_thumb_height) {

            // if window has no title bar area, then calculate some area

            m_title_rect.left   = rect->left;
            m_title_rect.top    = rect->top;
            m_title_rect.right  = rect->right;
            m_title_rect.bottom = rect->top + m_thumb_height * 2;
        }

         /*{WCHAR xxx[256];wsprintf(xxx, L"Window %08X Rect (%d,%d)-(%d,%d) title (%d,%d)-(%d,%d) cursor (%d,%d)\n",
            hwnd, rect->left,rect->top,rect->right,rect->bottom,
            m_title_rect.left,m_title_rect.top,m_title_rect.right,m_title_rect.bottom,
            cursor.x,cursor.y);
            OutputDebugString(xxx);}*/

#undef w
#undef t

        m_title_rect.top -= 8;
        if (m_title_rect.top < desktop->top)
            m_title_rect.top = desktop->top;

        m_title_mode = -1;
        POINT cursor;
        if (GetCursorPos(&cursor) && cursor.x >= m_title_rect.left
                                  && cursor.x <= m_title_rect.right
                                  && cursor.y >= m_title_rect.top
                                  && cursor.y <= m_title_rect.bottom)
            m_title_mode = +1;
        if (m_title_mode == -1)
            return;
    }

    //
    // replace background brush if necessary
    //

    if ((! m_border_brush) ||
            boxparm->color != m_border_brush_color) {

        HBRUSH hbr = CreateSolidBrush(boxparm->color);
        SetClassLongPtr(m_border_hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbr);
        if (m_border_brush)
            DeleteObject(m_border_brush);
        m_border_brush = hbr;

        m_border_brush_color = boxparm->color;
    }

    //
    // get work area of the screen containing the window,
    // then compute window bounds within the work area
    //

    desktop = &monitor.rcWork;


    int ax = rect->left;
    if (rect->left < desktop->left && (desktop->left - rect->left) < (boxparm->width + 4))
        ax = desktop->left;

    int ay = rect->top;
    if (rect->top < desktop->top && (desktop->top - rect->top) < (boxparm->width + 4))
        ay = desktop->top;

    int aw = -ax;
    if (rect->right > desktop->right && (desktop->right - rect->right) < (boxparm->width + 4))
        aw += desktop->right;
    else
        aw += rect->right;

    int ah = -ay;
    if (rect->bottom > desktop->bottom && (desktop->bottom - rect->bottom) < (boxparm->width + 4))
        ah += desktop->bottom;
    else
        ah += rect->bottom;
        


	if (rect->bottom == desktop->bottom) 
		ah -= 1;

    //int bb = 6;
    //if (rect->left   <= desktop->left &&
    //    rect->top    <= desktop->top  &&
    //    rect->right  >= desktop->right &&
    //    rect->bottom >= desktop->bottom)
    //    bb = 4;
    int bb = boxparm->width;

    //
    // don't display the border if any of it would be obscured by
    // some other window that has the "always on top" attribute
    //

    HWND hwndShellTrayWnd = FindWindow(L"Shell_TrayWnd", NULL);

    for (i = 0; i < 4; ++i) {

        POINT testpt;
        if (i == 0) {
            testpt.x = ax;
            testpt.y = ay;
        } else if (i == 1) {
            testpt.x = ax + aw;
            testpt.y = ay;
        } else if (i == 2) {
            testpt.x = ax;
            testpt.y = ay + ah;
        } else if (i == 3) {
            testpt.x = ax + aw;
            testpt.y = ay + ah;
        }

        if (testpt.x >= desktop->right)
            testpt.x = desktop->right - 1;
        if (testpt.y >= desktop->bottom)
            testpt.y = desktop->bottom - 1;

        HWND testwnd = WindowFromPoint(testpt);
        while (testwnd) {
            if ( (testwnd == hwndShellTrayWnd) || (testwnd == hwnd) )
                break;
            ULONG exStyle = GetWindowLong(testwnd, GWL_EXSTYLE);
            if (exStyle & WS_EX_TOPMOST) {

                //
                // Windows 8 places invisible "always on top" windows in
                // the corners of the desktop, we should ignore those
                //

                bool foundTopmostWindow = true;
                if (m_windows_8) {
                    WCHAR clsnm[64];
                    int n = GetClassName(testwnd, clsnm, 64);
                    if (n == 19 &&
                            _wcsicmp(clsnm, L"EdgeUiInputWndClass") == 0)
                        foundTopmostWindow = false;
                }
                if (foundTopmostWindow) {
                    m_active_hwnd = NULL;
                    m_active_pid = 0;
                    return;
                }
            }
            testwnd = GetParent(testwnd);
        }
    }

    //
    // compute new region
    //

    POINT points[16];
    int num_points = 0;
#define ADD_POINT(xx,yy) \
    points[num_points].x = (xx);    \
    points[num_points].y = (yy);    \
    ++num_points;

    ADD_POINT(0,        0       );
    ADD_POINT(aw,       0       );
    ADD_POINT(aw,       ah      );
    ADD_POINT(0,        ah      );
    ADD_POINT(0,        0       );
    ADD_POINT(0  + bb,  0  + bb );
    ADD_POINT(aw - bb,  0  + bb );
    ADD_POINT(aw - bb,  ah - bb );
    ADD_POINT(0 + bb,   ah - bb );
    ADD_POINT(0 + bb,   0  + bb );

#undef ADD_POINT

    HRGN hrgn = CreatePolygonRgn(points, num_points, ALTERNATE);
    SetWindowRgn(m_border_hwnd, hrgn, TRUE);
    DeleteObject(hrgn);
    SetWindowPos(m_border_hwnd, NULL, ax, ay, aw, ah,
                 SWP_SHOWWINDOW | SWP_NOACTIVATE);

    m_border_visible = TRUE;
}


//---------------------------------------------------------------------------
// RefreshConf2
//---------------------------------------------------------------------------


void CBorderGuard::RefreshConf2()
{
    /*if (GetSystemMetrics(SM_CXSCREEN) < 1000)
        m_border_width = 2;
    else
        m_border_width = 4;*/

    BoxBorderParms *boxparm;
    int i;

    while (m_boxes.GetSize()) {
        boxparm = (BoxBorderParms *)m_boxes.GetAt(0);
        delete boxparm;
        m_boxes.RemoveAt(0);
    }

    CBoxes &boxes = CBoxes::GetInstance();
    for (i = 1; i < boxes.GetSize(); ++i) {
        CBox &box = boxes.GetBox(i);
        if (! box.GetName().IsEmpty()) {
            COLORREF color;
            BOOL title;
            int width;
            BOOL enabled = box.GetBorder(&color, &title, &width);
            if (enabled) {
                boxparm = new BoxBorderParms;
                wcscpy(boxparm->boxname, box.GetName());
                boxparm->color = color;
                boxparm->title = title;
                boxparm->width = width;
                m_boxes.Add(boxparm);
            }
        }
    }

    if (m_boxes.GetSize()) {
        if (! m_timer_id) {
            m_fast_timer_start_ticks = 0;
            m_timer_id = SetTimer(NULL, 0, 100, MyTimerProc);
        }
    } else if (m_timer_id) {
        KillTimer(NULL, m_timer_id);
        m_timer_id = NULL;
    }
}


//---------------------------------------------------------------------------
// RefreshConf
//---------------------------------------------------------------------------


void CBorderGuard::RefreshConf()
{
    if (! m_instance)
        m_instance = new CBorderGuard();
    m_instance->RefreshConf2();
}
