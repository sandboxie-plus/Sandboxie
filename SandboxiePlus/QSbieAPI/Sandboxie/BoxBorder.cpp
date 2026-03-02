/*
 *
 * Copyright (c) 2020-2026, David Xanatos
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

struct SWindowRoleCacheEntry
{
	ULONG style;
	ULONG exStyle;
	HWND owner;
	bool isTransientPopup;
	bool isMainWindow;
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
	DWORD LastZOrderProbeTick;     // timestamp of last dedicated z-order probe
	int ZOrderProbeIntervalMs;     // cadence for z-order probe (independent of scene enum cadence)
	HWND LastForegroundWnd;        // last observed foreground window (for foreground-change detection)
	DWORD ForegroundChangeTick;    // GetTickCount() when foreground last changed; used for activity snap

	POINT LastMousePos;            // cursor position at last DrawAllSandboxedBorders call
	bool  LastButtonHeld;          // was LMB or RMB held on the previous tick (for release detection)

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
	std::unordered_map<HWND, SBoxBorderWnd> AllBorderWnds;     // key: target app window HWND
	std::unordered_map<HWND, HWND> AllBorderWndIndex;          // key: overlay HWND -> value: target app window HWND
	std::unordered_map<HWND, SWindowRoleCacheEntry> WindowRoleCache; // key: target HWND -> cached class/role traits
};

const WCHAR *Sandboxie_WindowClassName = L"Sandboxie_BorderWindow";

static const int kAdaptiveFastMs = 100;
static const int kAdaptiveMaxMs = 5000;     // max backoff for repaint cadence (truly idle)
static const int kAdaptiveMaxEnumMs = 500;  // max backoff for window enumeration (capped low so
                                            // a sudden move is never stale for more than ~0.5 s)
static const int kZOrderProbeFastMs = 50;    // fast z-order probe cadence during interaction/focus churn
static const int kZOrderProbeMaxMs = 500;    // idle z-order probe cap (still responsive)
static const int kFastMoveTimerMs = 10; // minimum floor for frame-aligned fast timer
static const int kForegroundChangePulseMs = 500; // duration after foreground change during which
                                                 // enum+scene backoffs are snapped back to fast
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
	for (auto& pair : m->AllBorderWnds)
	{
		if (pair.second.hWnd)
			m->AllBorderWndIndex.erase(pair.second.hWnd);
		DestroyBorderWindowResources(pair.second);
	}
	m->AllBorderWnds.clear();
	m->AllBorderWndIndex.clear();
}

static void EnsureOverlayBandMatchesTarget(HWND overlayHwnd, HWND targetHwnd)
{
	if (!overlayHwnd || !targetHwnd || !IsWindow(overlayHwnd) || !IsWindow(targetHwnd))
		return;

	ULONG targetEx = GetWindowLong(targetHwnd, GWL_EXSTYLE);
	ULONG overlayEx = GetWindowLong(overlayHwnd, GWL_EXSTYLE);
	bool targetTopmost = (targetEx & WS_EX_TOPMOST) != 0;
	bool overlayTopmost = (overlayEx & WS_EX_TOPMOST) != 0;

	if (targetTopmost == overlayTopmost)
		return;

	SetWindowPos(
		overlayHwnd,
		targetTopmost ? HWND_TOPMOST : HWND_NOTOPMOST,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);
}

static HWND GetInsertAfterForOverlay(HWND overlayHwnd, HWND targetHwnd)
{
	if (!overlayHwnd || !targetHwnd || !IsWindow(overlayHwnd) || !IsWindow(targetHwnd))
		return NULL;

	EnsureOverlayBandMatchesTarget(overlayHwnd, targetHwnd);

	ULONG targetEx = GetWindowLong(targetHwnd, GWL_EXSTYLE);
	bool targetTopmost = (targetEx & WS_EX_TOPMOST) != 0;

	// Walk upward and find the nearest window above target in the SAME topmost band.
	// SetWindowPos inserts hWnd AFTER hWndInsertAfter, so using this predecessor places
	// the overlay immediately above target while preserving shell/taskbar layering.
	HWND scan = GetWindow(targetHwnd, GW_HWNDPREV);
	while (scan)
	{
		if (scan == overlayHwnd)
			return NULL; // already directly above target in the proper band

		ULONG scanEx = GetWindowLong(scan, GWL_EXSTYLE);
		bool scanTopmost = (scanEx & WS_EX_TOPMOST) != 0;
		if (scanTopmost == targetTopmost)
			return scan;

		scan = GetWindow(scan, GW_HWNDPREV);
	}

	// Target is already top of its band.
	return targetTopmost ? HWND_TOPMOST : HWND_TOP;
}

static bool ProbeOverlayZOrder(HWND overlayHwnd, HWND targetHwnd)
{
	if (!overlayHwnd || !targetHwnd || !IsWindow(overlayHwnd) || !IsWindow(targetHwnd))
		return false;

	HWND insertAfter = GetInsertAfterForOverlay(overlayHwnd, targetHwnd);
	if (!insertAfter)
		return false;

	SetWindowPos(
		overlayHwnd,
		insertAfter,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);

	return true;
}

static void RunAllModeZOrderProbe(SBoxBorder* m, DWORD nowTick, bool forceFastProbe)
{
	if (forceFastProbe)
		m->ZOrderProbeIntervalMs = kZOrderProbeFastMs;

	if (nowTick - m->LastZOrderProbeTick < (DWORD)m->ZOrderProbeIntervalMs)
		return;

	m->LastZOrderProbeTick = nowTick;

	bool zOrderAdjusted = false;
	for (auto& pair : m->AllBorderWnds)
	{
		HWND targetHwnd = pair.first;
		SBoxBorderWnd& bwnd = pair.second;
		if (!bwnd.visible || !bwnd.hWnd)
			continue;

		if (ProbeOverlayZOrder(bwnd.hWnd, targetHwnd))
			zOrderAdjusted = true;
	}

	if (forceFastProbe || zOrderAdjusted)
	{
		m->ZOrderProbeIntervalMs = kZOrderProbeFastMs;
	}
	else
	{
		int nextMs = m->ZOrderProbeIntervalMs * 2;
		m->ZOrderProbeIntervalMs = (nextMs < kZOrderProbeMaxMs) ? nextMs : kZOrderProbeMaxMs;
	}
}

static void RemoveAllModeOverlayForTarget(SBoxBorder* m, HWND targetWnd)
{
	auto it = m->AllBorderWnds.find(targetWnd);
	if (it == m->AllBorderWnds.end())
		return;

	if (it->second.hWnd)
		m->AllBorderWndIndex.erase(it->second.hWnd);
	DestroyBorderWindowResources(it->second);
	m->AllBorderWnds.erase(it);
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
	auto idxIt = m->AllBorderWndIndex.find(hWnd);
	if (idxIt != m->AllBorderWndIndex.end()) {
		auto wndIt = m->AllBorderWnds.find(idxIt->second);
		if (wndIt != m->AllBorderWnds.end() && wndIt->second.hWnd == hWnd)
			return &wndIt->second;
		// Stale index entry: target/overlay mapping no longer valid.
		m->AllBorderWndIndex.erase(idxIt);
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
	m->LastZOrderProbeTick = 0;
	m->ZOrderProbeIntervalMs = kZOrderProbeFastMs;
	m->LastForegroundWnd = NULL;
	m->ForegroundChangeTick = 0;
	m->LastMousePos = { 0, 0 };
	m->LastButtonHeld = false;

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
		if (m->BorderMode != eBorderOff || m->MainBorder.visible || !m->AllBorderWnds.empty())
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
		m->ForegroundChangeTick = now;
	}

	bool shouldDrawAllBorders = !m->AllBorderWnds.empty();
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
				// Even stable geometry can have a stale z-order: a title-bar click on the
				// already-focused window reshuffles z-order without changing the foreground
				// window, so we must probe unconditionally on every stable tick.
				if (m->MainBorder.visible && m->MainBorder.hWnd)
					ProbeOverlayZOrder(m->MainBorder.hWnd, hWnd);

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
		// Use the same target-anchored z-ordering model as all-mode overlays:
		// place border directly above its target within the same topmost band.
		// GetInsertAfterForOverlay calls EnsureOverlayBandMatchesTarget internally.
		if (!SetWindowRgn(m->MainBorder.hWnd, hrgnBorder, TRUE))
			DeleteObject(hrgnBorder); // SetWindowRgn only owns the region on success
		HWND insertAfter = GetInsertAfterForOverlay(m->MainBorder.hWnd, hWnd);
		DWORD mainSwpFlags = SWP_NOACTIVATE;
		if (!insertAfter)
			mainSwpFlags |= SWP_NOZORDER;
		if (!m->MainBorder.visible) mainSwpFlags |= SWP_SHOWWINDOW;
		SetWindowPos(m->MainBorder.hWnd, insertAfter,
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
	// Scavenge stale overlay-index entries (overlay HWND destroyed/recycled, target removed).
	for (auto it = m->AllBorderWndIndex.begin(); it != m->AllBorderWndIndex.end(); )
	{
		HWND overlayHwnd = it->first;
		HWND targetHwnd = it->second;
		auto wndIt = m->AllBorderWnds.find(targetHwnd);
		if (!overlayHwnd || !IsWindow(overlayHwnd) ||
			wndIt == m->AllBorderWnds.end() ||
			wndIt->second.hWnd != overlayHwnd)
		{
			it = m->AllBorderWndIndex.erase(it);
		}
		else
		{
			++it;
		}
	}

	DWORD nowEnum = GetTickCount();
	HWND focusedWnd = GetForegroundWindow();
	// Foreground changed within recent window: snap backoffs regardless of cursor activity.
	bool recentForegroundChange = m->ForegroundChangeTick &&
		(nowEnum - m->ForegroundChangeTick <= (DWORD)kForegroundChangePulseMs);

	// --- Activity detection: snap the enum interval back to fast if the user is doing anything ---
	// This prevents a fully backed-off timer from leaving borders stale during window moves,
	// resizes, or focus changes even when the global scene hash appeared stable.
	{
		POINT pt = { 0, 0 };
		GetCursorPos(&pt);
		bool mouseActive = (pt.x != m->LastMousePos.x || pt.y != m->LastMousePos.y);
		bool buttonHeld  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 ||
		                   (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
		bool userInteracting = mouseActive || buttonHeld || m->LastButtonHeld;
		// Also snap on button-release so we catch end-of-drag immediately.
		if (userInteracting)
		{
			m->AdaptiveEnumIntervalMs = m->FastTimerMs;
			m->AdaptiveSceneRefreshMs = kAdaptiveFastMs;
		}
		m->LastMousePos   = pt;
		m->LastButtonHeld = buttonHeld;

		if (recentForegroundChange)
		{
			m->AdaptiveEnumIntervalMs = m->FastTimerMs;
			m->AdaptiveSceneRefreshMs = kAdaptiveFastMs;
		}

		// Dedicated z-order probe runs independently of scene enumeration cadence.
		if (!m->AllBorderWnds.empty())
			RunAllModeZOrderProbe(m, nowEnum, userInteracting || recentForegroundChange);
	}

	// Rate-limit the window enumeration independently of the timer rate.
	// During fast-timer cool-down the timer may fire every ~10 ms; this limiter avoids
	// re-enumerating top-level windows on every single tick when the scene is stable.
	if (nowEnum - m->LastAllBordersEnumTick < (DWORD)m->AdaptiveEnumIntervalMs)
		return;
	m->LastAllBordersEnumTick = nowEnum;

	// Raw top-level window snapshot entry in Z-order.
	struct SWindowInfo
	{
		HWND hWnd;
		RECT rect;
		MONITORINFO monitorInfo;
		int zOrder;
		ULONG pid;
		CSandBox* pBox;
		ULONG style;
		ULONG exStyle;
	};

	// Cached "all/alllbl" visual settings per sandbox.
	struct SAllStyle
	{
		bool enabled;
		bool labelOnly;
		COLORREF color;
		int width;
		int alpha;
		int labelMode;
		std::wstring boxName;
	};

	std::vector<SWindowInfo> allWindows;
	std::unordered_map<ULONG, CSandBoxPtr> pidBoxCache;  // shared_ptr keeps objects alive for the tick
	std::map<CSandBox*, SAllStyle> styleCache;
	ULONGLONG settingsHash = kHashSeed;
	if (m->WindowRoleCache.size() > 1024)
		m->WindowRoleCache.clear();

	// Enumerate top-level windows and keep only sandboxed candidates.
	// Per-window overlays are z-order anchored to their target window, so we don't need
	// to track non-sandboxed windows for explicit occlusion region math.
	int zOrder = 0;
	HWND hWnd = GetTopWindow(NULL);
	while (hWnd)
	{
		// Skip our own overlay windows to avoid recursive self-coverage.
		bool isBorderWnd = (hWnd == m->MainBorder.hWnd) ||
			(m->AllBorderWndIndex.find(hWnd) != m->AllBorderWndIndex.end());

		if (!isBorderWnd && IsWindowVisible(hWnd) && !IsIconic(hWnd))
		{
			// Skip DWM-cloaked windows (Peek/Desktop transitions, virtual desktop switch).
			if (m->DwmGetWindowAttribute) {
				DWORD cloaked = 0;
				if (SUCCEEDED(m->DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) {
					zOrder++;
					hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
					continue;
				}
			}

			ULONG Style = GetWindowLong(hWnd, GWL_STYLE);
			if (Style & WS_VISIBLE)
			{
				SWindowInfo wnd;
				wnd.hWnd = hWnd;
				wnd.zOrder = zOrder;
				wnd.pid = 0;
				wnd.style = Style;
				wnd.exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
				memset(&wnd.monitorInfo, 0, sizeof(MONITORINFO));

				GetWindowThreadProcessId(hWnd, &wnd.pid);
				CSandBox* pBox = NULL;
				auto pidIt = pidBoxCache.find(wnd.pid);
				if (pidIt != pidBoxCache.end())
				{
					pBox = pidIt->second.data();
				}
				else
				{
					CSandBoxPtr resolved = m_Api->GetBoxByProcessId(wnd.pid);
					pBox = resolved.data();
					pidBoxCache.insert(std::make_pair(wnd.pid, std::move(resolved)));
				}

				if (!pBox)
				{
					zOrder++;
					hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
					continue;
				}

				HWND ownerWnd = GetWindow(hWnd, GW_OWNER);
				bool isTransientPopup = false;
				bool isMainWindow = true;

				auto roleIt = m->WindowRoleCache.find(hWnd);
				if (roleIt != m->WindowRoleCache.end() &&
					roleIt->second.style == Style &&
					roleIt->second.exStyle == wnd.exStyle &&
					roleIt->second.owner == ownerWnd)
				{
					isTransientPopup = roleIt->second.isTransientPopup;
					isMainWindow = roleIt->second.isMainWindow;
				}
				else
				{
					WCHAR className[64] = { 0 };
					GetClassNameW(hWnd, className, 64);

					if (wcscmp(className, L"#32768") == 0 ||
						wcscmp(className, L"ComboLBox") == 0 ||
						wcscmp(className, L"tooltips_class32") == 0 ||
						wcscmp(className, L"SysShadow") == 0 ||
						wcscmp(className, L"DropDown") == 0)
					{
						isTransientPopup = true;
					}

					if ((wnd.exStyle & WS_EX_TOOLWINDOW) && !(Style & WS_CAPTION))
						isTransientPopup = true;

					if ((wnd.exStyle & WS_EX_NOACTIVATE) && (Style & WS_POPUP) && !(Style & WS_CAPTION))
						isTransientPopup = true;

					bool isSystemDialog = (wcscmp(className, L"#32770") == 0);
					if (ownerWnd != NULL && !(Style & WS_CAPTION) && !isSystemDialog)
						isMainWindow = false;
					else if ((wnd.exStyle & WS_EX_TOOLWINDOW) && !(wnd.exStyle & WS_EX_APPWINDOW))
						isMainWindow = false;
					else if ((Style & WS_POPUP) && !(Style & WS_CAPTION) && !(wnd.exStyle & WS_EX_APPWINDOW) && !isSystemDialog)
						isMainWindow = false;

					SWindowRoleCacheEntry roleEntry;
					roleEntry.style = Style;
					roleEntry.exStyle = wnd.exStyle;
					roleEntry.owner = ownerWnd;
					roleEntry.isTransientPopup = isTransientPopup;
					roleEntry.isMainWindow = isMainWindow;
					m->WindowRoleCache[hWnd] = roleEntry;
				}

				if (isTransientPopup)
				{
					zOrder++;
					hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
					continue;
				}

				wnd.pBox = pBox;

				// Exclude helper/secondary windows similar to old all-mode logic.
				if (!isMainWindow)
				{
					zOrder++;
					hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
					continue;
				}

				// Geometry query is relatively expensive; defer it until after cheap eligibility checks.
				GetActiveWindowRect(hWnd, &wnd.rect);

				// Skip fullscreen/no-monitor-eligible windows for border drawing.
				if (!ShouldDrawBorderForWindow(hWnd, wnd.rect, Style, &wnd.monitorInfo))
				{
					zOrder++;
					hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
					continue;
				}

				// Ignore degenerate rectangles.
				if (wnd.rect.right - wnd.rect.left > 2 && wnd.rect.bottom - wnd.rect.top > 2)
					allWindows.push_back(wnd);
			}
		}

		zOrder++;
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}

	// Lazy style cache so each sandbox setting is parsed once per tick.
	auto getAllStyleForBox = [&](CSandBox* pBox) -> const SAllStyle&
	{
		auto it = styleCache.find(pBox);
		if (it != styleCache.end())
			return it->second;

		SAllStyle style = {};
		style.enabled = false;
		style.labelOnly = false;
		style.color = RGB(255, 255, 0);
		style.width = 6;
		style.alpha = 192;
		style.labelMode = 1;

		if (pBox)
		{
			EBorderMode mode = eBorderOff;
			if (GetBoxBorderSettings(pBox, style.color, style.width, style.alpha, mode, style.labelMode) &&
				(mode == eBorderAllWindows || mode == eBorderAllWindowsLabelOnly))
			{
				style.enabled = true;
				style.labelOnly = (mode == eBorderAllWindowsLabelOnly);
				style.boxName = GetBoxDisplayName(pBox);

				HashMix64(settingsHash, (ULONGLONG)(ULONG_PTR)pBox);
				HashMix64(settingsHash, (ULONGLONG)style.color);
				HashMix64(settingsHash, (ULONGLONG)style.width);
				HashMix64(settingsHash, (ULONGLONG)style.alpha);
				HashMix64(settingsHash, (ULONGLONG)style.labelMode);
				HashMix64(settingsHash, (ULONGLONG)style.labelOnly);
				HashMixWString(settingsHash, style.boxName);
			}
		}

		auto inserted = styleCache.insert(std::make_pair(pBox, style));
		return inserted.first->second;
	};

	// Global scene hash: drives adaptive timer/enumeration backoff.
	ULONGLONG sceneHash = kHashSeed;
	for (const auto& wnd : allWindows)
	{
		HashMix64(sceneHash, (ULONGLONG)(ULONG_PTR)wnd.hWnd);
		HashMix64(sceneHash, (ULONGLONG)(LONG_PTR)wnd.rect.left);
		HashMix64(sceneHash, (ULONGLONG)(LONG_PTR)wnd.rect.top);
		HashMix64(sceneHash, (ULONGLONG)(LONG_PTR)wnd.rect.right);
		HashMix64(sceneHash, (ULONGLONG)(LONG_PTR)wnd.rect.bottom);
		HashMix64(sceneHash, (ULONGLONG)(ULONG_PTR)wnd.pBox);
		HashMix64(sceneHash, (ULONGLONG)wnd.zOrder);
		if (wnd.pBox)
		{
			const SAllStyle& style = getAllStyleForBox(wnd.pBox);
			HashMix64(sceneHash, (ULONGLONG)(style.enabled ? 1ULL : 0ULL));
			HashMix64(sceneHash, (ULONGLONG)(style.labelOnly ? 1ULL : 0ULL));
		}
	}
	HashMix64(sceneHash, settingsHash);

	bool sceneUnchanged =
		sceneHash == m->LastAllBordersSceneHash &&
		(int)allWindows.size() == m->LastAllBordersWindowCount;

	if (sceneUnchanged)
	{
		// Stable scene: progressively back off checks and repaint cadence.
		// Enum gets a lower ceiling (kAdaptiveMaxEnumMs) so cursor activity is never
		// stale for more than ~0.5 s.  Repaint can back off all the way to kAdaptiveMaxMs.
		int nextMs = m->AdaptiveEnumIntervalMs * 2;
		m->AdaptiveEnumIntervalMs = (nextMs < kAdaptiveMaxEnumMs) ? nextMs : kAdaptiveMaxEnumMs;

		int nextRefMs = m->AdaptiveSceneRefreshMs * 2;
		m->AdaptiveSceneRefreshMs = (nextRefMs < kAdaptiveMaxMs) ? nextRefMs : kAdaptiveMaxMs;

		if (nowEnum - m->LastAllBordersRenderTick < (DWORD)m->AdaptiveSceneRefreshMs)
			return;
	}
	else
	{
		// Scene changed: snap back to fast cadence to track movement smoothly.
		// Don't clear WindowRoleCache entirely — entries whose sentinel data still matches
		// remain valid. Stale entries are replaced on next cache miss.
		m->AdaptiveEnumIntervalMs = m->FastTimerMs;
		m->AdaptiveSceneRefreshMs = kAdaptiveFastMs;
		if (!m->FastTimerStartTicks)
			SetBorderTimerInterval(m, m->FastTimerMs);
		m->FastTimerStartTicks = (int)nowEnum;
	}

	m->LastAllBordersSceneHash = sceneHash;
	m->LastAllBordersWindowCount = (int)allWindows.size();
	m->LastAllBordersRenderTick = nowEnum;

	// Active target app windows (not overlay HWNDs) for stale-overlay cleanup.
	std::set<HWND> activeTargets;
	int enabledWindowCount = 0;

	// Per-window overlay build: each target HWND gets its own independent overlay.
	for (const auto& wnd : allWindows)
	{
		bool eligible = false;
		SAllStyle style = {};
		if (wnd.pBox)
		{
			const SAllStyle& cached = getAllStyleForBox(wnd.pBox);
			if (cached.enabled)
			{
				eligible = true;
				style = cached;
			}
		}

		if (eligible)
		{
			enabledWindowCount++;
			activeTargets.insert(wnd.hWnd);
			bool overlayAvailable = true;

			auto it = m->AllBorderWnds.find(wnd.hWnd);
			if (it == m->AllBorderWnds.end())
			{
				// First time this target window is seen: create overlay window.
				SBoxBorderWnd bwnd;
				InitializeBorderWindowData(bwnd);
				bwnd.hWnd = CreateBoxBorderWindow();
				if (bwnd.hWnd)
				{
					SetWindowLongPtr(bwnd.hWnd, 0, ULONG_PTR(this));
					auto inserted = m->AllBorderWnds.insert(std::make_pair(wnd.hWnd, bwnd));
					it = inserted.first;
					m->AllBorderWndIndex[it->second.hWnd] = wnd.hWnd;
				}
			}

			if (it != m->AllBorderWnds.end())
			{
				SBoxBorderWnd& bwnd = it->second;

				// Safety: overlay HWND may be externally destroyed; recreate defensively.
				if (!bwnd.hWnd || !IsWindow(bwnd.hWnd))
				{
					// Clean up stale index entry and free any leftover resources before recreating.
					if (bwnd.hWnd)
						m->AllBorderWndIndex.erase(bwnd.hWnd);
					DestroyBorderWindowResources(bwnd);
					bwnd.hWnd = CreateBoxBorderWindow();
					if (!bwnd.hWnd)
					{
						// GDI window creation failed; drop this entry cleanly.
						// Note: bwnd resources are already freed above; erase the map entry
						// directly to avoid a second DestroyBorderWindowResources call, and
						// do NOT dereference bwnd after this point (iterator invalidated).
						m->AllBorderWnds.erase(it);
						activeTargets.erase(wnd.hWnd);
						overlayAvailable = false;
					}
					else
					{
						SetWindowLongPtr(bwnd.hWnd, 0, ULONG_PTR(this));
						m->AllBorderWndIndex[bwnd.hWnd] = wnd.hWnd;
					}
				}

				if (!overlayAvailable)
				{
					// Continue with coverage/hash tracking below so lower windows still clip correctly.
				}
				else
				{
				// Per-window scene hash (target geom + style + coverage above). This isolates
				// rebuilds so moving one window doesn't force unrelated overlays to repaint.
				ULONGLONG perWindowHash = kHashSeed;
				HashMix64(perWindowHash, (ULONGLONG)(ULONG_PTR)wnd.hWnd);
				HashMix64(perWindowHash, (ULONGLONG)(ULONG_PTR)wnd.pBox);
				HashMix64(perWindowHash, (ULONGLONG)(LONG_PTR)wnd.rect.left);
				HashMix64(perWindowHash, (ULONGLONG)(LONG_PTR)wnd.rect.top);
				HashMix64(perWindowHash, (ULONGLONG)(LONG_PTR)wnd.rect.right);
				HashMix64(perWindowHash, (ULONGLONG)(LONG_PTR)wnd.rect.bottom);
				HashMix64(perWindowHash, (ULONGLONG)style.color);
				HashMix64(perWindowHash, (ULONGLONG)style.width);
				HashMix64(perWindowHash, (ULONGLONG)style.alpha);
				HashMix64(perWindowHash, (ULONGLONG)style.labelMode);
				HashMix64(perWindowHash, (ULONGLONG)(style.labelOnly ? 1ULL : 0ULL));
				HashMixWString(perWindowHash, style.boxName);

				bool fontDirty = (bwnd.boxName != style.boxName || bwnd.labelMode != style.labelMode || bwnd.width != style.width);
				bwnd.boxName = style.boxName;
				bwnd.labelMode = style.labelMode;
				bwnd.width = style.width;

				if (fontDirty || !bwnd.labelFont)
					UpdateBorderLabelFont(bwnd);

				bool skipRebuild = (bwnd.sceneHash == perWindowHash) && bwnd.visible;
				if (!skipRebuild)
				{
					// Rect normalization (desktop edge clipping + taskbar workaround).
					RECT adjustedRect = wnd.rect;
					if (wnd.monitorInfo.cbSize != 0)
						adjustedRect = AdjustRectToDesktop(wnd.rect, wnd.monitorInfo, style.width);

					RECT boundingRect = adjustedRect;
					if (bwnd.labelMode == -1 && bwnd.labelHeight > 0)
						boundingRect.top -= bwnd.labelHeight;

					HRGN hrgnOverlay = CreateRectRgn(0, 0, 0, 0);
					if (hrgnOverlay)
					{
						// Border frame contribution (disabled in label-only mode).
						if (!style.labelOnly)
						{
							HRGN hrgnFrame = CreateBorderRegion(&adjustedRect, style.width);
							if (hrgnFrame)
							{
								OffsetRgn(hrgnFrame, -boundingRect.left, -boundingRect.top);
								CombineRgn(hrgnOverlay, hrgnOverlay, hrgnFrame, RGN_OR);
								DeleteObject(hrgnFrame);
							}
						}

						std::vector<RECT> newLabelRects;
						bool hasDrawableContent = !style.labelOnly;
						if (bwnd.labelMode != 0 && bwnd.labelHeight > 0 && bwnd.labelFont)
						{
							hasDrawableContent = true;
							// One label per overlay (since this is now per-target-window mode).
							int lblBorderWidth = style.labelOnly ? 0 : style.width;
							RECT labelRectScr = CalculateLabelRect(adjustedRect, lblBorderWidth, bwnd.labelWidth, bwnd.labelHeight, bwnd.labelMode);

							HRGN hrgnLabel = CreateRectRgn(labelRectScr.left, labelRectScr.top, labelRectScr.right, labelRectScr.bottom);
							if (hrgnLabel)
							{
								OffsetRgn(hrgnLabel, -boundingRect.left, -boundingRect.top);
								CombineRgn(hrgnOverlay, hrgnOverlay, hrgnLabel, RGN_OR);
								DeleteObject(hrgnLabel);
							}

							RECT labelRectWnd = {
								labelRectScr.left - boundingRect.left,
								labelRectScr.top - boundingRect.top,
								labelRectScr.right - boundingRect.left,
								labelRectScr.bottom - boundingRect.top
							};
							newLabelRects.push_back(labelRectWnd);
						}

						if (bwnd.color != style.color)
							bwnd.color = style.color;

						if (bwnd.alpha != style.alpha)
						{
							SetLayeredWindowAttributes(bwnd.hWnd, 0, style.alpha, LWA_ALPHA);
							bwnd.alpha = style.alpha;
						}

						if (!hasDrawableContent)
						{
							DeleteObject(hrgnOverlay);
							HideBorderWindow(bwnd);
							bwnd.labelRects.clear();
							bwnd.sceneHash = perWindowHash;
							bwnd.lastChangeTick = nowEnum;
						}
						else
						{
							if (!SetWindowRgn(bwnd.hWnd, hrgnOverlay, FALSE))
								DeleteObject(hrgnOverlay);

							// Keep overlay directly above its target window (not globally topmost).
							HWND insertAfter = GetInsertAfterForOverlay(bwnd.hWnd, wnd.hWnd);
							DWORD swpFlags = SWP_NOACTIVATE;
							if (!insertAfter)
								swpFlags |= SWP_NOZORDER;
							if (!bwnd.visible)
								swpFlags |= SWP_SHOWWINDOW;

							bool geometryChanged = true;
							if (bwnd.visible)
							{
								RECT currentRect = { 0, 0, 0, 0 };
								if (GetWindowRect(bwnd.hWnd, &currentRect))
								{
									if (currentRect.left == boundingRect.left &&
										currentRect.top == boundingRect.top &&
										currentRect.right == boundingRect.right &&
										currentRect.bottom == boundingRect.bottom)
									{
										geometryChanged = false;
									}
								}
							}

							bool needSetWindowPos = !bwnd.visible || geometryChanged || (insertAfter != NULL);

							if (needSetWindowPos)
							{
								SetWindowPos(bwnd.hWnd, insertAfter,
									boundingRect.left,
									boundingRect.top,
									boundingRect.right - boundingRect.left,
									boundingRect.bottom - boundingRect.top,
									swpFlags);
							}

							bool labelRectsChanged = !RectVectorEquals(bwnd.labelRects, newLabelRects);
							if (labelRectsChanged || !bwnd.visible || !skipRebuild)
								InvalidateRect(bwnd.hWnd, NULL, TRUE);

							bwnd.labelRects = newLabelRects;
							bwnd.visible = true;
							bwnd.sceneHash = perWindowHash;
							bwnd.lastChangeTick = nowEnum;
						}
					}
					else
					{
						// Region creation failed: hide stale overlay to avoid presenting obsolete geometry.
						HideBorderWindow(bwnd);
						bwnd.labelRects.clear();
					}
				}
				else
				{
					// Hash unchanged: on a recent foreground change, keep z-order anchored to target.
					if (recentForegroundChange && (wnd.hWnd == focusedWnd))
						ProbeOverlayZOrder(bwnd.hWnd, wnd.hWnd);
				}
				}
			}
		}

	}

	// Destroy overlays whose target windows disappeared or are no longer all-mode eligible.
	for (auto it = m->AllBorderWnds.begin(); it != m->AllBorderWnds.end(); )
	{
		if (activeTargets.find(it->first) == activeTargets.end())
		{
			HWND targetWnd = it->first;
			++it;
			RemoveAllModeOverlayForTarget(m, targetWnd);
		}
		else
		{
			++it;
		}
	}

	if (enabledWindowCount == 0)
	{
		// No active all-mode targets this tick: refresh the global-mode probe soon.
		m->CachedHasGlobalAllMode = true;
		m->CachedGlobalAllMode = false;
		m->AdaptiveGlobalAllModeCheckMs = kAdaptiveFastMs;
	}

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
