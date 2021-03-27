/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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
// GUI Proxy Server
//---------------------------------------------------------------------------


#ifndef _MY_GUIWIRE_H
#define _MY_GUIWIRE_H


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


enum {

    GUI_SHUTDOWN = 1,
    GUI_INIT_PROCESS,
    GUI_GET_WINDOW_STATION,
    GUI_CREATE_CONSOLE,
    GUI_QUERY_WINDOW,
    GUI_IS_WINDOW,
    GUI_GET_WINDOW_LONG,
    GUI_GET_WINDOW_PROP,
    GUI_GET_WINDOW_HANDLE,
    GUI_GET_CLASS_NAME,
    GUI_GET_WINDOW_RECT,
    GUI_GET_WINDOW_INFO,
    GUI_GRANT_HANDLE,
    GUI_ENUM_WINDOWS,
    GUI_FIND_WINDOW,
    GUI_MAP_WINDOW_POINTS,
    GUI_SET_WINDOW_POS,
    GUI_CLOSE_CLIPBOARD,
    GUI_GET_CLIPBOARD_DATA,
    GUI_SEND_POST_MESSAGE,
    GUI_SEND_COPYDATA,
    GUI_CLIP_CURSOR,
    GUI_MONITOR_FROM_WINDOW,
    GUI_SET_FOREGROUND_WINDOW,
    GUI_SPLWOW64,
    GUI_CHANGE_DISPLAY_SETTINGS,
    GUI_SET_CURSOR_POS,
    GUI_GET_CLIPBOARD_METAFILE,
    GUI_REMOVE_HOST_WINDOW,
    GUI_GET_RAW_INPUT_DEVICE_INFO,
    GUI_WND_HOOK_NOTIFY,
    GUI_WND_HOOK_REGISTER,
    GUI_MAX_REQUEST_CODE
};


//---------------------------------------------------------------------------
// Initialize New Process
//---------------------------------------------------------------------------


struct tagGUI_INIT_PROCESS_REQ
{
    ULONG msgid;
    ULONG process_id;
    BOOLEAN add_to_job;
};

typedef struct tagGUI_INIT_PROCESS_REQ GUI_INIT_PROCESS_REQ;


//---------------------------------------------------------------------------
// Get Window Station
//---------------------------------------------------------------------------


struct tagGUI_GET_WINDOW_STATION_RPL
{
    ULONG status;
    ULONG peb64;
    ULONG64 hwinsta;
    ULONG64 hdesk;
    WCHAR name[96];
};

typedef struct tagGUI_GET_WINDOW_STATION_RPL GUI_GET_WINDOW_STATION_RPL;


//---------------------------------------------------------------------------
// Create Console (Windows 7)
//---------------------------------------------------------------------------


struct tagGUI_CREATE_CONSOLE_REQ
{
    ULONG msgid;
    ULONG pad_word;
    ULONG64 token;
    USHORT show_window;
};

struct tagGUI_CREATE_CONSOLE_RPL
{
    ULONG status;
    ULONG process_id;
};

typedef struct tagGUI_CREATE_CONSOLE_REQ GUI_CREATE_CONSOLE_REQ;
typedef struct tagGUI_CREATE_CONSOLE_RPL GUI_CREATE_CONSOLE_RPL;


//---------------------------------------------------------------------------
// Query Window
//---------------------------------------------------------------------------


struct tagGUI_QUERY_WINDOW_REQ
{
    ULONG msgid;
    ULONG error;
    ULONG hwnd;
    ULONG type;
};

struct tagGUI_QUERY_WINDOW_RPL
{
    ULONG status;
    ULONG error;
    ULONG64 result;
};

typedef struct tagGUI_QUERY_WINDOW_REQ GUI_QUERY_WINDOW_REQ;
typedef struct tagGUI_QUERY_WINDOW_RPL GUI_QUERY_WINDOW_RPL;


//---------------------------------------------------------------------------
// Is Window
//---------------------------------------------------------------------------


struct tagGUI_IS_WINDOW_REQ
{
    ULONG msgid;
    ULONG error;
    ULONG hwnd;
};

struct tagGUI_IS_WINDOW_RPL
{
    ULONG status;
    ULONG error;
    BOOLEAN window;
    BOOLEAN visible;
    BOOLEAN enabled;
    BOOLEAN unicode;
    BOOLEAN iconic;
    BOOLEAN zoomed;
};

typedef struct tagGUI_IS_WINDOW_REQ GUI_IS_WINDOW_REQ;
typedef struct tagGUI_IS_WINDOW_RPL GUI_IS_WINDOW_RPL;


//---------------------------------------------------------------------------
// Get Window/Class Long
//---------------------------------------------------------------------------


struct tagGUI_GET_WINDOW_LONG_REQ
{
    ULONG msgid;
    ULONG error;
    ULONG hwnd;
    LONG index;
    ULONG which;
};

struct tagGUI_GET_WINDOW_LONG_RPL
{
    ULONG status;
    ULONG error;
    ULONG64 result;
};

typedef struct tagGUI_GET_WINDOW_LONG_REQ GUI_GET_WINDOW_LONG_REQ;
typedef struct tagGUI_GET_WINDOW_LONG_RPL GUI_GET_WINDOW_LONG_RPL;


//---------------------------------------------------------------------------
// Get Window Property
//---------------------------------------------------------------------------


struct tagGUI_GET_WINDOW_PROP_REQ
{
    ULONG msgid;
    ULONG error;
    ULONG hwnd;
    ULONG prop_atom;
    BOOLEAN unicode;
    BOOLEAN prop_is_text;
    WCHAR prop_text[128];
};

struct tagGUI_GET_WINDOW_PROP_RPL
{
    ULONG status;
    ULONG error;
    ULONG64 result;
};

typedef struct tagGUI_GET_WINDOW_PROP_REQ GUI_GET_WINDOW_PROP_REQ;
typedef struct tagGUI_GET_WINDOW_PROP_RPL GUI_GET_WINDOW_PROP_RPL;


//---------------------------------------------------------------------------
// Get Window Handle
//---------------------------------------------------------------------------


struct tagGUI_GET_WINDOW_HANDLE_REQ
{
    ULONG msgid;
    ULONG error;
    ULONG which;
    ULONG hwnd;
};

struct tagGUI_GET_WINDOW_HANDLE_RPL
{
    ULONG status;
    ULONG error;
    ULONG hwnd;
};

typedef struct tagGUI_GET_WINDOW_HANDLE_REQ GUI_GET_WINDOW_HANDLE_REQ;
typedef struct tagGUI_GET_WINDOW_HANDLE_RPL GUI_GET_WINDOW_HANDLE_RPL;


//---------------------------------------------------------------------------
// Get Class Name
//---------------------------------------------------------------------------


struct tagGUI_GET_CLASS_NAME_REQ
{
    ULONG msgid;
    ULONG error;
    ULONG hwnd;
    ULONG maxlen;
    BOOLEAN unicode;
};

struct tagGUI_GET_CLASS_NAME_RPL
{
    ULONG status;
    ULONG result;
    ULONG error;
    WCHAR name[1];
};

typedef struct tagGUI_GET_CLASS_NAME_REQ GUI_GET_CLASS_NAME_REQ;
typedef struct tagGUI_GET_CLASS_NAME_RPL GUI_GET_CLASS_NAME_RPL;


//---------------------------------------------------------------------------
// Get Window Rect
//---------------------------------------------------------------------------


struct tagGUI_GET_WINDOW_RECT_REQ
{
    ULONG msgid;
    ULONG error;
    ULONG which;
    ULONG hwnd;
};

struct tagGUI_GET_WINDOW_RECT_RPL
{
    ULONG status;
    ULONG result;
    ULONG error;
    RECT rect;
};

typedef struct tagGUI_GET_WINDOW_RECT_REQ GUI_GET_WINDOW_RECT_REQ;
typedef struct tagGUI_GET_WINDOW_RECT_RPL GUI_GET_WINDOW_RECT_RPL;


//---------------------------------------------------------------------------
// Get Window Info
//---------------------------------------------------------------------------


struct tagGUI_GET_WINDOW_INFO_REQ
{
    ULONG msgid;
    ULONG error;
    ULONG hwnd;
};

struct tagGUI_GET_WINDOW_INFO_RPL
{
    ULONG status;
    ULONG result;
    ULONG error;
    WINDOWINFO info;
};

typedef struct tagGUI_GET_WINDOW_INFO_REQ GUI_GET_WINDOW_INFO_REQ;
typedef struct tagGUI_GET_WINDOW_INFO_RPL GUI_GET_WINDOW_INFO_RPL;


//---------------------------------------------------------------------------
// Grant Access to Handle
//---------------------------------------------------------------------------


struct tagGUI_GRANT_HANDLE_REQ
{
    ULONG msgid;
    ULONG handle_type;
    ULONG handle_value;
};

typedef struct tagGUI_GRANT_HANDLE_REQ GUI_GRANT_HANDLE_REQ;


//---------------------------------------------------------------------------
// Enum Windows
//---------------------------------------------------------------------------


struct tagGUI_ENUM_WINDOWS_REQ
{
    ULONG msgid;
    UCHAR which;
    ULONG arg;
};

struct tagGUI_ENUM_WINDOWS_RPL
{
    ULONG status;
    ULONG error;
    BOOL ok;
    ULONG num_hwnds;
    ULONG hwnds[1];
};

typedef struct tagGUI_ENUM_WINDOWS_REQ GUI_ENUM_WINDOWS_REQ;
typedef struct tagGUI_ENUM_WINDOWS_RPL GUI_ENUM_WINDOWS_RPL;


//---------------------------------------------------------------------------
// Find Window
//---------------------------------------------------------------------------


struct tagGUI_FIND_WINDOW_REQ
{
    ULONG msgid;
    ULONG which;
    ULONG hwnd_parent;
    ULONG hwnd_child;
    ULONG class_atom;
    BOOLEAN class_is_text;
    BOOLEAN title_is_text;
    WCHAR class_text[128];
    WCHAR title_text[128];
};

struct tagGUI_FIND_WINDOW_RPL
{
    ULONG status;
    ULONG error;
    ULONG hwnd;
};

typedef struct tagGUI_FIND_WINDOW_REQ GUI_FIND_WINDOW_REQ;
typedef struct tagGUI_FIND_WINDOW_RPL GUI_FIND_WINDOW_RPL;


//---------------------------------------------------------------------------
// Map Window Points
//---------------------------------------------------------------------------


struct tagGUI_MAP_WINDOW_POINTS_REQ
{
    ULONG msgid;
    ULONG hwnd_from;
    ULONG hwnd_to;
    ULONG num_pts;
    POINT pts[1];
};

struct tagGUI_MAP_WINDOW_POINTS_RPL
{
    ULONG status;
    ULONG error;
    ULONG retval;
    USHORT num_pts;
    POINT pts[1];
};

typedef struct tagGUI_MAP_WINDOW_POINTS_REQ GUI_MAP_WINDOW_POINTS_REQ;
typedef struct tagGUI_MAP_WINDOW_POINTS_RPL GUI_MAP_WINDOW_POINTS_RPL;


//---------------------------------------------------------------------------
// Set Window Pos
//---------------------------------------------------------------------------


struct tagGUI_SET_WINDOW_POS_REQ
{
    ULONG msgid;
    ULONG error;
    ULONG hwnd;
    ULONG hwnd_insert_after;
    ULONG x;
    ULONG y;
    ULONG w;
    ULONG h;
    ULONG flags;
};

struct tagGUI_SET_WINDOW_POS_RPL
{
    ULONG status;
    ULONG error;
    ULONG retval;
};

typedef struct tagGUI_SET_WINDOW_POS_REQ GUI_SET_WINDOW_POS_REQ;
typedef struct tagGUI_SET_WINDOW_POS_RPL GUI_SET_WINDOW_POS_RPL;


//---------------------------------------------------------------------------
// Get Clipboard Data
//---------------------------------------------------------------------------


struct tagGUI_GET_CLIPBOARD_DATA_REQ
{
    ULONG msgid;
    ULONG format;
};

struct tagGUI_GET_CLIPBOARD_DATA_RPL
{
    ULONG status;
    ULONG result;
    ULONG error;
    ULONG pad_word;
    ULONG64 section_length;
    ULONG64 section_handle;
};

typedef struct tagGUI_GET_CLIPBOARD_DATA_REQ GUI_GET_CLIPBOARD_DATA_REQ;
typedef struct tagGUI_GET_CLIPBOARD_DATA_RPL GUI_GET_CLIPBOARD_DATA_RPL;


//---------------------------------------------------------------------------
// Send or Post Message
//---------------------------------------------------------------------------


struct tagGUI_SEND_POST_MESSAGE_REQ
{
    ULONG msgid;
    ULONG which;
    ULONG hwnd;
    ULONG msg;
    ULONG64 wparam;
    ULONG64 lparam;
    ULONG flags;
    ULONG timeout;
};

struct tagGUI_SEND_POST_MESSAGE_RPL
{
    ULONG status;
    ULONG error;
    ULONG64 lresult1;
    ULONG64 lresult2;
};

typedef struct tagGUI_SEND_POST_MESSAGE_REQ GUI_SEND_POST_MESSAGE_REQ;
typedef struct tagGUI_SEND_POST_MESSAGE_RPL GUI_SEND_POST_MESSAGE_RPL;


//---------------------------------------------------------------------------
// Send WM_COPYDATA Message
//---------------------------------------------------------------------------


struct tagGUI_SEND_COPYDATA_REQ
{
    ULONG msgid;
    ULONG which;
    ULONG hwnd;
    ULONG pad_word;
    ULONG64 wparam;
    ULONG flags;
    ULONG timeout;
    ULONG64 cds_key;
    ULONG cds_len;
    WCHAR cds_buf[1];
};

struct tagGUI_SEND_COPYDATA_RPL
{
    ULONG status;
    ULONG error;
    ULONG64 lresult1;
    ULONG64 lresult2;
};

typedef struct tagGUI_SEND_COPYDATA_REQ GUI_SEND_COPYDATA_REQ;
typedef struct tagGUI_SEND_COPYDATA_RPL GUI_SEND_COPYDATA_RPL;


//---------------------------------------------------------------------------
// Clip Cursor
//---------------------------------------------------------------------------


struct tagGUI_CLIP_CURSOR_REQ
{
    ULONG msgid;
    BOOLEAN have_rect;
    RECT rect;
};

typedef struct tagGUI_CLIP_CURSOR_REQ GUI_CLIP_CURSOR_REQ;


//---------------------------------------------------------------------------
// Set Foreground Window
//---------------------------------------------------------------------------


struct tagGUI_SET_FOREGROUND_WINDOW_REQ
{
    ULONG msgid;
    ULONG hwnd;
};

typedef struct tagGUI_SET_FOREGROUND_WINDOW_REQ
                                            GUI_SET_FOREGROUND_WINDOW_REQ;


//---------------------------------------------------------------------------
// Monitor From Window
//---------------------------------------------------------------------------


struct tagGUI_MONITOR_FROM_WINDOW_REQ
{
    ULONG msgid;
    ULONG error;
    ULONG hwnd;
    ULONG flags;
};

struct tagGUI_MONITOR_FROM_WINDOW_RPL
{
    ULONG status;
    ULONG error;
    ULONG retval;
};

typedef struct tagGUI_MONITOR_FROM_WINDOW_REQ GUI_MONITOR_FROM_WINDOW_REQ;
typedef struct tagGUI_MONITOR_FROM_WINDOW_RPL GUI_MONITOR_FROM_WINDOW_RPL;


//---------------------------------------------------------------------------
// Set or Check SplWow64
//---------------------------------------------------------------------------


struct tagGUI_SPLWOW64_REQ
{
    ULONG msgid;
    BOOLEAN set;
    BOOLEAN win8;
};


typedef struct tagGUI_SPLWOW64_REQ GUI_SPLWOW64_REQ;


//---------------------------------------------------------------------------
// Change Display Settings
//---------------------------------------------------------------------------


struct tagGUI_CHANGE_DISPLAY_SETTINGS_REQ
{
    ULONG msgid;
    ULONG flags;
    BOOLEAN unicode;
    BOOLEAN have_devname;
    BOOLEAN have_devmode;
    WCHAR devname[64];
    DEVMODEW devmode;
};

struct tagGUI_CHANGE_DISPLAY_SETTINGS_RPL
{
    ULONG status;
    ULONG error;
    ULONG retval;
};


typedef struct tagGUI_CHANGE_DISPLAY_SETTINGS_REQ
                                            GUI_CHANGE_DISPLAY_SETTINGS_REQ;
typedef struct tagGUI_CHANGE_DISPLAY_SETTINGS_RPL
                                            GUI_CHANGE_DISPLAY_SETTINGS_RPL;


//---------------------------------------------------------------------------
// Set Cursor Pos
//---------------------------------------------------------------------------


struct tagGUI_SET_CURSOR_POS_REQ
{
    ULONG msgid;
    ULONG error;
    LONG x;
    LONG y;
};

struct tagGUI_SET_CURSOR_POS_RPL
{
    ULONG status;
    ULONG error;
    ULONG retval;
};

typedef struct tagGUI_SET_CURSOR_POS_REQ GUI_SET_CURSOR_POS_REQ;
typedef struct tagGUI_SET_CURSOR_POS_RPL GUI_SET_CURSOR_POS_RPL;


//---------------------------------------------------------------------------
// Remove host windows created by host and owned by guest
//---------------------------------------------------------------------------


struct tagGUI_REMOVE_HOST_WINDOW_REQ
{
    ULONG msgid;
    DWORD threadid;
};

struct tagGUI_REMOVE_HOST_WINDOW_RPL
{
    ULONG status;
    BOOL  bRemoved;
};

typedef struct tagGUI_REMOVE_HOST_WINDOW_REQ GUI_REMOVE_HOST_WINDOW_REQ;
typedef struct tagGUI_REMOVE_HOST_WINDOW_RPL GUI_REMOVE_HOST_WINDOW_RPL;


//---------------------------------------------------------------------------
// Get Raw Input Device Info
//---------------------------------------------------------------------------


struct tagGUI_GET_RAW_INPUT_DEVICE_INFO_REQ
{
    ULONG msgid;
    ULONG64 hDevice;
    UINT uiCommand;
    BOOLEAN unicode;
    BOOLEAN hasData;
    UINT cbSize;
};

struct tagGUI_GET_RAW_INPUT_DEVICE_INFO_RPL
{
    ULONG status;
    ULONG error;
    ULONG retval;
    BOOLEAN hasData;
    UINT cbSize;
};


typedef struct tagGUI_GET_RAW_INPUT_DEVICE_INFO_REQ GUI_GET_RAW_INPUT_DEVICE_INFO_REQ;
typedef struct tagGUI_GET_RAW_INPUT_DEVICE_INFO_RPL GUI_GET_RAW_INPUT_DEVICE_INFO_RPL;


//---------------------------------------------------------------------------
// Notify the service of a window creation and set global hooks
//---------------------------------------------------------------------------


struct tagGUI_WND_HOOK_NOTIFY_REQ
{
    ULONG msgid;
    DWORD threadid;
};

struct tagGUI_WND_HOOK_NOTIFY_RPL
{
    ULONG status;
};

typedef struct tagGUI_WND_HOOK_NOTIFY_REQ GUI_WND_HOOK_NOTIFY_REQ;
typedef struct tagGUI_WND_HOOK_NOTIFY_RPL GUI_WND_HOOK_NOTIFY_RPL;


//---------------------------------------------------------------------------
// Register and unregister global hooks
//---------------------------------------------------------------------------


struct tagGUI_WND_HOOK_REGISTER_REQ
{
    ULONG msgid;
    DWORD hthread;
    ULONG64 hproc;
    ULONG64 hhook;
};

struct tagGUI_WND_HOOK_REGISTER_RPL
{
    ULONG status;
};

typedef struct tagGUI_WND_HOOK_REGISTER_REQ GUI_WND_HOOK_REGISTER_REQ;
typedef struct tagGUI_WND_HOOK_REGISTER_RPL GUI_WND_HOOK_REGISTER_RPL;

//---------------------------------------------------------------------------


#endif /* _MY_GUIWIRE_H */
