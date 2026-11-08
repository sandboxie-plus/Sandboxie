@echo off
REM Builds docs.7z - the offline documentation the MCP server serves to an agent.
REM
REM Resolution order, so a local build stays cheap and CI stays current:
REM
REM   1. an existing docs.7z is used as-is          (local builds)
REM   2. otherwise a sandboxie-docs checkout is packed
REM   3. otherwise the docs are cloned, then packed
REM
REM Pass --force to skip step 1 and always rebuild from source. That is what CI
REM should do: a pack committed months ago would quietly ship stale settings
REM documentation, and the agent has no way to tell.
REM
REM English only, and that is deliberate rather than a limitation: the pack
REM exists for an agent, while people are sent to the online docs, so shipping
REM every translation would roughly triple it for nobody's benefit.
REM
REM Non-solid (-ms=off) is also deliberate. A solid archive must be decoded from
REM the start of the stream to reach any one member, and the access pattern here
REM is "read two pages out of six hundred". Non-solid costs about twice the size
REM and makes that read O(1).
REM
REM   build_docs_pack.cmd [--force] [--no-clone] [--repo <dir>] [--out <file>]

setlocal EnableDelayedExpansion

REM Captured before any shift. A shift moves %0 along with the rest, so after
REM parsing an option that takes a value, the script's own directory would
REM resolve to that value's directory instead. Every later path is built from
REM scriptDir for that reason.
set "scriptDir=%~dp0"

set "docsRepo="
set "outFile=%scriptDir%docs.7z"
set "force="
set "noClone="

:parseArgs
if "%~1" == "" goto argsDone
if /I "%~1" == "--force"    ( set "force=1"   & shift /1 & goto parseArgs )
if /I "%~1" == "--no-clone" ( set "noClone=1" & shift /1 & goto parseArgs )
if /I "%~1" == "--repo"     ( set "docsRepo=%~2" & shift /1 & shift /1 & goto parseArgs )
if /I "%~1" == "--out"      ( set "outFile=%~2"  & shift /1 & shift /1 & goto parseArgs )
echo ERROR: unknown argument "%~1"
exit /b 2
:argsDone

REM ---- 1. an existing pack wins unless we were told to rebuild --------------
if exist "%outFile%" (
	if not defined force (
		echo Using the existing pack "%outFile%".
		for %%S in ("%outFile%") do echo Size: %%~zS bytes
		echo Pass --force to rebuild it from source.
		exit /b 0
	)
	echo --force given, rebuilding "%outFile%" from source.
)

set "sevenZip=C:\Program Files\7-Zip\7z.exe"
set "settingsIni=%scriptDir%..\Sandboxie\install\SbieSettings.ini"
set "clonedInto="

REM ---- 2. find the documentation sources ------------------------------------
if not defined docsRepo set "docsRepo=%scriptDir%..\..\GitHub\sandboxie-docs"
if not exist "%docsRepo%\docs\Content" (
	REM A build-local clone rather than one beside the repository: a CI
	REM workspace is not guaranteed to be writable one level up.
	set "clonedInto=%scriptDir%_docs_src"
	if exist "!clonedInto!\docs\Content" (
		set "docsRepo=!clonedInto!"
	) else (
		if defined noClone (
			echo ERROR: no documentation at "%docsRepo%\docs\Content" and --no-clone was given.
			exit /b 1
		)
		where git >nul 2>&1
		if errorlevel 1 (
			echo ERROR: no documentation at "%docsRepo%\docs\Content" and git is not available.
			echo Pass --repo ^<dir^> pointing at a sandboxie-docs checkout.
			exit /b 1
		)
		echo Cloning sandboxie-docs into "!clonedInto!" ...
		rmdir /S /Q "!clonedInto!" >nul 2>&1
		git clone --depth 1 https://github.com/sandboxie-plus/sandboxie-docs.git "!clonedInto!"
		if errorlevel 1 (
			echo ERROR: the clone failed.
			exit /b 1
		)
		set "docsRepo=!clonedInto!"
	)
)

set "docsDir=%docsRepo%\docs"
if not exist "%docsDir%\Content" (
	echo ERROR: no documentation at "%docsDir%\Content"
	exit /b 1
)
if not exist "%sevenZip%" (
	echo ERROR: 7-Zip not found at "%sevenZip%"
	exit /b 1
)
if not exist "%settingsIni%" (
	echo ERROR: no settings reference at "%settingsIni%"
	exit /b 1
)

set "listFile=%TEMP%\sbie_docs_%RANDOM%.lst"
set "relFile=%TEMP%\sbie_docs_rel_%RANDOM%.lst"
set "indexFile=%docsDir%\index.json"

REM Content and PlusContent are the English pages; every other directory under
REM docs\ is a translation.
pushd "%docsDir%"
dir /b /s /a-d Content\*.md PlusContent\*.md > "%listFile%" 2>nul
popd
if not exist "%listFile%" (
	echo ERROR: could not enumerate the documentation
	exit /b 1
)

REM dir /s writes absolute paths; make them relative to docsDir so the archive
REM mirrors the layout of the site.
break > "%relFile%"
for /f "usebackq delims=" %%F in ("%listFile%") do (
	set "line=%%F"
	set "line=!line:%docsDir%\=!"
	echo !line!>> "%relFile%"
)
del "%listFile%" >nul 2>&1

REM python is on PATH on a developer machine and on a GitHub runner; the
REM launcher is the fallback for a machine where only "py" is registered.
set "pythonCmd=python"
python -c "" >nul 2>&1
if errorlevel 1 set "pythonCmd=py -3"

echo Building the agent index...
%pythonCmd% "%scriptDir%build_docs_index.py" "%docsDir%" "%settingsIni%" "%indexFile%" "@%relFile%"
if errorlevel 1 (
	echo ERROR: the index could not be built. Python 3 is required - the docs
	echo repository needs it for mkdocs anyway.
	del "%relFile%" >nul 2>&1
	exit /b 1
)
echo index.json>> "%relFile%"

echo Packing...
if exist "%outFile%" del "%outFile%" >nul 2>&1
pushd "%docsDir%"
"%sevenZip%" a -t7z -ms=off -mx=9 "%outFile%" "@%relFile%"
set "packResult=%ERRORLEVEL%"
popd

del "%relFile%" >nul 2>&1
del "%indexFile%" >nul 2>&1

if not "%packResult%" == "0" (
	echo ERROR: packing failed
	exit /b 1
)

echo.
echo Wrote "%outFile%"
for %%S in ("%outFile%") do echo Size: %%~zS bytes
endlocal
