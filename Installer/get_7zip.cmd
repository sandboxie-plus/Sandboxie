mkdir %~dp0\7-Zip
curl -L --url https://github.com/DavidXanatos/7z/releases/download/23.01/7z2301.zip -o %~dp0\7-Zip\7z2301.zip --ssl-no-revoke
"C:\Program Files\7-Zip\7z.exe" x -bd -o%~dp0\7-Zip\ %~dp0\7-Zip\7z2301.zip

