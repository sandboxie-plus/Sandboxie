REM @ECHO OFF

IF %1 == x86 (
  set archPath=Win32
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
  set qtPath=%~dp0..\..\Qt\5.15.2\msvc2019
  set instPath=%~dp0\SbiePlus_x86
)
IF %1 == x64 (
  set archPath=x64
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
REM  set qtPath=%~dp0..\..\Qt\6.3.1\msvc2019_64
  set qtPath=%~dp0..\..\Qt\5.15.2\msvc2019_64
  set instPath=%~dp0\SbiePlus_x64
)
IF %1 == ARM64 (
  set archPath=ARM64
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsamd64_arm64.bat"
  set qtPath=%~dp0..\..\Qt\6.3.1\msvc2019_arm64
  set instPath=%~dp0\SbiePlus_a64
)
set redistPath=%VCToolsRedistDir%\%1\Microsoft.VC142.CRT
@echo on

set srcPath=%~dp0..\SandboxiePlus\Bin\%archPath%\Release
set sbiePath=%~dp0..\Sandboxie\Bin\%archPath%\SbieRelease

echo inst: %instPath%
echo arch: %archPath%
echo redistr: %redistPath%
echo source: %srcPath%
echo source: %sbiePath%

mkdir %instPath%

ECHO Copying VC Runtime files
copy "%redistPath%\*" %instPath%\


ECHO Copying Qt libraries

IF NOT %archPath% == ARM64 (
REM IF %archPath% == Win32 (
	copy %qtPath%\bin\Qt5Core.dll %instPath%\
	copy %qtPath%\bin\Qt5Gui.dll %instPath%\
	copy %qtPath%\bin\Qt5Network.dll %instPath%\
	copy %qtPath%\bin\Qt5Widgets.dll %instPath%\
	copy %qtPath%\bin\Qt5WinExtras.dll %instPath%\
) ELSE (
	copy %qtPath%\bin\Qt6Core.dll %instPath%\
	copy %qtPath%\bin\Qt6Gui.dll %instPath%\
	copy %qtPath%\bin\Qt6Network.dll %instPath%\
	copy %qtPath%\bin\Qt6Widgets.dll %instPath%\
)


mkdir %instPath%\platforms
copy %qtPath%\plugins\platforms\qdirect2d.dll %instPath%\platforms\
copy %qtPath%\plugins\platforms\qminimal.dll %instPath%\platforms\
copy %qtPath%\plugins\platforms\qoffscreen.dll %instPath%\platforms\
copy %qtPath%\plugins\platforms\qwindows.dll %instPath%\platforms\

mkdir %instPath%\styles
copy %qtPath%\plugins\styles\qwindowsvistastyle.dll %instPath%\styles\


ECHO Copying OpenSSL libraries
IF %archPath% == Win32 (
  copy /y %~dp0OpenSSL\Win_x86\bin\libssl-1_1.dll %instPath%\
  copy /y %~dp0OpenSSL\Win_x86\bin\libcrypto-1_1.dll %instPath%\
)
IF NOT %archPath% == Win32 (
  copy /y %~dp0OpenSSL\Win_%archPath%\bin\libssl-1_1-%archPath%.dll %instPath%\
  copy /y %~dp0OpenSSL\Win_%archPath%\bin\libcrypto-1_1-%archPath%.dll %instPath%\
)


ECHO Copying 7zip library
copy /y %~dp07-Zip\7-Zip-%archPath%\7z.dll %instPath%\


ECHO Copying SandMan project and libraries
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
copy /y %~dp0\qttranslations\qm\qt_*.qm %instPath%\translations\
copy /y %~dp0\qttranslations\qm\qtbase_*.qm %instPath%\translations\
copy /y %~dp0\qttranslations\qm\qtmultimedia_*.qm %instPath%\translations\

IF NOT %archPath% == ARM64 (
REM IF %archPath% == Win32 (
copy /y %qtPath%\translations\qtscript_*.qm %instPath%\translations\
copy /y %qtPath%\translations\qtxmlpatterns_*.qm %instPath%\translations\
)

"C:\Program Files\7-Zip\7z.exe" a %instPath%\translations.7z %instPath%\translations\*
rmdir /S /Q %instPath%\translations\

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

  copy /y %~dp0..\SandboxiePlus\x64\Release\SbieShellExt.dll %instPath%\
  copy /y %~dp0..\SandboxiePlus\x64\Release\SbieShellPkg.msix %instPath%\
)
IF %archPath% == ARM64 (
  mkdir %instPath%\32\
  copy /y %~dp0..\Sandboxie\Bin\Win32\SbieRelease\SbieSvc.exe %instPath%\32\
  copy /y %~dp0..\Sandboxie\Bin\Win32\SbieRelease\SbieDll.dll %instPath%\32\

  mkdir %instPath%\64\
  copy /y %~dp0..\Sandboxie\Bin\ARM64EC\SbieRelease\SbieDll.dll %instPath%\64\

  copy /y %~dp0..\SandboxiePlus\ARM64\Release\SbieShellExt.dll %instPath%\
  copy /y %~dp0..\SandboxiePlus\ARM64\Release\SbieShellPkg.msix %instPath%\
)


copy /y %~dp0..\Sandboxie\install\Templates.ini %instPath%\

copy /y %~dp0..\Sandboxie\install\Manifest0.txt %instPath%\
copy /y %~dp0..\Sandboxie\install\Manifest1.txt %instPath%\
copy /y %~dp0..\Sandboxie\install\Manifest2.txt %instPath%\

ECHO Copying Sandboxie Live Updater

copy /y %~dp0..\SandboxieLive\%archPath%\Release\UpdUtil.exe %instPath%\



