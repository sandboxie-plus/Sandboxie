echo %*
IF "%~7" == "" ( set "ghQtBuilds_hash_x64=30290d82a02bfaa24c1bf37bcb9c074aba18a673a7176628fccdf71197cee898" ) ELSE ( set "ghQtBuilds_hash_x64=%~7" )
IF "%~6" == "" ( set "ghQtBuilds_hash_x86=bf4124046cc50ccbbeb3f786c041e884fd4205cd6e616070a75c850105cbf1db" ) ELSE ( set "ghQtBuilds_hash_x86=%~6" )
IF "%~5" == "" ( set "ghQtBuilds_repo=qt-builds" ) ELSE ( set "ghQtBuilds_repo=%~5" )
IF "%~4" == "" ( set "ghQtBuilds_user=xanasoft" ) ELSE ( set "ghQtBuilds_user=%~4" )
IF "%~3" == "" ( set "qt6_version=6.3.1" ) ELSE ( set "qt6_version=%~3" )
IF "%~2" == "" ( set "qt_version=5.15.14" ) ELSE ( set "qt_version=%~2" )

if %1 == Win32 (
    if exist %~dp0..\..\Qt\%qt_version%\msvc2019\bin\qmake.exe goto done

    curl -LsSO --output-dir %~dp0..\..\ https://github.com/%ghQtBuilds_user%/%ghQtBuilds_repo%/releases/download/v%qt_version%-ssl-lgpl/qt-everywhere-%qt_version%-Windows_10-MSVC2019-x86.7z
    "C:\Program Files\7-Zip\7z.exe" x -aoa -o%~dp0..\..\Qt\ %~dp0..\..\qt-everywhere-%qt_version%-Windows_10-MSVC2019-x86.7z
    certutil -hashfile %~dp0..\..\qt-everywhere-%qt_version%-Windows_10-MSVC2019-x86.7z SHA256 | find /i "%ghQtBuilds_hash_x86%"
)
if %1 == x64 (
    if exist %~dp0..\..\Qt\%qt_version%\msvc2019_64\bin\qmake.exe goto done

    curl -LsSO --output-dir %~dp0..\..\ https://github.com/%ghQtBuilds_user%/%ghQtBuilds_repo%/releases/download/v%qt_version%-ssl-lgpl/qt-everywhere-%qt_version%-Windows_10-MSVC2019-x86_64.7z
    "C:\Program Files\7-Zip\7z.exe" x -aoa -o%~dp0..\..\Qt\ %~dp0..\..\qt-everywhere-%qt_version%-Windows_10-MSVC2019-x86_64.7z
    certutil -hashfile %~dp0..\..\qt-everywhere-%qt_version%-Windows_10-MSVC2019-x86_64.7z SHA256 | find /i "%ghQtBuilds_hash_x64%"
)

if %ERRORLEVEL% == 1 exit /b 1

:done

REM dir %~dp0..\..\
REM dir %~dp0..\..\Qt
REM dir %~dp0..\..\Qt\%qt_version%
