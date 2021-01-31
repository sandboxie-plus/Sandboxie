#ifndef FINDTOOL_INCLUDED
#define FINDTOOL_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef UINT (CALLBACK * WNDFINDPROC) (HWND hwndTool, UINT uCode, HWND hwnd);

//
//  uCode can be one of these values:
//
#define WFN_BEGIN       0   // tool is about to become active. hwnd(0)
#define WFN_SELCHANGED  1   // sent when tool moves from window-window.
#define WFN_END         2   // sent when final window has been selected.
#define WFN_CANCELLED   3   // Tool canceled. hwnd is not valid (0)

#define WFN_CTRL_DOWN   4   // <Control> key was pressed
#define WFN_CTRL_UP     5   // <Control> key was released
#define WFN_SHIFT_DOWN  6   // <Shift> key was pressed
#define WFN_SHIFT_UP    7   // <Shift> key was released

#define WFN_CAPTURE     8   // Capture key pressed

BOOL MakeFinderTool    (HWND hwnd, WNDFINDPROC wfp);
void InvertWindow      (HWND hwnd, BOOL fShowHidden);
void FlashWindowBorder (HWND hwnd, BOOL fShowHidden);

#ifdef __cplusplus
}
#endif

#endif