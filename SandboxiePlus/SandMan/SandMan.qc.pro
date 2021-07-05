
TEMPLATE = app
TARGET = SandMan
PRECOMPILED_HEADER = stdafx.h

QT += core gui network widgets winextras concurrent

CONFIG += lrelease

CONFIG(debug, debug|release):contains(QMAKE_HOST.arch, x86_64):LIBS += -L../Bin/x64/Debug
CONFIG(release, debug|release):contains(QMAKE_HOST.arch, x86_64):LIBS += -L../Bin/x64/Release
CONFIG(debug, debug|release):!contains(QMAKE_HOST.arch, x86_64):LIBS += -L../Bin/Win32/Debug
CONFIG(release, debug|release):!contains(QMAKE_HOST.arch, x86_64):LIBS += -L../Bin/Win32/Release

LIBS += -lNtdll -lAdvapi32 -lOle32 -lUser32 -lShell32 -lGdi32 -lQSbieAPI -lMiscHelpers -lqtsingleapp -lUGlobalHotkey

CONFIG(release, debug|release):{
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
}

CONFIG(debug, debug|release):contains(QMAKE_HOST.arch, x86_64):DESTDIR = ../Bin/x64/Debug
CONFIG(release, debug|release):contains(QMAKE_HOST.arch, x86_64):DESTDIR = ../Bin/x64/Release
CONFIG(debug, debug|release):!contains(QMAKE_HOST.arch, x86_64):DESTDIR = ../Bin/Win32/Debug
CONFIG(release, debug|release):!contains(QMAKE_HOST.arch, x86_64):DESTDIR = ../Bin/Win32/Release

INCLUDEPATH += .
DEPENDPATH += .
MOC_DIR += .
OBJECTS_DIR += debug
UI_DIR += .
RCC_DIR += .



include(SandMan.pri)

win32:RC_FILE = SandMan.rc


