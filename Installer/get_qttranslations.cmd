mkdir %~dp0qttranslations
mkdir %~dp0qttranslations\ts
mkdir %~dp0qttranslations\qm
set fileName=qttranslations-everywhere-src-6.2.3.zip
set downloadUrl=https://download.qt.io/archive/qt/6.2/6.2.3/submodules/%filename%
curl -L %downloadUrl% -o %~dp0qttranslations\%filename%
"C:\Program Files\7-Zip\7z.exe" e -i!*\translations\qt_*.ts -i!*\translations\qtbase_*.ts -i!*\translations\qtmultimedia_*.ts %~dp0qttranslations\%filename% -o%~dp0qttranslations\ts\
for %%a in (%~dp0qttranslations\ts\*.ts) do (lrelease.exe -silent %%a -qm %~dp0qttranslations\qm\%%~na.qm)
