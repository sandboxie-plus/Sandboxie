### Sandboxie Build Instructions

Please note: there is another ReadMe file in the \install\ folder that explains how to create the Sandboxie installers.

1) Sandboxie builds under MS Visual Studio 2019.
2) Install the Windows Driver Kit (WDK) for Windows 10, version 2004:
	https://go.microsoft.com/fwlink/?linkid=2128854
3) The VS Solution File, Sandbox.sln, is in the source code root. Open this SLN in Visual Studio.

### Source Projects (in alphabetical order)

Note: the core of Sandboxie are the driver, SbieDrv, the service, SbieSvc, and the injection DLL, SbieDll. Study these projects first.

`Common (\apps\common)`. It builds common.lib, which contains some common GUI objects, and it is used by the Control and Start projects.

`KmdUtil (\install\kmdutil)`. It builds KmdUtil.exe, which is used during the installation process, e.g. to start/stop the Sbie driver (SbieDrv.sys).

`LowLevel (\core\low)`. It creates LowLevel.dll, which is used in code injection. LowLevel.dll is embedded into SbieSvc.exe as a resource (see core\svc\lowlevel.rc for more information).

`Parse (\msgs)`. It creates the Sbie messages files.

`SandboxBITS (apps\com\BITS)`. It creates SandboxieBITS.exe (Background Intelligent Transfer Service).

`SandboxCrypto (apps\com\Crypto)`. It creates SandboxieCrypto.exe.

`SandboxieInstall (\install\release)`. It creates the combined x64/x86 installer SandboxieInstall.exe by combining the x64 & x86 installer binaries into a RC file.
- Please note: SandboxieInstall is not built during the normal SLN build. It must be built manually after the x64 & x86 installers are completed.

`SandboxRpcSs (\apps\com\RpcSs)`. It creates SandboxieRpcSs.exe, the Sbie's wrapper for the Remote Procedure Call Sub-System.

`SandboxWUAU (\apps\com\WUAU)`. It creates SandboxieWUAU.exe, the Sbie's wrapper for the Windows Automatic Update Service (WUAUSERV).

`SbieControl (\apps\control)`. It builds SbieCtrl.exe, the Sandboxie Control app that displays real-time sandboxed application activity.

`SbieIni (\apps\ini)`. It creates SbieIni.exe, a utility for querying and updating the sandboxie.ini configuration file.

`SboxDcomLaunch (\apps\com\DcomLaunch)`. It creates SandboxieDcomLaunch.exe.

`SboxDll (\core\dll)`. It creates the Sbie injection DLL, which injects into every process running in the sandbox.

`SboxDrv (\core\drv)`. It creates the Sbie kernel-mode driver.

`SboxHostDll (\SboxHostDll)`. It builds the Sbie host injection DLL, which is injected into host processes that must be redirected into the sandbox.
Currently, the only app this is used for is MS Office. SboxHostDll.dll is injected into OfficeClickToRun.exe.

`SboxMsg (\msgs)`. It creates SboxMsg.dll, which contains the Sbie user messages in various languages.

`SboxSvc (\core\svc)`. It creates the Sbie service.

`Start (\apps\start)`. It creates start.exe, which is used to start processes in the sandbox.
