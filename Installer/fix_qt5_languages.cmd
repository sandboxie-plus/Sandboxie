if %1 == x64 if exist %~dp0..\..\Qt\5.15.13\msvc2019_64\bin\lrelease.exe set PATH=%PATH%;%~dp0..\..\Qt\5.15.13\msvc2019_64\bin\
if %1 == Win32 if exist %~dp0..\..\Qt\5.15.13\msvc2019\bin\lrelease.exe set PATH=%PATH%;%~dp0..\..\Qt\5.15.13\msvc2019\bin\
mkdir %~dp0qttranslations
mkdir %~dp0qttranslations\ts
mkdir %~dp0qttranslations\qm
set fileName=qttranslations-everywhere-src-6.3.1.zip
set downloadUrl=https://download.qt.io/archive/qt/6.3/6.3.1/submodules/%filename%
curl -LsSO --output-dir %~dp0qttranslations\ %downloadUrl%
"C:\Program Files\7-Zip\7z.exe" e -i!*\translations\qt_*.ts -i!*\translations\qtbase_*.ts -i!*\translations\qtmultimedia_*.ts %~dp0qttranslations\%filename% -bd -o%~dp0qttranslations\ts\
for %%a in (%~dp0qttranslations\ts\*.ts) do (lrelease.exe -silent %%a -qm %~dp0qttranslations\qm\%%~na.qm)
