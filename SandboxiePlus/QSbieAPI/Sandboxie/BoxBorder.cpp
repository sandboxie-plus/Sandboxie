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
#include <unordered_map>

typedef HRESULT(*P_DwmIsCompositionEnabled)(BOOL *enabled);
typedef HRESULT(*P_DwmGetWindowAttribute)(HWND hWnd, DWORD dwAttribute, void *pvAttribute, DWORD cbAttribute);

// DWMWA_CLOAKED (14) is defined in <dwmapi.h> (SDK 8.1+) but we load dwmapi.dll dynamically
// so we don't include that header.  Define it ourselves if the SDK didn't already provide it.
// A cloaked value != 0 means the window is hidden by DWM (Peek at Desktop = DWM_CLOAKED_DWM,
// virtual-desktop switch = DWM_CLOAKED_SHELL, app-hide = DWM_CLOAKED_INHERITED).
#ifndef DWMWA_CLOAKED
#define DWMWA_CLOAKED 14
#endif

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
	ULONGLONG sceneHash;   // per-box hash at last render; used to skip rebuilding unchanged boxes
	DWORD lastChangeTick;  // GetTickCount() when this box's scene last changed (for timer management)
};

enum EBorderMode { eBorderOff = 0, eBorderNormal = 1, eBorderTitleOnly = 2, eBorderAllWindows = 3, eBorderLabelOnly = 4, eBorderAllWindowsLabelOnly = 5, eBorderTitleOnlyLabelOnly = 6 };

struct SBoxBorder
{
	HANDLE hThread;
	UINT_PTR dwTimerId;
	int FastTimerStartTicks;
	int FastTimerMs;           // 1000/refreshRate, floored at kFastMoveTimerMs (10 ms)
	int CurrentTimerIntervalMs;
	int AdaptiveOtherModeMs;
	int AdaptiveGlobalAllModeCheckMs;
	int AdaptiveSceneRefreshMs;
	DWORD LastAllModeCheckTick;
	bool CachedHasGlobalAllMode;
	bool CachedGlobalAllMode;
	ULONGLONG LastAllBordersSceneHash;
	int LastAllBordersWindowCount;
	DWORD LastAllBordersRenderTick;
	DWORD LastAllBordersEnumTick;  // when enumeration was last run
	int AdaptiveEnumIntervalMs;    // how often to run the window enumeration (backs off when stable)
	HWND LastForegroundWnd;        // last observed foreground window (for click/focus pulse detection)
	DWORD FocusRaisePulseStartTick;// tick when foreground changed; used for short z-raise pulse

	CSandBox* pCurrentBox;
	EBorderMode BorderMode;
	EBorderMode CachedFocusBoxMode; // last boxMode read for pCurrentBox; detects setting changes for the same box

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
	std::unordered_map<HWND, CSandBox*> BoxBorderWndIndex;
};

const WCHAR *Sandboxie_WindowClassName = L"Sandboxie_BorderWindow";

static const int kAdaptiveFastMs = 100;
static const int kAdaptiveMaxMs = 5000;
static const int kFastMoveTimerMs = 10; // minimum floor for frame-aligned fast timer
static const int kFocusRaisePulseMs = 250; // short z-raise window after click/focus change
static const ULONGLONG kHashSeed = 1469598103934665603ULL; // hash initialiser (FNV-inspired)

static inline void HashMix64(ULONGLONG& hash, ULONGLONG value)
{
	hash ^= value + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
}

static inline void HashMixWString(ULONGLONG& hash, const std::wstring& value)
{
	for (wchar_t ch : value)
		HashMix64(hash, (ULONGLONG)(unsigned short)ch);
	HashMix64(hash, (ULONGLONG)value.size());
}

static inline int NextAdaptiveIntervalMs(int currentMs)
{
	int nextMs = currentMs * 2;
	return (nextMs < kAdaptiveMaxMs) ? nextMs : kAdaptiveMaxMs;
}

static inline bool RectEquals(const RECT& a, const RECT& b)
{
	return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
}

static bool RectVectorEquals(const std::vector<RECT>& a, const std::vector<RECT>& b)
{
	if (a.size() != b.size())
		return false;
	for (size_t i = 0; i < a.size(); ++i) {
		if (!RectEquals(a[i], b[i]))
			return false;
	}
	return true;
}


void WINAPI CBoxBorder__TimerProc(HWND hwnd, UINT uMsg, UINT_PTR dwTimerID, DWORD dwTime)
{
	CBoxBorder* This = (CBoxBorder*)GetWindowLongPtr(hwnd, 0);
	if (!This)
		return;
	This->TimerProc();
}

static void SetBorderTimerInterval(SBoxBorder* m, int intervalMs)
{
	if (m->CurrentTimerIntervalMs == intervalMs)
		return;
	if (!m->MainBorder.hWnd || !IsWindow(m->MainBorder.hWnd))
		return;

	UINT_PTR timerId = SetTimer(m->MainBorder.hWnd, m->dwTimerId, intervalMs, CBoxBorder__TimerProc);
	if (!timerId)
		return;

	m->dwTimerId = timerId;
	m->CurrentTimerIntervalMs = intervalMs;
}

static void InitializeBorderWindowData(SBoxBorderWnd& bwnd)
{
	bwnd.hWnd = NULL;
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
	bwnd.sceneHash = 0;
	bwnd.lastChangeTick = 0;
}

static void HideBorderWindow(SBoxBorderWnd& bwnd)
{
	if (bwnd.visible && bwnd.hWnd)
	{
		::ShowWindow(bwnd.hWnd, SW_HIDE);
		bwnd.visible = false;
	}
}

static void DestroyBorderWindowResources(SBoxBorderWnd& bwnd)
{
	if (bwnd.hWnd)
	{
		DestroyWindow(bwnd.hWnd);
		bwnd.hWnd = NULL;
	}

	if (bwnd.labelFont)
	{
		DeleteObject(bwnd.labelFont);
		bwnd.labelFont = NULL;
	}

	bwnd.visible = false;
	bwnd.labelRects.clear();
}

static void DestroyPerBoxBorderWindows(SBoxBorder* m)
{
	for (auto& pair : m->BoxBorderWnds)
	{
		if (pair.second.hWnd)
			m->BoxBorderWndIndex.erase(pair.second.hWnd);
		DestroyBorderWindowResources(pair.second);
	}
	m->BoxBorderWnds.clear();
	m->BoxBorderWndIndex.clear();
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

	// Calculate centered horizontal position, clamped to available space
	int availLeft  = windowRect.left  + borderWidth;
	int availRight = windowRect.right - borderWidth;
	int labelLeft = windowRect.left + (winWidth - labelWidth) / 2;
	if (labelLeft < availLeft)
		labelLeft = availLeft;
	int labelRight = labelLeft + labelWidth;
	if (labelRight > availRight) {
		labelRight = availRight;
		// Pull left back to preserve full label width when possible
		labelLeft = labelRight - labelWidth;
		if (labelLeft < availLeft)
			labelLeft = availLeft; // window too narrow; clip both sides
	}

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
	if (!hdc) {
		// GetDC failed (window hidden/not ready); discard the font and report no label size
		DeleteObject(bwnd.labelFont);
		bwnd.labelFont = NULL;
		bwnd.labelWidth = 0;
		bwnd.labelHeight = 0;
		return;
	}
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

	// Check per-box border windows via O(1) index
	auto idxIt = m->BoxBorderWndIndex.find(hWnd);
	if (idxIt != m->BoxBorderWndIndex.end()) {
		auto boxIt = m->BoxBorderWnds.find(idxIt->second);
		if (boxIt != m->BoxBorderWnds.end() && boxIt->second.hWnd == hWnd)
			return &boxIt->second;
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
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		if (pBorder && pBorder->labelMode != 0 && !pBorder->boxName.empty() && pBorder->labelFont && !pBorder->labelRects.empty()) 
		{
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
	m->hThread = NULL;

	m->hThread = CreateThread(NULL, 0, CBoxBorder__ThreadFunc, this, 0, NULL);
}

CBoxBorder::~CBoxBorder()
{
	if (m->hThread)
	{
		DWORD threadId = GetThreadId(m->hThread);
		if (threadId)
			PostThreadMessage(threadId, WM_QUIT, 0, 0);

		if (WaitForSingleObject(m->hThread, 10 * 1000) == WAIT_TIMEOUT)
		{
			TerminateThread(m->hThread, 0);
			WaitForSingleObject(m->hThread, 1000);
		}

		CloseHandle(m->hThread);
		m->hThread = NULL;
	}

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
	m->BorderMode = eBorderOff;
	m->CachedFocusBoxMode = eBorderOff;
	InitializeBorderWindowData(m->MainBorder);
	m->MainBorder.color = RGB(0, 0, 0);
	m->MainBorder.alpha = 192; // Default to 75% opacity (192/255)
	m->MainBorder.labelMode = -1; // Default to outside (above border)

	m->ActivePid = 0;
	m->ActiveWnd = NULL;
	m->CurrentTimerIntervalMs = kAdaptiveFastMs;
	m->AdaptiveOtherModeMs = kAdaptiveFastMs;
	m->AdaptiveGlobalAllModeCheckMs = kAdaptiveFastMs;
	m->AdaptiveSceneRefreshMs = kAdaptiveFastMs;
	m->LastAllModeCheckTick = 0;
	m->CachedHasGlobalAllMode = false;
	m->CachedGlobalAllMode = false;
	m->LastAllBordersSceneHash = 0;
	m->LastAllBordersWindowCount = -1;
	m->LastAllBordersRenderTick = 0;
	m->LastAllBordersEnumTick = 0;
	m->AdaptiveEnumIntervalMs = kAdaptiveFastMs;
	m->LastForegroundWnd = NULL;
	m->FocusRaisePulseStartTick = 0;

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

	// Derive fast-tick interval from the primary monitor refresh rate so the border
	// follows moving windows roughly frame-by-frame.  Floor at kFastMoveTimerMs (10 ms)
	// to avoid hammering the system on very high refresh-rate monitors.
	{
		DEVMODEW dm = {};
		dm.dmSize = sizeof(dm);
		int refreshRate = 60; // safe default if EnumDisplaySettings fails
		if (EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm) && dm.dmDisplayFrequency > 1)
			refreshRate = (int)dm.dmDisplayFrequency;
		int frameMs = 1000 / refreshRate;
		m->FastTimerMs = (frameMs > kFastMoveTimerMs) ? frameMs : kFastMoveTimerMs;
	}

	m->dwTimerId = SetTimer(m->MainBorder.hWnd, 0, 100, CBoxBorder__TimerProc);

	MSG  msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (m->MainBorder.hWnd && m->dwTimerId)
		KillTimer(m->MainBorder.hWnd, m->dwTimerId);
	m->dwTimerId = 0;

	if (m->MainBorder.hWnd)
	{
		SetWindowLongPtr(m->MainBorder.hWnd, 0, 0);
		DestroyWindow(m->MainBorder.hWnd);
		m->MainBorder.hWnd = NULL;
	}

	// Cleanup per-box border windows
	DestroyPerBoxBorderWindows(m);

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
		else if (StrMode.compare("ttllbl", Qt::CaseInsensitive) == 0)
			mode = eBorderTitleOnlyLabelOnly;
		else if (StrMode.compare("all", Qt::CaseInsensitive) == 0)
			mode = eBorderAllWindows;
		else if (StrMode.compare("alllbl", Qt::CaseInsensitive) == 0)
			mode = eBorderAllWindowsLabelOnly;
		else if (StrMode.compare("onlbl", Qt::CaseInsensitive) == 0)
			mode = eBorderLabelOnly;
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

static std::wstring GetBoxDisplayName(CSandBox* pBox)
{
	if (!pBox)
		return std::wstring();

	QString alias = pBox->GetText("BoxAlias").trimmed();
	if (!alias.isEmpty())
		return alias.toStdWString();

	return pBox->GetName().toStdWString();
}

static HRGN CreateBorderRegion(const RECT* rect, int borderWidth)
{
	// Create outer rectangle region
	HRGN hrgnOuter = CreateRectRgn(rect->left, rect->top, rect->right, rect->bottom);
	if (!hrgnOuter)
		return NULL;

	// Create inner rectangle region (the hollow part)
	HRGN hrgnInner = CreateRectRgn(
		rect->left + borderWidth,
		rect->top + borderWidth,
		rect->right - borderWidth,
		rect->bottom - borderWidth
	);
	if (!hrgnInner)
		return hrgnOuter; // Fall back to solid rect rather than crashing

	// Subtract inner from outer to create frame
	CombineRgn(hrgnOuter, hrgnOuter, hrgnInner, RGN_DIFF);
	DeleteObject(hrgnInner);

	return hrgnOuter;
}

void CBoxBorder::TimerProc()
{
	DWORD now = GetTickCount();

	auto allProcesses = m_Api->GetAllProcesses();
	bool hasActiveProcess = false;
	for (auto it = allProcesses.begin(); it != allProcesses.end(); ++it) {
		if (!it.value()->IsTerminated()) { hasActiveProcess = true; break; }
	}
	if (!hasActiveProcess)
	{
		// Transition: clean up borders on the first idle tick, then fast-return on all subsequent ones.
		if (m->BorderMode != eBorderOff || m->MainBorder.visible || !m->BoxBorderWnds.empty())
		{
			m->pCurrentBox = NULL;
			m->BorderMode = eBorderOff;
			m->CachedFocusBoxMode = eBorderOff;
			m->ActiveWnd = NULL;
			m->ActivePid = 0;
			m->FastTimerStartTicks = 0;

			HideBorderWindow(m->MainBorder);
			DestroyPerBoxBorderWindows(m);

			m->CachedHasGlobalAllMode = true;
			m->CachedGlobalAllMode = false;
			m->LastAllModeCheckTick = now;
			m->AdaptiveGlobalAllModeCheckMs = kAdaptiveMaxMs;
			m->AdaptiveSceneRefreshMs = kAdaptiveFastMs;
			m->LastAllBordersSceneHash = 0;
			m->LastAllBordersWindowCount = -1;
			m->LastAllBordersRenderTick = 0;
			m->AdaptiveOtherModeMs = kAdaptiveFastMs;
		}

		// Already idle: just back off the timer and return.
		m->AdaptiveOtherModeMs = NextAdaptiveIntervalMs(m->AdaptiveOtherModeMs);
		SetBorderTimerInterval(m, m->AdaptiveOtherModeMs);
		return;
	}

	// Check if any box has AllBordersMode enabled - draw borders for those boxes
	HWND foregroundForTick = GetForegroundWindow();
	if (foregroundForTick != m->LastForegroundWnd)
	{
		m->LastForegroundWnd = foregroundForTick;
		m->FocusRaisePulseStartTick = now;
	}

	bool shouldDrawAllBorders = !m->BoxBorderWnds.empty();
	if (!shouldDrawAllBorders)
	{
		bool shouldProbeGlobalAllMode = !m->CachedHasGlobalAllMode ||
			(now - m->LastAllModeCheckTick >= (DWORD)m->AdaptiveGlobalAllModeCheckMs);

		if (shouldProbeGlobalAllMode)
		{
			bool hadCachedValue = m->CachedHasGlobalAllMode;
			bool previousGlobalAllMode = m->CachedGlobalAllMode;
			m->CachedGlobalAllMode = CheckGlobalAllBordersMode();
			m->CachedHasGlobalAllMode = true;
			m->LastAllModeCheckTick = now;

			if (!hadCachedValue || previousGlobalAllMode != m->CachedGlobalAllMode)
				m->AdaptiveGlobalAllModeCheckMs = kAdaptiveFastMs;
			else
			{
				int nextMs = m->AdaptiveGlobalAllModeCheckMs * 2;
				m->AdaptiveGlobalAllModeCheckMs = (nextMs < kAdaptiveMaxMs) ? nextMs : kAdaptiveMaxMs;
			}
		}
		shouldDrawAllBorders = m->CachedGlobalAllMode;
	}
	else
	{
		m->AdaptiveGlobalAllModeCheckMs = kAdaptiveFastMs;
	}

	// Fast-move timer expiry: now that we know shouldDrawAllBorders, decide whether to bail.
	// If all-borders mode is active we must NOT skip a tick - DrawAllSandboxedBorders must still run.
	if (m->FastTimerStartTicks && now - m->FastTimerStartTicks >= 1000)
	{
		m->FastTimerStartTicks = 0;
		m->AdaptiveOtherModeMs = kAdaptiveFastMs;
		SetBorderTimerInterval(m, kAdaptiveFastMs);
		if (!shouldDrawAllBorders)
			return;
	}

	if (shouldDrawAllBorders)
	{
		// Only reset to 100 ms base rate if we are NOT in a fast-move cycle.
		// Fast-move (10 ms) must stay active for the focused border when dragging.
		if (!m->FastTimerStartTicks)
		{
			m->AdaptiveOtherModeMs = kAdaptiveFastMs;
			SetBorderTimerInterval(m, kAdaptiveFastMs);
		}
		DrawAllSandboxedBorders();
	}

	HWND hWnd = foregroundForTick;
	ULONG Style = hWnd ? GetWindowLong(hWnd, GWL_STYLE) : 0;
	if (!hWnd || !(Style & WS_VISIBLE)) {
		HideBorderWindow(m->MainBorder);
		return;
	}
	ULONG pid = 0;
	GetWindowThreadProcessId(hWnd, &pid);

	CSandBoxPtr pProcessBox = m_Api->GetBoxByProcessId(pid);

	// Get border settings for the focused window's box
	COLORREF boxColor;
	int boxWidth, boxAlpha, boxLabelMode;
	EBorderMode boxMode = eBorderOff;
	if (pProcessBox)
		GetBoxBorderSettings(pProcessBox.data(), boxColor, boxWidth, boxAlpha, boxMode, boxLabelMode);

	if (m->pCurrentBox != pProcessBox.data() || m->CachedFocusBoxMode != boxMode)
	{
		m->pCurrentBox = pProcessBox.data();
		m->CachedFocusBoxMode = boxMode;
		// Force full re-evaluation: reset cached position so the border is redrawn
		m->ActiveWnd = NULL;
		m->ActivePid = 0;
		if (!m->pCurrentBox || boxMode == eBorderOff)
		{
			m->BorderMode = eBorderOff;
		}
		else if (boxMode == eBorderAllWindows || boxMode == eBorderAllWindowsLabelOnly)
		{
			// This box uses "all"/"alllbl" mode, skip single-window border (handled by DrawAllSandboxedBorders)
			m->BorderMode = eBorderOff;
			m->CachedHasGlobalAllMode = true;
			m->CachedGlobalAllMode = true;
			m->LastAllModeCheckTick = now;
			m->AdaptiveGlobalAllModeCheckMs = kAdaptiveFastMs;
		}
		else
		{
			// Normal, title-only, label-only, or title-only-label-only mode
			m->BorderMode = (boxMode == eBorderTitleOnly) ? eBorderTitleOnly :
				(boxMode == eBorderTitleOnlyLabelOnly) ? eBorderTitleOnlyLabelOnly :
				(boxMode == eBorderLabelOnly) ? eBorderLabelOnly : eBorderNormal;
			m->MainBorder.color = boxColor;
			m->MainBorder.width = boxWidth;
			m->MainBorder.alpha = boxAlpha;
			m->MainBorder.labelMode = boxLabelMode;

			// Apply alpha transparency setting
			SetLayeredWindowAttributes(m->MainBorder.hWnd, 0, m->MainBorder.alpha, LWA_ALPHA);

			// Store sandbox name and create label font
			m->MainBorder.boxName = GetBoxDisplayName(pProcessBox.data());
			UpdateBorderLabelFont(m->MainBorder, m->MainBorder.hWnd);
		}
	}

	if (m->BorderMode == eBorderOff) // no border enabled or unsandboxed
	{
		m->ActiveWnd = NULL;
		m->ActivePid = 0;
		HideBorderWindow(m->MainBorder);
	}
	else
	{
		RECT rect;
		GetActiveWindowRect(hWnd, &rect);

		if (pid == m->ActivePid && hWnd == m->ActiveWnd) {
			if (RectEquals(rect, m->ActiveRect)) {
				if (!m->TitleState || m->TitleState == (CheckMousePointer() ? 1 : -1)) {
					if (!m->FastTimerStartTicks && !shouldDrawAllBorders)
					{
						m->AdaptiveOtherModeMs = NextAdaptiveIntervalMs(m->AdaptiveOtherModeMs);
						SetBorderTimerInterval(m, m->AdaptiveOtherModeMs);
					}
					return;
				}
			} 
			else { // window is being moved, increase refresh speed
				if (!m->FastTimerStartTicks)
					SetBorderTimerInterval(m, m->FastTimerMs);
				m->FastTimerStartTicks = now;
			}
		}

		if (!m->FastTimerStartTicks && !shouldDrawAllBorders)
		{
			m->AdaptiveOtherModeMs = kAdaptiveFastMs;
			SetBorderTimerInterval(m, kAdaptiveFastMs);
		}

		// Don't hide the border here - defer the hide to each early-return path below
		// so the border stays visible while being repositioned (prevents fast-timer flicker)

		m->ActiveWnd = hWnd;
		m->ActivePid = pid;
		memcpy(&m->ActiveRect, &rect, sizeof(RECT));
		m->TitleState = 0;
		if (rect.right - rect.left <= 2 || rect.bottom - rect.top <= 2)
		{
			HideBorderWindow(m->MainBorder);
			return;
		}

		// Use shared helper for monitor info and fullscreen check
		MONITORINFO Monitor;
		if (!ShouldDrawBorderForWindow(hWnd, rect, Style, &Monitor))
		{
			HideBorderWindow(m->MainBorder);
			return;
		}

		if (m->BorderMode == eBorderTitleOnly || m->BorderMode == eBorderTitleOnlyLabelOnly) {
			const RECT* Desktop = &Monitor.rcMonitor;
			if (!IsMouseOnTitle(hWnd, &rect, Desktop))
			{
				HideBorderWindow(m->MainBorder);
				return;
			}
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

		// For label-only modes with no label configured, nothing to show
		bool isLabelOnlyMode = (m->BorderMode == eBorderLabelOnly || m->BorderMode == eBorderTitleOnlyLabelOnly);
		if (isLabelOnlyMode && (m->MainBorder.labelMode == 0 || m->MainBorder.labelHeight <= 0))
		{
			HideBorderWindow(m->MainBorder);
			return;
		}

		// Create border frame region in window coordinates (empty for label-only modes)
		HRGN hrgnBorder;
		if (isLabelOnlyMode) {
			hrgnBorder = CreateRectRgn(0, 0, 0, 0); // No frame; only the label rect will be added below
		} else {
			RECT frameRect = { 0, labelOffset, rectWidth, labelOffset + rectHeight };
			hrgnBorder = CreateBorderRegion(&frameRect, m->MainBorder.width);
		}

		if (!hrgnBorder)
		{
			// GDI resource failure; leave the border as-is for this tick
			return;
		}

		// Add label region if enabled
		m->MainBorder.labelRects.clear();
		if (m->MainBorder.labelMode != 0 && m->MainBorder.labelHeight > 0)
		{
			// Calculate label rect in screen coordinates using shared helper
			// In label-only modes there is no border frame, so don't offset by border width
			int lblBorderWidth = isLabelOnlyMode ? 0 : m->MainBorder.width;
			RECT labelRectScr = CalculateLabelRect(adjustedRect, lblBorderWidth, m->MainBorder.labelWidth, m->MainBorder.labelHeight, m->MainBorder.labelMode);

			// Convert to window-relative coordinates
			RECT labelRect;
			labelRect.left = labelRectScr.left - windowRect.left;
			labelRect.right = labelRectScr.right - windowRect.left;
			labelRect.top = labelRectScr.top - windowRect.top;
			labelRect.bottom = labelRectScr.bottom - windowRect.top;

			m->MainBorder.labelRects.push_back(labelRect);

			// Add label region to border
			HRGN hrgnLabel = CreateRectRgn(labelRect.left, labelRect.top, labelRect.right, labelRect.bottom);
			if (hrgnLabel) {
				CombineRgn(hrgnBorder, hrgnBorder, hrgnLabel, RGN_OR);
				DeleteObject(hrgnLabel);
			}
		}

		// Position and show the border window.
		// Keep SWP_NOZORDER during stable focus mode so we don't permanently fight other
		// always-on-top overlays.  Temporarily re-raise only for a short click/focus
		// pulse after foreground changed.
		if (!SetWindowRgn(m->MainBorder.hWnd, hrgnBorder, TRUE))
			DeleteObject(hrgnBorder); // SetWindowRgn only owns the region on success
		bool focusRaisePulseActive = m->FocusRaisePulseStartTick &&
			(now - m->FocusRaisePulseStartTick <= (DWORD)kFocusRaisePulseMs);
		bool raiseDuringInteraction = focusRaisePulseActive;
		DWORD mainSwpFlags = SWP_NOACTIVATE;
		if (!raiseDuringInteraction)
			mainSwpFlags |= SWP_NOZORDER;
		if (!m->MainBorder.visible) mainSwpFlags |= SWP_SHOWWINDOW;
		SetWindowPos(m->MainBorder.hWnd, raiseDuringInteraction ? HWND_TOP : NULL,
			windowRect.left, windowRect.top,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			mainSwpFlags);
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

bool CBoxBorder::IsMouseOnTitle(struct HWND__* hWnd, struct tagRECT* rect, const struct tagRECT* Desktop)
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
	return GetCursorPos(&Cursor)
		&& Cursor.x >= m->TitleRect.left
		&& Cursor.x <= m->TitleRect.right
		&& Cursor.y >= m->TitleRect.top
		&& Cursor.y <= m->TitleRect.bottom;
}

void CBoxBorder::DrawAllSandboxedBorders()
{
	// Rate-limit the window enumeration independently of the timer rate.
	// During fast-timer cool-down the timer fires every 10 ms, but if the scene
	// hasn't changed we back off AdaptiveEnumIntervalMs quickly (doubles each stable
	// check: 10→20→40→80→100 ms), so only ~5-7 enumerations happen per cool-down
	// period instead of ~100.
	DWORD nowEnum = GetTickCount();
	if (nowEnum - m->LastAllBordersEnumTick < (DWORD)m->AdaptiveEnumIntervalMs)
		return;
	m->LastAllBordersEnumTick = nowEnum;

	HWND focusedWnd = GetForegroundWindow();
	ULONG focusedPid = 0;
	CSandBox* pFocusedBox = NULL;
	if (focusedWnd)
	{
		GetWindowThreadProcessId(focusedWnd, &focusedPid);
		CSandBoxPtr pFocusBox = m_Api->GetBoxByProcessId(focusedPid);
		pFocusedBox = pFocusBox.data();
	}
	bool focusRaisePulseActive = m->FocusRaisePulseStartTick &&
		(nowEnum - m->FocusRaisePulseStartTick <= (DWORD)kFocusRaisePulseMs);

	// Structure to hold window info for all-borders mode
	struct SWindowInfo
	{
		HWND hWnd;
		RECT rect;
		MONITORINFO monitorInfo; // Monitor info for edge clipping
		int zOrder; // Lower value = higher in Z-order (on top)
		ULONG pid;
		CSandBox* pBox; // NULL if not sandboxed or not border-eligible
	};

	// Collect ALL windows (sandboxed and non-sandboxed) in Z-order
	std::vector<SWindowInfo> allWindows;

	// Enumerate all top-level windows in Z-order
	int zOrder = 0;
	HWND hWnd = GetTopWindow(NULL);
	while (hWnd)
	{
		// Skip our border windows
		bool isBorderWnd = (hWnd == m->MainBorder.hWnd) ||
			(m->BoxBorderWndIndex.find(hWnd) != m->BoxBorderWndIndex.end());

		if (!isBorderWnd && IsWindowVisible(hWnd) && !IsIconic(hWnd))
		{
			// Skip DWM-cloaked windows (e.g. "Peek at Desktop", virtual desktop transitions).
			// Cloaked windows are IsWindowVisible==TRUE but are not actually rendered on screen.
			// Excluding them ensures the coverage hash changes on peek/unpeek transitions so
			// border regions rebuild correctly the moment the desktop peek ends.
			if (m->DwmGetWindowAttribute) {
				DWORD cloaked = 0;
				if (SUCCEEDED(m->DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) {
					zOrder++;
					hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
					continue;
				}
			}

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
					wnd.pid = 0;
					GetActiveWindowRect(hWnd, &wnd.rect);
					memset(&wnd.monitorInfo, 0, sizeof(MONITORINFO));

					// Check if this is a sandboxed window
					GetWindowThreadProcessId(hWnd, &wnd.pid);
					CSandBoxPtr pBox = m_Api->GetBoxByProcessId(wnd.pid);
					wnd.pBox = pBox.data();

					// For sandboxed windows, check fullscreen and get monitor info
					// Skip fullscreen sandboxed windows (no border) but still track for Z-order
					if (wnd.pBox)
					{
						// Exclude secondary/helper windows using three checks in order:
						//
						// 1. GW_OWNER != NULL  ->  owned window (dialog, toolbar shown via
						//    Show(owner), child-like popup). WinForms Show(IWin32Window owner)
						//    calls SetWindowLong(GWL_HWNDPARENT), so owned WinForms windows
						//    (e.g. ShapeManagerMenu toolbar shown via menuForm.Show(Form))
						//    are reliably caught here.
						//
						// 2. WS_EX_TOOLWINDOW without WS_EX_APPWINDOW  ->  classic hidden-from-
						//    taskbar windows (WinForms ShowInTaskbar=false legacy path, tray
						//    icons, floating panels).
						//
						// 3. WS_POPUP + no WS_CAPTION + no WS_EX_APPWINDOW  ->  unowned borderless
						//    popup. WinForms ShowInTaskbar=false (modern path) parks the form under
						//    a hidden parking window and does NOT set WS_EX_TOOLWINDOW, so tests 1
						//    and 2 may both miss it; this catches the rest.
						//
						// Exception for rules 1 & 3: class #32770 is the standard Windows dialog
						//    class (DialogBox / CreateDialog). Such windows are always real dialogs
						//    even when they lack WS_CAPTION (e.g. Vivaldi installer setup screen).
						//
						// Normal captioned windows (browsers, editors) never match any of these.
						// Fullscreen games / WS_EX_APPWINDOW popups pass all three checks.

						bool isSystemDialog = (wcscmp(className, L"#32770") == 0);
						bool isMainWindow = true;
						if (GetWindow(hWnd, GW_OWNER) != NULL && !(Style & WS_CAPTION) && !isSystemDialog)
							// Owned + no caption = toolbar/overlay/popup helper. Exclude.
							// Owned + caption = proper dialog (Open File, Save, Print...). Allow.
							// Owned + no caption + #32770 = standard captionless dialog. Allow.
							isMainWindow = false;
						else if ((ExStyle & WS_EX_TOOLWINDOW) && !(ExStyle & WS_EX_APPWINDOW))
							isMainWindow = false;
						else if ((Style & WS_POPUP) && !(Style & WS_CAPTION) && !(ExStyle & WS_EX_APPWINDOW) && !isSystemDialog)
							isMainWindow = false;

						if (!isMainWindow)
							wnd.pBox = NULL;
					}

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
		bool labelOnly;     // true = "alllbl" mode: show label but no border frame
		std::vector<SBoxWindowInfo> windows; // Individual windows for per-window labels
		std::vector<RECT> newLabelRects;     // label rects built this tick; compared against bwnd.labelRects
		ULONGLONG perBoxHash; // hash of settings + coverage-above + own windows
		bool skipRebuild;     // true when perBoxHash matches persisted bwnd.sceneHash
	};
	std::map<CSandBox*, SBoxBorderData> boxBorders;
	ULONGLONG sceneHash = kHashSeed;
	ULONGLONG settingsHash = kHashSeed;

	for (const auto& wnd : allWindows)
	{
		HashMix64(sceneHash, (ULONGLONG)(ULONG_PTR)wnd.hWnd);
		HashMix64(sceneHash, (ULONGLONG)(LONG_PTR)wnd.rect.left);
		HashMix64(sceneHash, (ULONGLONG)(LONG_PTR)wnd.rect.top);
		HashMix64(sceneHash, (ULONGLONG)(LONG_PTR)wnd.rect.right);
		HashMix64(sceneHash, (ULONGLONG)(LONG_PTR)wnd.rect.bottom);
		HashMix64(sceneHash, (ULONGLONG)(ULONG_PTR)wnd.pBox);
		HashMix64(sceneHash, (ULONGLONG)wnd.zOrder);
	}

	// Initialize border regions for each active box that has "all" mode enabled
	for (const auto& wnd : allWindows)
	{
		if (wnd.pBox && boxBorders.find(wnd.pBox) == boxBorders.end())
		{
			COLORREF color;
			int width, alpha, labelMode;
			EBorderMode mode;

			// Only add boxes that have "all" or "alllbl" mode enabled
			if (GetBoxBorderSettings(wnd.pBox, color, width, alpha, mode, labelMode) && (mode == eBorderAllWindows || mode == eBorderAllWindowsLabelOnly))
			{
				SBoxBorderData data;
				data.hrgnBorder = NULL;
				data.boundingRect = { 0, 0, 0, 0 };
				data.color = color;
				data.width = width;
				data.alpha = alpha;
				data.labelMode = labelMode;
				data.boxName = GetBoxDisplayName((CSandBox*)wnd.pBox);
				data.hasWindows = false;
				data.labelOnly = (mode == eBorderAllWindowsLabelOnly);
				// Seed per-box hash with settings; window data is mixed in during the hash-computation pass below
				data.perBoxHash = kHashSeed;
				HashMix64(data.perBoxHash, (ULONGLONG)(ULONG_PTR)wnd.pBox);
				HashMix64(data.perBoxHash, (ULONGLONG)data.color);
				HashMix64(data.perBoxHash, (ULONGLONG)data.width);
				HashMix64(data.perBoxHash, (ULONGLONG)data.alpha);
				HashMix64(data.perBoxHash, (ULONGLONG)data.labelMode);
				HashMix64(data.perBoxHash, (ULONGLONG)(data.labelOnly ? 1ULL : 0ULL));
				HashMixWString(data.perBoxHash, data.boxName);
				data.skipRebuild = false;
				boxBorders[wnd.pBox] = data;

				HashMix64(settingsHash, (ULONGLONG)(ULONG_PTR)wnd.pBox);
				HashMix64(settingsHash, (ULONGLONG)data.color);
				HashMix64(settingsHash, (ULONGLONG)data.width);
				HashMix64(settingsHash, (ULONGLONG)data.alpha);
				HashMix64(settingsHash, (ULONGLONG)data.labelMode);
				HashMix64(settingsHash, (ULONGLONG)data.labelOnly);
				HashMixWString(settingsHash, data.boxName);
			}
		}
	}

	HashMix64(sceneHash, settingsHash);
	HashMix64(sceneHash, (ULONGLONG)boxBorders.size());

	// Per-box hash pass: mix each box's own window positions + the rolling coverage of all
	// windows above them into data.perBoxHash.  This runs before the early-return check so
	// we can mark individual boxes as skipRebuild even when the global hash changed.
	{
		ULONGLONG rollingCovHash = kHashSeed;
		for (const auto& wnd : allWindows)
		{
			if (wnd.pBox)
			{
				auto it = boxBorders.find(wnd.pBox);
				if (it != boxBorders.end())
				{
					SBoxBorderData& data = it->second;
					// First window of this box: capture coverage accumulated above it
					if (!data.hasWindows) {
						HashMix64(data.perBoxHash, rollingCovHash);
						data.hasWindows = true; // repurposed here; reset below
					}
					HashMix64(data.perBoxHash, (ULONGLONG)(ULONG_PTR)wnd.hWnd);
					HashMix64(data.perBoxHash, (ULONGLONG)(LONG_PTR)wnd.rect.left);
					HashMix64(data.perBoxHash, (ULONGLONG)(LONG_PTR)wnd.rect.top);
					HashMix64(data.perBoxHash, (ULONGLONG)(LONG_PTR)wnd.rect.right);
					HashMix64(data.perBoxHash, (ULONGLONG)(LONG_PTR)wnd.rect.bottom);
					HashMix64(data.perBoxHash, (ULONGLONG)wnd.zOrder);
				}
			}
			// Roll all windows (sandboxed or not) into the coverage hash
			HashMix64(rollingCovHash, (ULONGLONG)(ULONG_PTR)wnd.hWnd);
			HashMix64(rollingCovHash, (ULONGLONG)(LONG_PTR)wnd.rect.left);
			HashMix64(rollingCovHash, (ULONGLONG)(LONG_PTR)wnd.rect.top);
			HashMix64(rollingCovHash, (ULONGLONG)(LONG_PTR)wnd.rect.right);
			HashMix64(rollingCovHash, (ULONGLONG)(LONG_PTR)wnd.rect.bottom);
		}
		// Reset hasWindows (region-building loop uses it for boundingRect init)
		// and decide which boxes can skip rebuilding based on persisted scene hashes.
		for (auto& pair : boxBorders)
		{
			pair.second.hasWindows = false;
			auto bwndIt = m->BoxBorderWnds.find(pair.first);
			pair.second.skipRebuild = (bwndIt != m->BoxBorderWnds.end()) &&
			                          (bwndIt->second.sceneHash == pair.second.perBoxHash);
		}
	}

	if (boxBorders.empty())
	{
		m->CachedHasGlobalAllMode = true;
		m->CachedGlobalAllMode = false;
		m->AdaptiveGlobalAllModeCheckMs = kAdaptiveFastMs;
	}

	bool sceneUnchanged =
		sceneHash == m->LastAllBordersSceneHash &&
		(int)allWindows.size() == m->LastAllBordersWindowCount;

	if (sceneUnchanged)
	{
		// Scene is stable: back off the enumeration interval, capped at kAdaptiveFastMs (100 ms).
		// Capping at 100 ms (not kAdaptiveMaxMs) ensures movement is detected within 100 ms at worst.
		int nextMs = m->AdaptiveEnumIntervalMs * 2;
		m->AdaptiveEnumIntervalMs = (nextMs < kAdaptiveFastMs) ? nextMs : kAdaptiveFastMs;

		int nextRefMs = m->AdaptiveSceneRefreshMs * 2;
		m->AdaptiveSceneRefreshMs = (nextRefMs < kAdaptiveMaxMs) ? nextRefMs : kAdaptiveMaxMs;

		if (nowEnum - m->LastAllBordersRenderTick < (DWORD)m->AdaptiveSceneRefreshMs)
			return;
	}
	else
	{
		// Scene changed: enumerate at frame rate and keep the timer at FastTimerMs.
		m->AdaptiveEnumIntervalMs = m->FastTimerMs;
		m->AdaptiveSceneRefreshMs = kAdaptiveFastMs;
		if (!m->FastTimerStartTicks)
			SetBorderTimerInterval(m, m->FastTimerMs);
		m->FastTimerStartTicks = (int)nowEnum;
		// Note: individual per-box skipRebuild flags are already set correctly by the
		// per-box hash pass above (which folds in rollingCovHash = coverage of all windows
		// above each box).  Do NOT clear them here: doing so would force every all-border
		// window to rebuild every tick whenever any unrelated window (e.g. a focus-mode
		// border) moves, causing label flicker.
	}

	m->LastAllBordersSceneHash = sceneHash;
	m->LastAllBordersWindowCount = (int)allWindows.size();
	m->LastAllBordersRenderTick = nowEnum;

	// Track covered area (all windows processed so far, from top to bottom)
	HRGN hrgnCovered = CreateRectRgn(0, 0, 0, 0);
	if (!hrgnCovered)
		return; // GDI resource failure; skip rendering for this tick

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

				if (!data.skipRebuild)
				{
				if (!data.hrgnBorder)
					data.hrgnBorder = CreateRectRgn(0, 0, 0, 0);

					// Apply edge clipping and taskbar adjustment if we have monitor info
					// Guard: skip this window's contribution if the accumulator region is unavailable
					if (data.hrgnBorder)
					{
						RECT adjustedRect = wnd.rect;
						if (wnd.monitorInfo.cbSize != 0)
							adjustedRect = AdjustRectToDesktop(wnd.rect, wnd.monitorInfo, data.width);
	
						// Create border region for this window (skip for label-only mode)
						if (!data.labelOnly) {
							HRGN hrgnBorder = CreateBorderRegion(&adjustedRect, data.width);
							if (hrgnBorder) {
								// Subtract the covered area (all windows above) from this border
								CombineRgn(hrgnBorder, hrgnBorder, hrgnCovered, RGN_DIFF);
	
								// Add this border to the box's combined border region
								CombineRgn(data.hrgnBorder, data.hrgnBorder, hrgnBorder, RGN_OR);
								DeleteObject(hrgnBorder);
							}
						}
	
						// Track this window for per-window labels (using adjusted rect)
						SBoxWindowInfo winInfo;
						winInfo.rect = adjustedRect;
						// Save covered area ABOVE this window for label clipping
						winInfo.hrgnCoveredAbove = CreateRectRgn(0, 0, 0, 0);
						if (winInfo.hrgnCoveredAbove)
							CombineRgn(winInfo.hrgnCoveredAbove, hrgnCovered, hrgnCovered, RGN_COPY);
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
					} // end if (data.hrgnBorder)
				} // end if (!data.skipRebuild)
				else
				{
					// Box unchanged: just mark it as having windows so its border isn't destroyed
					data.hasWindows = true;
				}
			} // end if (it != boxBorders.end())
		} // end if (wnd.pBox)

		// Add this window's rectangle to covered area (both sandboxed and non-sandboxed)
		HRGN hrgnWindow = CreateRectRgn(wnd.rect.left, wnd.rect.top, wnd.rect.right, wnd.rect.bottom);
		if (hrgnWindow) {
			CombineRgn(hrgnCovered, hrgnCovered, hrgnWindow, RGN_OR);
			DeleteObject(hrgnWindow);
		}
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
			if (data.hrgnBorder)
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
			InitializeBorderWindowData(bwnd);
			bwnd.hWnd = CreateBoxBorderWindow();
			if (!bwnd.hWnd)
			{
				// Clean up GDI resources that were already built for this box before bailing out
				if (data.hrgnBorder)
				{
					DeleteObject(data.hrgnBorder);
					data.hrgnBorder = NULL;
				}
				for (auto& winInfo : data.windows)
				{
					if (winInfo.hrgnCoveredAbove)
					{
						DeleteObject(winInfo.hrgnCoveredAbove);
						winInfo.hrgnCoveredAbove = NULL;
					}
				}
				continue;
			}
			SetWindowLongPtr(bwnd.hWnd, 0, ULONG_PTR(this));
			m->BoxBorderWnds[pBox] = bwnd;
			it = m->BoxBorderWnds.find(pBox);
			if (it != m->BoxBorderWnds.end() && it->second.hWnd)
				m->BoxBorderWndIndex[it->second.hWnd] = pBox;
		}

		SBoxBorderWnd& bwnd = it->second;

		// Per-box skip: windows + settings + coverage above are identical to last render.
		if (data.skipRebuild)
			continue;

		// Commit the new per-box hash so this render becomes the new baseline.
		bwnd.sceneHash = data.perBoxHash;
		bwnd.lastChangeTick = nowEnum; // record when this box last changed for timer management

		// Update color if changed (painting is handled in WM_ERASEBKGND)
		if (bwnd.color != data.color)
		{
			bwnd.color = data.color;
			// Force repaint with new color
			InvalidateRect(bwnd.hWnd, NULL, TRUE);
		}

		// Update alpha only when changed
		if (bwnd.alpha != data.alpha) {
			SetLayeredWindowAttributes(bwnd.hWnd, 0, data.alpha, LWA_ALPHA);
			bwnd.alpha = data.alpha;
		}

		// Update label settings (recreate font only when relevant settings change)
		bool fontDirty = (bwnd.boxName != data.boxName || bwnd.labelMode != data.labelMode || bwnd.width != data.width);
		bwnd.boxName = data.boxName;
		bwnd.labelMode = data.labelMode;
		bwnd.width = data.width;

		// Store original bounding rect values before any modifications
		int originalBoundingLeft = data.boundingRect.left;
		int originalBoundingTop = data.boundingRect.top;

		// Create/update label font and calculate dimensions
		if (fontDirty || !bwnd.labelFont)
			UpdateBorderLabelFont(bwnd);

		// For outside mode, extend window upward to make room for label
		if (bwnd.labelMode == -1 && bwnd.labelHeight > 0)
		{
			data.boundingRect.top -= bwnd.labelHeight;
		}

		// Offset the border region to window-relative coordinates
		// Border region was created in screen coordinates around the ORIGINAL boundingRect
		// Use original coordinates for offset, not the modified ones

		// Guard: if data.hrgnBorder is NULL (GDI allocation failed during window loop), skip this box
		if (!data.hrgnBorder)
		{
			for (auto& winInfo : data.windows) {
				if (winInfo.hrgnCoveredAbove) {
					DeleteObject(winInfo.hrgnCoveredAbove);
					winInfo.hrgnCoveredAbove = NULL;
				}
			}
			continue;
		}

		OffsetRgn(data.hrgnBorder, -originalBoundingLeft, -originalBoundingTop);

		// For outside mode, shift border down by labelHeight since window now extends above the original top
		if (bwnd.labelMode == -1 && bwnd.labelHeight > 0)
		{
			OffsetRgn(data.hrgnBorder, 0, bwnd.labelHeight);
		}

		// Add label region for EACH window in this box
		// Each window gets its own label centered on that window's border
		if (bwnd.labelMode != 0 && bwnd.labelHeight > 0 && bwnd.labelFont)
		{
			// Add a label for each window in this box
			for (auto& winInfo : data.windows)
			{
				// Calculate label rect in screen coordinates using shared helper
				// In label-only mode there is no border frame, so don't offset by border width
				int lblBorderWidth = data.labelOnly ? 0 : data.width;
				RECT labelRectScr = CalculateLabelRect(winInfo.rect, lblBorderWidth, bwnd.labelWidth, bwnd.labelHeight, bwnd.labelMode);

				// Convert to window coordinates (relative to border window origin)
				// Note: data.boundingRect.top was already extended upward for outside mode,
				// so subtracting it gives the correct window-relative position
				RECT labelRectWnd = {
					labelRectScr.left - data.boundingRect.left,
					labelRectScr.top - data.boundingRect.top,
					labelRectScr.right - data.boundingRect.left,
					labelRectScr.bottom - data.boundingRect.top
				};

				// Accumulate into data.newLabelRects; assigned to bwnd.labelRects only after
				// the comparison that decides whether to invalidate (prevents unnecessary repaints)
				data.newLabelRects.push_back(labelRectWnd);

				// Create label region in screen coordinates, clip, offset, and add to border region
				HRGN hrgnLabelScr = CreateRectRgn(labelRectScr.left, labelRectScr.top, labelRectScr.right, labelRectScr.bottom);
				if (hrgnLabelScr) {
					// Clip against windows ABOVE this specific window
					if (winInfo.hrgnCoveredAbove)
						CombineRgn(hrgnLabelScr, hrgnLabelScr, winInfo.hrgnCoveredAbove, RGN_DIFF);

					// Offset and add the region
					OffsetRgn(hrgnLabelScr, -data.boundingRect.left, -data.boundingRect.top);
					CombineRgn(data.hrgnBorder, data.hrgnBorder, hrgnLabelScr, RGN_OR);
					DeleteObject(hrgnLabelScr);
				}
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

		// Position and show the border window.
		// Pass FALSE to SetWindowRgn so it doesn't force a full repaint by itself;
		// we'll invalidate only the parts that actually changed below.
		if (!SetWindowRgn(bwnd.hWnd, data.hrgnBorder, FALSE))
			DeleteObject(data.hrgnBorder); // SetWindowRgn only owns the region on success
		data.hrgnBorder = NULL;

		// All-border windows stay SWP_NOZORDER in steady state.  Temporarily re-raise only
		// for the box that owns the newly-focused window (short focus pulse), so third-party
		// topmost borders don't hide the active box while avoiding global z-order jumps.
		bool raiseThisBox = focusRaisePulseActive && pFocusedBox && (pBox == pFocusedBox);
		DWORD swpFlags = SWP_NOACTIVATE;
		if (!raiseThisBox)
			swpFlags |= SWP_NOZORDER;
		if (!bwnd.visible) swpFlags |= SWP_SHOWWINDOW;
		SetWindowPos(bwnd.hWnd, raiseThisBox ? HWND_TOP : NULL,
			data.boundingRect.left,
			data.boundingRect.top,
			data.boundingRect.right - data.boundingRect.left,
			data.boundingRect.bottom - data.boundingRect.top,
			swpFlags);

		// Invalidate when label rects changed, on first show, or whenever the region was
		// rebuilt (skipRebuild=false).  The region rebuild case is critical after events like
		// Peek at Desktop ending: SetWindowRgn(FALSE) clips to the new shape instantly but
		// newly-revealed pixels (previously hidden behind a covering window) are unpainted
		// until an explicit InvalidateRect forces WM_ERASEBKGND to fill them with the border color.
		bool labelRectsChanged = !RectVectorEquals(bwnd.labelRects, data.newLabelRects);
		if (labelRectsChanged || !bwnd.visible || !data.skipRebuild)
			InvalidateRect(bwnd.hWnd, NULL, TRUE);

		bwnd.labelRects = data.newLabelRects;
		bwnd.visible = true;
		// Note: data.hrgnBorder is now owned by the window (on success), don't delete it
	}

	// Destroy and remove border windows for boxes that no longer have visible windows
	for (auto it = m->BoxBorderWnds.begin(); it != m->BoxBorderWnds.end(); )
	{
		if (activeBoxes.find(it->first) == activeBoxes.end())
		{
			// Destroy the window and clean up resources
			if (it->second.hWnd)
				m->BoxBorderWndIndex.erase(it->second.hWnd);
			DestroyBorderWindowResources(it->second);
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
		if (it.value()->IsTerminated())
			continue;
		CSandBox* pBox = it.value()->GetBoxPtr();
		if (!pBox || checkedBoxes.find(pBox) != checkedBoxes.end())
			continue;

		checkedBoxes.insert(pBox);

		COLORREF color;
		int width, alpha, labelMode;
		EBorderMode mode;

		if (GetBoxBorderSettings(pBox, color, width, alpha, mode, labelMode) && (mode == eBorderAllWindows || mode == eBorderAllWindowsLabelOnly))
			return true;
	}

	return false;
}
