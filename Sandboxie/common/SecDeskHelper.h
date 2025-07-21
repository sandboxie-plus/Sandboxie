#pragma once

#include <windows.h>
#include <string>

int ShowSecureMessageBox(const std::wstring& text, const std::wstring& title, UINT uType, const std::wstring& backgroundImagePath = std::wstring());

int ShowSecureDialog(int(*func)(HWND hWnd, void* param), void* param, const std::wstring& backgroundImagePath = std::wstring());

DWORD GetPromptOnSecureDesktop();