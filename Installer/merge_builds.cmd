@ECHO OFF

call %~dp0get_openssl.cmd

REM Moved to main.yml
REM call %~dp0fix_qt5_languages.cmd

cmd.exe /c %~dp0copy_build.cmd x64

cmd.exe /c %~dp0copy_build.cmd ARM64

cmd.exe /c %~dp0copy_build.cmd x86


REM mkdir %rootPath%\SbiePlus64\LogAPI
REM copy /y %~dp0..\LogApiDll\output\amd64\VerboseDebug\logapi64.dll %rootPath%\SbiePlus64\LogAPI\logapi64.dll
REM copy /y %~dp0..\LogApiDll\output\i386\VerboseDebug\logapi32.dll %rootPath%\SbiePlus64\LogAPI\logapi32.dll

REM mkdir %rootPath%\SbiePlus32\LogAPI
REM copy /y %~dp0..\LogApiDll\output\i386\VerboseDebug\logapi32.dll %rootPath%\SbiePlus32\LogAPI\logapi32.dll