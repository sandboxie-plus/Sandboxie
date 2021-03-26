curl --ssl-no-revoke -L https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/tools_qtcreator/qt.tools.qtcreator/4.14.0-0-202012170949jom.7z -o %~dp0..\..\jom.7z
"C:\Program Files\7-Zip\7z.exe" x -o%~dp0..\..\Qt\ %~dp0..\..\jom.7z

dir %~dp0..\..\
dir %~dp0..\..\Qt
dir %~dp0..\..\Qt\Tools
