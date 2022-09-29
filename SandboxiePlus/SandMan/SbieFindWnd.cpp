
//////////////////////////////////////////////////////////////////////////////////////////
// WinSpy based window finder
//

#include <windows.h>
#include "Helpers/FindTool.h"


typedef enum DEVICE_SCALE_FACTOR {
    DEVICE_SCALE_FACTOR_INVALID	= 0,
    SCALE_100_PERCENT	= 100,
    SCALE_120_PERCENT	= 120,
    SCALE_125_PERCENT	= 125,
    SCALE_140_PERCENT	= 140,
    SCALE_150_PERCENT	= 150,
    SCALE_160_PERCENT	= 160,
    SCALE_175_PERCENT	= 175,
    SCALE_180_PERCENT	= 180,
    SCALE_200_PERCENT	= 200,
    SCALE_225_PERCENT	= 225,
    SCALE_250_PERCENT	= 250,
    SCALE_300_PERCENT	= 300,
    SCALE_350_PERCENT	= 350,
    SCALE_400_PERCENT	= 400,
    SCALE_450_PERCENT	= 450,
    SCALE_500_PERCENT	= 500
} 	DEVICE_SCALE_FACTOR;

typedef HRESULT (CALLBACK *P_GetScaleFactorForMonitor)(HMONITOR, DEVICE_SCALE_FACTOR*);

UINT GetMonitorScaling(HWND hwnd)
{
    static HINSTANCE shcore = LoadLibrary(L"Shcore.dll");
    if (shcore != nullptr)
    {
        if (auto getScaleFactorForMonitor =
                P_GetScaleFactorForMonitor(GetProcAddress(shcore, "GetScaleFactorForMonitor")))
        {
			HMONITOR monitor =
                MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);

            DEVICE_SCALE_FACTOR Scale;

            getScaleFactorForMonitor(monitor, &Scale);

            return Scale;
        }
    }
    return 100;
}


#define IDD_FINDER_TOOL                 111
#define ID_FINDER_TARGET                112
#define ID_FINDER_EXPLAIN               113
#define ID_FINDER_RESULT                114

struct SFinderWndData {
	int Scale;
	HFONT hFont;
};

#define DS(x) ((x) * WndData.Scale / 100)

UINT CALLBACK FindProc(HWND hwndTool, UINT uCode, HWND hwnd)
{
	ULONG pid;
	if (uCode == WFN_END)
		GetWindowThreadProcessId(hwnd, &pid);
	else
		pid = 0;

	hwndTool = GetParent(hwndTool);

	SFinderWndData &WndData = *(SFinderWndData*)GetWindowLongPtr(hwndTool, 0);

	if (pid && pid != GetCurrentProcessId())
	{
		RECT rc;
		GetWindowRect(hwndTool, &rc);
		if (rc.bottom - rc.top <= DS(150)) 
			SetWindowPos(hwndTool, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top + DS(70), SWP_SHOWWINDOW | SWP_NOMOVE);

		CBoxedProcessPtr pProcess = theAPI->GetProcessById(pid);
		if (!pProcess.isNull()) 
		{
			std::wstring result = CSandMan::tr("The selected window is running as part of program %1 in sandbox %2").arg(pProcess->GetProcessName()).arg(pProcess->GetBoxName()).toStdWString();

			SetWindowText(GetDlgItem(hwndTool, ID_FINDER_RESULT), result.c_str());
			//::ShowWindow(GetDlgItem(hwndTool, ID_FINDER_YES_BOXED), SW_SHOW);
		}
		else
		{
			std::wstring result = CSandMan::tr("The selected window is not running as part of any sandboxed program.").toStdWString();

			SetWindowText(GetDlgItem(hwndTool, ID_FINDER_RESULT), result.c_str());
			//::ShowWindow(GetDlgItem(hwndTool, ID_FINDER_NOT_BOXED), SW_SHOW);
		}
		::ShowWindow(GetDlgItem(hwndTool, ID_FINDER_RESULT), SW_SHOW);
	}
	else
	{
		RECT rc;
		GetWindowRect(hwndTool, &rc);
		if (rc.bottom - rc.top > DS(150))
			SetWindowPos(hwndTool, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top - DS(70), SWP_SHOWWINDOW | SWP_NOMOVE);

		//::ShowWindow(GetDlgItem(hwndTool, ID_FINDER_YES_BOXED), SW_HIDE);
		//::ShowWindow(GetDlgItem(hwndTool, ID_FINDER_NOT_BOXED), SW_HIDE);
		::ShowWindow(GetDlgItem(hwndTool, ID_FINDER_RESULT), SW_HIDE);
	}

	return 0;
}

// hwnd:    All window processes are passed the handle of the window
//         that they belong to in hwnd.
// msg:     Current message (e.g., WM_*) from the OS.
// wParam:  First message parameter, note that these are more or less
//          integers, but they are really just "data chunks" that
//          you are expected to memcpy as raw data to float, etc.
// lParam:  Second message parameter, same deal as above.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
			SFinderWndData &WndData = *(SFinderWndData*)createStruct->lpCreateParams;
			SetWindowLongPtr(hwnd, 0, (LONG_PTR)&WndData);

			std::wstring info = CSandMan::tr("Drag the Finder Tool over a window to select it, then release the mouse to check if the window is sandboxed.").toStdWString();

			CreateWindow(L"Static", L"", SS_BITMAP | SS_NOTIFY | WS_VISIBLE | WS_CHILD, DS(10), DS(10), DS(32), DS(32), hwnd, (HMENU)ID_FINDER_TARGET, NULL, NULL);
			CreateWindow(L"Static", info.c_str(), WS_VISIBLE | WS_CHILD, DS(60), DS(10), DS(180), DS(85), hwnd, (HMENU)ID_FINDER_EXPLAIN, NULL, NULL);
			CreateWindow(L"Static", L"", WS_CHILD, DS(60), DS(100), DS(180), DS(50), hwnd, (HMENU)ID_FINDER_RESULT, NULL, NULL);

			WndData.hFont = CreateFont(DS(13), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));
			if (WndData.hFont) {
				SendMessage(GetDlgItem(hwnd, ID_FINDER_EXPLAIN), WM_SETFONT, (WPARAM)WndData.hFont, TRUE);
				SendMessage(GetDlgItem(hwnd, ID_FINDER_RESULT), WM_SETFONT, (WPARAM)WndData.hFont, TRUE);
			}

			MakeFinderTool(GetDlgItem(hwnd, ID_FINDER_TARGET), FindProc);

			break;
		}

		case WM_CLOSE:
			SFinderWndData &WndData = *(SFinderWndData*)GetWindowLongPtr(hwnd, 0);

			if (WndData.hFont) DeleteObject(WndData.hFont);

			//DestroyWindow(hwnd);
			PostQuitMessage(0);
			break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

DWORD WINAPI FinderThreadFunc(LPVOID lpParam)
{
	MSG  msg;
	WNDCLASS mainWindowClass = { 0 };

	HINSTANCE hInstance = NULL;

	// You can set the main window name to anything, but
	// typically you should prefix custom window classes
	// with something that makes it unique.
	mainWindowClass.lpszClassName = TEXT("SBp.WndFinder");

	mainWindowClass.hInstance = hInstance;
	mainWindowClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	mainWindowClass.lpfnWndProc = WndProc;
	mainWindowClass.hCursor = LoadCursor(0, IDC_ARROW);

	mainWindowClass.cbWndExtra = sizeof(void*); // SFinderWndData

	RegisterClass(&mainWindowClass);

	// Notes:
	// - The classname identifies the TYPE of the window. Not a C type.
	//   This is a (TCHAR*) ID that Windows uses internally.
	// - The window name is really just the window text, this is
	//   commonly used for captions, including the title
	//   bar of the window itself.
	// - parentHandle is considered the "owner" of this
	//   window. MessageBoxes can use HWND_MESSAGE to
	//   free them of any window.
	// - menuHandle: hMenu specifies the child-window identifier,
	//               an integer value used by a dialog box
	//               control to notify its parent about events.
	//               The application determines the child-window
	//               identifier; it must be unique for all
	//               child windows with the same parent window.

	SFinderWndData WndData;
	WndData.Scale = GetMonitorScaling(MainWndHandle);

	HWND hwnd = CreateWindow(mainWindowClass.lpszClassName, CSandMan::tr("Sandboxie-Plus - Window Finder").toStdWString().c_str()
		, WS_SYSMENU | WS_CAPTION | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, DS(275), DS(135), NULL, 0, hInstance, &WndData);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}


void CSandMan::OnWndFinder()
{
	m_pWndFinder->setEnabled(false);

	HANDLE hThread = CreateThread(NULL, 0, FinderThreadFunc, NULL, 0, NULL);

	QWinEventNotifier* finishedNotifier = new QWinEventNotifier(hThread);
	finishedNotifier->setEnabled(true);
	connect(finishedNotifier, &QWinEventNotifier::activated, this, [finishedNotifier, this, hThread]() {
		CloseHandle(hThread);

		m_pWndFinder->setEnabled(true);

		finishedNotifier->setEnabled(false);
		finishedNotifier->deleteLater();
	});
}
