@echo off
setlocal

rem Repository root
for %%I in ("%~dp0..") do set "REPO_ROOT=%%~fI"

rem Qt is installed next to the repository
for %%I in ("%~dp0..\..") do set "QT_ROOT=%%~fI"

set "ZIP=%REPO_ROOT%\jom_1_1_4.zip"
set "JOM_DIR=%QT_ROOT%\Qt\Tools\QtCreator\bin"
set "JOM_EXE=%JOM_DIR%\jom.exe"

if exist "%JOM_EXE%" goto :done

if not exist "%ZIP%" (
  curl -fL -o "%ZIP%" "https://download.qt.io/official_releases/jom/jom_1_1_4.zip" || exit /b 1
)

if not exist "%JOM_DIR%\" mkdir "%JOM_DIR%" || exit /b 1
"C:\Program Files\7-Zip\7z.exe" x -aoa -o"%JOM_DIR%\" "%ZIP%" || exit /b 1

if not exist "%JOM_EXE%" exit /b 1

:done
endlocal
exit /b 0
