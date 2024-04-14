if %1 == Win32 (
    if exist %~dp0..\..\Qt\5.15.13\msvc2019\bin\qmake.exe goto done

    curl -LsSO --output-dir %~dp0..\..\ https://github.com/xanasoft/qt-builds/releases/download/v5.15.13-ssl-lgpl/qt-everywhere-5.15.13-Windows_10-MSVC2019-x86.7z
    "C:\Program Files\7-Zip\7z.exe" x -aoa -o%~dp0..\..\Qt\ %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86.7z
    certutil -hashfile %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86.7z SHA256 | find /i "79755f2bf95d0ca305096fc33582cd557345a79aa63f9821002fdddefdc0fd94"
)
if %1 == x64 (
    if exist %~dp0..\..\Qt\5.15.13\msvc2019_64\bin\qmake.exe goto done

    curl -LsSO --output-dir %~dp0..\..\ https://github.com/xanasoft/qt-builds/releases/download/v5.15.13-ssl-lgpl/qt-everywhere-5.15.13-Windows_10-MSVC2019-x86_64.7z
    "C:\Program Files\7-Zip\7z.exe" x -aoa -o%~dp0..\..\Qt\ %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86_64.7z
    certutil -hashfile %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86_64.7z SHA256 | find /i "f9029e02badd6a79d9bb092f9fb0772214dbcf8cd0122422514291d755860c37"
)

if %ERRORLEVEL% == 1 exit /b 1

:done

REM dir %~dp0..\..\
REM dir %~dp0..\..\Qt
REM dir %~dp0..\..\Qt\5.15.13
