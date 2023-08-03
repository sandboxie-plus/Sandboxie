#pragma once

#include <QWidget>
#include <QAbstractNativeEventFilter>
#include <QSet>

#if defined(Q_OS_LINUX)
#include "xcb/xcb.h"
#include "xcb/xcb_keysyms.h"
#elif defined(Q_OS_MAC)
#include <Carbon/Carbon.h>
#endif

#include "ukeysequence.h"
#include "uexception.h"
#include "uglobal.h"

#if defined(Q_OS_LINUX)
struct UHotkeyData {
    xcb_keycode_t keyCode;
    int mods;
    bool operator ==(const UHotkeyData& data) const {
        return data.keyCode == this->keyCode && data.mods == this->mods;
    }
};
#endif

class UGLOBALHOTKEY_EXPORT UGlobalHotkeys : public QWidget
        #if defined(Q_OS_LINUX)
        , public QAbstractNativeEventFilter
        #endif
{
    Q_OBJECT
public:
    explicit UGlobalHotkeys(QWidget *parent = 0);
    void registerHotkey(const QString& keySeq, size_t id = 1);
    void registerHotkey(const UKeySequence& keySeq, size_t id = 1);
    void unregisterHotkey(size_t id = 1);
    void unregisterAllHotkeys();
    ~UGlobalHotkeys();
protected:
    #if defined(Q_OS_WIN)
    bool winEvent (MSG * message);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result);
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#endif
    #elif defined(Q_OS_LINUX)
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
    bool linuxEvent(xcb_generic_event_t *message);
    void regLinuxHotkey(const UKeySequence& keySeq, size_t id);
    void unregLinuxHotkey(size_t id);
    #endif
public:
    #if defined (Q_OS_MAC)
    void onHotkeyPressed(size_t id);
    #endif
signals:
    void activated(size_t id);
private:
    #if defined(Q_OS_WIN)
    QSet<size_t> Registered;
    #elif defined(Q_OS_LINUX)
    QHash<size_t, UHotkeyData> Registered;
    xcb_connection_t* X11Connection;
    xcb_window_t X11Wid;
    xcb_key_symbols_t* X11KeySymbs;
    #elif defined(Q_OS_MAC)
    QHash<size_t, EventHotKeyRef> HotkeyRefs;
    #endif
};
