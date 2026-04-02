@echo off
setlocal

set "RELEASE_VERSION=26.00"
set "PACKAGE_VERSION=2600"
set "SEVENZIP_DIR=%~dp0\7-Zip"
set "SEVENZIP_WIN32=%SEVENZIP_DIR%\7z%PACKAGE_VERSION%.exe"
set "SEVENZIP_X64=%SEVENZIP_DIR%\7z%PACKAGE_VERSION%-x64.exe"

mkdir "%SEVENZIP_DIR%" 2>nul
mkdir "%SEVENZIP_DIR%\7-Zip-Win32" 2>nul
mkdir "%SEVENZIP_DIR%\7-Zip-x64" 2>nul

curl -fL --url https://github.com/ip7z/7zip/releases/download/%RELEASE_VERSION%/7z%PACKAGE_VERSION%.exe -o "%SEVENZIP_WIN32%" --ssl-no-revoke
if errorlevel 1 exit /b %errorlevel%

curl -fL --url https://github.com/ip7z/7zip/releases/download/%RELEASE_VERSION%/7z%PACKAGE_VERSION%-x64.exe -o "%SEVENZIP_X64%" --ssl-no-revoke
if errorlevel 1 exit /b %errorlevel%

"C:\Program Files\7-Zip\7z.exe" x -y -aoa -bd -o"%SEVENZIP_DIR%\7-Zip-Win32" "%SEVENZIP_WIN32%"
if errorlevel 1 exit /b %errorlevel%

"C:\Program Files\7-Zip\7z.exe" x -y -aoa -bd -o"%SEVENZIP_DIR%\7-Zip-x64" "%SEVENZIP_X64%"
if errorlevel 1 exit /b %errorlevel%

endlocal

