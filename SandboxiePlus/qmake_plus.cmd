REM @echo off
REM echo Current dir: %cd%
REM echo folder: %~dp0
REM echo arch: %1

set my_dir=%~dp0

IF %1 == Win32 (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
  set qt_path=%my_dir%..\..\Qt\5.15.1\msvc2019
  set build_arch=Win32
)
IF %1 == x64 (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
  set qt_path=%my_dir%..\..\Qt\5.15.1\msvc2019_64
  set build_arch=x64
)


mkdir %my_dir%\Build_qtsingleapp_%build_arch%
cd %my_dir%\Build_qtsingleapp_%build_arch%

%qt_path%\bin\qmake.exe %my_dir%\QtSingleApp\qtsingleapp\qtsingleapp\qtsingleapp.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release


mkdir %my_dir%\Build_MiscHelpers_%build_arch%
cd %my_dir%\Build_MiscHelpers_%build_arch%

%qt_path%\bin\qmake.exe %my_dir%\MiscHelpers\MiscHelpers.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release


mkdir %my_dir%\Build_QSbieAPI_%build_arch%
cd %my_dir%\Build_QSbieAPI_%build_arch%

%qt_path%\bin\qmake.exe %my_dir%\QSbieAPI\QSbieAPI.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release


mkdir %my_dir%\Build_SandMan_%build_arch%
cd %my_dir%\Build_SandMan_%build_arch%

%qt_path%\bin\qmake.exe %my_dir%\SandMan\SandMan.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%my_dir%..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release


cd %my_dir%
dir .\bin
dir .\bin\%build_arch%
dir .\bin\%build_arch%\Release
