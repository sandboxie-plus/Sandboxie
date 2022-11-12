REM @echo off
REM echo Current dir: %cd%
REM echo folder: %~dp0
REM echo arch: %1


IF %1 == Win32 (
  set qt_path=%~dp0..\..\Qt\5.15.2\msvc2019

  REM get private headers for QtCore
  Xcopy /E /I /Y /Q %~dp0..\..\Qt\5.15.2\msvc2019\include\QtCore\5.15.2\QtCore %~dp0..\..\Qt\5.15.2\msvc2019\include\QtCore
  
  set build_arch=Win32
  set qt_params= 
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
)
IF %1 == x64 (
  set qt_path=%~dp0..\..\Qt\5.15.2\msvc2019_64
REM  set qt_path=%~dp0..\..\Qt\6.3.1\msvc2019_64
  
  REM get private headers for QtCore
  Xcopy /E /I /Y /Q %~dp0..\..\Qt\5.15.2\msvc2019_64\include\QtCore\5.15.2\QtCore %~dp0..\..\Qt\5.15.2\msvc2019_64\include\QtCore
REM  Xcopy /E /I /Y /Q %~dp0..\..\Qt\6.3.1\msvc2019_64\include\QtCore\6.3.1\QtCore %~dp0..\..\Qt\6.3.1\msvc2019_64\include\QtCore
  
  set build_arch=x64
  set qt_params= 
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)
IF %1 == ARM64 (
  set qt_path=%~dp0..\..\Qt\6.3.1\msvc2019_64
  
  REM get private headers for QtCore
  Xcopy /E /I /Y /Q %~dp0..\..\Qt\6.3.1\msvc2019_arm64\include\QtCore\6.3.1\QtCore %~dp0..\..\Qt\6.3.1\msvc2019_arm64\include\QtCore
  
  set build_arch=ARM64
  
REM  set qt_params=-qtconf "%~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\target_qt.conf"
  
REM type %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\target_qt.conf
  
  REM
  REM The target_qt.conf as provided by the windows-2019 github action runner
  REM is non functional, hence we create our own working edition here.
  REM
  
  echo [DevicePaths] > %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo Prefix=C:/Qt/Qt-6.3.1 >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo [Paths] >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo Prefix=../ >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo HostPrefix=../../msvc2019_64 >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo HostData=../msvc2019_arm64 >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo Sysroot= >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo SysrootifyPrefix=false >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo TargetSpec=win32-arm64-msvc >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo HostSpec=win32-msvc >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo Documentation=../../Docs/Qt-6.3.1 >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  echo Examples=../../Examples/Qt-6.3.1 >> %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf
  
  set qt_params=-qtconf "%~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\my_target_qt.conf"
  
REM  set VSCMD_DEBUG=3
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsamd64_arm64.bat"
)
@echo on



mkdir %~dp0\Build_UGlobalHotkey_%build_arch%
cd %~dp0\Build_UGlobalHotkey_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\UGlobalHotkey\uglobalhotkey.qc.pro %qt_params%
%~dp0..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release -j 8
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\UGlobalHotkey.dll goto :error



mkdir %~dp0\Build_qtsingleapp_%build_arch%
cd %~dp0\Build_qtsingleapp_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\QtSingleApp\qtsingleapp\qtsingleapp\qtsingleapp.qc.pro %qt_params%
%~dp0..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release -j 8
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\qtsingleapp.dll goto :error



mkdir %~dp0\Build_MiscHelpers_%build_arch%
cd %~dp0\Build_MiscHelpers_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\MiscHelpers\MiscHelpers.qc.pro %qt_params%
%~dp0..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release -j 8
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\MiscHelpers.dll goto :error



mkdir %~dp0\Build_QSbieAPI_%build_arch%
cd %~dp0\Build_QSbieAPI_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\QSbieAPI\QSbieAPI.qc.pro %qt_params%
%~dp0..\..\Qt\Tools\QtCreator\bin\jom.exe -f Makefile.Release -j 8
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\QSbieAPI.dll goto :error



mkdir %~dp0\Build_SandMan_%build_arch%
cd %~dp0\Build_SandMan_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\SandMan\SandMan.qc.pro %qt_params%
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
