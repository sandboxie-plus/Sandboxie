## Sandboxie Plus build instructions

- Please note: there is another [ReadMe.md](../Installer/ReadMe.md) file that explains how to create the Sandboxie Plus installers.
- Please note: the following instructions may lag behind the [CI workflow](../.github/workflows/main.yml), so be aware of any version change.

Sandboxie Plus builds under Visual Studio 2022 and uses the Sandboxie Classic components.

1) Install Visual Studio 2022 with the Windows SDK and WDK required by the [Classic build instructions](../Sandboxie/ReadMe.md)
2) Install the MSVC v142 build tools and Windows 10 SDK 10.0.19041.0 required by SbieShell
3) Install 7-Zip in `C:\Program Files\7-Zip`
4) Build Sandboxie Classic using the commands in the Classic build instructions
5) The Qt version is defined in [`Installer\buildVariables.cmd`](../Installer/buildVariables.cmd); `install_qt.cmd` downloads the matching Qt 6 build
   - The scripts use the Visual Studio 2022 Enterprise `vcvars*.bat` path used by CI; if you use Community or Professional, update that path in `qmake_plus.cmd` and `copy_build.cmd`
6) From a Visual Studio 2022 Developer Command Prompt in the repository root, run:

```bat
SandboxiePlus\install_qt.cmd x64
SandboxiePlus\install_jom.cmd
SandboxiePlus\qmake_plus.cmd x64 build_qt6
msbuild /t:restore,build -p:RestorePackagesConfig=true SandboxiePlus\SbieShell\SbieShell.sln /p:Configuration="Release" /p:Platform=x64
msbuild /t:build SandboxieTools\SandboxieTools.sln /p:Configuration="Release" /p:Platform=x64 -maxcpucount:8
Installer\fix_qt5_languages.cmd x64 build_qt6
Installer\get_openssl.cmd
Installer\get_7zip.cmd
Installer\copy_build.cmd x64 build_qt6
```

7) The assembled x64 build is placed in `Installer\SbiePlus_x64`
8) For ARM64, see the current commands in the [CI workflow](../.github/workflows/main.yml)

At this point, you may wonder how to run this build. In the end, the driver is not signed and we did not touch the process of signing the user mode components either.

As signing the driver is probably not feasible for most developers due to the lack of an EV code signing certificate, we will set up our Windows to run in test mode by entering "bcdedit /set testsigning on" in an elevated command prompt and rebooting. Once rebooted, we can start the SandMan UI and use the Maintenance menu to install all components, start and connect them with only one click.

You may notice that we did not get the obligatory "access denied" error we would expect by connecting an unsigned user mode component to the driver. This is because when the driver sees Windows being run in test mode, it also disables the custom signature verification mechanism, meaning you don't need to create any *.sig files for your own Sandboxie Plus test build.

With everything running, you can now go to the Global Settings and enter your supporter certificate to unlock all the exclusive Plus features.

### Source projects

> Note: the core of Sandboxie are the driver, SbieDrv, the service, SbieSvc, and the injection DLL, SbieDll:
[Sandboxie\ReadMe.md#source-projects-in-alphabetical-order](../Sandboxie/ReadMe.md#source-projects-in-alphabetical-order)

Sandboxie Plus is made up of the following components:

[MiscHelpers (\MiscHelpers)](./MiscHelpers)

[QSbieAPI (\QSbieAPI)](./QSbieAPI)

[QtSingleApp (\QtSingleApp)](./QtSingleApp)

[SandMan (\SandMan)](./SandMan)

[SbieShell (\SbieShell)](./SbieShell)

[UGlobalHotkey (\UGlobalHotkey)](./UGlobalHotkey)
