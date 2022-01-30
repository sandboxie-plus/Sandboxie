setlocal ENABLEDELAYEDEXPANSION

set INPUT=%1
set OUTPUT=%2
if not %1x==x goto :start
set INPUT=c:\work\sbie\common\my_version.h
set OUTPUT=temp.tmp

:start

echo. > %OUTPUT%

for /F "tokens=3*" %%A in ('findstr /R "^#define.SBIE_INSTALLER_PATH\>"    %INPUT%') do ( echo ^^!define SBIE_INSTALLER_PATH %%A) >> %OUTPUT%

IF [%SbieVer%]==[] (
for /F "tokens=3*" %%A in ('findstr /R "^#define.MY_VERSION_STRING\>"      %INPUT%') do ( echo ^^!define VERSION %%A) >> %OUTPUT%
) ELSE (
echo ^^!define VERSION "%SbieVer%" >> %OUTPUT%
)

for /F "tokens=3*" %%A in ('findstr /R "^#define.MY_PRODUCT_NAME_STRING\>" %INPUT%') do ( set C=%%A %%B& echo ^^!define PRODUCT_FULL_NAME !C! & set C=!C: =!& echo ^^!define PRODUCT_NAME !C!) >> %OUTPUT%

for /F "tokens=3*" %%A in ('findstr /R "^#define.MY_COMPANY_NAME_STRING\>" %INPUT%') do ( echo ^^!define COMPANY_NAME %%A %%B) >> %OUTPUT%

for /F "tokens=3*" %%A in ('findstr /R "^#define.MY_COPYRIGHT_STRING\>"    %INPUT%') do ( echo ^^!define COPYRIGHT_STRING %%A %%B) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SANDBOXIE\>"              %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SANDBOXIE !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SANDBOXIE_INI\>"          %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SANDBOXIE_INI !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SBIEDRV\>"                %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SBIEDRV !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SBIEDRV_SYS\>"            %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SBIEDRV_SYS !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SBIESVC\>"                %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SBIESVC !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SBIESVC_EXE\>"            %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SBIESVC_EXE !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SANDBOXIE_CONTROL\>"      %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SANDBOXIE_CONTROL !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SBIECTRL_EXE\>"           %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SBIECTRL_EXE !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.START_EXE\>"              %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define START_EXE !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SBIEDLL\>"                %INPUT%') do ( set C=%%A& set C=!C:~1!& set C=!C:~1,-1!& echo ^^!define SBIEDLL_DLL "!C!.dll") >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SBIEMSG_DLL\>"            %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SBIEMSG_DLL !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SBIEINI_EXE\>"            %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SBIEINI_EXE !C!) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.SANDBOX_VERB\>"           %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define SANDBOX_VERB !C!) >> %OUTPUT%

for /F "tokens=3*" %%A in ('findstr /R "^#define.MY_AUTOPLAY_CLSID_STR\>"  %INPUT%') do ( echo ^^!define MY_AUTOPLAY_CLSID %%A %%B) >> %OUTPUT%

for /F "tokens=3"  %%A in ('findstr /R "^#define.FILTER_ALTITUDE\>"        %INPUT%') do ( set C=%%A& set C=!C:~1!& echo ^^!define FILTER_ALTITUDE !C!) >> %OUTPUT%
