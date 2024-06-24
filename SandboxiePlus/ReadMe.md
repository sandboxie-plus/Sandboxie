## Sandboxie Plus build instructions

- Please note: there is another [ReadMe.md](../Installer/ReadMe.md) file that explains how to create the Sandboxie Plus installers.
- Please note: the following instructions may lag behind the [CI workflow](../.github/workflows/main.yml), so be aware of any version change.

Sandboxie Plus builds under Visual Studio 2019, as it offers the widest compatibility range, allowing us to build a driver which works with Windows 7 up to Windows 11.

1) We will be installing Visual Studio Community Edition which is sufficient for our purposes, during the installation we need to take care of selecting a Windows SDK version which matches the WDK version we will be installing in the next step.
	- If you have VS 2019 already installed, you can open the installer and check which SDK you have and add if necessary one matching the WDK.
2) Next, we will install WDK 20xxx which is required to build the driver, the WDK installer installs the required VS plugin at the end.
3) So far so good, at this point we already have all we need to build Sandboxie Classic. To build the SandMan UI of Sandboxie Plus, though, we also need the Qt Framework, we can use either 5.15.14 or 6.3.x, but since Qt 6.x does not support Windows 7 without custom patches to base components, we will stick with Qt 5.15.14 for this tutorial.
	- Note: however, if you want to build SandMan UI for ARM64, you will need to use Qt 6.3.x or later.
4) We use Qt's online installer and select all required components. I like to also install the source and debug information, but this is just for convenience when debugging and not needed for the build process.
5) Last but not least, we have to install the VS extension for Qt and configure it to point to our Qt installation.
6) Ok, now we are ready to build, we start with Sandboxie Classic, we open the Sandbox.sln, select our platform and build type, and run the build.
	- If we build for x64, we will need to also build the SbieSvc and SbieDll for 32-bit.
	- If we were building for ARM64, we would also need the ARM64EC version of SbieDll.
7) And now we continue with building the SandMan UI to create Sandboxie Plus. Here we open the Sandboxie-Plus.sln, select our platform and build type, and run the build.
8) Once that is done, we only need to combine the two and here it is: Sandboxie Plus is ready for service.

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
