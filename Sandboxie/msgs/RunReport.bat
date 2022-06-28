REM copy /y c:\work\sbie\msgs\*.txt
for %%a in (Text-*-*.txt) do call :MySub %%a
exit /b
:MySub
if %1 == Text-English-1033.txt goto :MySubEnd
for /f "delims=- tokens=2" %%b in ("%1") do set MySubLang=%%b
parse Text-English-1033.txt %1 > report\Report-%MySubLang%.txt 2>&1
:MySubEnd
exit /b
