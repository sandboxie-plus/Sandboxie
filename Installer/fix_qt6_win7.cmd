curl --ssl-no-revoke -L https://github.com/DavidXanatos/qtbase/releases/download/v6.3.1-w7/Qt6.3.1-w7.zip -o %~dp0..\..\Qt6.3.1-w7.zip
"C:\Program Files\7-Zip\7z.exe" x -o%~dp0..\..\Qt\6.3.1\msvc2019_64\ %~dp0..\..\Qt6.3.1-w7.zip -y
