REM @echo off
REM echo Current dir: %cd /d%
REM echo folder: %~dp0
REM echo arch: %1

set ORIGINAL_FOLDER=%cd%
set WIN32_QT_VER=5.15.13
set x64_QT_VER=5.15.13
set ARM64_QT_VER=6.3.2

IF "%3" == "" (
  set parallelism=8
) ELSE (
  set parallelism=%3
)
set parallelism=-j %parallelism%


IF "%2" == "" (
  set qt_root_dir=%~dp0..\..\Qt
) ELSE (
  set qt_root_dir=%2
)

REM Print usage if no architecture is specified
IF "%1" == "" (
  call :print_usage
  exit /b 2
) 

IF %1 == Win32 (
  set qt_path=%qt_root_dir%\%WIN32_QT_VER%\msvc2019
  
  REM get private headers for QtCore
  Xcopy /E /I /Y /Q %qt_root_dir%\%WIN32_QT_VER%\msvc2019\include\QtCore\%WIN32_QT_VER%\QtCore %qt_root_dir%\%WIN32_QT_VER%\msvc2019\include\QtCore
  
  set build_arch=Win32
  set qt_params= 
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
) ELSE IF %1 == x64 (
  set qt_path=%qt_root_dir%\%x64_QT_VER%\msvc2019_64
  
  REM get private headers for QtCore
  Xcopy /E /I /Y /Q %qt_root_dir%\%x64_QT_VER%\msvc2019_64\include\QtCore\%x64_QT_VER%\QtCore %qt_root_dir%\%x64_QT_VER%\msvc2019_64\include\QtCore
  
  set build_arch=x64
  set qt_params= 
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) ELSE IF %1 == ARM64 (
  set qt_path=%qt_root_dir%\%ARM64_QT_VER%\msvc2019_64
  
  REM get private headers for QtCore
  Xcopy /E /I /Y /Q %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\include\QtCore\%ARM64_QT_VER%\QtCore %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\include\QtCore
  
  set build_arch=ARM64
  
  REM  set qt_params=-qtconf "%~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\target_qt.conf"
    
  REM type %~dp0..\..\Qt\6.3.1\msvc2019_arm64\bin\target_qt.conf
    
  REM
  REM The target_qt.conf as provided by the windows-2019 github action runner
  REM is non functional, hence we create our own working edition here.
  REM
  
  echo [DevicePaths] > %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo Prefix=%qt_root_dir% >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo [Paths] >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo Prefix=../ >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo HostPrefix=../../msvc2019_64 >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo HostData=../msvc2019_arm64 >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo Sysroot= >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo SysrootifyPrefix=false >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo TargetSpec=win32-arm64-msvc >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo HostSpec=win32-msvc >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo Documentation=../../Docs/Qt-%ARM64_QT_VER% >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  echo Examples=../../Examples/Qt-%ARM64_QT_VER% >> %qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf
  
  set qt_params=-qtconf "%qt_root_dir%\%ARM64_QT_VER%\msvc2019_arm64\bin\my_target_qt.conf"
  
  REM  set VSCMD_DEBUG=3
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsamd64_arm64.bat"
) ELSE (
  echo Unknown architecture!
  call :print_usage
  exit /b 2
)
@echo on



mkdir %~dp0\Build_UGlobalHotkey_%build_arch%
cd /d %~dp0\Build_UGlobalHotkey_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\UGlobalHotkey\uglobalhotkey.qc.pro %qt_params%
%qt_root_dir%\Tools\QtCreator\bin\jom.exe -f Makefile.Release %parallelism%
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\UGlobalHotkey.dll goto :error



mkdir %~dp0\Build_qtsingleapp_%build_arch%
cd /d %~dp0\Build_qtsingleapp_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\QtSingleApp\qtsingleapp\qtsingleapp\qtsingleapp.qc.pro %qt_params%
%qt_root_dir%\Tools\QtCreator\bin\jom.exe -f Makefile.Release %parallelism%
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\qtsingleapp.dll goto :error



mkdir %~dp0\Build_MiscHelpers_%build_arch%
cd /d %~dp0\Build_MiscHelpers_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\MiscHelpers\MiscHelpers.qc.pro %qt_params%
%qt_root_dir%\Tools\QtCreator\bin\jom.exe -f Makefile.Release %parallelism%
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\MiscHelpers.dll goto :error



mkdir %~dp0\Build_QSbieAPI_%build_arch%
cd /d %~dp0\Build_QSbieAPI_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\QSbieAPI\QSbieAPI.qc.pro %qt_params%
%qt_root_dir%\Tools\QtCreator\bin\jom.exe -f Makefile.Release %parallelism%
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\QSbieAPI.dll goto :error



mkdir %~dp0\Build_SandMan_%build_arch%
cd /d %~dp0\Build_SandMan_%build_arch%

%qt_path%\bin\qmake.exe %~dp0\SandMan\SandMan.qc.pro %qt_params%
%qt_root_dir%\Tools\QtCreator\bin\jom.exe -f Makefile.Release %parallelism%
IF %ERRORLEVEL% NEQ 0 goto :error
if NOT EXIST %~dp0\bin\%build_arch%\Release\SandMan.exe goto :error



cd /d %~dp0

rem dir .\bin
rem dir .\bin\%build_arch%
rem dir .\bin\%build_arch%\Release

exit /b 0

:print_usage
echo Usage: qmake_plus.cmd ^<architecture^> ^[Qt root directory^] ^[Threads to use (jom -j value)^]
echo Architecture can be Win32 / x64 / ARM64
echo Qt root directory defaults to ..\..\Qt
echo Threads to use defaults to 8
echo Example: qmake_plus.cmd x64 D:\Qt 12
exit /b 0

:error
echo Build failed! Make sure you installed the correct Qt version and components
cd /d %ORIGINAL_FOLDER%
exit /b 1

