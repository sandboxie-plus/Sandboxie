@echo off
setlocal

rem CLI-only fork build entry point.
rem This repository keeps the full Sandboxie backend and CLI launcher, while omitting the Qt SandMan GUI and other non-essential Plus UI layers.
rem This is intentionally a lightweight console-first build.

echo ============================================================
echo Sandboxie CLI-only fork
echo ============================================================
echo.
echo This fork intentionally omits the SandMan Qt desktop GUI and
echo other Plus UI-only components. The backend and CLI launcher remain.
 echo.
 echo WARNING: Some Sandboxie configurations and advanced features may
 echo require a valid supporter certificate. If no supporter license
 echo is present, the runtime may display the standard reminder/warning
 echo flow instead of the unrestricted Plus feature set.
 echo.
 echo Supported build targets for this fork:
 echo   - Sandboxie/core/drv
 echo   - Sandboxie/core/svc
 echo   - Sandboxie/core/dll
 echo   - Sandboxie/apps/start
 echo   - Sandboxie/apps/ini
 echo   - Sandboxie/apps/common
 echo.
 echo Build the required Windows solution/projects from the Sandboxie tree
 echo and use the Start.exe launcher for console-driven operation.
 echo.
 echo For GUI builds, keep the original SandMan/Plus project files separate.
 echo ============================================================
exit /b 0
