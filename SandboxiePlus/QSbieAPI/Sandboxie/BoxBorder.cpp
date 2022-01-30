/*
 *
 * Copyright (c) 2020, David Xanatos
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "BoxBorder.h"
#include "../SbieAPI.h"

#include <wtypes.h>

typedef HRESULT(*P_DwmIsCompositionEnabled)(BOOL *enabled);
typedef HRESULT(*P_DwmGetWindowAttribute)(HWND hWnd, DWORD dwAttribute, void *pvAttribute, DWORD cbAttribute);

struct SBoxBorder
{
	CSandBox* pCurrentBox;
	COLORREF BorderColor;
	int BorderMode;
	int BorderWidth;

	ULONG ActivePid;
	HWND ActiveWnd;
	RECT ActiveRect;
	RECT TitleRect;
	int  TitleState;

	BOOL IsBorderVisible;
	HWND BorderWnd;
	HBRUSH BorderBrush;

	int ThumbWidth;
	int ThumbHeight;

	P_DwmIsCompositionEnabled DwmIsCompositionEnabled;
	P_DwmGetWindowAttribute DwmGetWindowAttribute;
};

const WCHAR *Sandboxie_WindowClassName = L"Sandboxie_BorderWindow";

CBoxBorder::CBoxBorder(CSbieAPI* pApi, QObject* parent) : QObject(parent)
{
	m_Api = pApi;

	m = new SBoxBorder;

	m->pCurrentBox = NULL;
	m->BorderColor = RGB(0, 0, 0);
	m->BorderMode = 0;
	m->BorderWidth = 0;

	m->ActivePid = 0;
	m->ActiveWnd = NULL;

	m->IsBorderVisible = FALSE;
	m->BorderWnd = NULL;
	m->BorderBrush = NULL;

	m->ThumbWidth = GetSystemMetrics(SM_CXHTHUMB);
	m->ThumbHeight = GetSystemMetrics(SM_CYVTHUMB);

	HMODULE dwmapi = LoadLibrary(L"dwmapi.dll");
	if (dwmapi) {
		m->DwmIsCompositionEnabled = (P_DwmIsCompositionEnabled)GetProcAddress(dwmapi, "DwmIsCompositionEnabled");
		if (m->DwmIsCompositionEnabled) {
			m->DwmGetWindowAttribute = (P_DwmGetWindowAttribute)GetProcAddress(dwmapi, "DwmGetWindowAttribute");
			if (!m->DwmGetWindowAttribute)
				m->DwmIsCompositionEnabled = NULL;
		}
	}

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
	wc.lpfnWndProc = ::DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = NULL; // (HINSTANCE)::GetModuleHandle(NULL);
	wc.hIcon = NULL; // ::LoadIcon(wc.hInstance, L"AAAPPICON");
	wc.hCursor = NULL; // ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = Sandboxie_WindowClassName;
	wc.hIconSm = NULL;
	if (ATOM lpClassName = RegisterClassEx(&wc))
	{
		m->BorderWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, (LPCWSTR)lpClassName,
			Sandboxie_WindowClassName, WS_POPUP | WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
	}
	if (!m->BorderWnd)
		return;

	SetLayeredWindowAttributes(m->BorderWnd, 0, 192, LWA_ALPHA);
	::ShowWindow(m->BorderWnd, SW_HIDE);

	m_uTimerID = startTimer(10);
}

CBoxBorder::~CBoxBorder()
{
	killTimer(m_uTimerID);

	if (m->BorderWnd) 
	{
		DestroyWindow(m->BorderWnd);
		m->BorderWnd = NULL;
	}

	delete m;
}

void CBoxBorder::timerEvent(QTimerEvent* pEvent)
{
	if (pEvent->timerId() != m_uTimerID)
		return;

	HWND hWnd = GetForegroundWindow();
	if (!hWnd)
		return;
	ULONG Style = GetWindowLong(hWnd, GWL_STYLE);
	if (!(Style & WS_VISIBLE))
		return;
	ULONG pid = 0;
	GetWindowThreadProcessId(hWnd, &pid);

	CSandBoxPtr pProcessBox = m_Api->GetBoxByProcessId(pid);

	if (m->pCurrentBox != pProcessBox.data())
	{
		m->pCurrentBox = pProcessBox.data();
		if(!m->pCurrentBox)
			m->BorderMode = 0;
		else
		{
			m->BorderMode = 1;
			m->BorderColor = RGB(255, 255, 0);
			m->BorderWidth = 6;

			QStringList BorderCfg = pProcessBox->GetText("BorderColor").split(",");
			if (BorderCfg.first().left(1) == L'#')
			{
				bool ok = false;
				m->BorderColor = BorderCfg.first().mid(1).toInt(&ok, 16);
				if(!ok)
					m->BorderColor = RGB(255, 255, 0);
				else 
				{
					if (BorderCfg.count() >= 2)
					{
						QString StrMode = BorderCfg.at(1);
						if (StrMode.compare("ttl", Qt::CaseInsensitive) == 0)
							m->BorderMode = 2;
						else if (StrMode.compare("off", Qt::CaseInsensitive) == 0)
							m->BorderMode = 0;
					}

					if (BorderCfg.count() >= 3)
					{
						m->BorderWidth = BorderCfg.at(2).toInt();
						if (!m->BorderWidth)
							m->BorderWidth = 6;
					}
				}
			}

			HBRUSH hbr = CreateSolidBrush(m->BorderColor);
			SetClassLongPtr(m->BorderWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbr);
			if (m->BorderBrush)
				DeleteObject(m->BorderBrush);
			m->BorderBrush = hbr;
		}
	}

	if (m->BorderMode == 0) // no border enabled or unsandboxed
	{
		m->ActiveWnd = NULL;
		m->ActivePid = 0;

		if (m->IsBorderVisible) 
		{
			::ShowWindow(m->BorderWnd, SW_HIDE);
			m->IsBorderVisible = FALSE;
		}
	}
	else
	{
		RECT rect;
		GetActiveWindowRect(hWnd, &rect);
		if (NothingChanged(hWnd, &rect, pid))
			return;

		if (m->IsBorderVisible)
			::ShowWindow(m->BorderWnd, SW_HIDE);
		m->IsBorderVisible = FALSE;

		m->ActiveWnd = hWnd;
		m->ActivePid = pid;
		memcpy(&m->ActiveRect, &rect, sizeof(RECT));
		m->TitleState = 0;
		if (rect.right - rect.left <= 2 || rect.bottom - rect.top <= 2)
			return;

		HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);
		if (!hMonitor)
			return;
		MONITORINFO Monitor;
		memset(&Monitor, 0, sizeof(MONITORINFO));
		Monitor.cbSize = sizeof(MONITORINFO);
		if (!GetMonitorInfo(hMonitor, &Monitor))
			return;

		const RECT *Desktop = &Monitor.rcMonitor;
		if (rect.left <= Desktop->left  && rect.top <= Desktop->top    &&
		  rect.right >= Desktop->right && rect.bottom >= Desktop->bottom &&
		  (Style & WS_CAPTION) != WS_CAPTION) 
			return;

		if (m->BorderMode == 2) {
			if(!IsMounseOnTitle(hWnd, &rect, Desktop))
				return;
		}


		int ax = rect.left;
		if (rect.left < Desktop->left && (Desktop->left - rect.left) < (m->BorderWidth + 4))
		    ax = Desktop->left;

		int ay = rect.top;
		if (rect.top < Desktop->top && (Desktop->top - rect.top) < (m->BorderWidth + 4))
		    ay = Desktop->top;

		int aw = -ax;
		if (rect.right > Desktop->right && (rect.right - Desktop->right) < (m->BorderWidth + 4))
			aw += Desktop->right;
		else
			aw += rect.right;

		int ah = -ay;
		if (rect.bottom > Desktop->bottom && (rect.bottom - Desktop->bottom) < (m->BorderWidth + 4))
			ah += Desktop->bottom;
		else
			ah += rect.bottom;


		// 
		// in windows 10 and 11 if this is truly fullscreen the taskbar does not appear when hidden
		// if its 1 px less on any side it works normally, so we pick bottom as thets where the taskbar usualyl is
		// but with the taskbar to the side it woudl also work
		//

		if (rect.bottom == Monitor.rcWork.bottom) 
			ah -= 1;


		POINT Points[10];
		int PointCount = 0;

#define ADD_POINT(xx,yy) \
		Points[PointCount].x = (xx);    \
		Points[PointCount].y = (yy);    \
		PointCount++;

#define ADD_SQUARE(_w,_h,_b) \
		ADD_POINT(0 + _b, 0 + _b); \
		ADD_POINT(_w - _b, 0 + _b); \
		ADD_POINT(_w - _b, _h - _b); \
		ADD_POINT(0 + _b, _h - _b); \
		ADD_POINT(0 + _b, 0 + _b);

		ADD_SQUARE(aw, ah, 0);
		ADD_SQUARE(aw, ah, m->BorderWidth);

		HRGN hrgn = CreatePolygonRgn(Points, PointCount, ALTERNATE);
		SetWindowRgn(m->BorderWnd, hrgn, TRUE);
		SetWindowPos(m->BorderWnd, NULL, ax, ay, aw, ah, SWP_SHOWWINDOW | SWP_NOACTIVATE);

		m->IsBorderVisible = TRUE;
	}
}

bool CBoxBorder::NothingChanged(struct HWND__* hWnd, struct tagRECT* rect, quint32 pid)
{
	if (pid == m->ActivePid && hWnd == m->ActiveWnd) {
		if (memcmp(rect, &m->ActiveRect, sizeof(RECT)) == 0) {
			if (!m->TitleState || m->TitleState == (CheckMousePointer() ? 1 : -1))
				return true;
		}
	}
	return false;
}

void CBoxBorder::GetActiveWindowRect(struct HWND__* hWnd, struct tagRECT* rect)
{
	if (m->DwmIsCompositionEnabled) {
		BOOL bEnabled = FALSE;
		if (SUCCEEDED(m->DwmIsCompositionEnabled(&bEnabled)) && bEnabled) {	
			const ULONG DWMWA_EXTENDED_FRAME_BOUNDS = 9;
			if (SUCCEEDED(m->DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, rect, sizeof(RECT))))
				return;
		}
	}
	GetWindowRect(hWnd, rect);
}

bool CBoxBorder::IsMounseOnTitle(struct HWND__* hWnd, struct tagRECT* rect, const struct tagRECT* Desktop)
{
	TITLEBARINFO TitleBarInfo;
	TitleBarInfo.cbSize = sizeof(TITLEBARINFO);
	GetTitleBarInfo(hWnd, &TitleBarInfo);
	memcpy(&m->TitleRect, &TitleBarInfo.rcTitleBar, sizeof(RECT));

	if (m->TitleRect.left   < rect->left || m->TitleRect.left   > rect->right  ||
		m->TitleRect.top    < rect->top  || m->TitleRect.top    > rect->bottom ||
		m->TitleRect.right  < rect->left || m->TitleRect.right  > rect->right  ||
		m->TitleRect.bottom < rect->top  || m->TitleRect.bottom > rect->bottom ||
		m->TitleRect.right - m->TitleRect.left <= m->ThumbWidth ||
		m->TitleRect.bottom - m->TitleRect.top <= m->ThumbHeight
	) {
		m->TitleRect.left = rect->left;
		m->TitleRect.top = rect->top;
		m->TitleRect.right = rect->right;
		m->TitleRect.bottom = rect->top + m->ThumbHeight * 2;
	}

	m->TitleRect.top -= 8;
	if (m->TitleRect.top < Desktop->top)
		m->TitleRect.top = Desktop->top;

	m->TitleState = CheckMousePointer() ? 1 : -1;
	if (m->TitleState == -1)
		return false;
	return true;
}

bool CBoxBorder::CheckMousePointer()
{
	POINT Cursor;
	if (GetCursorPos(&Cursor) 
	  && Cursor.x >= m->TitleRect.left
	  && Cursor.x <= m->TitleRect.right
	  && Cursor.y >= m->TitleRect.top
	  && Cursor.y <= m->TitleRect.bottom)
		return true;
	return false;
}
