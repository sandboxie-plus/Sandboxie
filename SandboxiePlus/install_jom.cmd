@echo off

::Check that needed programs exist in path
where /q curl
if %ERRORLEVEL% == 1 (
    echo curl.exe not found in path!
    exit /b 1
)

where /q 7z
if %ERRORLEVEL% == 1 (
    echo 7z.exe not found in path!
    exit /b 2
)

curl -LsSO --output-dir %~dp0..\..\ https://download.qt.io/official_releases/jom/jom.zip
7z x -aoa -o%~dp0..\..\Qt\Tools\QtCreator\bin\ %~dp0..\..\jom.zip

REM dir %~dp0..\..\
REM dir %~dp0..\..\Qt
REM dir %~dp0..\..\Qt\Tools

echo Success!
::/b terminates the script without closing the CMD
exit /b 0