if %1 == Win32 (
    if exist %~dp0..\..\Qt\5.15.13\msvc2019\bin\qmake.exe goto done

    curl -LsSO --output-dir %~dp0..\..\ https://github.com/xanasoft/qt-builds/releases/download/v5.15.13-lgpl/qt-everywhere-5.15.13-Windows_10-MSVC2019-x86.7z
    "C:\Program Files\7-Zip\7z.exe" x -aoa -o%~dp0..\..\Qt\ %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86.7z
    certutil -hashfile %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86.7z SHA256 | find /i "59ff4d30a756680f359cc937c78f878c3063510669fea306d48f4f745df952f6"
)
if %1 == x64 (
    if exist %~dp0..\..\Qt\5.15.13\msvc2019_64\bin\qmake.exe goto done

    curl -LsSO --output-dir %~dp0..\..\ https://github.com/xanasoft/qt-builds/releases/download/v5.15.13-lgpl/qt-everywhere-5.15.13-Windows_10-MSVC2019-x86_64.7z
    "C:\Program Files\7-Zip\7z.exe" x -aoa -o%~dp0..\..\Qt\ %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86_64.7z
    certutil -hashfile %~dp0..\..\qt-everywhere-5.15.13-Windows_10-MSVC2019-x86_64.7z SHA256 | find /i "3d7d7ac9d985c1cec1923fbe4feca630bcf5cc974f28e833d9c191e50906af2c"
)

if %ERRORLEVEL% == 1 exit /b 1

:done

REM dir %~dp0..\..\
REM dir %~dp0..\..\Qt
REM dir %~dp0..\..\Qt\5.15.13
