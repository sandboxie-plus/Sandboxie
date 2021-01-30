REM @echo off
REM echo Current dir: %cd%
REM echo folder: %~dp0
REM echo arch: %1

set my_dir=%~dp0

IF %1 == Win32 (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"

	set qt_path=%my_dir%..\..\Qt\5.15.1\msvc2019
)
IF %1 == x64 (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

	set qt_path=%my_dir%..\..\Qt\5.15.1\msvc2019_64
)

curl --ssl-no-revoke -L https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/tools_qtcreator/qt.tools.qtcreator/4.14.0-0-202012170949jom.7z -o %my_dir%../../jom.7z
"C:\Program Files\7-Zip\7z.exe" x -o%my_dir%..\..\Qt\ %my_dir%../../jom.7z

mkdir %my_dir%\Build_qtsingleapp
cd %my_dir%\Build_qtsingleapp

%qt_path%\bin\qmake.exe %my_dir%\QtSingleApp\qtsingleapp\qtsingleapp\qtsingleapp.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release


mkdir %my_dir%\Build_MiscHelpers
cd %my_dir%\Build_MiscHelpers

%qt_path%\bin\qmake.exe %my_dir%\MiscHelpers\MiscHelpers.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release


mkdir %my_dir%\Build_QSbieAPI
cd %my_dir%\Build_QSbieAPI

%qt_path%\bin\qmake.exe %my_dir%\QSbieAPI\QSbieAPI.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release


mkdir %my_dir%\Build_SandMan
cd %my_dir%\Build_SandMan

%qt_path%\bin\qmake.exe %my_dir%\SandMan\SandMan.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release


cd %my_dir%
dir .\Release
