REM @ECHO OFF


set archPath=%1
set sysPath=%windir%\System32
set qtPath=%~dp0..\..\Qt\5.15.2\msvc2019_64
set instPath=%~dp0\SbiePlus64
IF %archPath% == x86 (
	set archPath=Win32
	set instPath=%~dp0\SbiePlus32
	set sysPath=%windir%\SysWOW64
  set qtPath=%~dp0..\..\Qt\5.15.2\msvc2019
)
set srcPath=%~dp0..\SandboxiePlus\Bin\%archPath%\Release
set sbiePath=%~dp0..\Sandboxie\Bin\%archPath%\SbieRelease

echo inst: %instPath%
echo arch: %archPath%
echo sys: %sysPath%
echo source: %srcPath%
echo source: %sbiePath%

mkdir %instPath%

ECHO Copying VC Runtime
copy %sysPath%\msvcp140.dll %instPath%\
copy %sysPath%\vcruntime140.dll %instPath%\
copy %sysPath%\msvcp140_1.dll %instPath%\
copy %sysPath%\vcruntime140_1.dll %instPath%\


ECHO Copying Qt Librarys

copy %qtPath%\bin\Qt5Core.dll %instPath%\
copy %qtPath%\bin\Qt5Gui.dll %instPath%\
copy %qtPath%\bin\Qt5Network.dll %instPath%\
copy %qtPath%\bin\Qt5Widgets.dll %instPath%\
copy %qtPath%\bin\Qt5WinExtras.dll %instPath%\

mkdir %instPath%\platforms
copy %qtPath%\plugins\platforms\qdirect2d.dll %instPath%\platforms\
copy %qtPath%\plugins\platforms\qminimal.dll %instPath%\platforms\
copy %qtPath%\plugins\platforms\qoffscreen.dll %instPath%\platforms\
copy %qtPath%\plugins\platforms\qwindows.dll %instPath%\platforms\

mkdir %instPath%\styles
copy %qtPath%\plugins\styles\qwindowsvistastyle.dll %instPath%\styles\

ECHO Copying OpenSSL libs
rem copy /y %~dp0OpenSSL\%archPath%\libeay32.dll %instPath%\
rem copy /y %~dp0OpenSSL\%archPath%\ssleay32.dll %instPath%\
IF %archPath% == Win32 (
  copy /y %~dp0OpenSSL\Win32\libssl-1_1.dll %instPath%\
  copy /y %~dp0OpenSSL\Win32\libcrypto-1_1.dll %instPath%\
)
IF %archPath% == x64 (
  copy /y %~dp0OpenSSL\x64\libssl-1_1-x64.dll %instPath%\
  copy /y %~dp0OpenSSL\x64\libcrypto-1_1-x64.dll %instPath%\
)
rem for openssl
copy %sysPath%\msvcr100.dll %instPath%\

ECHO Copying Project and Librarys
copy %srcPath%\MiscHelpers.dll %instPath%\
copy %srcPath%\MiscHelpers.pdb %instPath%\
copy %srcPath%\QSbieAPI.dll %instPath%\
copy %srcPath%\QSbieAPI.pdb %instPath%\
copy %srcPath%\QtSingleApp.dll %instPath%\
copy %srcPath%\UGlobalHotkey.dll %instPath%\
copy %srcPath%\SandMan.exe %instPath%\
copy %srcPath%\SandMan.pdb %instPath%\

ECHO Copying SandMan translations

mkdir %instPath%\translations\
rem copy /y %~dp0..\SandboxiePlus\SandMan\sandman_*.qm %instPath%\translations\
copy /y %~dp0..\SandboxiePlus\Build_SandMan_%archPath%\release\sandman_*.qm %instPath%\translations\
copy /y %qtPath%\translations\qt_*.qm %instPath%\translations\
copy /y %qtPath%\translations\qtbase_*.qm %instPath%\translations\
copy /y %qtPath%\translations\qtmultimedia_*.qm %instPath%\translations\
copy /y %qtPath%\translations\qtscript_*.qm %instPath%\translations\
copy /y %qtPath%\translations\qtxmlpatterns_*.qm %instPath%\translations\

ECHO Copying Sandboxie

copy /y %sbiePath%\SbieSvc.exe %instPath%\
copy /y %sbiePath%\SbieSvc.pdb %instPath%\
copy /y %sbiePath%\SbieDll.dll %instPath%\
copy /y %sbiePath%\SbieDll.pdb %instPath%\

copy /y %sbiePath%\SbieDrv.sys %instPath%\
copy /y %sbiePath%\SbieDrv.pdb %instPath%\

copy /y %sbiePath%\SbieCtrl.exe %instPath%\
copy /y %sbiePath%\SbieCtrl.pdb %instPath%\
copy /y %sbiePath%\Start.exe %instPath%\
copy /y %sbiePath%\Start.pdb %instPath%\
copy /y %sbiePath%\kmdutil.exe %instPath%\
copy /y %sbiePath%\kmdutil.pdb %instPath%\
copy /y %sbiePath%\SbieIni.exe %instPath%\
copy /y %sbiePath%\SbieIni.pdb %instPath%\
copy /y %sbiePath%\SbieMsg.dll %instPath%\
copy /y %sbiePath%\SboxHostDll.dll %instPath%\
copy /y %sbiePath%\SboxHostDll.pdb %instPath%\

copy /y %sbiePath%\SandboxieBITS.exe %instPath%\
copy /y %sbiePath%\SandboxieBITS.pdb %instPath%\
copy /y %sbiePath%\SandboxieCrypto.exe %instPath%\
copy /y %sbiePath%\SandboxieCrypto.pdb %instPath%\
copy /y %sbiePath%\SandboxieDcomLaunch.exe %instPath%\
copy /y %sbiePath%\SandboxieDcomLaunch.pdb %instPath%\
copy /y %sbiePath%\SandboxieRpcSs.exe %instPath%\
copy /y %sbiePath%\SandboxieRpcSs.pdb %instPath%\
copy /y %sbiePath%\SandboxieWUAU.exe %instPath%\
copy /y %sbiePath%\SandboxieWUAU.pdb %instPath%\

IF %archPath% == x64 (
  mkdir %instPath%\32\
  copy /y %~dp0..\Sandboxie\Bin\Win32\SbieRelease\SbieSvc.exe %instPath%\32\
  copy /y %~dp0..\Sandboxie\Bin\Win32\SbieRelease\SbieDll.dll %instPath%\32\
)

copy /y %~dp0..\Sandboxie\install\Templates.ini %instPath%\

copy /y %~dp0..\Sandboxie\install\Manifest0.txt %instPath%\
copy /y %~dp0..\Sandboxie\install\Manifest1.txt %instPath%\
copy /y %~dp0..\Sandboxie\install\Manifest2.txt %instPath%\



