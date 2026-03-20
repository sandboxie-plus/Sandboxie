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

typedef HRESULT(WINAPI *P_DwmIsCompositionEnabled)(BOOL *enabled);
typedef HRESULT(WINAPI *P_DwmGetWindowAttribute)(HWND hWnd, DWORD dwAttribute, void *pvAttribute, DWORD cbAttribute);

// DWMWA_CLOAKED (14) is defined in <dwmapi.h> (SDK 8.1+) but we load dwmapi.dll dynamically
// so we don't include that header.  Define it ourselves if the SDK didn't already provide it.
// A cloaked value != 0 means the window is hidden by DWM (Peek at Desktop = DWM_CLOAKED_DWM,
// virtual-desktop switch = DWM_CLOAKED_SHELL, app-hide = DWM_CLOAKED_INHERITED).
#ifndef DWMWA_CLOAKED
#define DWMWA_CLOAKED 14
#endif

// Added in Windows 10 version 2004 (SDK may be older on some build machines)
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

// Structure to hold border window data (used for both main and per-box borders)
struct SBoxBorderWnd
{
	HWND hWnd;
	COLORREF color;
	int width;
	int alpha;
	int labelBorderWidth;
	bool visible;
	std::wstring boxName;
	HFONT labelFont;
	int labelMode; // 0=disabled, -1=outside (above border), 1=inside (below border line)
	int labelWidth;
	int labelHeight;
	int labelPadding;
	std::vector<RECT> labelRects; // Label positions (one per window for per-box, one for main)
	ULONGLONG sceneHash;   // per-box hash at last render; used to skip rebuilding unchanged boxes
	bool affinityEnabled;  // last value passed to SetWindowDisplayAffinity; avoids redundant calls
};

struct SWindowRoleCacheEntry
{
	ULONG style;
	ULONG exStyle;
	HWND owner;
	bool isTransientPopup;
	bool isMainWindow;
	DWORD lastSeenTick;   // GetTickCount() when this entry was last accessed; used for stale eviction
};

struct SCoverBoxedWindowsCacheEntry
{
	bool enabled;
	DWORD lastRefreshTick;
	DWORD lastSeenTick;
};

enum EBorderMode { eBorderOff = 0, eBorderNormal = 1, eBorderTitleOnly = 2, eBorderAllWindows = 3, eBorderLabelOnly = 4, eBorderAllWindowsLabelOnly = 5, eBorderTitleOnlyLabelOnly = 6 };

struct SBoxBorder
{
	HANDLE hThread;
	UINT_PTR dwTimerId;
	DWORD FastTimerStartTicks;
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
	std::unordered_map<CSandBox*, SCoverBoxedWindowsCacheEntry> CoverBoxedWindowsCache; // key: box ptr -> cached CoverBoxedWindows flag
};

const WCHAR *Sandboxie_WindowClassName = L"Sandboxie_BorderWindow";

static const int kAdaptiveFastMs = 100;
static const int kAdaptiveMaxMs = 5000;     // max backoff for repaint cadence (truly idle)
static const int kAdaptiveMaxEnumMs = 500;  // max backoff for window enumeration (capped low so
                                            // a sudden move is never stale for more than ~0.5 s)
static const int kFastMoveTimerMs = 10; // minimum floor for frame-aligned fast timer
static const int kFocusRaisePulseMs = 250; // short z-raise window after click/focus change
static const ULONGLONG kHashSeed = 1469598103934665603ULL; // hash initialiser (FNV-inspired)
static const DWORD kCoverBoxedWindowsCacheRefreshMs = 1000;
static const DWORD kCoverBoxedWindowsCacheStaleMs = 30000;

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
	bwnd.labelBorderWidth = 0;
	bwnd.visible = false;
	bwnd.boxName.clear();
	bwnd.labelFont = NULL;
	bwnd.labelMode = 0;
	bwnd.labelWidth = 0;
	bwnd.labelHeight = 0;
	bwnd.labelPadding = 8;
	bwnd.labelRects.clear();
	bwnd.sceneHash = 0;
	bwnd.affinityEnabled = false;
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

	// Window narrower than 2*borderWidth: no room for a label.
	if (availLeft >= availRight)
		return { windowRect.left, windowRect.top, windowRect.left, windowRect.top }; // empty rect

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

	// Acquire the DC first so we can query the monitor DPI before creating the font.
	// NULL hWndForDC gives the screen DC, which reports the primary-monitor DPI.
	HDC hdc = GetDC(hWndForDC);
	if (!hdc) {
		// GetDC failed (window hidden/not ready); report no label size.
		bwnd.labelWidth = 0;
		bwnd.labelHeight = 0;
		return;
	}

	// Scale font height by the DC's vertical DPI so text renders at the same physical
	// size on every monitor regardless of the Windows scaling factor.
	int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
	if (dpi <= 0) dpi = 96;
	int fontHeight = MulDiv(bwnd.labelBorderWidth + 8, dpi, 96);

	bwnd.labelFont = CreateFontW(fontHeight, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

	if (!bwnd.labelFont) {
		ReleaseDC(hWndForDC, hdc);
		bwnd.labelWidth = 0;
		bwnd.labelHeight = 0;
		return;
	}

	// Calculate text dimensions
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
	int adjLeft = rect.left;
	if (rect.left < desktop->left && (desktop->left - rect.left) < (borderWidth + 4))
		adjLeft = desktop->left;

	int adjTop = rect.top;
	if (rect.top < desktop->top && (desktop->top - rect.top) < (borderWidth + 4))
		adjTop = desktop->top;

	int adjRight;
	if (rect.right > desktop->right && (rect.right - desktop->right) < (borderWidth + 4))
		adjRight = desktop->right;
	else
		adjRight = rect.right;

	int adjBottom;
	if (rect.bottom > desktop->bottom && (rect.bottom - desktop->bottom) < (borderWidth + 4))
		adjBottom = desktop->bottom;
	else
		adjBottom = rect.bottom;

	// Taskbar workaround for Windows 10/11:
	// If window bottom matches work area bottom, reduce by 1px to avoid hiding auto-hide taskbar
	if (rect.bottom == monitor.rcWork.bottom)
		adjBottom -= 1;

	return { adjLeft, adjTop, adjRight, adjBottom };
}

// Add a rectangle to an aggregate region using OR composition.
static void AddRectToRegion(HRGN hrgnAggregate, const RECT& rect)
{
	if (!hrgnAggregate)
		return;

	if (rect.right <= rect.left || rect.bottom <= rect.top)
		return;

	HRGN hrgnPart = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
	if (!hrgnPart)
		return;

	if (CombineRgn(hrgnAggregate, hrgnAggregate, hrgnPart, RGN_OR) == ERROR)
	{
		// On failure the destination region becomes undefined - reset to empty.
		CombineRgn(hrgnAggregate, hrgnPart, hrgnPart, RGN_DIFF);
	}

	DeleteObject(hrgnPart);
}

struct STaskbarOcclusionBuildCtx
{
	HRGN hrgnCovered;
};

static BOOL CALLBACK BuildTaskbarOcclusionEnumProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam)
{
	STaskbarOcclusionBuildCtx* ctx = reinterpret_cast<STaskbarOcclusionBuildCtx*>(lParam);
	if (!ctx || !ctx->hrgnCovered)
		return TRUE;

	MONITORINFO monitor = {};
	monitor.cbSize = sizeof(MONITORINFO);
	if (!GetMonitorInfo(hMonitor, &monitor))
		return TRUE;

	const RECT& mon = monitor.rcMonitor;
	const RECT& work = monitor.rcWork;

	// Left appbar strip.
	if (work.left > mon.left)
	{
		RECT r = { mon.left, mon.top, work.left, mon.bottom };
		AddRectToRegion(ctx->hrgnCovered, r);
	}

	// Right appbar strip.
	if (work.right < mon.right)
	{
		RECT r = { work.right, mon.top, mon.right, mon.bottom };
		AddRectToRegion(ctx->hrgnCovered, r);
	}

	// Top appbar strip.
	if (work.top > mon.top)
	{
		RECT r = { mon.left, mon.top, mon.right, work.top };
		AddRectToRegion(ctx->hrgnCovered, r);
	}

	// Bottom appbar strip (classic taskbar area).
	if (work.bottom < mon.bottom)
	{
		RECT r = { mon.left, work.bottom, mon.right, mon.bottom };
		AddRectToRegion(ctx->hrgnCovered, r);
	}

	return TRUE;
}

// Build occlusion for monitor work-area exclusions (taskbar / appbars) and merge into the
// cumulative coverage region, so overlay border fragments are clipped in those areas.
static void MergeTaskbarOcclusionIntoCoveredRegion(HRGN hrgnCovered)
{
	if (!hrgnCovered)
		return;

	STaskbarOcclusionBuildCtx ctx = {};
	ctx.hrgnCovered = hrgnCovered;
	EnumDisplayMonitors(NULL, NULL, BuildTaskbarOcclusionEnumProc, reinterpret_cast<LPARAM>(&ctx));
}

LRESULT CALLBACK CBoxBorder__WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CBoxBorder* This = (CBoxBorder*)GetWindowLongPtr(hwnd, 0);
	if (!This)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	SBoxBorder* m = This->GetBorderData();

	// Suppress the default system erase; all painting is done atomically in WM_PAINT.
	if (uMsg == WM_ERASEBKGND)
		return 1;

	if (uMsg == WM_PAINT) 
	{
		SBoxBorderWnd* pBorder = GetBorderWndByHwnd(m, hwnd);
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		// Paint everything via an off-screen memory DC so background fill and
		// text appear in one atomic blit, eliminating the erase-then-draw flicker.
		RECT clientRC;
		GetClientRect(hwnd, &clientRC);
		int cw = clientRC.right - clientRC.left;
		int ch = clientRC.bottom - clientRC.top;

		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP hbmMem = CreateCompatibleBitmap(hdc, cw, ch);

		// Guard against GDI resource exhaustion: if either allocation failed, skip
		// the off-screen pass entirely rather than cascading null handles into
		// SelectObject / FillRect / BitBlt.
		if (hdcMem && hbmMem)
		{
			HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

			if (pBorder)
			{
				// Fill background with border colour
				HBRUSH hBrush = CreateSolidBrush(pBorder->color);
				FillRect(hdcMem, &clientRC, hBrush);
				DeleteObject(hBrush);

				// Draw label text on top in the same DC
				if (pBorder->labelMode != 0 && !pBorder->boxName.empty() && pBorder->labelFont && !pBorder->labelRects.empty())
				{
					int r = GetRValue(pBorder->color);
					int g = GetGValue(pBorder->color);
					int b = GetBValue(pBorder->color);
					int luminance = (r * 299 + g * 587 + b * 114) / 1000;
					COLORREF textColor = luminance > 128 ? RGB(0, 0, 0) : RGB(255, 255, 255);
					for (const RECT& labelRect : pBorder->labelRects)
					{
						HFONT hOldFont = (HFONT)SelectObject(hdcMem, pBorder->labelFont);
						SetBkMode(hdcMem, TRANSPARENT);
						SetTextColor(hdcMem, textColor);
						RECT rc = labelRect;
						DrawTextW(hdcMem, pBorder->boxName.c_str(), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
						SelectObject(hdcMem, hOldFont);
					}
				}
			}

			// Blit the completed off-screen frame to the window in one operation
			BitBlt(hdc, 0, 0, cw, ch, hdcMem, 0, 0, SRCCOPY);

			SelectObject(hdcMem, hbmOld);
		}

		if (hbmMem) DeleteObject(hbmMem);
		if (hdcMem) DeleteDC(hdcMem);

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

static void ApplyCaptureExclusionAffinity(HWND hWnd, bool enabled);

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
		// New windows start with WDA_NONE by default; no need to set it explicitly.
		SetLayeredWindowAttributes(hWnd, 0, 192, LWA_ALPHA);
		::ShowWindow(hWnd, SW_HIDE);
	}
	return hWnd;
}

static void ApplyCaptureExclusionAffinity(HWND hWnd, bool enabled)
{
	if (!hWnd || !IsWindow(hWnd))
		return;
	if (!enabled)
	{
		SetWindowDisplayAffinity(hWnd, WDA_NONE);
		return;
	}

	// WDA_EXCLUDEFROMCAPTURE is preferred; on older OS builds fallback to WDA_MONITOR.
	if (!SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE))
		SetWindowDisplayAffinity(hWnd, WDA_MONITOR);
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
	m->LastMousePos = { 0, 0 };
	m->LastButtonHeld = false;

	m->ThumbWidth = GetSystemMetrics(SM_CXHTHUMB);
	m->ThumbHeight = GetSystemMetrics(SM_CYVTHUMB);

	// Zero-init before load so call sites can safely guard on non-null;
	// if LoadLibraryW fails these remain null and both guards below stay false.
	m->DwmIsCompositionEnabled = nullptr;
	m->DwmGetWindowAttribute   = nullptr;

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
	if (!m->dwTimerId)
	{
		// Without a timer the border thread would loop forever pumping messages
		// but never updating; bail out so the destructor can clean up the window.
		SetWindowLongPtr(m->MainBorder.hWnd, 0, 0);
		DestroyWindow(m->MainBorder.hWnd);
		m->MainBorder.hWnd = NULL;
		return;
	}

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

static bool GetBoxBorderSettings(CSandBox* pBox, COLORREF& color, int& width, int& alpha, EBorderMode& mode, int& labelMode, int& labelWidth)
{
	// Default values
	color = RGB(255, 255, 0);
	width = 6;
	alpha = 192;
	mode = eBorderNormal;
	labelMode = 1; // Default to inside
	labelWidth = 0;

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
		if (width <= 0) width = 6;  // reject zero and negative values
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

	if (BorderCfg.count() >= 6) {
		labelWidth = BorderCfg.at(5).toInt();
		if (labelWidth <= 0)
			labelWidth = width;
	}

	if (labelWidth <= 0)
		labelWidth = width;

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

static bool IsCoverBoxedWindowsEnabled(SBoxBorder* m, CSandBox* pBox, DWORD nowTick)
{
	if (!m || !pBox)
		return false;

	auto it = m->CoverBoxedWindowsCache.find(pBox);
	if (it != m->CoverBoxedWindowsCache.end())
	{
		it->second.lastSeenTick = nowTick;
		if (nowTick - it->second.lastRefreshTick < kCoverBoxedWindowsCacheRefreshMs)
			return it->second.enabled;

		it->second.enabled = pBox->GetBool("CoverBoxedWindows", false);
		it->second.lastRefreshTick = nowTick;
		return it->second.enabled;
	}

	SCoverBoxedWindowsCacheEntry entry = {};
	entry.enabled = pBox->GetBool("CoverBoxedWindows", false);
	entry.lastRefreshTick = nowTick;
	entry.lastSeenTick = nowTick;
	m->CoverBoxedWindowsCache.insert(std::make_pair(pBox, entry));

	if (m->CoverBoxedWindowsCache.size() > 512)
	{
		for (auto cacheIt = m->CoverBoxedWindowsCache.begin(); cacheIt != m->CoverBoxedWindowsCache.end(); )
		{
			if (nowTick - cacheIt->second.lastSeenTick > kCoverBoxedWindowsCacheStaleMs)
				cacheIt = m->CoverBoxedWindowsCache.erase(cacheIt);
			else
				++cacheIt;
		}
	}

	return entry.enabled;
}

static HRGN CreateBorderRegion(const RECT* rect, int borderWidth)
{
	// Create outer rectangle region
	HRGN hrgnOuter = CreateRectRgn(rect->left, rect->top, rect->right, rect->bottom);
	if (!hrgnOuter)
		return NULL;

	// If the window is smaller than 2*borderWidth in either dimension the inner rect
	// would be inverted, meaning the entire window is effectively the border frame.
	// Return the outer region as-is (correct full-fill for a tiny window).
	LONG innerLeft  = rect->left  + borderWidth;
	LONG innerTop   = rect->top   + borderWidth;
	LONG innerRight = rect->right - borderWidth;
	LONG innerBottom= rect->bottom- borderWidth;
	if (innerRight <= innerLeft || innerBottom <= innerTop)
		return hrgnOuter; // window too thin to hollow out – paint the entire rect as border

	// Create inner rectangle region (the hollow part)
	HRGN hrgnInner = CreateRectRgn(innerLeft, innerTop, innerRight, innerBottom);
	if (!hrgnInner)
	{
		// GDI resource failure; skip this tick rather than painting a solid block.
		DeleteObject(hrgnOuter);
		return NULL;
	}

	// Subtract inner from outer to create frame.
	// On GDI failure hrgnOuter is undefined; discard both regions and report failure.
	if (CombineRgn(hrgnOuter, hrgnOuter, hrgnInner, RGN_DIFF) == ERROR)
	{
		DeleteObject(hrgnInner);
		DeleteObject(hrgnOuter);
		return NULL;
	}
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
			m->CoverBoxedWindowsCache.clear();

			// Mark the cache as stale so the global-all-mode probe fires immediately
			// on the first active tick after idle, avoiding the ~5 s backoff delay.
			m->CachedHasGlobalAllMode = false;
			m->CachedGlobalAllMode = false;
			m->LastAllModeCheckTick = now;
			m->AdaptiveGlobalAllModeCheckMs = kAdaptiveFastMs;
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
				m->AdaptiveGlobalAllModeCheckMs = NextAdaptiveIntervalMs(m->AdaptiveGlobalAllModeCheckMs);
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
	bool coverBoxedWindows = pProcessBox && IsCoverBoxedWindowsEnabled(m, pProcessBox.data(), now);
	bool hideBordersFromCapture = pProcessBox ? pProcessBox->GetBool("HideBordersFromCapture", coverBoxedWindows, true) : coverBoxedWindows;

	// Get border settings for the focused window's box
	COLORREF boxColor;
	int boxWidth, boxAlpha, boxLabelMode, boxLabelWidth;
	EBorderMode boxMode = eBorderOff;
	if (pProcessBox)
		GetBoxBorderSettings(pProcessBox.data(), boxColor, boxWidth, boxAlpha, boxMode, boxLabelMode, boxLabelWidth);

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
			m->MainBorder.labelBorderWidth = boxLabelWidth;
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
					// Affinity may have changed even if geometry is unchanged; apply now so we don't
					// need to wait for a move/resize event to pick up a CoverBoxedWindows/HideBordersFromCapture change.
					if (hideBordersFromCapture != m->MainBorder.affinityEnabled) {
						ApplyCaptureExclusionAffinity(m->MainBorder.hWnd, hideBordersFromCapture);
						m->MainBorder.affinityEnabled = hideBordersFromCapture;
					}
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
		std::vector<RECT> prevLabelRects = m->MainBorder.labelRects;
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
				if (CombineRgn(hrgnBorder, hrgnBorder, hrgnLabel, RGN_OR) == ERROR) {
					// hrgnBorder is undefined; abandon this tick to avoid setting a corrupt region.
					DeleteObject(hrgnLabel);
					DeleteObject(hrgnBorder);
					hrgnBorder = NULL;
				} else {
					DeleteObject(hrgnLabel);
				}
			}
		}

		if (!hrgnBorder)
			return; // GDI region combination failed; leave border as-is for this tick

		// Position and show the border window.
		// Keep SWP_NOZORDER during stable focus mode so we don't permanently fight other
		// always-on-top overlays.  Temporarily re-raise only for a short click/focus
		// pulse after foreground changed.
		if (!SetWindowRgn(m->MainBorder.hWnd, hrgnBorder, FALSE))
			DeleteObject(hrgnBorder); // SetWindowRgn only owns the region on success
		ApplyCaptureExclusionAffinity(m->MainBorder.hWnd, hideBordersFromCapture);
		m->MainBorder.affinityEnabled = hideBordersFromCapture;
		bool focusRaisePulseActive = m->FocusRaisePulseStartTick &&
			(now - m->FocusRaisePulseStartTick <= (DWORD)kFocusRaisePulseMs);
		bool raiseDuringInteraction = focusRaisePulseActive;
		DWORD mainSwpFlags = SWP_NOACTIVATE;
		if (!raiseDuringInteraction)
			mainSwpFlags |= SWP_NOZORDER;
		if (!m->MainBorder.visible) mainSwpFlags |= SWP_SHOWWINDOW;

		bool needSetWindowPos = true;
		if (m->MainBorder.visible && !raiseDuringInteraction)
		{
			RECT currentRect = { 0, 0, 0, 0 };
			if (GetWindowRect(m->MainBorder.hWnd, &currentRect))
			{
				if (currentRect.left == windowRect.left &&
					currentRect.top == windowRect.top &&
					currentRect.right == windowRect.right &&
					currentRect.bottom == windowRect.bottom)
				{
					needSetWindowPos = false;
				}
			}
		}

		if (needSetWindowPos)
		{
			SetWindowPos(m->MainBorder.hWnd, raiseDuringInteraction ? HWND_TOP : NULL,
				windowRect.left, windowRect.top,
				windowRect.right - windowRect.left,
				windowRect.bottom - windowRect.top,
				mainSwpFlags);
		}

		bool labelRectsChanged = !RectVectorEquals(prevLabelRects, m->MainBorder.labelRects);
		if (!m->MainBorder.visible || needSetWindowPos || labelRectsChanged)
			InvalidateRect(m->MainBorder.hWnd, NULL, FALSE);

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

	// --- Activity detection: snap the enum interval back to fast if the user is doing anything ---
	// This prevents a fully backed-off timer from leaving borders stale during window moves,
	// resizes, or focus changes even when the global scene hash appeared stable.
	{
		POINT pt = { 0, 0 };
		GetCursorPos(&pt);
		bool mouseActive = (pt.x != m->LastMousePos.x || pt.y != m->LastMousePos.y);
		bool buttonHeld  = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 ||
		                   (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
		// Also snap on button-release so we catch end-of-drag immediately.
		if (mouseActive || buttonHeld || m->LastButtonHeld)
		{
			m->AdaptiveEnumIntervalMs = m->FastTimerMs;
			m->AdaptiveSceneRefreshMs = kAdaptiveFastMs;
		}
		m->LastMousePos   = pt;
		m->LastButtonHeld = buttonHeld;
	}

	// Foreground/focus pulse: snap back immediately so z-order raise propagates without
	// waiting for the next enum window.
	HWND focusedWnd = GetForegroundWindow();
	bool focusRaisePulseActive = m->FocusRaisePulseStartTick &&
		(nowEnum - m->FocusRaisePulseStartTick <= (DWORD)kFocusRaisePulseMs);
	if (focusRaisePulseActive)
	{
		m->AdaptiveEnumIntervalMs = m->FastTimerMs;
		m->AdaptiveSceneRefreshMs = kAdaptiveFastMs;
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
		int labelWidth;
		std::wstring boxName;
	};

	std::vector<SWindowInfo> allWindows;
	std::unordered_map<ULONG, CSandBoxPtr> pidBoxCache;
	std::map<CSandBox*, SAllStyle> styleCache;
	ULONGLONG settingsHash = kHashSeed;
	// Evict stale/dead entries when the cache grows large.
	// Prefer selective eviction (dead HWNDs + entries unseen for > 30 s) over a wholesale
	// clear to avoid reclassifying all live windows in a single tick.
	if (m->WindowRoleCache.size() > 512)
	{
		const DWORD kRoleCacheStaleMs = 30000;
		for (auto it = m->WindowRoleCache.begin(); it != m->WindowRoleCache.end(); )
		{
			if ((nowEnum - it->second.lastSeenTick) > kRoleCacheStaleMs || !IsWindow(it->first))
				it = m->WindowRoleCache.erase(it);
			else
				++it;
		}
		if (m->WindowRoleCache.size() > 1024)
			m->WindowRoleCache.clear(); // last resort: still above hard cap after selective sweep
	}

	// Enumerate ALL top-level windows in current Z-order (sandboxed + non-sandboxed).
	// Non-sandboxed windows still matter because they occlude overlays under them.
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
			ULONG ExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
			if (Style & WS_VISIBLE)
			{
				HWND ownerWnd = GetWindow(hWnd, GW_OWNER);
				bool isTransientPopup = false;
				bool isMainWindow = true;

				auto roleIt = m->WindowRoleCache.find(hWnd);
				if (roleIt != m->WindowRoleCache.end() &&
					roleIt->second.style == Style &&
					roleIt->second.exStyle == ExStyle &&
					roleIt->second.owner == ownerWnd)
				{
					isTransientPopup = roleIt->second.isTransientPopup;
					isMainWindow = roleIt->second.isMainWindow;
					roleIt->second.lastSeenTick = nowEnum; // refresh LRU timestamp
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

					if ((ExStyle & WS_EX_TOOLWINDOW) && !(Style & WS_CAPTION))
						isTransientPopup = true;

					if ((ExStyle & WS_EX_NOACTIVATE) && (Style & WS_POPUP) && !(Style & WS_CAPTION))
						isTransientPopup = true;

					bool isSystemDialog = (wcscmp(className, L"#32770") == 0);
					if (ownerWnd != NULL && !(Style & WS_CAPTION) && !isSystemDialog)
						isMainWindow = false;
					else if ((ExStyle & WS_EX_TOOLWINDOW) && !(ExStyle & WS_EX_APPWINDOW))
						isMainWindow = false;
					else if ((Style & WS_POPUP) && !(Style & WS_CAPTION) && !(ExStyle & WS_EX_APPWINDOW) && !isSystemDialog)
						isMainWindow = false;

					SWindowRoleCacheEntry roleEntry;
					roleEntry.style = Style;
					roleEntry.exStyle = ExStyle;
					roleEntry.owner = ownerWnd;
					roleEntry.isTransientPopup = isTransientPopup;
					roleEntry.isMainWindow = isMainWindow;
					roleEntry.lastSeenTick = nowEnum;
					m->WindowRoleCache.insert_or_assign(hWnd, roleEntry);
				}

				if (isTransientPopup)
				{
					zOrder++;
					hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
					continue;
				}

				SWindowInfo wnd;
				wnd.hWnd = hWnd;
				wnd.zOrder = zOrder;
				wnd.pid = 0;
				wnd.style = Style;
				wnd.exStyle = ExStyle;
				wnd.pBox = NULL;
				memset(&wnd.monitorInfo, 0, sizeof(MONITORINFO));

				GetWindowThreadProcessId(hWnd, &wnd.pid);
				auto pidIt = pidBoxCache.find(wnd.pid);
				if (pidIt != pidBoxCache.end())
				{
					wnd.pBox = pidIt->second.data();
				}
				else
				{
					CSandBoxPtr resolved = m_Api->GetBoxByProcessId(wnd.pid);
					wnd.pBox = resolved.data();
					pidBoxCache.insert(std::make_pair(wnd.pid, std::move(resolved)));
				}

				if (wnd.pBox && !isMainWindow)
				{
					zOrder++;
					hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
					continue;
				}

				// Defer geometry lookup until after cheap eligibility checks.
				GetActiveWindowRect(hWnd, &wnd.rect);

				if (wnd.pBox)
				{
					// Skip fullscreen/no-monitor-eligible windows for border drawing.
					if (!ShouldDrawBorderForWindow(hWnd, wnd.rect, Style, &wnd.monitorInfo))
						wnd.pBox = NULL;
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
		style.labelWidth = style.width;

		if (pBox)
		{
			EBorderMode mode = eBorderOff;
			if (GetBoxBorderSettings(pBox, style.color, style.width, style.alpha, mode, style.labelMode, style.labelWidth) &&
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
				HashMix64(settingsHash, (ULONGLONG)style.labelWidth);
				HashMix64(settingsHash, (ULONGLONG)style.labelOnly);
				HashMixWString(settingsHash, style.boxName);
			}
		}

		auto inserted = styleCache.insert(std::make_pair(pBox, style));
		return inserted.first->second;
	};

	bool globalBorderExcludeTaskbar = m_Api->GetGlobalSettings()->GetBool("BorderExcludeTaskbar", true);

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
			if (style.enabled)
			{
				bool coverForAffinity = IsCoverBoxedWindowsEnabled(m, wnd.pBox, nowEnum);
				bool applyCaptureAffinity = wnd.pBox->GetBool("HideBordersFromCapture", coverForAffinity, true);
				bool excludeTaskbar = wnd.pBox->GetBool("BorderExcludeTaskbar", globalBorderExcludeTaskbar);
				HashMix64(sceneHash, (ULONGLONG)(applyCaptureAffinity ? 1ULL : 0ULL));
				HashMix64(sceneHash, (ULONGLONG)(excludeTaskbar ? 1ULL : 0ULL));
			}
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
		m->AdaptiveEnumIntervalMs = m->FastTimerMs;
		m->AdaptiveSceneRefreshMs = kAdaptiveFastMs;
		if (!m->FastTimerStartTicks)
			SetBorderTimerInterval(m, m->FastTimerMs);
		m->FastTimerStartTicks = nowEnum;
	}

	m->LastAllBordersSceneHash = sceneHash;
	m->LastAllBordersWindowCount = (int)allWindows.size();
	m->LastAllBordersRenderTick = nowEnum;

	// Cumulative occlusion region of windows already processed (top -> bottom).
	HRGN hrgnCovered = CreateRectRgn(0, 0, 0, 0);
	if (!hrgnCovered)
		return;

	// Build taskbar occlusion region separately so BorderExcludeTaskbar can be overridden per-box.
	HRGN hrgnTaskbar = CreateRectRgn(0, 0, 0, 0);
	if (hrgnTaskbar)
		MergeTaskbarOcclusionIntoCoveredRegion(hrgnTaskbar);

	ULONGLONG rollingCoverageHash = kHashSeed;
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
			bool coverForAffinity = IsCoverBoxedWindowsEnabled(m, wnd.pBox, nowEnum);
			bool applyCaptureAffinity = wnd.pBox->GetBool("HideBordersFromCapture", coverForAffinity, true);
			bool excludeTaskbar = wnd.pBox->GetBool("BorderExcludeTaskbar", globalBorderExcludeTaskbar);
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
				HashMix64(perWindowHash, (ULONGLONG)style.labelWidth);
				HashMix64(perWindowHash, (ULONGLONG)(style.labelOnly ? 1ULL : 0ULL));
				HashMixWString(perWindowHash, style.boxName);
				HashMix64(perWindowHash, rollingCoverageHash);
				HashMix64(perWindowHash, (ULONGLONG)(applyCaptureAffinity ? 1ULL : 0ULL)); // cover state change invalidates skip-rebuild
				HashMix64(perWindowHash, (ULONGLONG)(excludeTaskbar ? 1ULL : 0ULL)); // per-box taskbar exclusion setting

				bool fontDirty = (bwnd.boxName != style.boxName || bwnd.labelMode != style.labelMode || bwnd.labelBorderWidth != style.labelWidth);
				bwnd.boxName = style.boxName;
				bwnd.labelMode = style.labelMode;
				bwnd.width = style.width;
				bwnd.labelBorderWidth = style.labelWidth;

				if (fontDirty || !bwnd.labelFont)
					UpdateBorderLabelFont(bwnd, bwnd.hWnd);

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

					// Build effective clip region: hrgnCovered plus taskbar areas if BorderExcludeTaskbar is set for this box.
					HRGN hrgnEffective = hrgnCovered;
					bool hrgnEffectiveOwned = false;
					if (excludeTaskbar && hrgnTaskbar)
					{
						HRGN hrgnTemp = CreateRectRgn(0, 0, 0, 0);
						if (hrgnTemp)
						{
							if (CombineRgn(hrgnTemp, hrgnCovered, hrgnTaskbar, RGN_OR) != ERROR)
							{
								hrgnEffective = hrgnTemp;
								hrgnEffectiveOwned = true;
							}
							else
								DeleteObject(hrgnTemp);
						}
					}

					HRGN hrgnOverlay = CreateRectRgn(0, 0, 0, 0);
					if (hrgnOverlay)
					{
						bool rgnFailed = false;

						// Border frame contribution (disabled in label-only mode), clipped by coverage above.
						if (!style.labelOnly)
						{
							HRGN hrgnFrame = CreateBorderRegion(&adjustedRect, style.width);
							if (hrgnFrame)
							{
								if (CombineRgn(hrgnFrame, hrgnFrame, hrgnEffective, RGN_DIFF) == ERROR)
									rgnFailed = true;
								else
								{
									OffsetRgn(hrgnFrame, -boundingRect.left, -boundingRect.top);
									if (CombineRgn(hrgnOverlay, hrgnOverlay, hrgnFrame, RGN_OR) == ERROR)
										rgnFailed = true;
								}
								DeleteObject(hrgnFrame);
							}
						}

						std::vector<RECT> newLabelRects;
						bool hasDrawableContent = !style.labelOnly && !rgnFailed;
						if (!rgnFailed && bwnd.labelMode != 0 && bwnd.labelHeight > 0 && bwnd.labelFont)
						{
							hasDrawableContent = true;
							// One label per overlay (since this is now per-target-window mode).
							int lblBorderWidth = style.labelOnly ? 0 : style.width;
							RECT labelRectScr = CalculateLabelRect(adjustedRect, lblBorderWidth, bwnd.labelWidth, bwnd.labelHeight, bwnd.labelMode);

							HRGN hrgnLabel = CreateRectRgn(labelRectScr.left, labelRectScr.top, labelRectScr.right, labelRectScr.bottom);
							if (hrgnLabel)
							{
								if (CombineRgn(hrgnLabel, hrgnLabel, hrgnEffective, RGN_DIFF) == ERROR)
									rgnFailed = true;
								else
								{
									OffsetRgn(hrgnLabel, -boundingRect.left, -boundingRect.top);
									if (CombineRgn(hrgnOverlay, hrgnOverlay, hrgnLabel, RGN_OR) == ERROR)
										rgnFailed = true;
								}
								DeleteObject(hrgnLabel);
							}

							if (rgnFailed)
								hasDrawableContent = false;
							else
							{
								RECT labelRectWnd = {
									labelRectScr.left - boundingRect.left,
									labelRectScr.top - boundingRect.top,
									labelRectScr.right - boundingRect.left,
									labelRectScr.bottom - boundingRect.top
								};
								newLabelRects.push_back(labelRectWnd);
							}
						}

						if (bwnd.color != style.color)
							bwnd.color = style.color;

						if (bwnd.alpha != style.alpha)
						{
							SetLayeredWindowAttributes(bwnd.hWnd, 0, style.alpha, LWA_ALPHA);
							bwnd.alpha = style.alpha;
						}

						if (rgnFailed || !hasDrawableContent)
						{
							DeleteObject(hrgnOverlay);
							HideBorderWindow(bwnd);
							bwnd.labelRects.clear();
							bwnd.sceneHash = perWindowHash;
						}
						else
						{
							if (!SetWindowRgn(bwnd.hWnd, hrgnOverlay, FALSE))
								DeleteObject(hrgnOverlay);
							ApplyCaptureExclusionAffinity(bwnd.hWnd, applyCaptureAffinity);
							bwnd.affinityEnabled = applyCaptureAffinity;

							// Keep steady-state z-order unless short focus pulse is active for this target.
							bool raiseThisWindow = focusRaisePulseActive && (wnd.hWnd == focusedWnd);
							DWORD swpFlags = SWP_NOACTIVATE;
							if (!raiseThisWindow)
								swpFlags |= SWP_NOZORDER;
							if (!bwnd.visible)
								swpFlags |= SWP_SHOWWINDOW;

							bool needSetWindowPos = true;
							if (bwnd.visible && !raiseThisWindow)
							{
								RECT currentRect = { 0, 0, 0, 0 };
								if (GetWindowRect(bwnd.hWnd, &currentRect))
								{
									if (currentRect.left == boundingRect.left &&
										currentRect.top == boundingRect.top &&
										currentRect.right == boundingRect.right &&
										currentRect.bottom == boundingRect.bottom)
									{
										needSetWindowPos = false;
									}
								}
							}

							if (needSetWindowPos)
							{
								SetWindowPos(bwnd.hWnd, raiseThisWindow ? HWND_TOP : NULL,
									boundingRect.left,
									boundingRect.top,
									boundingRect.right - boundingRect.left,
									boundingRect.bottom - boundingRect.top,
									swpFlags);
							}

							bool labelRectsChanged = !RectVectorEquals(bwnd.labelRects, newLabelRects);
							if (labelRectsChanged || !bwnd.visible || !skipRebuild)
								InvalidateRect(bwnd.hWnd, NULL, FALSE);

							bwnd.labelRects = newLabelRects;
							bwnd.visible = true;
							bwnd.sceneHash = perWindowHash;
						}
					}
					else
					{
						// Region creation failed: hide stale overlay to avoid presenting obsolete geometry.
						HideBorderWindow(bwnd);
						bwnd.labelRects.clear();
					}
					if (hrgnEffectiveOwned)
						DeleteObject(hrgnEffective);
				}
				else
				{
					// Hash unchanged: only optional focus pulse raise.
					if (focusRaisePulseActive && (wnd.hWnd == focusedWnd)) {
						ApplyCaptureExclusionAffinity(bwnd.hWnd, applyCaptureAffinity);
						SetWindowPos(bwnd.hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					}
				}
				}
			}
		}

		// Extend the cumulative occlusion region by this window's visible rectangle.
		HRGN hrgnWindow = CreateRectRgn(wnd.rect.left, wnd.rect.top, wnd.rect.right, wnd.rect.bottom);
		if (hrgnWindow)
		{
			if (CombineRgn(hrgnCovered, hrgnCovered, hrgnWindow, RGN_OR) == ERROR)
			{
				// hrgnCovered is undefined; reset to empty so subsequent windows aren't over-clipped.
				DeleteObject(hrgnCovered);
				hrgnCovered = CreateRectRgn(0, 0, 0, 0);
			}
			DeleteObject(hrgnWindow);
		}

		HashMix64(rollingCoverageHash, (ULONGLONG)(ULONG_PTR)wnd.hWnd);
		HashMix64(rollingCoverageHash, (ULONGLONG)(LONG_PTR)wnd.rect.left);
		HashMix64(rollingCoverageHash, (ULONGLONG)(LONG_PTR)wnd.rect.top);
		HashMix64(rollingCoverageHash, (ULONGLONG)(LONG_PTR)wnd.rect.right);
		HashMix64(rollingCoverageHash, (ULONGLONG)(LONG_PTR)wnd.rect.bottom);
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

	// Done with cumulative occlusion region.
	if (hrgnTaskbar)
		DeleteObject(hrgnTaskbar);
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
		int width, alpha, labelMode, labelWidth;
		EBorderMode mode;

		if (GetBoxBorderSettings(pBox, color, width, alpha, mode, labelMode, labelWidth) && (mode == eBorderAllWindows || mode == eBorderAllWindowsLabelOnly))
			return true;
	}

	return false;
}
