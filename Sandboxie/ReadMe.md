## Sandboxie Classic build instructions

- Please note: there is another [ReadMe.md](./install/ReadMe.md) file in the [install](./install/) folder that explains how to create the Sandboxie Classic installers.
- Please note: the following instructions may lag behind the [CI workflow](../.github/workflows/main.yml), so be aware of any version change.

Sandboxie Classic builds under Visual Studio 2022.

1) Download [Visual Studio 2022](https://visualstudio.microsoft.com/vs/)
2) In the Visual Studio Installer, select _Desktop development with C++_ and a current Windows SDK
3) Install the _MSVC v143_ and _MFC_ components for each platform you want to build
4) Install a matching [Windows Driver Kit (WDK)](https://learn.microsoft.com/windows-hardware/drivers/download-the-wdk)
5) The WDK installer should also install its Visual Studio extension; rerun the WDK installer if the driver project is unavailable in Visual Studio
6) The `Sandbox.sln` solution is in the `Sandboxie` directory
7) Keep the toolsets selected by the project files; do not retarget every project or add WDK include paths manually
8) To compile an x64 release, first build the required Win32 service and DLL components, then the x64 solution and driver:

```bat
msbuild /t:build Sandboxie\SandboxDll.sln /p:Configuration="SbieRelease" /p:Platform=Win32 -maxcpucount:8
msbuild /t:build Sandboxie\Sandbox.sln /p:Configuration="SbieRelease" /p:Platform=x64 -maxcpucount:8
msbuild /t:build Sandboxie\SandboxDrv.sln /p:Configuration="SbieRelease" /p:Platform=x64 -maxcpucount:8
```

These commands can be run from a Visual Studio 2022 Developer Command Prompt in the repository root. For ARM64 and ARM64EC, see the current commands in the [CI workflow](../.github/workflows/main.yml).

### Source projects (in alphabetical order)

> Note: the core of Sandboxie are the driver, SbieDrv, the service, SbieSvc, and the injection DLL, SbieDll. Study these projects first.

[Common (\apps\common)](./apps/common). It builds common.lib, which contains some common GUI objects, and it is used by the Control and Start projects.

[KmdUtil (\install\kmdutil)](./install/kmdutil). It builds KmdUtil.exe, which is used during the installation process, e.g. to start/stop the Sbie driver (SbieDrv.sys).

[LowLevel (\core\low)](./core/low). It creates LowLevel.dll, which is used in code injection. LowLevel.dll is embedded into SbieSvc.exe as a resource (see core\svc\lowlevel.rc for more information).

[Parse (\msgs)](./msgs). It creates the Sbie messages files.

[SandboxBITS (apps\com\BITS)](./apps/com/BITS). It creates SandboxieBITS.exe (Background Intelligent Transfer Service).

[SandboxCrypto (apps\com\Crypto)](./apps/com/Crypto). It creates SandboxieCrypto.exe.

[SandboxieInstall (\install\release)](./install/release). It creates the combined x64/x86 installer SandboxieInstall.exe by combining the x64 & x86 installer binaries into a RC file.
> Note: SandboxieInstall is not built during the normal SLN build. It must be built manually after the x64 & x86 installers are completed.

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
