## Sandboxie Build Instructions

Please note: there is another [ReadMe.md](./install/ReadMe.md) file in the [\install](./install/) folder that explains how to create the Sandboxie installers.  
 
Sandboxie builds under MS Visual Studio 2019.  
1) Acquire [Visual Studio 2019](https://visualstudio.microsoft.com/vs/older-downloads/#visual-studio-2019-and-other-products)
2) In the Visual Studio Installer, tick "Desktop development with C++"
	- This will install all needed components to build, including the Windows 10 SDK 10.0.19041
3) If you need to build for other platforms, install the corresponding components
	- MSVC v142 - VS 2019 C++ {architecture} build tools (Latest)
	- MFC for latest v142 build tools {architecture}
4) Install the Windows Driver Kit (WDK) for Windows 10, version 2004 (10.0.19041):
	https://go.microsoft.com/fwlink/?linkid=2128854
5) The VS Solution File, Sandbox.sln, is in the source code root. Open this SLN in Visual Studio.
6) If the WDK Extension doesn't install automatically, install it (can be found in <Windows Kits directory>\10\Vsix\VS2019)
7) If you have a more recent Windows SDK version installed, retarget the solution to 10.0.19041
	- This is for example necessary if VS2022 is also installed with the default desktop C++ components
8) To compile for x64, it's necessary to first compile `Solution/core/LowLevel` for Win32

### Source Projects (in alphabetical order)

Note: the core of Sandboxie are the driver, SbieDrv, the service, SbieSvc, and the injection DLL, SbieDll. Study these projects first.

[Common (\apps\common)](./apps/common). It builds common.lib, which contains some common GUI objects, and it is used by the Control and Start projects.

[KmdUtil (\install\kmdutil)](./install/kmdutil). It builds KmdUtil.exe, which is used during the installation process, e.g. to start/stop the Sbie driver (SbieDrv.sys).

[LowLevel (\core\low)](./core/low). It creates LowLevel.dll, which is used in code injection. LowLevel.dll is embedded into SbieSvc.exe as a resource (see core\svc\lowlevel.rc for more information).

[Parse (\msgs)](./msgs). It creates the Sbie messages files.

[SandboxBITS (apps\com\BITS)](./apps/com/BITS). It creates SandboxieBITS.exe (Background Intelligent Transfer Service).

[SandboxCrypto (apps\com\Crypto)](./apps/com/Crypto). It creates SandboxieCrypto.exe.

[SandboxieInstall (\install\release)](./install/release). It creates the combined x64/x86 installer SandboxieInstall.exe by combining the x64 & x86 installer binaries into a RC file.
- Please note: SandboxieInstall is not built during the normal SLN build. It must be built manually after the x64 & x86 installers are completed.

[SandboxRpcSs (\apps\com\RpcSs)](./apps/com/RpcSs). It creates SandboxieRpcSs.exe, the Sbie's wrapper for the Remote Procedure Call Sub-System.

[SandboxWUAU (\apps\com\WUAU)](./apps/com/WUAU). It creates SandboxieWUAU.exe, the Sbie's wrapper for the Windows Automatic Update Service (WUAUSERV).

[SbieControl (\apps\control)](./apps/control). It builds SbieCtrl.exe, the Sandboxie Control app that displays real-time sandboxed application activity.

[SbieIni (\apps\ini)](./apps/ini). It creates SbieIni.exe, a utility for querying and updating the sandboxie.ini configuration file.

[SboxDcomLaunch (\apps\com\DcomLaunch)](./apps/com/DcomLaunch). It creates SandboxieDcomLaunch.exe.

[SboxDll (\core\dll)](./core/dll). It creates the Sbie injection DLL, which injects into every process running in the sandbox.

[SboxDrv (\core\drv)](./core/drv). It creates the Sbie kernel-mode driver.

[SboxHostDll (\SboxHostDll)](./SboxHostDll). It builds the Sbie host injection DLL, which is injected into host processes that must be redirected into the sandbox.
Currently, the only app this is used for is MS Office. SboxHostDll.dll is injected into OfficeClickToRun.exe.

[SboxMsg (\msgs)](./msgs). It creates SboxMsg.dll, which contains the Sbie user messages in various languages.

[SboxSvc (\core\svc)](./core/svc). It creates the Sbie service.

[Start (\apps\start)](./apps/start). It creates start.exe, which is used to start processes in the sandbox.
