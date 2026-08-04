
TEMPLATE = app
TARGET = SandMan
PRECOMPILED_HEADER = stdafx.h

QT += core gui network widgets widgets-private concurrent core-private qml qml-private

CONFIG += lrelease

MY_ARCH=$$(build_arch)
equals(MY_ARCH, ARM64) {
#  message("Building ARM64")
  CONFIG(debug, debug|release):LIBS = -L../Bin/ARM64/Debug
  CONFIG(release, debug|release):LIBS = -L../Bin/ARM64/Release
} else:equals(MY_ARCH, x64) {
#  message("Building x64")
  CONFIG(debug, debug|release):LIBS += -L../Bin/x64/Debug
  CONFIG(release, debug|release):LIBS += -L../Bin/x64/Release
  QT += winextras
} else {
#  message("Building x86")
  CONFIG(debug, debug|release):LIBS = -L../Bin/Win32/Debug
  CONFIG(release, debug|release):LIBS = -L../Bin/Win32/Release
  QT += winextras
}


CONFIG(debug, debug|release):!contains(QMAKE_HOST.arch, x86_64):LIBS += -L../Bin/Win32/Debug
CONFIG(release, debug|release):!contains(QMAKE_HOST.arch, x86_64):LIBS += -L../Bin/Win32/Release

LIBS += -lNtdll -lAdvapi32 -lOle32 -lUser32 -lShell32 -lGdi32 -lQSbieAPI -lMiscHelpers -lqtsingleapp -lUGlobalHotkey -lbcrypt -lVersion -ltaskschd -lws2_32 -liphlpapi

CONFIG(release, debug|release):{
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
}


equals(MY_ARCH, ARM64) {
#  message("Building ARM64")
  CONFIG(debug, debug|release):DESTDIR = ../Bin/ARM64/Debug
  CONFIG(release, debug|release):DESTDIR = ../Bin/ARM64/Release
} else:equals(MY_ARCH, x64) {
#  message("Building x64")
  CONFIG(debug, debug|release):DESTDIR = ../Bin/x64/Debug
  CONFIG(release, debug|release):DESTDIR = ../Bin/x64/Release
} else {
#  message("Building x86")
  CONFIG(debug, debug|release):DESTDIR = ../Bin/Win32/Debug
  CONFIG(release, debug|release):DESTDIR = ../Bin/Win32/Release
}


INCLUDEPATH += .
DEPENDPATH += .
MOC_DIR += .
OBJECTS_DIR += debug
UI_DIR += .
RCC_DIR += .



include(SandMan.pri)

win32:RC_FILE = SandMan.rc


