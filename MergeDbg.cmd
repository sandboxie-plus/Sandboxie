@echo off

mkdir %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\Sandboxie\Bin\x64\SbieRelease\SbieDrv.sys %~dp0\SandboxiePlus\x64\Debug\

copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\SbieSvc.exe %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\SbieDll.dll %~dp0\SandboxiePlus\x64\Debug\

rem copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\SbieCtrl.exe %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\Start.exe %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\kmdutil.exe %~dp0\SandboxiePlus\x64\Debug\
rem copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\SbieIni.exe %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\Sandboxie\Bin\x64\SbieRelease\SbieMsg.dll %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\SboxHostDll.dll %~dp0\SandboxiePlus\x64\Debug\

copy /y %~dp0\Sandboxie\install\Templates.ini %~dp0\SandboxiePlus\x64\Debug\

copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\SandboxieBITS.exe %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\SandboxieCrypto.exe %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\SandboxieDcomLaunch.exe %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\SandboxieRpcSs.exe %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\Sandboxie\Bin\x64\SbieDebug\SandboxieWUAU.exe %~dp0\SandboxiePlus\x64\Debug\

mkdir %~dp0\SandboxiePlus\x64\Debug\32\
copy /y %~dp0\Sandboxie\Bin\Win32\SbieDebug\SbieSvc.exe %~dp0\SandboxiePlus\x64\Debug\32\
copy /y %~dp0\Sandboxie\Bin\Win32\SbieDebug\SbieDll.dll %~dp0\SandboxiePlus\x64\Debug\32\

copy /y %~dp0\SandboxieTools\x64\Debug\ImBox.exe %~dp0\SandboxiePlus\x64\Debug\
copy /y %~dp0\SandboxieTools\x64\Debug\UpdUtil.exe %~dp0\SandboxiePlus\x64\Debug\
