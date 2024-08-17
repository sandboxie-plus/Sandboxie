echo %*
IF "%~3" == "" ( set "ghSsl_repo=openssl-builds" ) ELSE ( set "ghSsl_repo=%~3" )
IF "%~2" == "" ( set "ghSsl_user=xanasoft" ) ELSE ( set "ghSsl_user=%~2" )
IF "%~1" == "" ( set "openssl_version=3.3.1" ) ELSE ( set "openssl_version=%~1" )

set "openssl_version_underscore=%openssl_version:.=_%"

mkdir %~dp0\OpenSSL

rem https://github.com/<repo>/openssl/releases/download/OpenSSL_1_1_1p/OpenSSL-1_1_1p.zip
rem https://github.com/<repo>/openssl/releases/download/openssl-3.3.0/openssl-3.3.0.zip
curl -L -f --url https://github.com/%ghSsl_user%/%ghSsl_repo%/releases/download/OpenSSL_%openssl_version_underscore%/OpenSSL-%openssl_version_underscore%.zip -o %~dp0\OpenSSL\OpenSSL-%openssl_version%.zip --ssl-no-revoke
IF %ERRORLEVEL% EQU 0 goto done
curl -L -f --url https://github.com/%ghSsl_user%/%ghSsl_repo%/releases/download/openssl-%openssl_version%/openssl-%openssl_version%.zip -o %~dp0\OpenSSL\OpenSSL-%openssl_version%.zip --ssl-no-revoke
IF %ERRORLEVEL% EQU 0 goto done

:urlfallback
setlocal enabledelayedexpansion
set "opensslFolders=openssl- openssl_ OpenSSL- OpenSSL_"
set "opensslFiles=openssl- openssl_ OpenSSL- OpenSSL_"
set "opensslVersions=%openssl_version% %openssl_version_underscore%"
for %%i in (%opensslFolders%) do (
 	echo 1=%%i
	for %%j in (%opensslFiles%) do (
		echo 2=%%j
		for %%k in (%opensslVersions%) do (
 			echo 3=%%k
			for %%l in (%opensslVersions%) do (
				echo 4=%%l
				timeout 2 >nul
				curl -L -f --url https://github.com/%ghSsl_user%/%ghSsl_repo%/releases/download/%%i%%k/%%j%%l.zip -o %~dp0\OpenSSL\OpenSSL-%openssl_version%.zip --ssl-no-revoke
				IF !ERRORLEVEL! EQU 0 goto done
			)
		)
	)
)
echo No valid URL found.
if %ERRORLEVEL% NEQ 0 exit /b 404
endlocal

:done

"C:\Program Files\7-Zip\7z.exe" x -bd -o%~dp0\OpenSSL\ %~dp0\OpenSSL\OpenSSL-%openssl_version%.zip
 
