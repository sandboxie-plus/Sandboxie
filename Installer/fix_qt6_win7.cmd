echo %*
IF "%~3" == "" ( set "ghQt6Win7_repo=qtbase" ) ELSE ( set "ghQt6Win7_repo=%~3" )
IF "%~2" == "" ( set "ghQt6Win7_user=DavidXanatos" ) ELSE ( set "ghQt6Win7_user=%~2" )
IF "%~1" == "" ( set "qt6_version=6.3.1" ) ELSE ( set "qt6_version=%~1" )

curl --ssl-no-revoke -L https://github.com/%ghQt6Win7_user%/%ghQt6Win7_repo%/releases/download/v%qt6_version%-w7/Qt%qt6_version%-w7.zip -o %~dp0..\..\Qt%qt6_version%-w7.zip
"C:\Program Files\7-Zip\7z.exe" x -o%~dp0..\..\Qt\%qt6_version%\msvc2019_64\ %~dp0..\..\Qt%qt6_version%-w7.zip -y
