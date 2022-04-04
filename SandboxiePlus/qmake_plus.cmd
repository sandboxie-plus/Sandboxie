REM @echo off
REM echo Current dir: %cd%
REM echo folder: %~dp0
REM echo arch: %1


IF %1 == Win32 (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
  set qt_path=%~dp0..\..\Qt\5.15.2\msvc2019
  set build_arch=Win32
)
IF %1 == x64 (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
  set qt_path=%~dp0..\..\Qt\5.15.2\msvc2019_64
  set build_arch=x64
)



mkdir %~dp0\Build_UGlobalHotkey_%build_arch%
cd %~dp0\Build_UGlobalHotkey_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\UGlobalHotkey\uglobalhotkey.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%~dp0..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release -j 8
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\UGlobalHotkey.dll goto :error



mkdir %~dp0\Build_qtsingleapp_%build_arch%
cd %~dp0\Build_qtsingleapp_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\QtSingleApp\qtsingleapp\qtsingleapp\qtsingleapp.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%~dp0..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release -j 8
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\qtsingleapp.dll goto :error



mkdir %~dp0\Build_MiscHelpers_%build_arch%
cd %~dp0\Build_MiscHelpers_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\MiscHelpers\MiscHelpers.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%~dp0..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release -j 8
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\MiscHelpers.dll goto :error



mkdir %~dp0\Build_QSbieAPI_%build_arch%
cd %~dp0\Build_QSbieAPI_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\QSbieAPI\QSbieAPI.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%~dp0..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release -j 8
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\QSbieAPI.dll goto :error



mkdir %~dp0\Build_SandMan_%build_arch%
cd %~dp0\Build_SandMan_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\SandMan\SandMan.qc.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
%~dp0..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release -j 8
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\SandMan.exe goto :error



cd %~dp0

rem dir .\bin
rem dir .\bin\%build_arch%
rem dir .\bin\%build_arch%\Release

goto :eof

:error
echo Build failed
exit 1
