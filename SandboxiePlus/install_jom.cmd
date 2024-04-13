@echo off

IF "%1" == "" (
  set qt_root_dir=%~dp0..\..\Qt
) ELSE (
  set qt_root_dir=%1
)

if exist %qt_root_dir%\Tools\QtCreator\bin\jom.exe (
    echo jom already installed, nothing to do
    goto done
)

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

curl -LsSO --output-dir %qt_root_dir%\ https://download.qt.io/official_releases/jom/jom.zip
7z x -aoa -o%qt_root_dir%\Tools\QtCreator\bin\ %qt_root_dir%\jom.zip

:done

REM dir %~dp0..\..\
REM dir %~dp0..\..\Qt
REM dir %~dp0..\..\Qt\Tools

echo Success!
::/b terminates the script without closing the CMD
exit /b 0
