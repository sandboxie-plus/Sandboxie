mkdir %~dp0\7-Zip
curl -L --url https://github.com/DavidXanatos/7z/releases/download/22.00/7z2201.zip -o %~dp0\7-Zip\7z2201.zip --ssl-no-revoke
"C:\Program Files\7-Zip\7z.exe" x -bd -o%~dp0\7-Zip\ %~dp0\7-Zip\7z2201.zip

