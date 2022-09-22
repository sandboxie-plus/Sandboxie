curl -LsSO --output-dir %~dp0..\..\ https://download.qt.io/official_releases/jom/jom.zip
"C:\Program Files\7-Zip\7z.exe" x -aoa -o%~dp0..\..\Qt\Tools\QtCreator\bin\ %~dp0..\..\jom.zip

REM dir %~dp0..\..\
REM dir %~dp0..\..\Qt
REM dir %~dp0..\..\Qt\Tools
