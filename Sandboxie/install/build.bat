@echo off

REM
REM Sign executables
REM

setlocal
set TIMESTAMPING_OPTION=
if "%WITH_TMS%"=="" goto :SkipTMS1
set TIMESTAMPING_OPTION=/t http://timestamp.globalsign.com/scripts/timstamp.dll
:SkipTMS1

set SIGNCMD=c:\work\tools\SelfSign\signtool.exe sign /v /f "%SRC_ROOT%\common\certs\Sandboxie.pfx" /p EqyUpkXrR6GW

if /I "%WITH_SIGN%"=="n" set SIGNCMD=cmd /c echo --- 

if /I "%1"=="rel" goto :Release
if /I "%1"=="release" goto :Release
if /I "%1"=="sign" goto :JustSign

if "%_BUILDARCH%"=="AMD64" goto :Sign64
set OBJDIR=obj\i386
goto :SignNow

:Sign64
set OBJDIR=obj\amd64
call :SignFile "core\dll\obj\i386\%SBIEDLL%.dll" "" "+DYNAMIC_BASE"

:SignNow
c:\work\tools\EditPE\EditPE.exe +FORCE_INTEGRITY "%SRC_ROOT%\core\drv\%OBJDIR%\SbieDrv.sys"
set DRV_FLAG=/ph /ac "%SRC_ROOT%\common\certs\MSCV-GlobalSign.cer"
call :SignFile "core\drv\%OBJDIR%\SbieDrv.sys" "%DRV_FLAG%"

call :SignFile "msgs\%OBJDIR%\SbieMsg.dll"
call :SignFile "core\dll\%OBJDIR%\%SBIEDLL%.dll" "" "+DYNAMIC_BASE"
call :SignFile "core\svc\%OBJDIR%\SbieSvc.exe" "" "+DYNAMIC_BASE"
call :SignFile "apps\start\%OBJDIR%\Start.exe" "" "+DYNAMIC_BASE"
call :SignFile "apps\control\%OBJDIR%\SbieCtrl.exe" "" "+DYNAMIC_BASE"
call :SignFile "apps\license\%OBJDIR%\License.exe" "" "+DYNAMIC_BASE"
call :SignFile "apps\ini\%OBJDIR%\SbieIni.exe" "" "+DYNAMIC_BASE"
call :SignFile "apps\com\RpcSs\%OBJDIR%\SandboxieRpcSs.exe" "" "+DYNAMIC_BASE"
call :SignFile "apps\com\DcomLaunch\%OBJDIR%\SandboxieDcomLaunch.exe" "" "+DYNAMIC_BASE"
call :SignFile "apps\com\Crypto\%OBJDIR%\SandboxieCrypto.exe" "" "+DYNAMIC_BASE"
call :SignFile "apps\com\BITS\%OBJDIR%\SandboxieBITS.exe" "" "+DYNAMIC_BASE"
call :SignFile "apps\com\WUAU\%OBJDIR%\SandboxieWUAU.exe" "" "+DYNAMIC_BASE"
call :SignFile "install\kmdutil\%OBJDIR%\KmdUtil.exe" "" "+DYNAMIC_BASE"

goto :PostSign

#
# Sign a binary x only if the associated file x.sign does not
# exist or has a date earlier than the binary
#

:SignFile
call :GetTimeStamp %SRC_ROOT%\%~1
set BinTimeStamp=%TimeStamp%
if not exist %SRC_ROOT%\%~1.sign goto :SignFile1
call :GetTimeStamp %SRC_ROOT%\%~1.sign
if "%BinTimeStamp%" gtr "%TimeStamp%" goto :SignFile1
echo Signing not necessary for %SRC_ROOT%\%~1
goto :eof

:SignFile1
if "%~3" == "" goto :SignFile2
echo Executing: EditPE.exe %~3 %SRC_ROOT%\%~1
c:\work\tools\EditPE\EditPE.exe %~3 %SRC_ROOT%\%~1

:SignFile2
echo Executing: %SIGNCMD% %~2 %SRC_ROOT%\%~1
%SIGNCMD% %~2 %TIMESTAMPING_OPTION% %SRC_ROOT%\%~1
if errorlevel 1 goto :SignFile3
echo %SRC_ROOT%\%~1 signed at %BinTimeStamp% > %SRC_ROOT%\%~1.sign
goto :eof

:SignFile3
echo ***********************************
echo *
echo * Problem in signing %SRC_ROOT%\%~1
echo *
echo ***********************************
pause
goto :eof

:GetTimeStamp
set TimeStamp=%~t1
set TimeStamp=%TimeStamp:~6,4%%TimeStamp:~0,2%%TimeStamp:~3,2%%TimeStamp:~11,2%%TimeStamp:~14,2%
goto :eof



:JustSign
%SIGNCMD% %TIMESTAMPING_OPTION% %2
goto :eof


:Release

REM
REM Build Release Installer
REM

if "%_BUILDARCH%"=="AMD64" goto :Release64
cd release
build /c
cd ..
move release\obj\i386\SandboxieInstall.exe .
%SIGNCMD% %TIMESTAMPING_OPTION% SandboxieInstall.exe
goto :eof

:Release64
echo Use 32-bit build environment to build release installer
goto :eof

:PostSign

REM
REM Build Installer
REM 

c:\work\tools\nsis\MakeNSIS Sandboxie.nsi
if NOT "%_BUILDARCH%"=="AMD64" %SIGNCMD% %TIMESTAMPING_OPTION% SandboxieInstall32.exe
if     "%_BUILDARCH%"=="AMD64" %SIGNCMD% %TIMESTAMPING_OPTION% SandboxieInstall64.exe

if NOT "%WITH_TMS%"=="" goto :SkipTMS2
echo.
echo ***********************************
echo *
echo * Remember to set WITH_TMS before building the official version
echo *
echo ***********************************
echo.
:SkipTMS2
