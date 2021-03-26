
SANDBOXIE BUILD INSTRUCTIONS

Please note: there is another ReadMe.txt in the \install\ folder that explains how to create the Sandboxie installers.

1) Sandboxie builds under MS Visual Studio 2019.
2) Install the Windows Driver Kit (WDK) for Windows 10, version 2004.
	https://go.microsoft.com/fwlink/?linkid=2128854
3) The VS Solution File, Sandbox.sln, is in the source code root. Open this SLN in Visual Studio.

Source Projects (in aplhabetical order)

Note: the core of Sandboxie are the driver, SbieDrv, the service, SbieSvc, and the injection DLL, SbieDll. Study these projects first.

Common (\apps\common). This builds common.lib which is used by the Control and Start projects. It contains some common GUI objects.

KmdUtil (\install\kmdutil). Builds KmdUtil.exe which is used during the installtion process. E.g. to start/stop the Sbie driver (SbieDrv.sys).

LowLevel (\core\low). Creates LowLevel.dll which is used in code injection. LowLevel.dll is embeded into SbieSvc.exe as a resource (see core\svc\lowlevel.rc for more information).

Parse (\msgs). Creates the Sbie messages files.

SandboxBITS (apps\com\BITS). Creates SandboxieBITS.exe (Background  Intelligent Transfer Service)

SandboxCrypto (apps\com\Crypto) Creates SandboxieCrypto.exe

SandboxieInstall (\install\release). Creates the combined x64/x86 installer SandboxieInstall.exe by combining the x64 & x86 installer binaries into a RC file.
	NOTE: SandboxieInstall is not built during the normal SLN build. It must be built manually after the x64 & x86 installers are completed.

SandboxRpcSs (\apps\com\RpcSs) Creates SandboxieRpcSs.exe. Sbie's wrapper for the Remote Procedure Call Sub-System).

SandboxWUAU (\apps\com\WUAU) Creates SandboxieWUAU.exe. Sbie's wrapper for the Windows Automatic Update Service (WUAUSERV).

SbieControl (\apps\control). Builds SbieCtrl.exe. This is the Sandboxie Control app that displays real-time sandboxed application activity.

SbieIni (\apps\ini). Creates SbieIni.exe. A utility for querying and updating the sandboxie.ini configuration file.

SboxDcomLaunch (\apps\com\DcomLaunch). Creates SandboxieDcomLaunch.exe. 

SboxDll (\core\dll). Creates the Sbie injection DLL. This DLL injects into every process running in the sandbox.

SboxDrv (\core\drv). Creates the Sbie kernel-mode driver.

SboxHostDll (\SboxHostDll). Builds the Sbie host injection DLL. This DLL is injected into host processes that must be redirected into the sandbox.
	Currently, the only app this is used for is MS Office. SboxHostDll.dll is injected into OfficeClickToRun.exe.

SboxMsg (\msgs). Creates SboxMsg.dll which contains the Sbie user messages in various languages.

SboxSvc (\core\svc). Creates the Sbie service.

Start (\apps\start). Creates start.exe which is used to start processes in the sandbox.



