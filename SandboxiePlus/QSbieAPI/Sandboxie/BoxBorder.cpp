/*
 *
 * Copyright (c) 2020-2022, David Xanatos
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
#include <string>

typedef HRESULT(*P_DwmIsCompositionEnabled)(BOOL *enabled);
typedef HRESULT(*P_DwmGetWindowAttribute)(HWND hWnd, DWORD dwAttribute, void *pvAttribute, DWORD cbAttribute);

struct SBoxBorder
{
	HANDLE hThread;
	UINT_PTR dwTimerId;
	int FastTimerStartTicks;

	CSandBox* pCurrentBox;
	COLORREF BorderColor;
	int BorderMode;
	int BorderWidth;
	int BorderAlpha;

	ULONG ActivePid;
	HWND ActiveWnd;
	RECT ActiveRect;
	RECT TitleRect;
	int  TitleState;

	BOOL IsBorderVisible;
	HWND BorderWnd;
	HBRUSH BorderBrush;
	HFONT LabelFont;
	std::wstring BoxName;
	int LabelWidth;
	int LabelHeight;
	int LabelPadding;
	int LabelMode; // 0=disabled, -1=outside (above border), 1=inside (below border line)
	RECT LabelRect;

	int ThumbWidth;
	int ThumbHeight;

	P_DwmIsCompositionEnabled DwmIsCompositionEnabled;
	P_DwmGetWindowAttribute DwmGetWindowAttribute;
};

const WCHAR *Sandboxie_WindowClassName = L"Sandboxie_BorderWindow";


void WINAPI CBoxBorder__TimerProc(HWND hwnd, UINT uMsg, UINT_PTR dwTimerID, DWORD dwTime)
{
	CBoxBorder* This = (CBoxBorder*)GetWindowLongPtr(hwnd, 0);
	This->TimerProc();
}

LRESULT CALLBACK CBoxBorder__WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CBoxBorder* This = (CBoxBorder*)GetWindowLongPtr(hwnd, 0);
	if (uMsg == WM_PAINT && This) {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		SBoxBorder* m = This->GetBorderData();
		if (m && m->LabelMode != 0 && !m->BoxName.empty() && m->LabelFont) {
			HFONT hOldFont = (HFONT)SelectObject(hdc, m->LabelFont);
			SetBkMode(hdc, TRANSPARENT);
			// Use contrasting text color based on border color luminance
			int r = GetRValue(m->BorderColor);
			int g = GetGValue(m->BorderColor);
			int b = GetBValue(m->BorderColor);
			int luminance = (r * 299 + g * 587 + b * 114) / 1000;
			SetTextColor(hdc, luminance > 128 ? RGB(0, 0, 0) : RGB(255, 255, 255));
			DrawTextW(hdc, m->BoxName.c_str(), -1, &m->LabelRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			SelectObject(hdc, hOldFont);
		}
		EndPaint(hwnd, &ps);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI CBoxBorder__ThreadFunc(LPVOID lpParam)
{
	CBoxBorder* This = ((CBoxBorder*)lpParam);
	This->ThreadFunc();
	return 0;
}

CBoxBorder::CBoxBorder(CSbieAPI* pApi, QObject* parent) : QObject(parent)
{
	m_Api = pApi;

	m = new SBoxBorder;

	m->hThread = CreateThread(NULL, 0, CBoxBorder__ThreadFunc, this, 0, NULL);
}

CBoxBorder::~CBoxBorder()
{
	PostThreadMessage(GetThreadId(m->hThread), WM_QUIT, 0, 0);

	if(WaitForSingleObject(m->hThread, 10*1000) == WAIT_TIMEOUT)
		TerminateThread(m->hThread, 0);

	delete m;
}

void CBoxBorder::ThreadFunc()
{
	m->pCurrentBox = NULL;
	m->BorderColor = RGB(0, 0, 0);
	m->BorderMode = 0;
	m->BorderWidth = 0;
	m->BorderAlpha = 192; // Default to 75% opacity (192/255)

	m->ActivePid = 0;
	m->ActiveWnd = NULL;

	m->IsBorderVisible = FALSE;
	m->BorderWnd = NULL;
	m->BorderBrush = NULL;
	m->LabelFont = NULL;
	m->BoxName.clear();
	m->LabelWidth = 0;
	m->LabelHeight = 0;
	m->LabelPadding = 8;
	m->LabelMode = -1; // Default to outside (above border)
	memset(&m->LabelRect, 0, sizeof(RECT));

	m->ThumbWidth = GetSystemMetrics(SM_CXHTHUMB);
	m->ThumbHeight = GetSystemMetrics(SM_CYVTHUMB);

	HMODULE dwmapi = LoadLibraryW(L"dwmapi.dll");
	if (dwmapi) {
		m->DwmIsCompositionEnabled = (P_DwmIsCompositionEnabled)GetProcAddress(dwmapi, "DwmIsCompositionEnabled");
		if (m->DwmIsCompositionEnabled) {
			m->DwmGetWindowAttribute = (P_DwmGetWindowAttribute)GetProcAddress(dwmapi, "DwmGetWindowAttribute");
			if (!m->DwmGetWindowAttribute)
				m->DwmIsCompositionEnabled = NULL;
		}
	}

	WNDCLASSEXW wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
	wc.lpfnWndProc = CBoxBorder__WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(ULONG_PTR);
	wc.hInstance = NULL; // (HINSTANCE)::GetModuleHandle(NULL);
	wc.hIcon = NULL; // ::LoadIcon(wc.hInstance, L"AAAPPICON");
	wc.hCursor = NULL; // ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = Sandboxie_WindowClassName;
	wc.hIconSm = NULL;
	if (ATOM lpClassName = RegisterClassExW(&wc))
	{
		m->BorderWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, (LPCWSTR)lpClassName,
			Sandboxie_WindowClassName, WS_POPUP | WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);
	}
	if (!m->BorderWnd)
		return;

	// Initially set to 75% opacity, will be updated when border is configured
	SetLayeredWindowAttributes(m->BorderWnd, 0, 192, LWA_ALPHA);
	::ShowWindow(m->BorderWnd, SW_HIDE);

	SetWindowLongPtr(m->BorderWnd, 0, ULONG_PTR(this));

	m->FastTimerStartTicks = 0;
	m->dwTimerId = SetTimer(m->BorderWnd, 0, 100, CBoxBorder__TimerProc);

	MSG  msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	KillTimer(m->BorderWnd, m->dwTimerId);

	if (m->BorderWnd)
	{
		DestroyWindow(m->BorderWnd);
		m->BorderWnd = NULL;
	}

	if (m->LabelFont) {
		DeleteObject(m->LabelFont);
		m->LabelFont = NULL;
	}
}

void CBoxBorder::TimerProc() 
{
    if (m->FastTimerStartTicks && GetTickCount() - m->FastTimerStartTicks >= 1000) {
        m->FastTimerStartTicks = 0;
        m->dwTimerId = SetTimer(m->BorderWnd, m->dwTimerId, 100, CBoxBorder__TimerProc);
		return;
    }

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
		{
			m->BorderMode = 0;
			m->BoxName.clear();
		}
		else
		{
			m->BorderMode = 1;
			m->BorderColor = RGB(255, 255, 0);
			m->BorderWidth = 6;
			m->BorderAlpha = 192; // Default to 75% opacity

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

					// Parse alpha value (4th parameter) - default to 192 (75% opacity) for backward compatibility
					if (BorderCfg.count() >= 4)
					{
						bool alphaOk = false;
						m->BorderAlpha = BorderCfg.at(3).toInt(&alphaOk);
						if (!alphaOk || m->BorderAlpha < 0 || m->BorderAlpha > 255)
							m->BorderAlpha = 192;
					}
					else
					{
						m->BorderAlpha = 192; // 75% opacity by default (192/255)
					}

					// Parse label mode (5th parameter) - no=disabled, in=inside, out=outside
					if (BorderCfg.count() >= 5)
					{
						if(BorderCfg.at(4) == "no")
							m->LabelMode = 0;
						else if(BorderCfg.at(4) == "out")
							m->LabelMode = -1;
						else // "in" or default
							m->LabelMode = 1;
					}
					else
					{
						m->LabelMode = 1; // Default to inside the border
					}
				}
			}

			HBRUSH hbr = CreateSolidBrush(m->BorderColor);
			SetClassLongPtr(m->BorderWnd, GCLP_HBRBACKGROUND, (LONG_PTR)hbr);
			if (m->BorderBrush)
				DeleteObject(m->BorderBrush);
			m->BorderBrush = hbr;

			// Apply alpha transparency setting
			SetLayeredWindowAttributes(m->BorderWnd, 0, m->BorderAlpha, LWA_ALPHA);

			// Store sandbox name and calculate label dimensions
			m->BoxName = pProcessBox->GetName().toStdWString();

			// Create font for label (bold, sized to fit in the border)
			if (m->LabelFont) {
				DeleteObject(m->LabelFont);
				m->LabelFont = NULL;
			}
			int fontHeight = m->BorderWidth + 8; // Slightly larger than border width
			m->LabelFont = CreateFontW(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

			// Calculate text dimensions
			if (m->LabelFont) {
				HDC hdc = GetDC(m->BorderWnd);
				HFONT hOldFont = (HFONT)SelectObject(hdc, m->LabelFont);
				SIZE textSize;
				GetTextExtentPoint32W(hdc, m->BoxName.c_str(), (int)m->BoxName.length(), &textSize);
				SelectObject(hdc, hOldFont);
				ReleaseDC(m->BorderWnd, hdc);
				m->LabelWidth = textSize.cx + m->LabelPadding * 2;
				m->LabelHeight = textSize.cy + 4;
			}
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

		if (pid == m->ActivePid && hWnd == m->ActiveWnd) {
			if (memcmp(&rect, &m->ActiveRect, sizeof(RECT)) == 0) { // sane rect
				if (!m->TitleState || m->TitleState == (CheckMousePointer() ? 1 : -1))
					return;
			} 
			else { // window is being moved, increase refresh speed
                if (! m->FastTimerStartTicks)
                    m->dwTimerId = SetTimer(m->BorderWnd, m->dwTimerId, 10, CBoxBorder__TimerProc);
                m->FastTimerStartTicks = GetTickCount();
            }
		}

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
		// if its 1 px less on any side it works normally, so we pick bottom as that's where the taskbar usually is
		// but with the taskbar to the side it would also work
		//

		if (rect.bottom == Monitor.rcWork.bottom) 
			ah -= 1;


		HRGN hrgn = NULL;
		int windowOffsetY = 0;
		int windowHeight = ah;

		if (m->LabelMode == 0)
		{
			// Mode 0: No label - simple rectangular border frame
			POINT OuterPoints[5];
			OuterPoints[0] = { 0, 0 };
			OuterPoints[1] = { aw, 0 };
			OuterPoints[2] = { aw, ah };
			OuterPoints[3] = { 0, ah };
			OuterPoints[4] = { 0, 0 };

			HRGN hrgnOuter = CreatePolygonRgn(OuterPoints, 5, WINDING);

			POINT InnerPoints[4];
			InnerPoints[0] = { m->BorderWidth, m->BorderWidth };
			InnerPoints[1] = { aw - m->BorderWidth, m->BorderWidth };
			InnerPoints[2] = { aw - m->BorderWidth, ah - m->BorderWidth };
			InnerPoints[3] = { m->BorderWidth, ah - m->BorderWidth };

			HRGN hrgnInner = CreatePolygonRgn(InnerPoints, 4, WINDING);
			CombineRgn(hrgnOuter, hrgnOuter, hrgnInner, RGN_DIFF);
			DeleteObject(hrgnInner);

			hrgn = hrgnOuter;
			memset(&m->LabelRect, 0, sizeof(RECT));
		}
		else if (m->LabelMode == -1)
		{
			// Mode -1: Label outside (above border)
			int labelTabHeight = m->LabelHeight;
			int labelTabWidth = m->LabelWidth;
			int labelTabLeft = (aw - labelTabWidth) / 2;
			int labelTabRight = labelTabLeft + labelTabWidth;

			if (labelTabLeft < m->BorderWidth)
				labelTabLeft = m->BorderWidth;
			if (labelTabRight > aw - m->BorderWidth)
				labelTabRight = aw - m->BorderWidth;

			POINT OuterPoints[8];
			int OuterCount = 0;
			OuterPoints[OuterCount++] = { 0, labelTabHeight };
			OuterPoints[OuterCount++] = { labelTabLeft, labelTabHeight };
			OuterPoints[OuterCount++] = { labelTabLeft, 0 };
			OuterPoints[OuterCount++] = { labelTabRight, 0 };
			OuterPoints[OuterCount++] = { labelTabRight, labelTabHeight };
			OuterPoints[OuterCount++] = { aw, labelTabHeight };
			OuterPoints[OuterCount++] = { aw, ah + labelTabHeight };
			OuterPoints[OuterCount++] = { 0, ah + labelTabHeight };

			HRGN hrgnOuter = CreatePolygonRgn(OuterPoints, OuterCount, WINDING);

			POINT InnerPoints[4];
			InnerPoints[0] = { m->BorderWidth, labelTabHeight + m->BorderWidth };
			InnerPoints[1] = { aw - m->BorderWidth, labelTabHeight + m->BorderWidth };
			InnerPoints[2] = { aw - m->BorderWidth, ah + labelTabHeight - m->BorderWidth };
			InnerPoints[3] = { m->BorderWidth, ah + labelTabHeight - m->BorderWidth };

			HRGN hrgnInner = CreatePolygonRgn(InnerPoints, 4, WINDING);
			CombineRgn(hrgnOuter, hrgnOuter, hrgnInner, RGN_DIFF);
			DeleteObject(hrgnInner);

			m->LabelRect.left = labelTabLeft;
			m->LabelRect.top = 0;
			m->LabelRect.right = labelTabRight;
			m->LabelRect.bottom = labelTabHeight;

			hrgn = hrgnOuter;
			windowOffsetY = -labelTabHeight;
			windowHeight = ah + labelTabHeight;
		}
		else // m->LabelMode == 1
		{
			// Mode 1: Label inside (thicker section at top center, extending into window area)
			int labelTabHeight = m->LabelHeight;
			int labelTabWidth = m->LabelWidth;
			int labelTabLeft = (aw - labelTabWidth) / 2;
			int labelTabRight = labelTabLeft + labelTabWidth;

			if (labelTabLeft < m->BorderWidth)
				labelTabLeft = m->BorderWidth;
			if (labelTabRight > aw - m->BorderWidth)
				labelTabRight = aw - m->BorderWidth;

			// Outer rectangle is normal
			POINT OuterPoints[5];
			OuterPoints[0] = { 0, 0 };
			OuterPoints[1] = { aw, 0 };
			OuterPoints[2] = { aw, ah };
			OuterPoints[3] = { 0, ah };
			OuterPoints[4] = { 0, 0 };

			HRGN hrgnOuter = CreatePolygonRgn(OuterPoints, 5, WINDING);

			// Inner region: top at BorderWidth, but center section pushed down for label
			POINT InnerPoints[8];
			int InnerCount = 0;
			InnerPoints[InnerCount++] = { m->BorderWidth, m->BorderWidth };
			InnerPoints[InnerCount++] = { labelTabLeft, m->BorderWidth };
			InnerPoints[InnerCount++] = { labelTabLeft, m->BorderWidth + labelTabHeight };
			InnerPoints[InnerCount++] = { labelTabRight, m->BorderWidth + labelTabHeight };
			InnerPoints[InnerCount++] = { labelTabRight, m->BorderWidth };
			InnerPoints[InnerCount++] = { aw - m->BorderWidth, m->BorderWidth };
			InnerPoints[InnerCount++] = { aw - m->BorderWidth, ah - m->BorderWidth };
			InnerPoints[InnerCount++] = { m->BorderWidth, ah - m->BorderWidth };

			HRGN hrgnInner = CreatePolygonRgn(InnerPoints, InnerCount, WINDING);
			CombineRgn(hrgnOuter, hrgnOuter, hrgnInner, RGN_DIFF);
			DeleteObject(hrgnInner);

			m->LabelRect.left = labelTabLeft;
			m->LabelRect.top = m->BorderWidth;
			m->LabelRect.right = labelTabRight;
			m->LabelRect.bottom = m->BorderWidth + labelTabHeight;

			hrgn = hrgnOuter;
		}

		SetWindowRgn(m->BorderWnd, hrgn, TRUE);
		SetWindowPos(m->BorderWnd, NULL, ax, ay + windowOffsetY, aw, windowHeight, SWP_SHOWWINDOW | SWP_NOACTIVATE);
		InvalidateRect(m->BorderWnd, NULL, TRUE);

		m->IsBorderVisible = TRUE;
	}

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
