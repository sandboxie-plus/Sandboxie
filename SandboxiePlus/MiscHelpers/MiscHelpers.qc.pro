
TEMPLATE = lib
TARGET = MiscHelpers
QT += core network widgets
#CONFIG += debug
# DEFINES += QT_LARGEFILE_SUPPORT QTSERVICE_LIB QT_WIDGETS_LIB QT_QTSINGLEAPPLICATION_EXPORT
DEFINES += MISCHELPERS_LIB
#LIBS += -L"."
PRECOMPILED_HEADER = stdafx.h
#DEPENDPATH += .
#MOC_DIR += ./GeneratedFiles/$(ConfigurationName)
#OBJECTS_DIR += debug
#UI_DIR += ./GeneratedFiles
#RCC_DIR += ./GeneratedFiles
win32:LIBS += -lUser32 -lShell32 -lOleAut32

!mac:unix:QMAKE_LFLAGS += -Wl,-rpath,'\$\$ORIGIN'
mac:QMAKE_CXXFLAGS += -std=c++11

!win32:QMAKE_LFLAGS +=-rdynamic

CONFIG(release, debug|release):{
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
}

MY_ARCH=$$(build_arch)
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
#MOC_DIR += ./GeneratedFiles
#OBJECTS_DIR += ./ObjectFiles
#UI_DIR += ./GeneratedFiles
#RCC_DIR += ./GeneratedFiles


include(MiscHelpers.pri)
