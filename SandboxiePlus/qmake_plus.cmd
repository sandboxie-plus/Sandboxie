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

echo on

mkdir %my_dir%\Build_qtsingleapp
cd %my_dir%\Build_qtsingleapp

echo current folder: %cd%

%qt_path%\bin\qmake.exe %~dp0\QtSingleApp\qtsingleapp\qtsingleapp\qtsingleapp.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"

dir


dir %my_dir%..\..\
dir %my_dir%..\..\Qt\
dir %my_dir%..\..\Qt\5.15.1\
dir %my_dir%..\..\Qt\Tools\
dir %my_dir%..\..\Qt\Tools\QtCreator\bin\

%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release

mkdir %my_dir%\Build_MiscHelpers
cd %my_dir%\Build_MiscHelpers

%qt_path%\bin\qmake.exe %~dp0\MiscHelpers\MiscHelpers.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release


mkdir %my_dir%\Build_QSbieAPI
cd %my_dir%\Build_QSbieAPI

%qt_path%\bin\qmake.exe %~dp0\QSbieAPI\QSbieAPI.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release

mkdir %my_dir%\Build_SandMan
cd %my_dir%\Build_SandMan

%qt_path%\bin\qmake.exe %~dp0\SandMan\SandMan.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release

cd %my_dir%

dir

dir .\Release

dir C:\

dir D:\
