curl --ssl-no-revoke -L https://download.qt.io/official_releases/jom/jom.zip -o %~dp0..\..\jom.zip
"C:\Program Files\7-Zip\7z.exe" x -o%~dp0..\..\Qt\Tools\QtCreator\bin\ %~dp0..\..\jom.zip

dir %~dp0..\..\
dir %~dp0..\..\Qt
dir %~dp0..\..\Qt\Tools
