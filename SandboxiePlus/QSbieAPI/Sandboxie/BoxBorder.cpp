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
#include <vector>
#include <algorithm>
#include <map>
#include <set>

typedef HRESULT(*P_DwmIsCompositionEnabled)(BOOL *enabled);
typedef HRESULT(*P_DwmGetWindowAttribute)(HWND hWnd, DWORD dwAttribute, void *pvAttribute, DWORD cbAttribute);

// Structure to hold border window data (used for both main and per-box borders)
struct SBoxBorderWnd
{
	HWND hWnd;
	COLORREF color;
	int width;
	int alpha;
	bool visible;
	std::wstring boxName;
	HFONT labelFont;
	int labelMode; // 0=disabled, -1=outside (above border), 1=inside (below border line)
	int labelWidth;
	int labelHeight;
	int labelPadding;
	std::vector<RECT> labelRects; // Label positions (one per window for per-box, one for main)
};

enum EBorderMode { eBorderOff = 0, eBorderNormal = 1, eBorderTitleOnly = 2, eBorderAllWindows = 3 };

struct SBoxBorder
{
	HANDLE hThread;
	UINT_PTR dwTimerId;
	int FastTimerStartTicks;

	CSandBox* pCurrentBox;
	EBorderMode BorderMode;

	ULONG ActivePid;
	HWND ActiveWnd;
	RECT ActiveRect;
	RECT TitleRect;
	int  TitleState;

	int ThumbWidth;
	int ThumbHeight;

	P_DwmIsCompositionEnabled DwmIsCompositionEnabled;
	P_DwmGetWindowAttribute DwmGetWindowAttribute;

	// Main border window for in-focus mode
	SBoxBorderWnd MainBorder;

	// Per-box border windows for AllBordersMode
	std::map<CSandBox*, SBoxBorderWnd> BoxBorderWnds;
};

const WCHAR *Sandboxie_WindowClassName = L"Sandboxie_BorderWindow";


void WINAPI CBoxBorder__TimerProc(HWND hwnd, UINT uMsg, UINT_PTR dwTimerID, DWORD dwTime)
{
	CBoxBorder* This = (CBoxBorder*)GetWindowLongPtr(hwnd, 0);
	This->TimerProc();
}

// Calculates centered label rect for a window, clamped to border width
// windowRect: the window rectangle (can be in any coordinate system)
// borderWidth: width of the border
// labelWidth, labelHeight: dimensions of the label
// labelMode: -1=outside (above), 1=inside (below top border)
// Returns: label rect in same coordinate system as windowRect
static RECT CalculateLabelRect(const RECT& windowRect, int borderWidth, int labelWidth, int labelHeight, int labelMode)
{
	int winWidth = windowRect.right - windowRect.left;

	// Calculate centered horizontal position, clamped to border width
	int labelLeft = windowRect.left + (winWidth - labelWidth) / 2;
	if (labelLeft < windowRect.left + borderWidth)
		labelLeft = windowRect.left + borderWidth;
	int labelRight = labelLeft + labelWidth;
	if (labelRight > windowRect.right - borderWidth)
		labelRight = windowRect.right - borderWidth;

	// Calculate vertical position based on label mode
	int labelTop, labelBottom;
	if (labelMode == -1) { // outside (above window)
		labelTop = windowRect.top - labelHeight;
		labelBottom = windowRect.top;
	} else { // inside (below top border)
		labelTop = windowRect.top + borderWidth;
		labelBottom = labelTop + labelHeight;
	}

	return { labelLeft, labelTop, labelRight, labelBottom };
}

// Creates/updates the label font and calculates label dimensions for a border window
static void UpdateBorderLabelFont(SBoxBorderWnd& bwnd, HWND hWndForDC = NULL)
{
	// Delete old font if exists
	if (bwnd.labelFont) {
		DeleteObject(bwnd.labelFont);
		bwnd.labelFont = NULL;
	}

	// Skip if labels are disabled or no box name
	if (bwnd.labelMode == 0 || bwnd.boxName.empty()) {
		bwnd.labelWidth = 0;
		bwnd.labelHeight = 0;
		return;
	}

	// Create font sized to fit in the border
	int fontHeight = bwnd.width + 8;
	bwnd.labelFont = CreateFontW(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

	if (!bwnd.labelFont) {
		bwnd.labelWidth = 0;
		bwnd.labelHeight = 0;
		return;
	}

	// Calculate text dimensions
	HDC hdc = GetDC(hWndForDC); // NULL = screen DC
	HFONT hOldFont = (HFONT)SelectObject(hdc, bwnd.labelFont);
	SIZE textSize;
	GetTextExtentPoint32W(hdc, bwnd.boxName.c_str(), (int)bwnd.boxName.length(), &textSize);
	SelectObject(hdc, hOldFont);
	ReleaseDC(hWndForDC, hdc);

	bwnd.labelWidth = textSize.cx + bwnd.labelPadding * 2;
	bwnd.labelHeight = textSize.cy + 4;
}

static SBoxBorderWnd* GetBorderWndByHwnd(SBoxBorder* m, HWND hWnd)
{
	// Check main border window
	if (m->MainBorder.hWnd == hWnd)
		return &m->MainBorder;

	// Check per-box border windows
	for (auto& pair : m->BoxBorderWnds) {
		if (pair.second.hWnd == hWnd)
			return &pair.second;
	}

	return NULL;
}

// Gets monitor info for a window and checks if the window should have a border drawn
// Returns true if window should be processed, false if it should be skipped (no monitor or fullscreen)
// If returns true, fills in monitorInfo for later use
static bool ShouldDrawBorderForWindow(HWND hWnd, const RECT& rect, ULONG style, MONITORINFO* pMonitorInfo)
{
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);
	if (!hMonitor)
		return false;

	MONITORINFO monitor;
	memset(&monitor, 0, sizeof(MONITORINFO));
	monitor.cbSize = sizeof(MONITORINFO);
	if (!GetMonitorInfo(hMonitor, &monitor))
		return false;

	// Check if window is fullscreen (covers entire monitor without caption)
	const RECT* desktop = &monitor.rcMonitor;
	if (rect.left <= desktop->left && rect.top <= desktop->top &&
		rect.right >= desktop->right && rect.bottom >= desktop->bottom &&
		(style & WS_CAPTION) != WS_CAPTION)
	{
		return false; // Skip fullscreen windows
	}

	if (pMonitorInfo)
		*pMonitorInfo = monitor;

	return true;
}

// Adjusts a window rect to stay within desktop bounds and applies taskbar workaround
// rect: the original window rect
// monitor: monitor info from ShouldDrawBorderForWindow
// borderWidth: border width for edge clipping threshold
// Returns the adjusted rect suitable for border drawing
static RECT AdjustRectToDesktop(const RECT& rect, const MONITORINFO& monitor, int borderWidth)
{
	const RECT* desktop = &monitor.rcMonitor;

	// Clip to desktop edges if window is slightly off-screen (within borderWidth + 4 pixels)
	int ax = rect.left;
	if (rect.left < desktop->left && (desktop->left - rect.left) < (borderWidth + 4))
		ax = desktop->left;

	int ay = rect.top;
	if (rect.top < desktop->top && (desktop->top - rect.top) < (borderWidth + 4))
		ay = desktop->top;

	int aw = -ax;
	if (rect.right > desktop->right && (rect.right - desktop->right) < (borderWidth + 4))
		aw += desktop->right;
	else
		aw += rect.right;

	int ah = -ay;
	if (rect.bottom > desktop->bottom && (rect.bottom - desktop->bottom) < (borderWidth + 4))
		ah += desktop->bottom;
	else
		ah += rect.bottom;

	// Taskbar workaround for Windows 10/11:
	// If window bottom matches work area bottom, reduce by 1px to avoid hiding auto-hide taskbar
	if (rect.bottom == monitor.rcWork.bottom)
		ah -= 1;

	return { ax, ay, ax + aw, ay + ah };
}

LRESULT CALLBACK CBoxBorder__WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CBoxBorder* This = (CBoxBorder*)GetWindowLongPtr(hwnd, 0);
	if (!This)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	SBoxBorder* m = This->GetBorderData();

	// Handle WM_ERASEBKGND to paint the border with the correct color
	// Works for both the main in-focus border window and per-box border windows
	if (uMsg == WM_ERASEBKGND) 
	{
		SBoxBorderWnd* pBorder = GetBorderWndByHwnd(m, hwnd);
		if (pBorder)
		{
			HDC hdc = (HDC)wParam;
			RECT rc;
			GetClientRect(hwnd, &rc);

			HBRUSH hBrush = CreateSolidBrush(pBorder->color);
			FillRect(hdc, &rc, hBrush);
			DeleteObject(hBrush);
		}

		return 1; // We handled it
	}

	if (uMsg == WM_PAINT) 
	{
		SBoxBorderWnd* pBorder = GetBorderWndByHwnd(m, hwnd);
		if (pBorder && pBorder->labelMode != 0 && !pBorder->boxName.empty() && pBorder->labelFont && !pBorder->labelRects.empty()) 
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			// Draw label at each stored rect (one per window in this box)
			for (const RECT& labelRect : pBorder->labelRects) 
			{
				HFONT hOldFont = (HFONT)SelectObject(hdc, pBorder->labelFont);
				SetBkMode(hdc, TRANSPARENT);
				// Use contrasting text color based on border color luminance
				int r = GetRValue(pBorder->color);
				int g = GetGValue(pBorder->color);
				int b = GetBValue(pBorder->color);
				int luminance = (r * 299 + g * 587 + b * 114) / 1000;
				SetTextColor(hdc, luminance > 128 ? RGB(0, 0, 0) : RGB(255, 255, 255));
				RECT rc = labelRect; // DrawText may modify the rect
				DrawTextW(hdc, pBorder->boxName.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				SelectObject(hdc, hOldFont);
			}
			EndPaint(hwnd, &ps);
		}

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

static HWND CreateBoxBorderWindow()
{
	// Create a border window (used for both main border and per-box borders)
	HWND hWnd = CreateWindowExW(
		WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		Sandboxie_WindowClassName,
		Sandboxie_WindowClassName,
		WS_POPUP | WS_CLIPSIBLINGS,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL
	);
	if (hWnd) {
		SetLayeredWindowAttributes(hWnd, 0, 192, LWA_ALPHA);
		::ShowWindow(hWnd, SW_HIDE);
	}
	return hWnd;
}

void CBoxBorder::ThreadFunc()
{
	m->pCurrentBox = NULL;
	m->MainBorder.color = RGB(0, 0, 0);
	m->BorderMode = eBorderOff;
	m->MainBorder.width = 0;
	m->MainBorder.alpha = 192; // Default to 75% opacity (192/255)

	m->ActivePid = 0;
	m->ActiveWnd = NULL;

	m->MainBorder.visible = FALSE;
	m->MainBorder.hWnd = NULL;
	m->MainBorder.labelFont = NULL;
	m->MainBorder.boxName.clear();
	m->MainBorder.labelWidth = 0;
	m->MainBorder.labelHeight = 0;
	m->MainBorder.labelPadding = 8;
	m->MainBorder.labelMode = -1; // Default to outside (above border)
	m->MainBorder.labelRects.clear();
	m->MainBorder.labelRects.push_back({ 0, 0, 0, 0 });

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
	wc.hInstance = NULL;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL; // Let WM_ERASEBKGND handle painting
	wc.lpszMenuName = NULL;
	wc.lpszClassName = Sandboxie_WindowClassName;
	wc.hIconSm = NULL;
	RegisterClassExW(&wc);

	// Create main border window using shared function
	m->MainBorder.hWnd = CreateBoxBorderWindow();
	if (!m->MainBorder.hWnd)
		return;
	SetWindowLongPtr(m->MainBorder.hWnd, 0, ULONG_PTR(this));

	m->FastTimerStartTicks = 0;
	m->dwTimerId = SetTimer(m->MainBorder.hWnd, 0, 100, CBoxBorder__TimerProc);

	MSG  msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	KillTimer(m->MainBorder.hWnd, m->dwTimerId);

	if (m->MainBorder.hWnd)
	{
		DestroyWindow(m->MainBorder.hWnd);
		m->MainBorder.hWnd = NULL;
	}

	// Cleanup per-box border windows
	for (auto& pair : m->BoxBorderWnds)
	{
		if (pair.second.hWnd)
			DestroyWindow(pair.second.hWnd);
		if (pair.second.labelFont)
			DeleteObject(pair.second.labelFont);
	}
	m->BoxBorderWnds.clear();

	if (m->MainBorder.labelFont) {
		DeleteObject(m->MainBorder.labelFont);
		m->MainBorder.labelFont = NULL;
	}
}

static bool GetBoxBorderSettings(CSandBox* pBox, COLORREF& color, int& width, int& alpha, EBorderMode& mode, int& labelMode)
{
	// Default values
	color = RGB(255, 255, 0);
	width = 6;
	alpha = 192;
	mode = eBorderNormal;
	labelMode = 1; // Default to inside

	if (!pBox)
		return false;

	QStringList BorderCfg = pBox->GetText("BorderColor").split(",");
	if (BorderCfg.first().left(1) != L'#')
		return true; // Use defaults

	bool ok = false;
	color = BorderCfg.first().mid(1).toInt(&ok, 16);
	if (!ok) {
		color = RGB(255, 255, 0);
		return true;
	}

	if (BorderCfg.count() >= 2) {
		QString StrMode = BorderCfg.at(1);
		if (StrMode.compare("off", Qt::CaseInsensitive) == 0) {
			mode = eBorderOff;
			return false; // Border disabled for this box
		}
		else if (StrMode.compare("ttl", Qt::CaseInsensitive) == 0)
			mode = eBorderTitleOnly;
		else if (StrMode.compare("all", Qt::CaseInsensitive) == 0)
			mode = eBorderAllWindows;
		// else default is eBorderNormal
	}

	if (BorderCfg.count() >= 3) {
		width = BorderCfg.at(2).toInt();
		if (!width) width = 6;
	}

	if (BorderCfg.count() >= 4) {
		bool alphaOk = false;
		alpha = BorderCfg.at(3).toInt(&alphaOk);
		if (!alphaOk || alpha < 0 || alpha > 255)
			alpha = 192;
	}

	if (BorderCfg.count() >= 5) {
		QString labelStr = BorderCfg.at(4);
		if (labelStr.compare("no", Qt::CaseInsensitive) == 0)
			labelMode = 0;
		else if (labelStr.compare("out", Qt::CaseInsensitive) == 0)
			labelMode = -1;
		else // "in" or default
			labelMode = 1;
	}

	return true;
}

static HRGN CreateBorderRegion(const RECT* rect, int borderWidth)
{
	// Create outer rectangle region
	HRGN hrgnOuter = CreateRectRgn(rect->left, rect->top, rect->right, rect->bottom);

	// Create inner rectangle region (the hollow part)
	HRGN hrgnInner = CreateRectRgn(
		rect->left + borderWidth,
		rect->top + borderWidth,
		rect->right - borderWidth,
		rect->bottom - borderWidth
	);

	// Subtract inner from outer to create frame
	CombineRgn(hrgnOuter, hrgnOuter, hrgnInner, RGN_DIFF);
	DeleteObject(hrgnInner);

	return hrgnOuter;
}

void CBoxBorder::TimerProc()
{
    if (m->FastTimerStartTicks && GetTickCount() - m->FastTimerStartTicks >= 1000) {
        m->FastTimerStartTicks = 0;
        m->dwTimerId = SetTimer(m->MainBorder.hWnd, m->dwTimerId, 100, CBoxBorder__TimerProc);
		return;
    }

	// Check if any box has AllBordersMode enabled - draw borders for those boxes
	if (!m->BoxBorderWnds.empty() || CheckGlobalAllBordersMode())
	{
		DrawAllSandboxedBorders();
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

	// Get border settings for the focused window's box
	COLORREF boxColor;
	int boxWidth, boxAlpha, boxLabelMode;
	EBorderMode boxMode = eBorderOff;
	if (pProcessBox)
		GetBoxBorderSettings(pProcessBox.data(), boxColor, boxWidth, boxAlpha, boxMode, boxLabelMode);

	if (m->pCurrentBox != pProcessBox.data())
	{
		m->pCurrentBox = pProcessBox.data();
		if (!m->pCurrentBox || boxMode == eBorderOff)
		{
			m->BorderMode = eBorderOff;
		}
		else if (boxMode == eBorderAllWindows)
		{
			// This box uses "all" mode, skip single-window border (handled by DrawAllSandboxedBorders)
			m->BorderMode = eBorderOff;
		}
		else
		{
			// Normal or title-only mode
			m->BorderMode = (boxMode == eBorderTitleOnly) ? eBorderTitleOnly : eBorderNormal;
			m->MainBorder.color = boxColor;
			m->MainBorder.width = boxWidth;
			m->MainBorder.alpha = boxAlpha;
			m->MainBorder.labelMode = boxLabelMode;

			// Apply alpha transparency setting
			SetLayeredWindowAttributes(m->MainBorder.hWnd, 0, m->MainBorder.alpha, LWA_ALPHA);

			// Store sandbox name and create label font
			m->MainBorder.boxName = pProcessBox->GetName().toStdWString();
			UpdateBorderLabelFont(m->MainBorder, m->MainBorder.hWnd);
		}
	}

	if (m->BorderMode == eBorderOff) // no border enabled or unsandboxed
	{
		m->ActiveWnd = NULL;
		m->ActivePid = 0;

		if (m->MainBorder.visible)
		{
			::ShowWindow(m->MainBorder.hWnd, SW_HIDE);
			m->MainBorder.visible = FALSE;
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
                    m->dwTimerId = SetTimer(m->MainBorder.hWnd, m->dwTimerId, 10, CBoxBorder__TimerProc);
                m->FastTimerStartTicks = GetTickCount();
            }
		}

		if (m->MainBorder.visible)
			::ShowWindow(m->MainBorder.hWnd, SW_HIDE);
		m->MainBorder.visible = FALSE;

		m->ActiveWnd = hWnd;
		m->ActivePid = pid;
		memcpy(&m->ActiveRect, &rect, sizeof(RECT));
		m->TitleState = 0;
		if (rect.right - rect.left <= 2 || rect.bottom - rect.top <= 2)
			return;

		// Use shared helper for monitor info and fullscreen check
		MONITORINFO Monitor;
		if (!ShouldDrawBorderForWindow(hWnd, rect, Style, &Monitor))
			return;

		if (m->BorderMode == eBorderTitleOnly) {
			const RECT* Desktop = &Monitor.rcMonitor;
			if(!IsMounseOnTitle(hWnd, &rect, Desktop))
				return;
		}

		// Use shared helper for edge clipping and taskbar adjustment
		RECT adjustedRect = AdjustRectToDesktop(rect, Monitor, m->MainBorder.width);
		int rectWidth = adjustedRect.right - adjustedRect.left;
		int rectHeight = adjustedRect.bottom - adjustedRect.top;

		// Calculate window rect (may extend upward for outside label mode)
		RECT windowRect = adjustedRect;
		int labelOffset = 0;
		if (m->MainBorder.labelMode == -1 && m->MainBorder.labelHeight > 0) {
			windowRect.top -= m->MainBorder.labelHeight;
			labelOffset = m->MainBorder.labelHeight;
		}

		// Create border frame region in window coordinates
		RECT frameRect = { 0, labelOffset, rectWidth, labelOffset + rectHeight };
		HRGN hrgnBorder = CreateBorderRegion(&frameRect, m->MainBorder.width);

		// Add label region if enabled
		m->MainBorder.labelRects.clear();
		if (m->MainBorder.labelMode != 0 && m->MainBorder.labelHeight > 0)
		{
			// Calculate label rect in screen coordinates using shared helper
			RECT labelRectScr = CalculateLabelRect(adjustedRect, m->MainBorder.width, m->MainBorder.labelWidth, m->MainBorder.labelHeight, m->MainBorder.labelMode);

			// Convert to window-relative coordinates
			RECT labelRect;
			labelRect.left = labelRectScr.left - windowRect.left;
			labelRect.right = labelRectScr.right - windowRect.left;
			labelRect.top = labelRectScr.top - windowRect.top;
			labelRect.bottom = labelRectScr.bottom - windowRect.top;

			m->MainBorder.labelRects.push_back(labelRect);

			// Add label region to border
			HRGN hrgnLabel = CreateRectRgn(labelRect.left, labelRect.top, labelRect.right, labelRect.bottom);
			CombineRgn(hrgnBorder, hrgnBorder, hrgnLabel, RGN_OR);
			DeleteObject(hrgnLabel);
		}

		// Position and show the border window
		SetWindowRgn(m->MainBorder.hWnd, hrgnBorder, TRUE);
		SetWindowPos(m->MainBorder.hWnd, NULL,
			windowRect.left, windowRect.top,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			SWP_SHOWWINDOW | SWP_NOACTIVATE);
		InvalidateRect(m->MainBorder.hWnd, NULL, TRUE);

		m->MainBorder.visible = true;
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

void CBoxBorder::DrawAllSandboxedBorders()
{
	// Structure to hold window info for all-borders mode
	struct SWindowInfo
	{
		HWND hWnd;
		RECT rect;
		MONITORINFO monitorInfo; // Monitor info for edge clipping
		int zOrder; // Lower value = higher in Z-order (on top)
		CSandBox* pBox; // NULL if not sandboxed
	};

	// Collect ALL windows (sandboxed and non-sandboxed) in Z-order
	std::vector<SWindowInfo> allWindows;

	// Enumerate all top-level windows in Z-order
	int zOrder = 0;
	HWND hWnd = GetTopWindow(NULL);
	while (hWnd)
	{
		// Skip our border windows
		bool isBorderWnd = (hWnd == m->MainBorder.hWnd);
		for (auto& bwnd : m->BoxBorderWnds) {
			if (bwnd.second.hWnd == hWnd) {
				isBorderWnd = true;
				break;
			}
		}

		if (!isBorderWnd && IsWindowVisible(hWnd) && !IsIconic(hWnd))
		{
			ULONG Style = GetWindowLong(hWnd, GWL_STYLE);
			ULONG ExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
			if (Style & WS_VISIBLE)
			{
				// Skip menus, tooltips, and other transient popup windows
				// Check window class name for known popup types
				WCHAR className[64] = { 0 };
				GetClassNameW(hWnd, className, 64);

				bool isTransientPopup = false;
				// #32768 = Menu, ComboLBox = Combo dropdown, tooltips_class32 = Tooltips
				if (wcscmp(className, L"#32768") == 0 ||           // Menu
					wcscmp(className, L"ComboLBox") == 0 ||        // Combo box dropdown
					wcscmp(className, L"tooltips_class32") == 0 || // Tooltips
					wcscmp(className, L"SysShadow") == 0 ||        // Shadow windows
					wcscmp(className, L"DropDown") == 0)           // Generic dropdowns
				{
					isTransientPopup = true;
				}

				// Also skip tool windows and popup windows without caption (likely menus/dropdowns)
				if ((ExStyle & WS_EX_TOOLWINDOW) && !(Style & WS_CAPTION))
					isTransientPopup = true;

				// Skip windows marked as not activatable with no caption (transient popups)
				if ((ExStyle & WS_EX_NOACTIVATE) && (Style & WS_POPUP) && !(Style & WS_CAPTION))
					isTransientPopup = true;

				if (!isTransientPopup)
				{
					SWindowInfo wnd;
					wnd.hWnd = hWnd;
					wnd.zOrder = zOrder;
					GetActiveWindowRect(hWnd, &wnd.rect);
					memset(&wnd.monitorInfo, 0, sizeof(MONITORINFO));

					// Check if this is a sandboxed window
					ULONG pid = 0;
					GetWindowThreadProcessId(hWnd, &pid);
					CSandBoxPtr pBox = m_Api->GetBoxByProcessId(pid);
					wnd.pBox = pBox.data();

					// For sandboxed windows, check fullscreen and get monitor info
					// Skip fullscreen sandboxed windows (no border) but still track for Z-order
					if (wnd.pBox)
					{
						if (!ShouldDrawBorderForWindow(hWnd, wnd.rect, Style, &wnd.monitorInfo))
						{
							// Fullscreen sandboxed window - track for Z-order but don't draw border
							wnd.pBox = NULL;
						}
					}

					// Only add if window has valid size
					if (wnd.rect.right - wnd.rect.left > 2 && wnd.rect.bottom - wnd.rect.top > 2)
						allWindows.push_back(wnd);
				}
			}
		}

		zOrder++;
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}

	// Collect per-box border regions
	// Map from box to its border region and settings
	struct SBoxWindowInfo {
		RECT rect;
		HRGN hrgnCoveredAbove; // Covered area above this specific window
	};
	struct SBoxBorderData {
		HRGN hrgnBorder;
		RECT boundingRect;
		COLORREF color;
		int width;
		int alpha;
		int labelMode;
		std::wstring boxName;
		bool hasWindows;
		std::vector<SBoxWindowInfo> windows; // Individual windows for per-window labels
	};
	std::map<CSandBox*, SBoxBorderData> boxBorders;

	// Initialize border regions for each active box that has "all" mode enabled
	for (const auto& wnd : allWindows)
	{
		if (wnd.pBox && boxBorders.find(wnd.pBox) == boxBorders.end())
		{
			COLORREF color;
			int width, alpha, labelMode;
			EBorderMode mode;

			// Only add boxes that have "all" mode enabled
			if (GetBoxBorderSettings(wnd.pBox, color, width, alpha, mode, labelMode) && mode == eBorderAllWindows)
			{
				SBoxBorderData data;
				data.hrgnBorder = CreateRectRgn(0, 0, 0, 0);
				data.boundingRect = { 0, 0, 0, 0 };
				data.color = color;
				data.width = width;
				data.alpha = alpha;
				data.labelMode = labelMode;
				data.boxName = ((CSandBox*)wnd.pBox)->GetName().toStdWString();
				data.hasWindows = false;
				boxBorders[wnd.pBox] = data;
			}
		}
	}

	// Track covered area (all windows processed so far, from top to bottom)
	HRGN hrgnCovered = CreateRectRgn(0, 0, 0, 0);

	// Process windows from top to bottom (already in Z-order)
	for (const auto& wnd : allWindows)
	{
		if (wnd.pBox)
		{
			// This is a sandboxed window - create its border
			auto it = boxBorders.find(wnd.pBox);
			if (it != boxBorders.end())
			{
				SBoxBorderData& data = it->second;

				// Apply edge clipping and taskbar adjustment if we have monitor info
				RECT adjustedRect = wnd.rect;
				if (wnd.monitorInfo.cbSize != 0)
					adjustedRect = AdjustRectToDesktop(wnd.rect, wnd.monitorInfo, data.width);

				// Create border region for this window (using adjusted rect)
				HRGN hrgnBorder = CreateBorderRegion(&adjustedRect, data.width);

				// Subtract the covered area (all windows above) from this border
				CombineRgn(hrgnBorder, hrgnBorder, hrgnCovered, RGN_DIFF);

				// Add this border to the box's combined border region
				CombineRgn(data.hrgnBorder, data.hrgnBorder, hrgnBorder, RGN_OR);
				DeleteObject(hrgnBorder);

				// Track this window for per-window labels (using adjusted rect)
				SBoxWindowInfo winInfo;
				winInfo.rect = adjustedRect;
				// Save covered area ABOVE this window for label clipping
				winInfo.hrgnCoveredAbove = CreateRectRgn(0, 0, 0, 0);
				CombineRgn(winInfo.hrgnCoveredAbove, hrgnCovered, NULL, RGN_COPY);
				data.windows.push_back(winInfo);

				// Update bounding rect (using adjusted rect)
				if (!data.hasWindows) {
					data.boundingRect = adjustedRect;
					data.hasWindows = true;
				} else {
					if (adjustedRect.left < data.boundingRect.left) data.boundingRect.left = adjustedRect.left;
					if (adjustedRect.top < data.boundingRect.top) data.boundingRect.top = adjustedRect.top;
					if (adjustedRect.right > data.boundingRect.right) data.boundingRect.right = adjustedRect.right;
					if (adjustedRect.bottom > data.boundingRect.bottom) data.boundingRect.bottom = adjustedRect.bottom;
				}
			}
		}

		// Add this window's rectangle to covered area (both sandboxed and non-sandboxed)
		HRGN hrgnWindow = CreateRectRgn(wnd.rect.left, wnd.rect.top, wnd.rect.right, wnd.rect.bottom);
		CombineRgn(hrgnCovered, hrgnCovered, hrgnWindow, RGN_OR);
		DeleteObject(hrgnWindow);
	}

	// Note: Don't delete hrgnCovered yet - we need it to clip label regions

	// Update per-box border windows
	std::set<CSandBox*> activeBoxes;
	for (auto& pair : boxBorders)
	{
		CSandBox* pBox = pair.first;
		SBoxBorderData& data = pair.second;

		if (!data.hasWindows)
		{
			DeleteObject(data.hrgnBorder);
			// Clean up any window info (shouldn't have any, but just in case)
			for (auto& winInfo : data.windows) {
				if (winInfo.hrgnCoveredAbove)
					DeleteObject(winInfo.hrgnCoveredAbove);
			}
			continue;
		}

		activeBoxes.insert(pBox);

		// Get or create border window for this box
		auto it = m->BoxBorderWnds.find(pBox);
		if (it == m->BoxBorderWnds.end())
		{
			SBoxBorderWnd bwnd;
			bwnd.hWnd = CreateBoxBorderWindow();
			SetWindowLongPtr(bwnd.hWnd, 0, ULONG_PTR(this));
			bwnd.color = 0;
			bwnd.width = 0;
			bwnd.alpha = 0;
			bwnd.visible = false;
			bwnd.boxName.clear();
			bwnd.labelFont = NULL;
			bwnd.labelMode = 0;
			bwnd.labelWidth = 0;
			bwnd.labelHeight = 0;
			bwnd.labelPadding = 8;
			bwnd.labelRects.clear();
			m->BoxBorderWnds[pBox] = bwnd;
			it = m->BoxBorderWnds.find(pBox);
		}

		SBoxBorderWnd& bwnd = it->second;

		// Update color if changed (painting is handled in WM_ERASEBKGND)
		if (bwnd.color != data.color)
		{
			bwnd.color = data.color;
			// Force repaint with new color
			InvalidateRect(bwnd.hWnd, NULL, TRUE);
		}

		// Update alpha if changed
		if (bwnd.alpha != data.alpha)
		{
			SetLayeredWindowAttributes(bwnd.hWnd, 0, data.alpha, LWA_ALPHA);
			bwnd.alpha = data.alpha;
		}

		// Update label settings
		bwnd.boxName = data.boxName;
		bwnd.labelMode = data.labelMode;
		bwnd.width = data.width;

		// Store original bounding rect values before any modifications
		int originalBoundingLeft = data.boundingRect.left;
		int originalBoundingTop = data.boundingRect.top;
		int windowWidth = data.boundingRect.right - data.boundingRect.left;

		// Create/update label font and calculate dimensions
		UpdateBorderLabelFont(bwnd);

		// For outside mode, extend window upward to make room for label
		if (bwnd.labelMode == -1 && bwnd.labelHeight > 0)
		{
			data.boundingRect.top -= bwnd.labelHeight;
		}

		// Offset the border region to window-relative coordinates
		// Border region was created in screen coordinates around the ORIGINAL boundingRect
		// Use original coordinates for offset, not the modified ones
		OffsetRgn(data.hrgnBorder, -originalBoundingLeft, -originalBoundingTop);

		// For outside mode, shift border down by labelHeight since window now extends above the original top
		if (bwnd.labelMode == -1 && bwnd.labelHeight > 0)
		{
			OffsetRgn(data.hrgnBorder, 0, bwnd.labelHeight);
		}

		// Add label region for EACH window in this box
		// Each window gets its own label centered on that window's border
		bwnd.labelRects.clear(); // Clear old label rects
		if (bwnd.labelMode != 0 && bwnd.labelHeight > 0 && bwnd.labelFont)
		{
			// Add a label for each window in this box
			for (auto& winInfo : data.windows)
			{
				// Calculate label rect in screen coordinates using shared helper
				RECT labelRectScr = CalculateLabelRect(winInfo.rect, data.width, bwnd.labelWidth, bwnd.labelHeight, bwnd.labelMode);

				// Create label region in screen coordinates
				HRGN hrgnLabelScr = CreateRectRgn(labelRectScr.left, labelRectScr.top, labelRectScr.right, labelRectScr.bottom);

				// Clip against windows ABOVE this specific window
				if (winInfo.hrgnCoveredAbove)
					CombineRgn(hrgnLabelScr, hrgnLabelScr, winInfo.hrgnCoveredAbove, RGN_DIFF);

				// Convert to window coordinates (relative to border window origin)
				// Note: data.boundingRect.top was already extended upward for outside mode,
				// so subtracting it gives the correct window-relative position
				RECT labelRectWnd = {
					labelRectScr.left - data.boundingRect.left,
					labelRectScr.top - data.boundingRect.top,
					labelRectScr.right - data.boundingRect.left,
					labelRectScr.bottom - data.boundingRect.top
				};

				// Store the label rect for WM_PAINT
				bwnd.labelRects.push_back(labelRectWnd);

				// Also offset and add the region
				OffsetRgn(hrgnLabelScr, -data.boundingRect.left, -data.boundingRect.top);

				// Add to border region
				CombineRgn(data.hrgnBorder, data.hrgnBorder, hrgnLabelScr, RGN_OR);
				DeleteObject(hrgnLabelScr);
			}
		}

		// Clean up the per-window covered area regions
		for (auto& winInfo : data.windows)
		{
			if (winInfo.hrgnCoveredAbove)
			{
				DeleteObject(winInfo.hrgnCoveredAbove);
				winInfo.hrgnCoveredAbove = NULL;
			}
		}

		// Position and show the border window
		SetWindowRgn(bwnd.hWnd, data.hrgnBorder, TRUE);
		SetWindowPos(bwnd.hWnd, NULL,
			data.boundingRect.left,
			data.boundingRect.top,
			data.boundingRect.right - data.boundingRect.left,
			data.boundingRect.bottom - data.boundingRect.top,
			SWP_SHOWWINDOW | SWP_NOACTIVATE);

		// Force repaint to show label
		InvalidateRect(bwnd.hWnd, NULL, TRUE);

		bwnd.visible = true;
		// Note: data.hrgnBorder is now owned by the window, don't delete it
	}

	// Destroy and remove border windows for boxes that no longer have visible windows
	for (auto it = m->BoxBorderWnds.begin(); it != m->BoxBorderWnds.end(); )
	{
		if (activeBoxes.find(it->first) == activeBoxes.end())
		{
			// Destroy the window and clean up resources
			if (it->second.hWnd)
				DestroyWindow(it->second.hWnd);
			if (it->second.labelFont)
				DeleteObject(it->second.labelFont);
			it = m->BoxBorderWnds.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Clean up the covered area region
	DeleteObject(hrgnCovered);
}

bool CBoxBorder::CheckGlobalAllBordersMode()
{
	// Check if any active box has AllBordersMode enabled
	QMap<quint32, CBoxedProcessPtr> processes = m_Api->GetAllProcesses();

	std::set<CSandBox*> checkedBoxes;
	for (auto it = processes.begin(); it != processes.end(); ++it)
	{
		CSandBox* pBox = it.value()->GetBoxPtr();
		if (!pBox || checkedBoxes.find(pBox) != checkedBoxes.end())
			continue;

		checkedBoxes.insert(pBox);

		COLORREF color;
		int width, alpha, labelMode;
		EBorderMode mode;

		if (GetBoxBorderSettings(pBox, color, width, alpha, mode, labelMode) && mode == eBorderAllWindows)
			return true;
	}

	return false;
}
