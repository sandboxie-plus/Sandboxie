/*
 * Copyright 2024 David Xanatos, xanasoft.com
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
// MiniDumpFilter - Reusable exception filter library
//---------------------------------------------------------------------------

#pragma once

#ifndef _MINIDUMP_FILTER_H_
#define _MINIDUMP_FILTER_H_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// Dump Types
//---------------------------------------------------------------------------

#define MDF_TYPE_TRIAGE        0       // Small triage dump (~100 KB, default)
#define MDF_TYPE_STANDARD      1       // With heap data (~20 MB)
#define MDF_TYPE_FULL          2       // Full memory dump (100+ MB)

//---------------------------------------------------------------------------
// Callback Types
//---------------------------------------------------------------------------

/**
 * Callback function type for sensitive data cleanup.
 *
 * This function is called BEFORE the crash dump is created, allowing the
 * application to clear sensitive data (passwords, tokens, keys) from memory.
 *
 * IMPORTANT SAFETY NOTES:
 * - This function runs in a separate thread with exception handling
 * - If it crashes or hangs, the dump will still be created
 * - Keep it simple and fast (5 second timeout)
 * - Do NOT allocate memory or call complex APIs
 * - Just zero out known sensitive memory locations
 *
 * Example:
 *   void MyCleanupCallback(void) {
 *       SecureZeroMemory(g_password, sizeof(g_password));
 *       SecureZeroMemory(g_authToken, sizeof(g_authToken));
 *   }
 */
typedef void (*MDF_CLEANUP_CALLBACK)(void);

//---------------------------------------------------------------------------
// API Functions
//---------------------------------------------------------------------------

/**
 * Initialize the MiniDump exception filter.
 *
 * @param pszDumperPath  Path to MiniDump.exe. If NULL, looks for MiniDump.exe
 *                       in the same directory as the calling executable.
 * @param pszAppName     Application name to display in the crash dialog.
 *                       If NULL, uses "Unknown".
 * @param dwDumpType     Default dump type (MDF_TYPE_*). User can change in dialog.
 * @param pfnCleanup     Optional callback to clear sensitive data before dump.
 *                       Can be NULL if no cleanup is needed. The callback runs
 *                       in a protected context - if it crashes or times out,
 *                       the dump is still created.
 *
 * @return TRUE on success, FALSE on failure.
 *
 * This function:
 *  - Installs an unhandled exception filter
 *  - When a crash occurs, spawns MiniDump.exe to capture the dump
 *  - Shows a dialog allowing the user to choose save location and dump type
 *
 * Example usage:
 *   MiniDumpFilter_Init(NULL, L"MyApp", MDF_TYPE_TRIAGE, MyCleanupCallback);
 */
BOOL MiniDumpFilter_Init(LPCWSTR pszDumperPath, LPCWSTR pszAppName, DWORD dwDumpType,
                         MDF_CLEANUP_CALLBACK pfnCleanup);

/**
 * Shutdown the MiniDump exception filter.
 *
 * Restores the previous exception filter. Call this before unloading
 * the library or when crash handling is no longer needed.
 */
void MiniDumpFilter_Shutdown(void);

#ifdef __cplusplus
}
#endif

#endif // _MINIDUMP_FILTER_H_
