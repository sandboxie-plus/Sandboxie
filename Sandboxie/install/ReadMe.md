## Sandboxie Classic installer instructions

### To create Sbie NSIS installer environment

1) Install NSIS 2.5 (installer is located at https://sourceforge.net/projects/nsis/files/NSIS%202/2.50/)
<br>Later versions will not work with the Sbie NSI file.
2) You will also need NSIS InetC plugin https://nsis.sourceforge.io/Inetc_plug-in
3) From Inetc.zip, copy `\Plugins\x86-ansi\InetC.dll` to your NSIS plugins folder (e.g. `C:\Program Files (x86)\NSIS\Plugins`)
4) Overwrite `C:\Program Files (x86)\NSIS\Contrib\Modern UI\System.nsh` with `install\nsis_updates.zip\nsis\Contrib\Modern UI\System.nsh`
5) Copy language files from `install\nsis_updates.zip\nsis\Contrib\Modern UI\Language files` to `C:\Program Files (x86)\NSIS\Contrib\Modern UI\Language files`
6) The install also requires Iconv 1.9.2.1747 (for international language conversion) which can be downloaded from https://gnuwin32.sourceforge.net/packages/libiconv.htm

	You need 2 zip files, the `Binaries` and the `Dependencies`. These contain the 3 binaries that are required:
	`Iconv.exe`
	`Libiconv2.dll`
	`Libintl3.dll`

	Copy these 3 binaries into a folder under the Sandboxie source tree named `tools\iconv` at the same level as Sandbox.sln (e.g. `C:\src\Sbie\tools\iconv`)

Everything above only needs to be done once. Then your machine should be good to go.

### To create the Sbie installers

The Sbie installer NSI file is [\install\SandboxieVS.nsi](../install/SandboxieVS.nsi).

Note: the following procedure is only valid for the old code signing not mediated by Microsoft, please refer to issue [#1248](https://github.com/sandboxie-plus/Sandboxie/issues/1248).

1) Sign the binaries (if applicable).
2) Set the #defines in [\common\my_version.h](../common/my_version.h) (SBIE_INSTALLER_PATH, etc.) to the location of your binaries. (These #defines are read by SandboxieVS.nsi).
3) Edit [\install\SandboxieVS.nsi](../install/SandboxieVS.nsi) and uncomment the `!define _BUILDARCH` for the version you want to create an installer for.
4) In Explorer, right-click on [\install\SandboxieVS.nsi](../install/SandboxieVS.nsi), and select `Compile NSIS Script`.
	The resulting installer binary (SandboxieInstall64.exe or SandboxieInstall32.exe) will be saved to SBIE_INSTALLER_PATH (from [my_version.h](../common/my_version.h)).
5) Sign the installers.
6) To create the combined 32/64 bit installer in Visual Studio, right-click on the `SandboxieInstall` project, click `Project Only` -> `Build Only SandboxieInstall`.
	This will create a SandboxieInstall.exe in the install folder.
7) Sign the combined installer.
