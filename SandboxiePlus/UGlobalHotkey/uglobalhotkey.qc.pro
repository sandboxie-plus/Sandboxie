QT = core gui
unix {
    QT += gui-private
}
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UGlobalHotkey
TEMPLATE = lib
CONFIG += c++11

# Switch ABI to export (vs import, which is default)
DEFINES += UGLOBALHOTKEY_LIBRARY

include(uglobalhotkey-headers.pri)
include(uglobalhotkey-sources.pri)
include(uglobalhotkey-libs.pri)

CONFIG(debug, debug|release):contains(QMAKE_HOST.arch, x86_64):DESTDIR = ../Bin/x64/Debug
CONFIG(release, debug|release):contains(QMAKE_HOST.arch, x86_64):DESTDIR = ../Bin/x64/Release
CONFIG(debug, debug|release):!contains(QMAKE_HOST.arch, x86_64):DESTDIR = ../Bin/Win32/Debug
CONFIG(release, debug|release):!contains(QMAKE_HOST.arch, x86_64):DESTDIR = ../Bin/Win32/Release