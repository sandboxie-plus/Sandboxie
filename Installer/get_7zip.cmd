@echo off
setlocal
setlocal enabledelayedexpansion

set "RELEASE_VERSION=26.00"
set "PACKAGE_VERSION=2600"
set "SEVENZIP_DIR=%~dp0\7-Zip"
set "SEVENZIP_WIN32=%SEVENZIP_DIR%\7z%PACKAGE_VERSION%.exe"
set "SEVENZIP_X64=%SEVENZIP_DIR%\7z%PACKAGE_VERSION%-x64.exe"
set "SEVENZIP_HASH_WIN32=d605eb609aa67796dca7cfe26d7e28792090bb8048302d6e05ede16e8e33145c"
set "SEVENZIP_HASH_X64=6fe18d5b3080e39678cabfa6cef12cfb25086377389b803a36a3c43236a8a82c"

mkdir "%SEVENZIP_DIR%" 2>nul
mkdir "%SEVENZIP_DIR%\7-Zip-Win32" 2>nul
mkdir "%SEVENZIP_DIR%\7-Zip-x64" 2>nul

curl -fL --url https://github.com/ip7z/7zip/releases/download/%RELEASE_VERSION%/7z%PACKAGE_VERSION%.exe -o "%SEVENZIP_WIN32%" --ssl-no-revoke
if errorlevel 1 exit /b %errorlevel%
call :verify_hash "%SEVENZIP_WIN32%" "%SEVENZIP_HASH_WIN32%"
if errorlevel 1 exit /b %errorlevel%

curl -fL --url https://github.com/ip7z/7zip/releases/download/%RELEASE_VERSION%/7z%PACKAGE_VERSION%-x64.exe -o "%SEVENZIP_X64%" --ssl-no-revoke
if errorlevel 1 exit /b %errorlevel%
call :verify_hash "%SEVENZIP_X64%" "%SEVENZIP_HASH_X64%"
if errorlevel 1 exit /b %errorlevel%

"C:\Program Files\7-Zip\7z.exe" x -y -aoa -bd -o"%SEVENZIP_DIR%\7-Zip-Win32" "%SEVENZIP_WIN32%"
if errorlevel 1 exit /b %errorlevel%

"C:\Program Files\7-Zip\7z.exe" x -y -aoa -bd -o"%SEVENZIP_DIR%\7-Zip-x64" "%SEVENZIP_X64%"
if errorlevel 1 exit /b %errorlevel%

endlocal & endlocal
exit /b 0

:verify_hash
set "FILE_PATH=%~1"
set "EXPECTED_HASH=%~2"
set "ACTUAL_HASH="

for /f %%i in ('powershell -NoProfile -Command "$hash = Get-FileHash -Algorithm SHA256 -LiteralPath \"%FILE_PATH%\"; $hash.Hash.ToLowerInvariant()"') do (
	set "ACTUAL_HASH=%%i"
)

if not defined ACTUAL_HASH (
	echo Failed to compute SHA256 for "%FILE_PATH%".
	exit /b 1
)

if /i not "!ACTUAL_HASH!"=="%EXPECTED_HASH%" (
	echo SHA256 mismatch for "%FILE_PATH%".
	echo Expected: %EXPECTED_HASH%
	echo Actual:   !ACTUAL_HASH!
	exit /b 1
)

exit /b 0

