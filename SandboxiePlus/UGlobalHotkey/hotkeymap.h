#include <QtCore>

#if defined(Q_OS_WIN)
inline size_t QtKeyToWin(size_t key) {
    // TODO: other mapping or full keys list

    if (key >= 0x01000030 && key <= 0x01000047) {
        return VK_F1 + (key - Qt::Key_F1);
    }

    return key;
}
#elif defined(Q_OS_LINUX)
#include "ukeysequence.h"
#include "xcb/xcb.h"
#include "xcb/xcb_keysyms.h"
#include "X11/keysym.h"

struct UKeyData {
    int key;
    int mods;
};

inline UKeyData QtKeyToLinux(const UKeySequence &keySeq)
{
    UKeyData data = {0, 0};

    auto key = keySeq.GetSimpleKeys();
    if (key.size() > 0)
        data.key = key[0];
    else
        throw UException("Invalid hotkey");

    // Key conversion
    // Qt's F keys need conversion
    if (data.key >= Qt::Key_F1 && data.key <= Qt::Key_F35) {
        const size_t DIFF = Qt::Key_F1 - XK_F1;
        data.key -= DIFF;
    } else if (data.key >= Qt::Key_Space && data.key <= Qt::Key_QuoteLeft) {
        // conversion is not necessary, if the value in the range Qt::Key_Space - Qt::Key_QuoteLeft
    } else {
        throw UException("Invalid hotkey: key conversion is not defined");
    }

    // Modifiers conversion
    auto mods = keySeq.GetModifiers();

    for (auto i : mods) {
        if (i == Qt::Key_Shift)
            data.mods |= XCB_MOD_MASK_SHIFT;
        else if (i == Qt::Key_Control)
            data.mods |= XCB_MOD_MASK_CONTROL;
        else if (i == Qt::Key_Alt)
            data.mods |= XCB_MOD_MASK_1;
        else if (i == Qt::Key_Meta)
            data.mods |= XCB_MOD_MASK_4; // !
    }

    return data;
}
#elif defined(Q_OS_MAC)

#include "ukeysequence.h"
#include <Carbon/Carbon.h>
#include <unordered_map>

struct UKeyData {
    uint32_t key;
    uint32_t mods;
};

static std::unordered_map<uint32_t, uint32_t> KEY_MAP = {
    {Qt::Key_A, kVK_ANSI_A},
    {Qt::Key_B, kVK_ANSI_B},
    {Qt::Key_C, kVK_ANSI_C},
    {Qt::Key_D, kVK_ANSI_D},
    {Qt::Key_E, kVK_ANSI_E},
    {Qt::Key_F, kVK_ANSI_F},
    {Qt::Key_G, kVK_ANSI_G},
    {Qt::Key_H, kVK_ANSI_H},
    {Qt::Key_I, kVK_ANSI_I},
    {Qt::Key_J, kVK_ANSI_J},
    {Qt::Key_K, kVK_ANSI_K},
    {Qt::Key_L, kVK_ANSI_L},
    {Qt::Key_M, kVK_ANSI_M},
    {Qt::Key_N, kVK_ANSI_N},
    {Qt::Key_O, kVK_ANSI_O},
    {Qt::Key_P, kVK_ANSI_P},
    {Qt::Key_Q, kVK_ANSI_Q},
    {Qt::Key_R, kVK_ANSI_R},
    {Qt::Key_S, kVK_ANSI_S},
    {Qt::Key_T, kVK_ANSI_T},
    {Qt::Key_U, kVK_ANSI_U},
    {Qt::Key_V, kVK_ANSI_V},
    {Qt::Key_W, kVK_ANSI_W},
    {Qt::Key_X, kVK_ANSI_X},
    {Qt::Key_Y, kVK_ANSI_Y},
    {Qt::Key_Z, kVK_ANSI_Z},
    {Qt::Key_0, kVK_ANSI_0},
    {Qt::Key_1, kVK_ANSI_1},
    {Qt::Key_2, kVK_ANSI_2},
    {Qt::Key_3, kVK_ANSI_3},
    {Qt::Key_4, kVK_ANSI_4},
    {Qt::Key_5, kVK_ANSI_5},
    {Qt::Key_6, kVK_ANSI_6},
    {Qt::Key_7, kVK_ANSI_7},
    {Qt::Key_8, kVK_ANSI_8},
    {Qt::Key_9, kVK_ANSI_9},
    {Qt::Key_F1, kVK_F1},
    {Qt::Key_F2, kVK_F2},
    {Qt::Key_F3, kVK_F3},
    {Qt::Key_F4, kVK_F4},
    {Qt::Key_F5, kVK_F5},
    {Qt::Key_F6, kVK_F6},
    {Qt::Key_F7, kVK_F7},
    {Qt::Key_F8, kVK_F8},
    {Qt::Key_F9, kVK_F9},
    {Qt::Key_F10, kVK_F10},
    {Qt::Key_F11, kVK_F11},
    {Qt::Key_F12, kVK_F12},
    {Qt::Key_F13, kVK_F13},
    {Qt::Key_F14, kVK_F14},
};

static std::unordered_map<uint32_t, uint32_t> MOD_MAP = {
    {Qt::Key_Shift, shiftKey},
    {Qt::Key_Alt, optionKey},
    {Qt::Key_Control, controlKey},
    {Qt::Key_Option, optionKey},
    {Qt::Key_Meta, cmdKey},
};

inline UKeyData QtKeyToMac(const UKeySequence &keySeq) {
    UKeyData data = {0, 0};
    auto key = keySeq.GetSimpleKeys();
    auto mods = keySeq.GetModifiers();
    if (key.size() == 1 && KEY_MAP.find(key[0]) != KEY_MAP.end())
        data.key = KEY_MAP[key[0]];
    else
        throw UException("Invalid hotkey");
    for (auto&& mod: mods) {
        if (MOD_MAP.find(mod) == MOD_MAP.end())
            throw UException("Invalid hotkey");
        data.mods += MOD_MAP[mod];
    }
    return data;
}

#endif
