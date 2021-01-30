rem @echo off
REM echo Current dir: %cd%
echo folder: %~dp0
echo arch: %1

set my_dir=%~dp0

IF %1 == Win32 (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"

	set qt_path=%my_dir%..\..\Qt\5.15.1\msvc2019
)
IF %1 == x64 (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

	set qt_path=%my_dir%..\..\Qt\5.15.1\msvc2019_64
)

mkdir %my_dir%\Build_qtsingleapp
cd %my_dir%\Build_qtsingleapp

%qt_path%\bin\qmake.exe ..\QtSingleApp\qtsingleapp\qtsingleapp\qtsingleapp.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe

mkdir %my_dir%\Build_MiscHelpers
cd %my_dir%\Build_MiscHelpers

%qt_path%\bin\qmake.exe ..\MiscHelpers\MiscHelpers.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe


mkdir %my_dir%\Build_QSbieAPI
cd %my_dir%\Build_QSbieAPI

%qt_path%\bin\qmake.exe ..\QSbieAPI\QSbieAPI.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe

mkdir %my_dir%\Build_SandMan
cd %my_dir%\Build_SandMan

%qt_path%\bin\qmake.exe ..\SandMan\SandMan.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe

dir %my_dir%

dir %my_dir%\Release