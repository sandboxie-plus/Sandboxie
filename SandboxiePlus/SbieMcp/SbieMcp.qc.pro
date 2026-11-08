
TEMPLATE = app
TARGET = SbieMcp
QT = core network
CONFIG += console
CONFIG -= app_bundle

MY_ARCH=$$(build_arch)
equals(MY_ARCH, ARM64) {
  CONFIG(debug, debug|release):DESTDIR = ../Bin/ARM64/Debug
  CONFIG(release, debug|release):DESTDIR = ../Bin/ARM64/Release
} else:equals(MY_ARCH, x64) {
  CONFIG(debug, debug|release):DESTDIR = ../Bin/x64/Debug
  CONFIG(release, debug|release):DESTDIR = ../Bin/x64/Release
} else {
  CONFIG(debug, debug|release):DESTDIR = ../Bin/Win32/Debug
  CONFIG(release, debug|release):DESTDIR = ../Bin/Win32/Release
}

CONFIG(release, debug|release):{
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
}

INCLUDEPATH += .
DEPENDPATH += .
MOC_DIR += .
OBJECTS_DIR += debug
UI_DIR += .
RCC_DIR += .

SOURCES += ./main.cpp
