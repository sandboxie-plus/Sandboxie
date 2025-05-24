/*
* Copyright 2025 David Xanatos, xanasoft.com
*
* This program is free software; you can redistribute it and/or
*   modify it under the terms of the GNU Lesser General Public
*   License as published by the Free Software Foundation; either
*   version 3 of the License, or (at your option) any later version.
* 
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   Lesser General Public License for more details.
* 
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//#include "pch.h"
#include "SecDeskHelper.h"

#include <wincodec.h> // WIC for PNG loading

#pragma comment(lib, "windowscodecs.lib")

struct MessageBoxParams
{
    std::wstring text;
    std::wstring title;
    UINT uType;
    int result;
    std::wstring backgroundImagePath;
};

struct DialogParams
{
    int(*func)(HWND hWnd, void* param);
    void* param;
    int result;
    std::wstring backgroundImagePath;
};

class CSecureDesktop
{
public:
    CSecureDesktop(const std::wstring& backgroundImagePath)
    {
        // Load the PNG as HBITMAP
        if (!backgroundImagePath.empty())
        {
            m_hBitmap = LoadBitmapFromImage(backgroundImagePath.c_str(), &m_BitmapWidth, &m_BitmapHeight);
        }
    }
    ~CSecureDesktop()
    {
        SwitchToOriginalDesktop();

        if (m_hBitmap)
        {
            DeleteObject(m_hBitmap);
            m_hBitmap = NULL;
        }
    }

    HWND GetBackgroundWnd() const { return m_BackgroundWnd; }

    BOOLEAN SwitchToSecureDesktop()
    {
        if (m_hWinlogonDesktop)
            return TRUE;

        // Save current desktop
        m_hOriginalDesktop = GetThreadDesktop(GetCurrentThreadId());

        // Open Winlogon desktop
        m_hWinlogonDesktop = OpenDesktopW(L"Winlogon", 0, FALSE, GENERIC_ALL);
        if (!m_hWinlogonDesktop)
            return FALSE;
        
        if (!SetThreadDesktop(m_hWinlogonDesktop) || !SwitchDesktop(m_hWinlogonDesktop))
        {
            CloseDesktop(m_hWinlogonDesktop);
            m_hWinlogonDesktop = NULL;
            return FALSE;
        }

        // Register window class
        WNDCLASSW wc = { 0 };
        wc.lpfnWndProc = BackgroundWndProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = L"BackgroundWindow";

        RegisterClassW(&wc);

        // Create fullscreen window
        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);

        m_BackgroundWnd = CreateWindowExW(
            0, //WS_EX_TOPMOST,
            wc.lpszClassName,
            L"",
            WS_POPUP,
            0, 0, screenW, screenH,
            NULL, NULL, wc.hInstance, this
        );

        if (m_BackgroundWnd)
        {
            ShowWindow(m_BackgroundWnd, SW_SHOW);
            UpdateWindow(m_BackgroundWnd);
        }

        return TRUE;
    }

    VOID SwitchToOriginalDesktop()
    {
        // Cleanup
        if (m_BackgroundWnd) {
            DestroyWindow(m_BackgroundWnd);
            m_BackgroundWnd = NULL;
        }

        if (m_hWinlogonDesktop) {
            SwitchDesktop(m_hOriginalDesktop);
            CloseDesktop(m_hWinlogonDesktop);
            m_hWinlogonDesktop = NULL;
        }
    }

protected:
    // Load PNG file into HBITMAP
    HBITMAP LoadBitmapFromImage(LPCWSTR filename, UINT* outWidth, UINT* outHeight)
    {
        IWICImagingFactory* pFactory = NULL;
        IWICBitmapDecoder* pDecoder = NULL;
        IWICBitmapFrameDecode* pFrame = NULL;
        IWICFormatConverter* pConverter = NULL;
        HBITMAP hBitmap = NULL;

        if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
            return NULL;

        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory))))
        {
            CoUninitialize();
            return NULL;
        }

        if (SUCCEEDED(pFactory->CreateDecoderFromFilename(filename, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder)))
        {
            if (SUCCEEDED(pDecoder->GetFrame(0, &pFrame)))
            {
                if (SUCCEEDED(pFactory->CreateFormatConverter(&pConverter)))
                {
                    if (SUCCEEDED(pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppBGRA,
                        WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom)))
                    {
                        HDC hdcScreen = GetDC(NULL);
                        UINT width = 0, height = 0;
                        pConverter->GetSize(&width, &height);

                        if (outWidth) *outWidth = width;
                        if (outHeight) *outHeight = height;

                        BITMAPINFO bmi = {};
                        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                        bmi.bmiHeader.biWidth = width;
                        bmi.bmiHeader.biHeight = -(LONG)height;
                        bmi.bmiHeader.biPlanes = 1;
                        bmi.bmiHeader.biBitCount = 32;
                        bmi.bmiHeader.biCompression = BI_RGB;

                        void* pvImageBits = NULL;
                        hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
                        if (hBitmap && pvImageBits)
                        {
                            UINT stride = width * 4;
                            pConverter->CopyPixels(NULL, stride, stride * height, (BYTE*)pvImageBits);
                        }
                        ReleaseDC(NULL, hdcScreen);
                    }
                    pConverter->Release();
                }
                pFrame->Release();
            }
            pDecoder->Release();
        }
        pFactory->Release();
        CoUninitialize();
        return hBitmap;
    }

    // Background Window Procedure
    static LRESULT CALLBACK BackgroundWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        CSecureDesktop* This = (CSecureDesktop*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

        switch (msg)
        {
        case WM_CREATE: 
        {
            LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
            This = (CSecureDesktop*)pcs->lpCreateParams;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)This);
            break;
        }

        case WM_PAINT:
        {
            if (!This)
                break;

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Fill the background with a solid color
            HBRUSH brush = CreateSolidBrush(RGB(30, 30, 30)); // Example: dark gray-blue
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, brush);
            DeleteObject(brush);

            if (This->m_hBitmap)
            {
                HDC hMemDC = CreateCompatibleDC(hdc);
                HBITMAP oldBitmap = (HBITMAP)SelectObject(hMemDC, This->m_hBitmap);

                int bmpX = (rc.right - (int)This->m_BitmapWidth) / 2;
                int bmpY = (rc.bottom - (int)This->m_BitmapHeight) / 2;

                BitBlt(
                    hdc,
                    bmpX, bmpY,
                    This->m_BitmapWidth, This->m_BitmapHeight,
                    hMemDC,
                    0, 0,
                    SRCCOPY
                );

                SelectObject(hMemDC, oldBitmap);
                DeleteDC(hMemDC);
            }

            EndPaint(hwnd, &ps);
            break;
        }
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

protected:
    HBITMAP m_hBitmap = NULL;
    UINT m_BitmapWidth = 0;
    UINT m_BitmapHeight = 0;
    HWND m_BackgroundWnd = NULL;
    HDESK m_hOriginalDesktop = NULL;
    HDESK m_hWinlogonDesktop = NULL;
};

// Thread to show background + MessageBox
DWORD WINAPI MessageBoxThreadProc(LPVOID lpParam)
{
    MessageBoxParams* params = (MessageBoxParams*)lpParam;

    CSecureDesktop SecureDesktop(params->backgroundImagePath);
    if (SecureDesktop.SwitchToSecureDesktop()) 
        params->result = MessageBoxW(SecureDesktop.GetBackgroundWnd(), params->text.c_str(), params->title.c_str(), params->uType);

    return 0;
}

// Public API
int ShowSecureMessageBox(const std::wstring& text, const std::wstring& title, UINT uType, const std::wstring& backgroundImagePath)
{
    MessageBoxParams params = {};
    params.text = text;
    params.title = title;
    params.uType = uType;
    params.result = -1;
    params.backgroundImagePath = backgroundImagePath;

    HANDLE hThread = CreateThread(NULL, 0, MessageBoxThreadProc, &params, 0, NULL);
    if (!hThread)
        return -1;

    // Wait for the MessageBox to finish
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    return params.result;
}

// Thread to show background + Dialog
DWORD WINAPI DialogThreadProc(LPVOID lpParam)
{
    DialogParams* params = (DialogParams*)lpParam;

    CSecureDesktop SecureDesktop(params->backgroundImagePath);
    if (SecureDesktop.SwitchToSecureDesktop())
        params->result = params->func(SecureDesktop.GetBackgroundWnd(), params->param);

    return 0;
}

int ShowSecureDialog(int(*func)(HWND hWnd, void* param), void* param, const std::wstring& backgroundImagePath)
{
    DialogParams params = {};
    params.func = func;
    params.param = param;
    params.result = -1;
    params.backgroundImagePath = backgroundImagePath;

    HANDLE hThread = CreateThread(NULL, 0,DialogThreadProc, &params, 0, NULL );
    if (!hThread)
        return -1;

    // Wait for the Dialog to finish
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    return params.result;
}

DWORD GetPromptOnSecureDesktop()
{
    HKEY hKey;
    DWORD value = 0;
    DWORD dataSize = sizeof(DWORD);
    LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_READ, &hKey );
    if (result == ERROR_SUCCESS)
    {
        result = RegQueryValueExW(hKey, L"PromptOnSecureDesktop", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &dataSize);
        RegCloseKey(hKey);
    }

    if (result != ERROR_SUCCESS)
    {
        // Handle error or return default value (e.g., 1 for secure desktop enabled)
        value = 1;
    }

    return value;
}