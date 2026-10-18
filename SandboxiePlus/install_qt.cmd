call "%~dp0..\Installer\buildVariables.cmd" %*

REM echo %*
REM IF "%~7" == "" ( set "ghQtBuilds_hash_x64=673c288feeabd11ec66f9f454d49cde3945cbd3e3f71283b7a6c4df0893b19f2" ) ELSE ( set "ghQtBuilds_hash_x64=%~7" )
REM IF "%~6" == "" ( set "ghQtBuilds_hash_x86=502e9a36a52918af4e116cd74c16c6c260d029087aaeee3775ab0e5d3f6a2705" ) ELSE ( set "ghQtBuilds_hash_x86=%~6" )
REM IF "%~5" == "" ( set "ghQtBuilds_repo=qt-builds" ) ELSE ( set "ghQtBuilds_repo=%~5" )
REM IF "%~4" == "" ( set "ghQtBuilds_user=xanasoft" ) ELSE ( set "ghQtBuilds_user=%~4" )
REM IF "%~3" == "" ( set "qt6_version=6.3.1" ) ELSE ( set "qt6_version=%~3" )
REM IF "%~2" == "" ( set "qt_version=5.15.16" ) ELSE ( set "qt_version=%~2" )

if %1 == Win32 (
    if exist %~dp0..\..\Qt\%qt_version%\msvc2022\bin\qmake.exe goto done

    curl -LsSO --output-dir %~dp0..\..\ https://github.com/%ghQtBuilds_user%/%ghQtBuilds_repo%/releases/download/v%qt_version%-ssl-lgpl/qt-everywhere-%qt_version%-Windows_7-MSVC2022-x86.7z
    call :extract_archive "%~dp0..\..\qt-everywhere-%qt_version%-Windows_7-MSVC2022-x86.7z" "%~dp0..\..\Qt"
    certutil -hashfile %~dp0..\..\qt-everywhere-%qt_version%-Windows_7-MSVC2022-x86.7z SHA256 | "%windir%\System32\find.exe" /i "%ghQtBuilds_hash_x86%"
)
if %1 == x64 (
    if exist %~dp0..\..\Qt\%qt_version%\msvc2022_64\bin\qmake.exe goto done

    curl -LsSO --output-dir %~dp0..\..\ https://github.com/%ghQtBuilds_user%/%ghQtBuilds_repo%/releases/download/v%qt_version%-ssl-lgpl/qt-everywhere-%qt_version%-Windows_7-MSVC2022-x86_64.7z
    call :extract_archive "%~dp0..\..\qt-everywhere-%qt_version%-Windows_7-MSVC2022-x86_64.7z" "%~dp0..\..\Qt"
    certutil -hashfile %~dp0..\..\qt-everywhere-%qt_version%-Windows_7-MSVC2022-x86_64.7z SHA256 | "%windir%\System32\find.exe" /i "%ghQtBuilds_hash_x64%"
)

if %ERRORLEVEL% == 1 exit /b 1

:done

REM dir %~dp0..\..\
REM dir %~dp0..\..\Qt
REM dir %~dp0..\..\Qt\%qt_version%

exit /b 0

:extract_archive
if not exist "%~2\" mkdir "%~2"
if not exist "%~2\" exit /b 1

if exist "C:\Program Files\7-Zip\7z.exe" (
    "C:\Program Files\7-Zip\7z.exe" x -aoa -o"%~2" "%~1"
) else (
    "%windir%\System32\tar.exe" -xf "%~1" -C "%~2"
)
exit /b %ERRORLEVEL%
