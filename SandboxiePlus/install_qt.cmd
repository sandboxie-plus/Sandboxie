if %1 == Win32 (
    if exist %~dp0..\..\Qt\5.15.13\msvc2019\bin\qmake.exe goto done

    curl -LsSO --output-dir %~dp0..\..\ https://github.com/LumitoLuma/qt-opensource-lts-builds/releases/download/v5.15.13-lgpl/qt-everywhere-5.15.13-Windows_10-MSVC2019-x86.7z
    "C:\Program Files\7-Zip\7z.exe" x -aoa -o%~dp0..\..\Qt\ %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86.7z
    certutil -hashfile %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86.7z SHA256 | find /i "b90392ed27bfa20cc8a9d2e42908c1a721d952ab4e7c7d70d478246437a8eeac"
)
if %1 == x64 (
    if exist %~dp0..\..\Qt\5.15.13\msvc2019_64\bin\qmake.exe goto done

    curl -LsSO --output-dir %~dp0..\..\ https://github.com/LumitoLuma/qt-opensource-lts-builds/releases/download/v5.15.13-lgpl/qt-everywhere-5.15.13-Windows_10-MSVC2019-x86_64.7z
    "C:\Program Files\7-Zip\7z.exe" x -aoa -o%~dp0..\..\Qt\ %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86_64.7z
    certutil -hashfile %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86_64.7z SHA256 | find /i "e17ece39f1f0521085fa8d9d014cfa6bc85ccfbc07ae5688ed9dc08d9f1f6d28"
)

if %ERRORLEVEL% == 1 exit /b 1

:done

REM dir %~dp0..\..\
REM dir %~dp0..\..\Qt
REM dir %~dp0..\..\Qt\5.15.13
