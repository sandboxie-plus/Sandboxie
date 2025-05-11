# Changelog
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).


## [1.16.0 / 5.71.0] - 2025-04-??

### Added
- added option to copy hwid from the support page
- added OpenClsid and fixed an issue with OpenClsid handling to enable firefox's hardware acceleration
- added 'MarkOfTheWebBox=BoxName' option to force all files with the Mark Of the Web set into a preset sandbox
- added wildcard support with ForceProcess [#2603](https://github.com/sandboxie-plus/Sandboxie/issues/2603)




## [1.15.12 / 5.70.12] - 2025-05-01

### Added
- added Indonesian translation to the Plus UI [4bad4d1](https://github.com/sandboxie-plus/Sandboxie/commit/4bad4d190a90a6329d549e5a31ca8ee13b4eb9a1) (thanks ???)
- added template for WeType Input Method (IME) [#4715](https://github.com/sandboxie-plus/Sandboxie/pull/4715) (thanks bot-1450)
- added host name support to proxy configuration (looking up host at WSA_Startup)
- instead of binding to IP, Sandboxie can now bind to adapter (resolving IPs at WSA_Startup)
- added support for %ProgramFiles(x86)% variable [#2402](https://github.com/sandboxie-plus/Sandboxie/issues/2402)
- added TemplateDefaultFolders containing all the default paths to be created in a privacy enhanced box

### Changed
- weird sort in the Move Sandbox list [#4615](https://github.com/sandboxie-plus/Sandboxie/issues/4615)
- name and HwID information in the Support tab is now hidden by default but can be toggled as needed [#4714](https://github.com/sandboxie-plus/Sandboxie/pull/4714)
- added alternative proxy relay implementation (instead of hacking existing sockets it starts relay threads) [EXPERIMENTAL]

### Fixed
- fixed the 'run in box' selection prompt not show BoxAlias [#4709](https://github.com/sandboxie-plus/Sandboxie/issues/4709)
- fixed after Ctrl+F, the expansion and closure records of the Box Group were disrupted [#4708](https://github.com/sandboxie-plus/Sandboxie/issues/4708)
- added additional checks to registry set/get functions [5a6a2f7](https://github.com/sandboxie-plus/Sandboxie/commit/5a6a2f7f8a99eb9d36906b73bc883ebc4c268cc4)
- fixed issue in Pool_Alloc [5a6a2f7](https://github.com/sandboxie-plus/Sandboxie/commit/5a6a2f7f8a99eb9d36906b73bc883ebc4c268cc4)
- fixed SbieCtrl.exe and SandMan.exe having a constant 3 MB/s IO rate in the Process Explorer [#4693](https://github.com/sandboxie-plus/Sandboxie/issues/4693)
- fixed template for Tencent TIM causes drag and drop not available [#4688](https://github.com/sandboxie-plus/Sandboxie/issues/4688)
- improved when Sandboxie.ini is huge, the response speed gets worse [#4573](https://github.com/sandboxie-plus/Sandboxie/issues/4573)
- fixed MSI install can't create AppData\Roaming\Microsoft folder in Data Protection Box [#4711](https://github.com/sandboxie-plus/Sandboxie/issues/4711)
- fixed two errors in Sandboxie about time speeding and add two time function hook [#4721](https://github.com/sandboxie-plus/Sandboxie/pull/4721) (thanks pwnmelife)
- fixed an issue where the translation string of the protocol selection was saved in the configuration file
- fixed box picker icon
- fixed when the UI language change, toolbar unchanged [#4726](https://github.com/sandboxie-plus/Sandboxie/issues/4726)
- fixed PingInfoView gives an error when running in Sandboxie [#4718](https://github.com/sandboxie-plus/Sandboxie/issues/4718)
- fixed CollectProducts() stuck in loop when Uninstall key not found [#4753](https://github.com/sandboxie-plus/Sandboxie/issues/4753)
- fixed crashes with SbieDll.dll [#4754](https://github.com/sandboxie-plus/Sandboxie/issues/4754)
- fixed OneDrive automatically downloads file when creating shortcut [#4663](https://github.com/sandboxie-plus/Sandboxie/issues/4663) [4750](https://github.com/sandboxie-plus/Sandboxie/pull/4750) (thanks WZ-Tong)

### Removed
- removed the not-working "delete content" button [#4720](https://github.com/sandboxie-plus/Sandboxie/pull/4720) (thanks habatake)



## [1.15.11 / 5.70.11] - 2025-04-16

### Fixed
- fixed issue with SboxHostDll.dll failing to be injected into OfficeClickToRun.exe



## [1.15.10 / 5.70.10] - 2025-04-15

### Added
- added 'BindAdapterIP=192.168.100.123' and/or 'BindAdapterIP=::ffff:c0a8:647b' to force sandboxed programs to use a defined host IP
  - Note: if the IP is not bound to one of the hosts NICs the connections will fail
  - the configuration can be set per process like 'BindAdapterIP=program.exe,192.168.100.123', but only in the INI, not in the UI

### Changed
- greatly improved handling of Patreon certificates
- improved certificate tab; serial entry is now always visible, and the certificate text edit as well, though it does not show the critical information UPDATE_KEY/SIGNATURE, instead only ... when mouse clicked the full data gets displayed
- streamlined application of new certificates

### Fixed
- fixed Firefox 137.0 generates SBIE2328 notifications in Sandboxie Plus v1.15.9 [#4652](https://github.com/sandboxie-plus/Sandboxie/issues/4638) [#4652](https://github.com/sandboxie-plus/Sandboxie/issues/4652) [#4640](https://github.com/sandboxie-plus/Sandboxie/issues/4640)
- fixed SbieShellExt.dll missing in 1.15.9
- fixed Process_GetCommandLine causing memory leak [#4658](https://github.com/sandboxie-plus/Sandboxie/issues/4658)
- fixed Thunderbird 102.15.1 does not load all extensions under Sandboxie Plus v1.15.9 [#4653](https://github.com/sandboxie-plus/Sandboxie/issues/4653)
- fixed in Sandboxie, invoke SetParent change window to message-only window failed [#4656](https://github.com/sandboxie-plus/Sandboxie/issues/4656)
- fixed issue with Unicode names when retrieving an evaluation certificate
- fixed should the delete content message box always on top? [#4673](https://github.com/sandboxie-plus/Sandboxie/issues/4673)



## [1.15.9 / 5.70.9] - 2025-04-02

### Added
- added duplicate sandbox with content from UI [#4542](https://github.com/sandboxie-plus/Sandboxie/issues/4542)
- added new insecure debug option 'OpenAllSysCalls=y'
- added template for WindowGrid
- added new certificate type [DEVELOPER](https://xanasoft.com/product/sandboxie-plus-developer/); it deactivates the user mode component verification, but can only be used in node-locked form
  - Note: all current and future holders of Contributor or Eternal/Huge certificates are eligible to receive a free Dev Key; please request by email if required
- added mechanism to update only Templates.ini

### Changed
- when no proxy for a given IP type is configured, the connections fail
  - this change prevents IP leakage when running dual stack IPv4 and IPv6, but having only one proxy type configured
- INI editor is no longer reset when saving
- DisableWinNtHook can now be changed without a driver reload
- disabled update checks when no internet connectivity is detected

### Fixed
- fixed the encoding issue with the bullet character in the box delete/remove confirmation dialog [#4591](https://github.com/sandboxie-plus/Sandboxie/issues/4591)
- fixed changing the box settings/type causes some template settings in the box to be duplicated [#4595](https://github.com/sandboxie-plus/Sandboxie/issues/4595)
- added warning when attempting to put a sandbox is in the root of a partition [#4582](https://github.com/sandboxie-plus/Sandboxie/issues/4582)
- fixed bug when uninstalling [#4540](https://github.com/sandboxie-plus/Sandboxie/issues/4540)
- fixed Golang Cmd.Run() doesn't return when running in Sandboxie [#4538](https://github.com/sandboxie-plus/Sandboxie/issues/4538)
  - fixed a generic handle duplication issue which may help many other applications to
- improved when Sandboxie.ini is huge, the response speed gets worse [#4573](https://github.com/sandboxie-plus/Sandboxie/issues/4573)
- fixed Sandboxie freezes/crashes in certain installations [#4537](https://github.com/sandboxie-plus/Sandboxie/issues/4537)
  - when SandMan hangs and gets terminated, it will ask on next start if to disable update scanning for software compatibility checks
- fixed issue with hook monitor display
- fixed issue with INI section editing, esc no longer closes the window
- fixed hooking issues introduced in Firefox 137 Nightly and later [#4522](https://github.com/sandboxie-plus/Sandboxie/issues/4522)
- fixed issue with insecure debug option 'OriginalToken=y'
- fixed box group name accidentally entered carriage return character [#4608](https://github.com/sandboxie-plus/Sandboxie/issues/4608)
- fixed paths selected from the path selection drop-down menu in the New Box Wizard return a "not a valid path" error [#4611](https://github.com/sandboxie-plus/Sandboxie/issues/4611)
- FIXED SECURITY ISSUE ID-25: new method of exploiting HostInjectDll mechanism for local privilege escalation (thanks hg421)



## [1.15.8 / 5.70.8] - 2025-03-12

### Changed
- added 'Creation Time' column to snapshot manager (thanks PenXLa) [#4555](https://github.com/sandboxie-plus/Sandboxie/pull/4555)
- improved sandbox deletion confirmation prompt (thanks PenXLa) [#4561](https://github.com/sandboxie-plus/Sandboxie/pull/4561)
- updated online updater
- added [ImDisk 3.0](https://github.com/DavidXanatos/ImDisk/releases/tag/v3.0.0) driver to Sandboxie Plus installer (x64 only)

### Fixed
- fixed "WriteFilePath" not working correctly [#4553](https://github.com/sandboxie-plus/Sandboxie/issues/4553)
- fixed an issue where Box Notes deleted matching lines when saved [#4547](https://github.com/sandboxie-plus/Sandboxie/issues/4547)



## [1.15.7 / 5.70.7] - 2025-02-23

### Added
- service can now also log monitor events

### Changed
- updated Notepad++ template to work in Sandboxie Classic
- updated MiscHelpers.dll, improved finder

### Fixed
- fixed obsolete entries in the Sandboxie Plus installer
- fixed Pinned shortcuts created from 'Run from Start Menu' are not functional [#4502](https://github.com/sandboxie-plus/Sandboxie/issues/4502)
- fixed potential BSoD issue with driver in low resource conditions
- fixed a crash in NsiAllocateAndGetTable (thanks tzf-omkey) [#4511](https://github.com/sandboxie-plus/Sandboxie/issues/4511)
- fixed a memory leak when device change notification is received by a sandboxed process
- fixed a memory leak in NtQueryDirectoryFile hook manifesting in privacy enhanced boxes [#4509](https://github.com/sandboxie-plus/Sandboxie/issues/4509)



## [1.15.6 / 5.70.6] - 2025-02-03

### Added
- added generic troubleshooting script for SBIE2224: Sandboxed program has crashed
  - provides a workaround for Viber (Viber.exe)
- added debug option "DropChildProcessToken=program.exe,y" to not set a child process token useful to debug green boxes
- added missing hook to NtQueryInformationByName

### Changed
- reworked CreateDesktop handling to fix issues with browsers and browser-based applications

### Fixed
- fixed Sandboxie Plus data folder not always working properly after updating to version 1.15.5
- fixed a too long held lock in the troubleshooting engine
- fixed Adobe Acrobat Reader crashes in Application Compartment box [#4363](https://github.com/sandboxie-plus/Sandboxie/issues/4363)
- fixed The Viber software has been updated, but when opened using the sandboxie program, the system program always crashes [#4298](https://github.com/sandboxie-plus/Sandboxie/issues/4298)
- fixed Cannot run qutebrowser in Sandboxie [#4419](https://github.com/sandboxie-plus/Sandboxie/issues/4419)
- fixed Unable to launch application in sandbox with ApiTrace enabled in version 1.15.5 [#4488](https://github.com/sandboxie-plus/Sandboxie/issues/4488)
- fixed BSoD when using OpenWndStation=y on Windows 10 with apps that call CreateDesktopW/A



## [1.15.5 / 5.70.5] - 2025-01-27

### Added
- added 'HookTrace=y' to debug API hooking

### Changed
- moved Sandboxie Plus data folder from %LOCALAPPDATA%\\Sandboxie-Plus to %LOCALAPPDATA%\\Xanasoft\\Sandboxie-Plus
  - Note: the folder will be moved automatically. When downgrading to an older version it needs to be moved back manually.
- re-ordered box context menu options [#4444](https://github.com/sandboxie-plus/Sandboxie/issues/4444)

### Fixed
- fixed crash when pressing run as admin with non-advanced view
- fixed WerFault.exe errors for all applications on the latest Windows 11 ARM64 [#4422](https://github.com/sandboxie-plus/Sandboxie/issues/4422)
- fixed issue preventing x64 executables from running on ARM64 [#4415](https://github.com/sandboxie-plus/Sandboxie/issues/4415)
- fixed issue with process self-termination
- fixed msedge_elf.dll not found on Windows 10 ARM64
- fixed Windows 10 ARM64 compatibility issue [#3600](https://github.com/sandboxie-plus/Sandboxie/issues/3600)
- fixed infinite loop when Gui_ConnectConsole waits for SbieSvc.exe to quit [#4462](https://github.com/sandboxie-plus/Sandboxie/pull/4462) (thanks bot-1450)
- fixed program icon not displaying if the command path in RunCommand setting contains spaces [#4416](https://github.com/sandboxie-plus/Sandboxie/issues/4416)
- fixed template scanning not working properly [#4401](https://github.com/sandboxie-plus/Sandboxie/issues/4401)
- fixed credentials dialog not displaying when connecting to RDP server by mstsc.exe in encrypted box [#4389](https://github.com/sandboxie-plus/Sandboxie/issues/4389)
- fixed folder recovery only moving files inside that folder [#4453](https://github.com/sandboxie-plus/Sandboxie/issues/4453)
- fixed failure to query file properties causing program exceptions [#4457](https://github.com/sandboxie-plus/Sandboxie/issues/4457) (thanks lmou523)
- fixed applications crashing if IP is not defined for the domain name in DNS Filtering [#4475](https://github.com/sandboxie-plus/Sandboxie/issues/4475)
- fixed files not getting written when launched from "Run Program" menu [#4398](https://github.com/sandboxie-plus/Sandboxie/issues/4398)



## [1.15.4 / 5.70.4] - 2024-12-19

### Added
- added template for Joplin [#4402](https://github.com/sandboxie-plus/Sandboxie/pull/4402) (thanks offhub)

### Fixed
- fixed dark mode for the INI editor
- fixed deleting sandbox content [#4407](https://github.com/sandboxie-plus/Sandboxie/pull/4407) (thanks bot-1450)
- fixed "run unsandboxed" no longer working from box picker window [#4403](https://github.com/sandboxie-plus/Sandboxie/issues/4403)
- fixed Firefox tab crashes when running with `ProtectHostImages=y` enabled [#4394](https://github.com/sandboxie-plus/Sandboxie/issues/4394)
  - only default installation locations are considered; Firefox-based browsers installed outside of these locations may still crash
  - to prevent this, manually add `DontCopy=<CustomInstallPath>` for custom installation paths to your configuration
- fixed BSoD "SYSTEM_SERVICE_EXCEPTION (3b)" when opening a DLL from AlertFolder using x64dbg [#4421](https://github.com/sandboxie-plus/Sandboxie/issues/4421)
- fixed BSoD "CRITICAL_PROCESS_DIED" when terminating all sandboxed programs [#1316](https://github.com/sandboxie-plus/Sandboxie/issues/1316)
  - Note: we now terminate boxed processes individually instead of using the job object, unless "TerminateJobObject=y" is set
- fixed INI editor font selection not working after addition of the highlighting feature [#4429](https://github.com/sandboxie-plus/Sandboxie/issues/4429)
- fixed BSoD issue with 'LogMessageEvents=y'



## [1.15.3 / 5.70.3] - 2024-11-28

### Added
- improved INI editor, it now supports search (hotkey: Ctrl+F)
- added SBIE1321 to log all force process events, can be enabled with "NotifyForceProcessEnabled=y", improves [#4113](https://github.com/sandboxie-plus/Sandboxie/issues/4113)
- added custom font configuration for Sandboxie Plus UI [#4397](https://github.com/sandboxie-plus/Sandboxie/pull/4397) (thanks habatake)

### Changed
- improved support notification
- improved deletion of Sandboxie Plus leftovers [#4374](https://github.com/sandboxie-plus/Sandboxie/pull/4374)

### Fixed
- fixed SSL issues on ARM64 platform that broke updater and certificate retrieval



## [1.15.2 / 5.70.2] - 2024-11-18

### Added
- added "NetworkAdapterMAC=0,AA-BB-CC-DD-EE-FF" to set MAC address for each box (thanks Yeyixiao)
- added "DiskSerialNumber=DeviceName,1234-ABCD" to set disk serial number for an individual box (thanks Yeyixiao)
- added the ability to hide certificates in the edit box in the global settings (idea by Yeyixiao)
- added opening a program in several sandboxes at once [#4231](https://github.com/sandboxie-plus/Sandboxie/issues/4231)
- added "Description" field inside the sandbox settings [#4243](https://github.com/sandboxie-plus/Sandboxie/issues/4243)
- added "NotifyMsiInstaller=y" enabled by default to display message SBIE2194 when an MSI installer is run in a box without the recommended exemptions [#4330](https://github.com/sandboxie-plus/Sandboxie/issues/4330)
  - SBIE2194: MSI installer requires 'MsiInstallerExemptions=y' option to be set in the INI to be able to work correctly, however this option weakens the isolation.
- added option to hide installed programs [#4139](https://github.com/sandboxie-plus/Sandboxie/issues/4139)
- added hide tray icon [#4075](https://github.com/sandboxie-plus/Sandboxie/issues/4075)
- added improved trace logging filtering [#4338](https://github.com/sandboxie-plus/Sandboxie/issues/4338)
- added EventLog monitoring for SbieMessages [#4113](https://github.com/sandboxie-plus/Sandboxie/issues/4113)
  - add 'LogMessageEvents=y' to the global settings to log all Sbie events to the system event log

### Changed
- validated compatibility with Windows build 27749 and updated DynData
- when running via drag and drop, now the app's parent folder is used as working directory [#4073](https://github.com/sandboxie-plus/Sandboxie/issues/4073)
- updated to Qt 5.15.16 with OpenSSL 3.4.0 [#4370](https://github.com/sandboxie-plus/Sandboxie/pull/4370) (thanks offhub)

### Fixed
- fixed .tmp file signing during Sandboxie Plus installation or update [#2643](https://github.com/sandboxie-plus/Sandboxie/issues/2643) [#4343](https://github.com/sandboxie-plus/Sandboxie/issues/4343)
- fixed issue with DLL unloading
- fixed "Files Resource Access" granting access to excluded folders [#4007](https://github.com/sandboxie-plus/Sandboxie/issues/4007)
- fixed "Run Outside Sandbox" disappearing from the run menu if "ForceDisableAdminOnly=y" is set [#4233](https://github.com/sandboxie-plus/Sandboxie/issues/4233)
- fixed deadlock on no-op condition when renaming file or folder [#4304](https://github.com/sandboxie-plus/Sandboxie/issues/4304)
- fixed moving sandboxed files or folders [#4329](https://github.com/sandboxie-plus/Sandboxie/issues/4329)
- fixed Sandboxie causing Firefox Nightly crashes [#4183](https://github.com/sandboxie-plus/Sandboxie/issues/4183)
- "Run Sandboxed" from the quick-previewer should have only one option [#4339](https://github.com/sandboxie-plus/Sandboxie/issues/4339)



## [1.15.1 / 5.70.1] - 2024-10-29

### Changed
- validated compatibility with Windows build 27729 and updated DynData
- updated Templates.ini to grant access to the Multimedia Class Scheduler Service [#4312](https://github.com/sandboxie-plus/Sandboxie/pull/4312) (thanks offhub)
- updated to Inno Setup 6.3.3 [#4020](https://github.com/sandboxie-plus/Sandboxie/issues/4020)

### Fixed
- fixed Sandboxie crypto fails to start in red boxes
- fixed issue with breakout process when using explorer.exe



## [1.15.0 / 5.70.0] - 2024-10-19

### Added
- added new user proxy mechanism to enable user specific operations
- added support for EFS using the user proxy [#1980](https://github.com/sandboxie-plus/Sandboxie/issues/1980)
  - to enable it, add 'EnableEFS=y' to the sandbox configuration (requires an advanced supporter certificate)
- added breakout document functionality [#2741](https://github.com/sandboxie-plus/Sandboxie/issues/2741)
  - use a syntax like this 'BreakoutDocument=C:\path\*.txt' to specify path and extension
  - Security Warning: do not use paths terminated with a wildcard like 'BreakoutDocument=C:\path\*' as they will allow for execution of malicious scripts outside the sandbox!
- added mechanism to set box folder ACLs to allow only the creating user access 'LockBoxToUser=y'
- added option to keep original ACLs on sandboxed files 'UseOriginalACLs=y'
- added option 'OpenWPADEndpoint=y' [#4292](https://github.com/sandboxie-plus/Sandboxie/issues/4292)

### Changed
- improved SandboxieCrypto startup
- improved sandboxed RPCSS startup
- updated to Qt 5.15.15 with OpenSSL 3.3.2 [#4223](https://github.com/sandboxie-plus/Sandboxie/pull/4223) (thanks offhub)
- set tab orders and buddies of UI controls [#4300](https://github.com/sandboxie-plus/Sandboxie/pull/4300) (thanks gexgd0419)

### Fixed
- fixed ImDiskApp uninstall key is always written to the registry [#4282](https://github.com/sandboxie-plus/Sandboxie/issues/4282)
- FIXED SECURITY ISSUE ID-24 by adding new ACLS handling [CVE-2024-49360](https://github.com/sandboxie-plus/Sandboxie/security/advisories/GHSA-4chj-3c28-gvmp)
  - Note: set 'LockBoxToUser=y' on multi user systems



## [1.14.10 / 5.69.10] - 2024-10-03

### Added
- added ability to import encrypted archive files directly [#4255](https://github.com/sandboxie-plus/Sandboxie/issues/4255)

### Changed
- when the SbieSvc.exe worker crashes it now can automatically be restarted

### Fixed
- fixed issue with sandbox path entry combo boxes
- fixed proxy for GetRawInputDeviceInfoW() causes a buffer overflow [#4267](https://github.com/sandboxie-plus/Sandboxie/issues/4267) (thanks marti4d)



## [1.14.9 / 5.69.9] - 2024-09-19

### Added
- added alternative default sandbox paths to the box wizard:
  - \\??\\%SystemDrive%\\Sandbox\\%USER%\\%SANDBOX%
  - \\??\\%SystemDrive%\\Sandbox\\%SANDBOX%
  - \\??\\%SystemDrive%\\Users\\%USER%\Sandbox\\%SANDBOX%
- added Sandbox Import dialog

### Changed
- sandbox root selection in global settings is now a combo box

### Fixed
- fixed exported encrypted archive files cannot be unpacked by Sandboxie [#4229](https://github.com/sandboxie-plus/Sandboxie/issues/4229)



## [1.14.8 / 5.69.8] - 2024-09-09

### Changed
- allow users to import/export boxes with .zip files [#4200](https://github.com/sandboxie-plus/Sandboxie/pull/4200)

### Fixed
- fixed a supporter certificate issue introduced with 1.14.7



## [1.14.7 / 5.69.7] - 2024-09-05

### Added
- added "RandomRegUID" (bool) which sets Windows Product ID in the registry to a random value
- added "HideDiskSerialNumber" (bool) return random value when applications try to get disk serial number
- added option to get free 10 days evaluation certificates from the support settings page
  - the evaluation certificates are node locked to the HwID and for each HwID up to 3 certificates can be requested
- added "TerminateWhenExit" (bool, in Sandboxie-Plus.ini) to terminate all processes when SandMan exits for [#4171](https://github.com/sandboxie-plus/Sandboxie/issues/4171)
- added a question box to ask for Sandbox Import Location for [#4169](https://github.com/sandboxie-plus/Sandboxie/issues/4169)
- added UI option to configure DropConHostIntegrity
- added "HideNetworkAdapterMAC" (bool) return random value when applications try to get network adapter MAC address
- added shared template selection to the Shared Template feature in the advanced options of the New Box Wizard [#4199](https://github.com/sandboxie-plus/Sandboxie/issues/4199)
  - the number of available shared templates has been increased to 10
  - to update the names displayed in the list, simply adjust the "Tmpl.Title" setting within each template

### Fixed
- fixed and improved HideDiskSerialNumber option causes applications to crash [#4185](https://github.com/sandboxie-plus/Sandboxie/issues/4185)
- fixed encrypted proxy password was improperly formatted [#4197](https://github.com/sandboxie-plus/Sandboxie/issues/4197)
- fixed NtQueryDirectoryObject (should not return "STATUS_MORE_ENTRIES") as this is an easy sandbox detection [#4201](https://github.com/sandboxie-plus/Sandboxie/issues/4201)



## [1.14.6 / 5.69.6] - 2024-07-30

### Added
- added alias for sandboxes [#4112](https://github.com/sandboxie-plus/Sandboxie/issues/4112)

### Fixed
- fixed CustomLCID issue under Windows 7 [#4117](https://github.com/sandboxie-plus/Sandboxie/issues/4117)
- fixed settings window issue with non-advanced certificates introduced in 1.14.0
- fixed issue with API_PROCESS_EXEMPTION_CONTROL



## [1.14.5 / 5.69.5] - 2024-07-23

### Added
- added HwID display
- added language spoofing "CustomLCID=1033" [#4024](https://github.com/sandboxie-plus/Sandboxie/pull/4024) (thanks Yeyixiao)
- added option to always run the SandMan UI as admin [#4090](https://github.com/sandboxie-plus/Sandboxie/issues/4090)
- added proxy exclusion [#4036](https://github.com/sandboxie-plus/Sandboxie/issues/4036)
- added "ForceChildren=Program.exe" [#4070](https://github.com/sandboxie-plus/Sandboxie/issues/4070)
- added UI options for "ForceRestartAll" and "UseCreateToken" in OptionWindow
- added an optional context menu option to make folder/file forced quickly
  - Note: you can also use "SandMan.exe /add_force program_path" to do it

### Changed
- the certificate format can now take an explicit validity days specification, needed for gapless certificate renewal

### Fixed
- fixed multiple supporter certificate pop-ups if sandbox settings are opened [#4074](https://github.com/sandboxie-plus/Sandboxie/issues/4074)
- fixed issue with HwID-bound serial keys failing when no HwID could be obtained
- fixed issue with "UseChangeSpeed=y"
- fixed broken "HideFirmwareInfo=y" implementation
  - changed reg path to key "HKCU\\System\\SbieCustom", value: "SMBiosTable"
  - added UI options
- fixed schannel error SEC_E_SECPKG_NOT_FOUND in encrypted sandboxes [#4081](https://github.com/sandboxie-plus/Sandboxie/issues/4081)
- fixed "The name of the sandbox is too long", causing an error in Sbie2327 [#4064](https://github.com/sandboxie-plus/Sandboxie/issues/4064)
- fixed memory limit for job objects, which can now exceed 4GB [#4096](https://github.com/sandboxie-plus/Sandboxie/issues/4096)



## [1.14.4 / 5.69.4] - 2024-07-13

### Changed
- improved removal of leftovers [#4050](https://github.com/sandboxie-plus/Sandboxie/pull/4050)

### Fixed
- fixed the Start Restrictions tab layout breaking [#4045](https://github.com/sandboxie-plus/Sandboxie/issues/4045)
- fixed administrators unable to change the sandbox configuration [#4057](https://github.com/sandboxie-plus/Sandboxie/issues/4057) [#4068](https://github.com/sandboxie-plus/Sandboxie/issues/4068)



## [1.14.3 / 5.69.3] - 2024-07-01

### Changed
- updated to Qt 5.15.14 with OpenSSL 3.3.1 [#3994](https://github.com/sandboxie-plus/Sandboxie/pull/3994) (thanks offhub)

### Fixed
- fixed applications unable to launch as admin in a sandbox with "UseCreateToken/SandboxieAllGroup" when using an MSFT account [#4022](https://github.com/sandboxie-plus/Sandboxie/issues/4022)
- fixed Firefox issue with Sbie 1.14.1 and 1.14.2 [#4012](https://github.com/sandboxie-plus/Sandboxie/issues/4012)
  - rolled back the driver verifier fix added in 1.14.1
- fixed CustomChromiumFlags and --single-argument issue [#4033](https://github.com/sandboxie-plus/Sandboxie/issues/4033)
- fixed Sandboxie programs not terminating after closing programs that run as admin with UseCreateToken/SandboxieAllGroup enabled [#4030](https://github.com/sandboxie-plus/Sandboxie/issues/4030)



## [1.14.2 / 5.69.2] - 2024-06-19

### Added
- added SbieIni option to modify password-protected configurations [#3903](https://github.com/sandboxie-plus/Sandboxie/issues/3903)
  - Usage: set|append|insert|delete [/passwd:********] <section> <setting> <value>
  - Note: use /passwd without the password to have SbieIni prompt for the password on the console, this hides the password from view and prevents capture with the command line
- added checkbox for "PromptForInternetAccess" option to the New Box Wizard
- added option "HideNonSystemProcesses" to hide processes not in a sandbox from processes lists for sandboxed processes
- added option "HideSbieProcesses" to hide Sandboxie Work Process (SbieSvc, SandboxieRpcSs etc.)
- added option "HideFirmwareInfo"
  - when it is set, the programs that try getting firmware information will get false data from HKEY_CURRENT_USER\\SOFTWARE\\SandboxieHide\\FalseFirmwareValue
- added template "BlockAccessWMI" to prevent sandboxed processes from accessing system information through WMI
- added template "BlockLocalConnect" to prevent sandboxed processes from sending network packets to localhost to breakout sandbox
- added new option "AllowCoverTaskbar" for [#3975](https://github.com/sandboxie-plus/Sandboxie/issues/3975)
- added RPC Port message filter mechanism to block unsafe RDP calls via the driver [#3930](https://github.com/sandboxie-plus/Sandboxie/issues/3930)
  - Usage: "RpcPortFilter=Port,ID,Label" label is optional
- added "Job Object" Options page to collect all job object-related options

### Changed
- extended "Temp Template" to make it could delete local template section

### Fixed
- fixed security issue with the newly introduced experimental "UseCreateToken=y" mechanism
- fixed issue with "UseCreateToken=y" when using an MSFT online account
- fixed export sandbox function not including hidden files [#3980](https://github.com/sandboxie-plus/Sandboxie/issues/3980) (thanks L4cache)
- fixed Chrome printer stoppage [#3926](https://github.com/sandboxie-plus/Sandboxie/issues/3926)
  - Sandboxie will add CustomChromiumFlags=--disable-features=PrintCompositorLPAC to Chromium-based browsers command line
  - Note: Less Privileged App Container (LPAC) aren't working with Sandboxie presently
- fixed accessing relative symlinks which refer to a target that starts with a dot [#3981](https://github.com/sandboxie-plus/Sandboxie/issues/3981)
- fixed opening a sandbox's properties window via double-click in the systray context window [#3861](https://github.com/sandboxie-plus/Sandboxie/issues/3861)
- fixed delay in launching forced programs after version 1.12.9 [#3868](https://github.com/sandboxie-plus/Sandboxie/issues/3868)
  - this issue was introduced in 1.13.0 and may have affected other use cases causing various issues
- fixed issue with Misc Options list
- improved compatibility with Steam running sandboxed
- fixed compatibility issue with FakeAdminRights [#3989](https://github.com/sandboxie-plus/Sandboxie/pull/3989) (thanks offhub)



## [1.14.1 / 5.69.1] - 2024-06-06

### Added
- added "Sandboxie\All Sandboxes" SID into token with SandboxieLogon [#3191](https://github.com/sandboxie-plus/Sandboxie/issues/3191)
  - to use this feature "SandboxieAllGroup=y" must be enabled
  - Note: this fundamentally changes the mechanism Sbie uses for token creation, the new mechanism can be enabled separately with "UseCreateToken=y"
- added "EditAdminOnly=y" can now be configured per box
- added UI for CoverBoxedWindows in NewBoxWizard
- added UI option to start unsandboxed process but force child processes in SelectBoxWindow
- added option "AlertBeforeStart"
  - when it is set, a prompt pops up before launching a new program into the sandbox using "Start.exe" and checks if the program that started "Start.exe" is a Sandboxie component itself, if it is not, a warning pops up
- added option for EditAdminOnly in SetupWizard

### Changed
- split the advanced new box wizard to stretch over two pages
- reorganized box options

### Fixed
- fixed issue with proxy authentication setting
- fixed memory leak in SbieSvc
- fixed issue with inconsistent WFP option application [#3900](https://github.com/sandboxie-plus/Sandboxie/issues/3900)
- fixed resource leak in buffer hashing function
- fixed DLL name corruption when BlockInterferenceControl is enabled [#3945](https://github.com/sandboxie-plus/Sandboxie/issues/3945)
- fixed issue with driver verifier



## [1.14.0 / 5.69.0] - 2024-05-17

### Added
- added option to limit the memory of sandboxed processes and the number of processes in a single sandbox through job object (thanks Yeyixiao)
  - use "TotalMemoryLimit" (number, in bytes) to set the overall memory limit for the sandbox, and "ProcessMemoryLimit" (number, in bytes) to limit memory for individual processes
  - use "ProcessNumberLimit" (number) to set process number limit
- added ability to adjust the logic speed of sandboxed processes, including reduced fixed latency and modified single-player speed (thanks Yeyixiao)
  - Note: you can set "UseChangeSpeed=y" to configure the following options: "AddTickSpeed", "AddSleepSpeed", "AddTimerSpeed", "LowTickSpeed", "LowSleepSpeed" and "LowTimerSpeed" (integer values only)
  - Note: these options use multiples instead of adding or subtracting; the "Add" series is configured by multiplication, while the "Low" series by division
  - Note: when set to "AddSleepSpeed=0", all sleep function calls will be skipped. For example, you can bypass fixed delay code in hidden malware, reducing analysis time without affecting essential operations, which is useful for virus analysts
- added /fcp /force_children command line option to Start.exe; it allows to start a program unsandboxed but have all its children sandboxed
- added ability to force sandboxed processes to use a pre-defined SOCKS5 proxy
- added ability to intercept DNS queries so that they can be logged and/or redirected
- added support for SOCKS5 proxy authentication based on RFC1928 (thanks Deezzir)
- added test dialog UI for SOCKS5 proxy (thanks Deezzir)
- added ability to automatically remove template references that begin with "Template_Temp_"

### Changed
- validated compatibility with Windows build 26217 and updated DynData

### Fixed
- fixed an issue with an early batch of Large Supporter certificates



## [1.13.7 / 5.68.7] - 2024-05-01

### Added
- added file version information for SbieDll.dll and SbieSvc.exe in the Sandboxie Plus about dialog

### Changed
- improved checkboxes about DropAdminRights in SandMan [#3851](https://github.com/sandboxie-plus/Sandboxie/pull/3851) (thanks offhub)

### Fixed
- fixed symbolic linking of files [#3852](https://github.com/sandboxie-plus/Sandboxie/issues/3852)
- fixed issue with start agent option [#3844](https://github.com/sandboxie-plus/Sandboxie/pull/3844) (thanks offhub)
- fixed issue with Delete V2 introduced in 1.13.5



## [1.13.6 / 5.68.6] - 2024-04-21

### Added
- added "BlockInterferenceControl=y" option to prevent sandboxed processes from forcing windows on top and moving the mouse pointer (thanks Yeyixiao)
  - Note: this option may cause issues in games hence it's not recommended for gaming boxes
- added support for hard links [#3826](https://github.com/sandboxie-plus/Sandboxie/issues/3826)
- added mechanism to terminate stuck sandboxed processes from the driver
- added editable trigger list [#3742](https://github.com/sandboxie-plus/Sandboxie/issues/3742)
- added optional extension of the screenshot protection to the UI [#3739](https://github.com/sandboxie-plus/Sandboxie/issues/3739)
- added a button to edit local/custom templates [#3738](https://github.com/sandboxie-plus/Sandboxie/issues/3738)
- added adjustable resizing of the "Run Sandboxed" window [#3697](https://github.com/sandboxie-plus/Sandboxie/issues/3697)
- added Notepad++ template [#3836](https://github.com/sandboxie-plus/Sandboxie/pull/3836) (thanks offhub)

### Changed
- improved Avast template [#3824](https://github.com/sandboxie-plus/Sandboxie/pull/3824) (thanks offhub)
- renamed a bunch of experimental options and marked them as experimental in the UI
  - "IsBlockCapture=y" -> "BlockScreenCapture=y"
  - "IsProtectScreen=>" -> "CoverBoxedWindows=y"

### Fixed
- fixed inactive apply button when changing BlockDNS or BlockPorts options [#3807](https://github.com/sandboxie-plus/Sandboxie/issues/3807)
- fixed troubleshooting wizard breaking with new Qt [#3810](https://github.com/sandboxie-plus/Sandboxie/discussions/3810)
- fixed Settings dialog now showing the correct RAM drive letter
- fixed broken updater due to missing SSL support in the latest Qt build [#3810](https://github.com/sandboxie-plus/Sandboxie/discussions/3810)
- fixed Enabling "DropAdminRights/FakeAdminRights" adds "BlockInterferePower and ForceProtectionOnMount" to the INI [#3825](https://github.com/sandboxie-plus/Sandboxie/issues/3825)
- fixed KeePass "Out of Memory" crash due to "BlockScreenCapture=y" [#3768](https://github.com/sandboxie-plus/Sandboxie/issues/3768)
- fixed Sandboxie 1.13.4 with IsBlockCapture=y not working on Windows 7 [#3769](https://github.com/sandboxie-plus/Sandboxie/issues/3769)
- fixed explorer.exe issue "FakeAdminRights=y" [#3638](https://github.com/sandboxie-plus/Sandboxie/issues/3638)
- fixed forced folder warning notification [#3569](https://github.com/sandboxie-plus/Sandboxie/issues/3569)



## [1.13.5 / 5.68.5] - 2024-04-10

### Added
- added ability to set all processes to a certain core per box [#3276](https://github.com/sandboxie-plus/Sandboxie/issues/3276)
  - set "CpuAffinityMask=0x00000001" in Sandboxie.ini, where 0x00000001 is a bit mask indicating which cores are to be used
  - only supports cores 0-31, for 32+ will be always disabled when this option is used
- added checkbox for Samba and DNS port blocking
- added Weasel template [#3806](https://github.com/sandboxie-plus/Sandboxie/pull/3806) (thanks xWTF)

### Changed
- on systems in test signing mode, Sandboxie will try outdated offsets by default
- updated to Qt 5.15.13 with latest security patches [#3694](https://github.com/sandboxie-plus/Sandboxie/pull/3694) (thanks LumitoLuma)
- moved network restrictions from general restrictions to their own tab on the network page
- improved certificate retrieval UI messages
- improved MPC-BE template [#3798](https://github.com/sandboxie-plus/Sandboxie/pull/3798) (thanks offhub)

### Fixed
 - fixed Virtualization scheme Version 2 causing extremely slow file deletion speeds [#3650](https://github.com/sandboxie-plus/Sandboxie/issues/3650)

### Removed
- removed obsolete recommendations in Templates.ini [#3802](https://github.com/sandboxie-plus/Sandboxie/pull/3802)



## [1.13.4 / 5.68.4] - 2024-03-25

### Added
- added option to prevent sandboxed processes from accessing the images of the window outside the sandbox [#1985](https://github.com/sandboxie-plus/Sandboxie/issues/1985) (thanks Yeyixiao)
  - it can be enabled with "IsBlockCapture=y"
  - see the sandbox option "Prevent sandboxed processes from using public methods to capture window images" in SandMan UI
- added "LingerExemptWnds=n" to make the lingering process monitor mechanism no longer exempt lingering processes with windows from termination
- added option 'SharedTemplate' to Box Wizard [#3737](https://github.com/sandboxie-plus/Sandboxie/pull/3737) (thanks offhub)
- added an option to force the protection of an encrypted sandbox to be enabled [#3736](https://github.com/sandboxie-plus/Sandboxie/pull/3736) (thanks Yeyixiao)
- added a menu and button/icon to suspend all processes [#3741] (https://github.com/sandboxie-plus/Sandboxie/issues/3741)

### Changed
- option "LingerLeniency=n" now also disables the 5 second grace period for freshly started lingerers [#1892](https://github.com/sandboxie-plus/Sandboxie/issues/1892)

### Fixed
- fixed issue with symlinks related to start menu folders



## [1.13.3 / 5.68.3] - 2024-03-16

### Added
- added certificate usage guide link to support page

### Fixed
- fixed issues with "IsProtectScreen=y" [3656](https://github.com/sandboxie-plus/Sandboxie/pull/3656#discussion_r1518549704)
- fixed issue with hotkeys and changed the default 'suspend all' hotkey to Shift+Alt+Pause
- fixed issue with suspended state not being updated when the global hotkey was used
- fixed issue with new ** pattern failing in some cases



## [1.13.2 / 5.68.2] - 2024-03-07

### Added
- added menu entry to restart SandMan as admin [#3581](https://github.com/sandboxie-plus/Sandboxie/issues/3581) (thanks Yeyixiao)
- added option to block taking screen capture/screenshot of sandboxed processes (thanks Yeyixiao)
  - can be enabled with "IsProtectScreen=y"
  - see the sandbox option "Prevent processes from capturing window images from sandboxed windows" in SandMan UI
- added option to prevent sandboxed processes from interfering with power operations [#3640](https://github.com/sandboxie-plus/Sandboxie/issues/3640) (thanks Yeyixiao)
  - can be enabled with "BlockInterferePower=y"
  - see the sandbox option "Prevent sandboxed processes from interfering with power operations" in SandMan UI
- added new pattern mechanism using a `**` as a placeholder to indicate an arbitrary path element not containing a `\` [1ff2867](https://github.com/sandboxie-plus/Sandboxie/commit/1ff28679b0557a642d29f1651a90a8f7fec77171)

### Changed
- reworked option for suspending all processes in SandMan (introduced in 1.13.1) [#3582](https://github.com/sandboxie-plus/Sandboxie/issues/3582#issuecomment-1969628215)

### Fixed
- fixed privacy mode, NormalFilePath and symbolic link issue [#3660](https://github.com/sandboxie-plus/Sandboxie/issues/3660)
- fixed access to recycle bin in an Application Compartment sandbox with data protection [#3665](https://github.com/sandboxie-plus/Sandboxie/issues/3665)



## [1.13.1 / 5.68.1] - 2024-02-27

### Added
- added option for suspending all processes in SandMan [#3582](https://github.com/sandboxie-plus/Sandboxie/issues/3582) (thanks Yeyixiao)
- added "On Terminate" trigger [#3584](https://github.com/sandboxie-plus/Sandboxie/issues/3584) (thanks Yeyixiao)

### Changed
- changed DynData format to add flags
- reverted the new sandbox directory structure for volumes without drive letters [#3632](https://github.com/sandboxie-plus/Sandboxie/issues/3632)
  - GUID usage can be re-enabled with "UseVolumeGuidWhenNoLetter=y"

### Fixed
- added missing checkbox for API tracing
- fixed incompatibility with Windows ARM64 Insider build 26052 and later
- fixed symlink issue [#3537](https://github.com/sandboxie-plus/Sandboxie/issues/3537#issuecomment-1954496527)
- fixed file redirection issue in an Application Compartment sandbox [#3637](https://github.com/sandboxie-plus/Sandboxie/issues/3637)
- fixed issues with compartment mode compatibility fallback
- fixed missing maximum password length check [#3639](https://github.com/sandboxie-plus/Sandboxie/issues/3639)
- fixed issue with launching executables from volumes without a drive letter in a sandbox on Windows 1803 and earlier [#3627](https://github.com/sandboxie-plus/Sandboxie/issues/3627)

### Removed
- removed UseNewSymlinkResolver setting, as the new mechanism is always used



## [1.13.0 / 5.68.0] - 2024-02-10

### Added
- added advanced API trace functionality

### Changed
- reworked SCM hooking to improve Windows 10 compatibility
- reworked offset dependent handling of undocumented Windows kernel objects
  - the required offsets can be now updated independently from the driver
  - the DynData blob is digitally signed, when in test signing mode the signature is however ignored
  - when Sandboxie encounters a yet unsupported kernel build, token based isolation is disabled to prevent system instability
  - this safety mechanism is disabled on systems participating in the Windows Insider program
  - for systems in the Insider program, the latest known offsets are tried
- reworked part of the low level code injection mechanism to add compatibility with Windows Insider build 26040 and later
- enabled CET Shadow Stack compatible flag for core Sandboxie binaries

### Fixed
- fixed incompatibility with Windows Insider build 26040 and later

### Removed
- cleaned up code and removed obsolete VC 6.0 workarounds



## [1.12.9 / 5.67.9] - 2024-02-06

### Fixed
- fixed issue with symlink resolver introduced in the previous build [#3481](https://github.com/sandboxie-plus/Sandboxie/issues/3481)



## [1.12.8 / 5.67.8] - 2024-01-31

### Added
- added Japanese language on Plus UI [#3573](https://github.com/sandboxie-plus/Sandboxie/pull/3573)

### Fixed
- fixed warning issue: `SBIE2321 Cannot manage device map: C0000034 / 11` [#2996](https://github.com/sandboxie-plus/Sandboxie/issues/2996)
- fixed issue with explorer.exe that could not run on emulate admin sandbox [#3516](https://github.com/sandboxie-plus/Sandboxie/issues/3516)
- fixed potential BSoD issue with WFP when trace logging is enabled [#2471](https://github.com/sandboxie-plus/Sandboxie/issues/2471)
- fixed issue with running programs pinned to the Run menu that did not use the same working directory [#3555](https://github.com/sandboxie-plus/Sandboxie/issues/3555)
- fixed UseNewSymlinkResolver causing applications to create both the link and the target folder [#3481](https://github.com/sandboxie-plus/Sandboxie/issues/3481)
- fixed Plus UI notification window becoming stuck when a modal dialog is displayed [#3586](https://github.com/sandboxie-plus/Sandboxie/issues/3586)



## [1.12.7 / 5.67.7] - 2024-01-10

### Fixed
- fixed "Duplicate Box Config" option not being able to keep the order of the original sandbox configuration [#3523](https://github.com/sandboxie-plus/Sandboxie/issues/3523)
- fixed "Save options as new defaults" option not appearing when "Configure Advanced Options" is enabled [#3522](https://github.com/sandboxie-plus/Sandboxie/issues/3522)
- fixed Windows Explorer issue when running in a sandbox with data protection enabled on Windows 11 [#3517](https://github.com/sandboxie-plus/Sandboxie/issues/3517)
  - the default template for privacy boxes now sets `NormalFilePath=%ProgramData%\Microsoft\*`
- fixed message boxes with MB_DEFAULT_DESKTOP_ONLY or MB_SERVICE_NOTIFICATION not being able to display title and text correctly in Security Hardened sandboxes [#3529](https://github.com/sandboxie-plus/Sandboxie/issues/3529)
- fixed issue with npm failing to run inside a Security Hardened sandbox on Windows 11 [#3505](https://github.com/sandboxie-plus/Sandboxie/issues/3505)
- fixed symlink and open path issue introduced in 1.12.6 [#3537](https://github.com/sandboxie-plus/Sandboxie/issues/3537)
  - when a volume without an associated drive letter is encountered, Sandboxie uses `\drive\{guid}` instead of `\drive\[letter]`
  - Note: if the volume is subsequently assigned a drive letter, the data under `\drive\{guid}` will be ignored!
- fixed "Run Unsandboxed" command when right-clicking shortcuts created with Sandboxie Plus [#3528](https://github.com/sandboxie-plus/Sandboxie/issues/3528)
  - Note: for the fix to take full effect, the shell integration needs to be re-applied
- fixed `Error Status: 0x0000065b (Function failed during execution)` [#3504](https://github.com/sandboxie-plus/Sandboxie/issues/3504)
- fixed Privacy Enhanced sandboxes failing with error code SBIE2204 [#3542](https://github.com/sandboxie-plus/Sandboxie/issues/3542)
- fixed OpenFilePath directories not being enumerated within their parent folders [#3519](https://github.com/sandboxie-plus/Sandboxie/issues/3519)



## [1.12.6 / 5.67.6] - 2024-01-02

### Changed
- improved behaviour of the toolbar customization menu

### Fixed
- fixed issue introduced in 1.12.4 with Start.exe failing to run in an encrypted box [#3514](https://github.com/sandboxie-plus/Sandboxie/issues/3514)
- fixed "The directory name is invalid" when starting a process in an encrypted private box [#3475](https://github.com/sandboxie-plus/Sandboxie/issues/3475)
- fixed symbolic links created inside a sandbox not working properly [#3181](https://github.com/sandboxie-plus/Sandboxie/issues/3181)
- fixed issue with drives mounted to multiple folders or a drive letter and a folder
- fixed issue with file paths when using sandboxes with relocated root (e.g. to an ImDisk volume) [#3506](https://github.com/sandboxie-plus/Sandboxie/issues/3506)
- fixed issue with explorer.exe on Windows 11 when using "SysCallLockDown=y" [#3516](https://github.com/sandboxie-plus/Sandboxie/issues/3516)
- fixed SandMan not showing icons of processes located on an ImDisk volume



## [1.12.5 / 5.67.5] - 2023-12-19

### Fixed
- fixed bug with Chromium-based browsers affecting "--type=crashpad-handler" instances [#post-3177139](https://www.wilderssecurity.com/threads/sandboxie-plus-1-12-5.453131/#post-3177139)



## [1.12.4 / 5.67.4] - 2023-12-18

### Added
- added the display of date and time in the Sbie Messages tab [#3429](https://github.com/sandboxie-plus/Sandboxie/issues/3429)

### Changed
- without an active supporter certificate, automatic updates are no longer available for the stable channel
  - the automatic updater will still notify about new stable releases and the user will be guided to the download page to download the latest installer manually
- the clean-up button in SandMan UI is now enabled even when not connected to the Sandboxie core components
- the box creation wizard now allows to create encrypted sandboxes based on any other box type

### Fixed
- fixed running sandboxed processes located in an ImDisk volume [#3472](https://github.com/sandboxie-plus/Sandboxie/discussions/3472)
- fixed sample 634d066fd4f9a8b201a3ddf346e880be unable to be terminated on Windows 7 x64 [#3482](https://github.com/sandboxie-plus/Sandboxie/issues/3482)
- fixed UseNewSymlinkResolver causing applications to create both the link and the target folder [#3481](https://github.com/sandboxie-plus/Sandboxie/issues/3481)
- fixed issue with sandbox renaming that broke the group hierarchy [#3430](https://github.com/sandboxie-plus/Sandboxie/issues/3430)
- fixed encrypted sandbox with red box preset blocking access to its root directories [#3475](https://github.com/sandboxie-plus/Sandboxie/issues/3475)
- fixed SandMan crash issue introduced in 1.12.0 [#3492](https://github.com/sandboxie-plus/Sandboxie/issues/3492)



## [1.12.3 / 5.67.3] - 2023-12-02

### Added
- added template providing useful exclusions for confidential boxes

### Fixed
- FIXED SECURITY ISSUE ID-23: SeManageVolumePrivilege is now blocked, as it allowed to read MFT data (thanks Diversenok)
- fixed program launch when forcing processes into a confidential box [#3173](https://github.com/sandboxie-plus/Sandboxie/issues/3173)



## [1.12.2 / 5.67.2] - 2023-11-28

### Added
- added options dialog when exporting a box [#3409](https://github.com/sandboxie-plus/Sandboxie/issues/3409)

### Changed
- moved process info retrieval to SbieSvc, resolves some information not being available in compartment type boxes when SandMan does not run as admin
- moved Process Suspend/Resume to SbieSvc [#3156](https://github.com/sandboxie-plus/Sandboxie/issues/3156)

### Fixed
- fixed issue with Microsoft Edge when using AutoDelete option [#post-3173507](https://www.wilderssecurity.com/threads/sandboxie-plus-v1-12-1-pre-release.452939/#post-3173507)
- fixed warning issue with Acrobat Reader: `SBIE2205 Service not implemented: CredEnumerateA` [#issuecomment-1826280016](https://github.com/sandboxie-plus/Sandboxie/issues/3441#issuecomment-1826280016)
- fixed UNEXPECTED_KERNEL_MODE_TRAP BSoD when opening any web link from sandboxed Microsoft 365 app (e.g. Outlook, Word) [#3427](https://github.com/sandboxie-plus/Sandboxie/issues/3427)
- fixed issue with force process warning message
- fixed online updater not respecting portable mode [#3406](https://github.com/sandboxie-plus/Sandboxie/issues/3406)
- fixed snapshot feature not working on encrypted boxes [#3439](https://github.com/sandboxie-plus/Sandboxie/issues/3439)



## [1.12.1 / 5.67.1] - 2023-11-23

### Changed
- improved open URL dialog [#3401](https://github.com/sandboxie-plus/Sandboxie/issues/3401)
- improved suspended process detection

### Fixed
- fixed issue with hotkey to bring SandMan always-on-top
- fixed issue with ThreadSuspendCount failing for already terminated threads [25054d0](https://github.com/sandboxie-plus/Sandboxie/commit/25054d0b2bd34c12471eefdf6ba9550e50ef02e5) [#3375](https://github.com/sandboxie-plus/Sandboxie/issues/3375)
- fixed message text [#3408](https://github.com/sandboxie-plus/Sandboxie/issues/3408)
- fixed warning issue with Firefox 120.0: `SBIE2205 Service not implemented: CredWriteA` [#3441](https://github.com/sandboxie-plus/Sandboxie/issues/3441)
- fixed warning issue with Firefox 120.0: `SBIE2205 Service not implemented: CredReadA` [#3453](https://github.com/sandboxie-plus/Sandboxie/issues/3453)
- fixed outdated Chromium paths [#3436](https://github.com/sandboxie-plus/Sandboxie/pull/3436)



## [1.12.0 / 5.67.0] - 2023-10-25

### Added
- added mechanism to automatically set USB drives as forced folders (requires a supporter certificate)
- added troubleshooting script for parental controls [#3318](https://github.com/sandboxie-plus/Sandboxie/issues/3318)
- started German translation of the troubleshooting scripts ...\SandboxiePlus\SandMan\Troubleshooting\lang_de.json [#3293](https://github.com/sandboxie-plus/Sandboxie/issues/3293)
- added "get_cert SBIEX-XXXXX-XXXXX-XXXXX-XXXXX" command to UpdUtil.exe allowing to receive a certificate by serial number using command line
- added mechanism to revoke leaked or refunded certificates
- added new global hotkey to bring SandMan in front as top most ALT+Break [#3320](https://github.com/sandboxie-plus/Sandboxie/issues/3320)
- added option to exclude specific boxes from 'Terminate all processes' [#3108](https://github.com/sandboxie-plus/Sandboxie/issues/3108)
  - Note: press the panic button hotkey 3 times with less than 1 second between clicks to Terminate All without exceptions
- added customizable global hotkey that toggles "pause forced programs" [#2441](https://github.com/sandboxie-plus/Sandboxie/issues/2441)
- added warning to prevent broad "forced folder" settings [#650](https://github.com/sandboxie-plus/Sandboxie/issues/650)
- added CheckExpectFile function to Sandboxie Plus [#768](https://github.com/sandboxie-plus/Sandboxie/issues/768)

### Changed
- improved suspend process handling [#3375](https://github.com/sandboxie-plus/Sandboxie/issues/3375)
- improved handling of issue reports
- updated reminder schedule

### Fixed
- fixed issue with auto updater not offering version updates
- fixed issue with new symlink handling code [#3340](https://github.com/sandboxie-plus/Sandboxie/issues/3340)
- fixed issue with Scm_StartServiceCtrlDispatcherX not behaving correctly when not run as service [#1246](https://github.com/sandboxie-plus/Sandboxie/issues/1246) [#3297](https://github.com/sandboxie-plus/Sandboxie/issues/3297)
- fixed issue with configuring the original folder of a symbolic link created using mklink to OpenPipePath [#3207](https://github.com/sandboxie-plus/Sandboxie/issues/3207)

### Removed
- removed obsolete /nosbiectrl switch [#3391](https://github.com/sandboxie-plus/Sandboxie/issues/3391)



## [1.11.4 / 5.66.4] - 2023-10-06

### Changed
- improved behaviour with Lock All Encrypted Boxes option [#3350](https://github.com/sandboxie-plus/Sandboxie/issues/3350)
- fixed typo: 'NoRestartOnPAC' option becomes 'NoRestartOnPCA' [0e175ee](https://github.com/sandboxie-plus/Sandboxie/commit/0e175eedfde50198a3fa7bdef3a91fdbad5f5226)

### Fixed
- fixed issue with unmounting protected images [#3347](https://github.com/sandboxie-plus/Sandboxie/issues/3347)
- fixed issue with highlighting not being able to be turned off in the Sbie Messages tab [#3338](https://github.com/sandboxie-plus/Sandboxie/issues/3338)
- fixed issue with programs installed in the sandbox not being displayed via Run from Start Menu [#3334](https://github.com/sandboxie-plus/Sandboxie/issues/3334)
- fixed issue with NtQueryDirectoryObject [#3310](https://github.com/sandboxie-plus/Sandboxie/issues/3310)
- fixed some GUI functions using the wrong box path if the FileRootPath value starts with Volume{GUID} [#3345](https://github.com/sandboxie-plus/Sandboxie/issues/3345)
- fixed programs not running when they are kept in RAM [#3349](https://github.com/sandboxie-plus/Sandboxie/issues/3349)
- fixed Process Restrictions toggle not working correctly [#3355](https://github.com/sandboxie-plus/Sandboxie/issues/3355)



## [1.11.3 / 5.66.3] - 2023-09-27

### Added
- added container header backup/restore options to the Sandbox Options window [#post-3165085](https://www.wilderssecurity.com/threads/sandboxie-plus-v1-11-3.452487/#post-3165085)

### Changed
- updated to 7-Zip 23.01 [4ee1464](https://github.com/sandboxie-plus/Sandboxie/commit/4ee146430f70c91917fbcbfb77909b5b0b84a78c)

### Fixed
- fixed incorrect text display when changing the password of encrypted boxes or when exporting encrypted boxes [#3296](https://github.com/sandboxie-plus/Sandboxie/pull/3296)
- fixed image files not being created as sparse files



## [1.11.2 / 5.66.2] - 2023-09-09

### Added
- added update warning if a personal certificate expires
- added an option to terminate the UI by clicking the close button [#3253](https://github.com/sandboxie-plus/Sandboxie/issues/3253)
- added an option to restore a Sandboxie window which has been minimized to the system tray with a single click [#3251](https://github.com/sandboxie-plus/Sandboxie/issues/3251)
- added an option to minimize Sandboxie to the system tray by clicking minimize button [#3252](https://github.com/sandboxie-plus/Sandboxie/issues/3252)
- added an independent desktop integration setting [#3246](https://github.com/sandboxie-plus/Sandboxie/issues/3246)
- added template preset for AdGuard Browser Assistant [#3283](https://github.com/sandboxie-plus/Sandboxie/pull/3283)

### Changed
- moved tray options to their own tab
- replaced some graphics with better compressed ones (thanks idealths) [1af82bb](https://github.com/sandboxie-plus/Sandboxie/commit/1af82bb39643e7470dd4839bf3cebe8a4acf9d06)
- changed the /mount_protected command line switch to be checked before the /mount command line switch [#3289](https://github.com/sandboxie-plus/Sandboxie/pull/3289)

### Fixed
- fixed subscription certificate recognition issue
- fixed logo cut-off in the About window [#3249](https://github.com/sandboxie-plus/Sandboxie/issues/3249)
- fixed issue with file recovery when using a RAM drive [d82b62e](https://github.com/sandboxie-plus/Sandboxie/commit/d82b62ee78d865e21005b9b81dfa9dac9f524b90)



## [1.11.1 / 5.66.1] - 2023-08-31

### Added
- added 'RamDiskLetter=R:\' option allowing to mount the RAM drive root to a drive letter [938e0a8](https://github.com/sandboxie-plus/Sandboxie/commit/938e0a8c8d88e3780ece674c6702654d0b4e6ddc)

### Changed
- changed the new option layout to be the default for non-vintage views (can be changed back in the settings) [94c3f5e](https://github.com/sandboxie-plus/Sandboxie/commit/94c3f5e35bf9e7c993557f2c9d4e6e5129e9d1df)

### Fixed
- fixed issue when re-creating a RAM sandbox junction [2542351](https://github.com/sandboxie-plus/Sandboxie/commit/254235136fa8b74ad147f03b646d4015208c14be)
- fixed Sandboxie logo scaling during the setup wizard [#3227](https://github.com/sandboxie-plus/Sandboxie/issues/3227)
- fixed text cut-off in box creation wizard [#3226](https://github.com/sandboxie-plus/Sandboxie/issues/3226)
- fixed Windows 7 compatibility issue with ImBox.exe [1f0b2b7](https://github.com/sandboxie-plus/Sandboxie/commit/1f0b2b71ba47436252fd55eece2c3624085b46dc)
- fixed a bug where InjectDll/InjectDll64 settings were not correctly added to the sandbox configuration [#3254](https://github.com/sandboxie-plus/Sandboxie/pull/3254)
- fixed issue with 'UseNewSymlinkResolver=y' [01a2670](https://github.com/sandboxie-plus/Sandboxie/commit/01a2670b9cc6f00bdc81b990e0c75c2688fd54fd)
- fixed SandMan crash in Vintage View mode [#3264](https://github.com/sandboxie-plus/Sandboxie/issues/3264)
- fixed pinning of more than one shortcut that points to the same file [#3259](https://github.com/sandboxie-plus/Sandboxie/issues/3259)



## [1.11.0 / 5.66.0] - 2023-08-25

### Added
- added ImDisk driver which enables creation of sandboxes in a RAM drive
- added Encrypted Sandbox support; this creates confidential boxes that do not leak data to the host PC
  - using the ImDisk driver and a new ImBox component featuring the cryptographic implementation from [DiskCryptor](https://diskcryptor.org/) the sandbox root folder is stored in an encrypted container file
  - using the SbieDrv to prevent processes not belonging to the sandbox from accessing an encrypted sandbox's root folder
  - with the 'ConfidentialBox=y' option the host process read access to sandboxed processes is blocked
- added certificate info to the About dialog
- added support for a more flexible certificate style
- added option for business customers to retrieve hardware-bound certificates from a serial number
- added option to upgrade existing certificates using an upgrade serial number

### Changed
- improved online updater code
- replaced drop-down list with radio buttons in the box creation wizard [#1381](https://github.com/sandboxie-plus/Sandboxie/issues/1381)

### Fixed
- fixed symbolic links created inside a sandbox not working properly [#3181](https://github.com/sandboxie-plus/Sandboxie/issues/3181)
- fixed text cut-off issues in Plus UI pop-ups [#3195](https://github.com/sandboxie-plus/Sandboxie/issues/3195)



## [1.10.5 / 5.65.5] - 2023-08-12

### Changed
- submenus don't close on mouse-leave (thanks typpos) [#2963](https://github.com/sandboxie-plus/Sandboxie/issues/2963)

### Fixed
- fixed verification of business certificates



## [1.10.4 / 5.65.4] - 2023-08-11

### Fixed
- fixed Sandboxie-Plus-x64-v1.10.3 crash on startup [#3174](https://github.com/sandboxie-plus/Sandboxie/issues/3174)
- fixed issue with untranslated buttons in all wizards [#3133](https://github.com/sandboxie-plus/Sandboxie/issues/3133)



## [1.10.3 / 5.65.3] - 2023-08-06

### Added
- added support for URL shortcut files in Run Menu [#3151](https://github.com/sandboxie-plus/Sandboxie/issues/3151)
- added workaround for NtQueryObject locking up under exotic circumstances; to enable, use 'UseDriverObjLookup=y'
- Add-Ons Manager: added tooltip to version column with maintainer information [#3167](https://github.com/sandboxie-plus/Sandboxie/issues/3167)
- added mechanism to open websites for add-ons [#3166](https://github.com/sandboxie-plus/Sandboxie/issues/3166)

### Changed
- improved business certificate handling, added usage count and machine-bound options

### Fixed
- fixed issues with pinned shortcuts
- fixed Process Suspend/Resume context menu [#3156](https://github.com/sandboxie-plus/Sandboxie/issues/3156)
- fixed issues with Qt-based installers [#2493](https://github.com/sandboxie-plus/Sandboxie/issues/2493) [#3153](https://github.com/sandboxie-plus/Sandboxie/issues/3153)



## [1.10.2 / 5.65.2] - 2023-07-31

### Added
- re-added option to suspend sandboxed processes [#3126](https://github.com/sandboxie-plus/Sandboxie/issues/3126)

### Changed
- changed format of the add-on data [#3135](https://github.com/sandboxie-plus/Sandboxie/issues/3135)
  - all users coming from versions 1.10.0 and 1.10.1 will need to reinstall the components in the Add-On Manager
- "OpenClipboard=n" now also denies writes to clipboard [#1367](https://github.com/sandboxie-plus/Sandboxie/issues/1367)

### Fixed
- fixed cross-renaming of directories
- fixed auto-scroll not working [#393](https://github.com/sandboxie-plus/Sandboxie/issues/393)
- fixed UI issue with box removal protection [#3104](https://github.com/sandboxie-plus/Sandboxie/issues/3104)
- fixed issue with link argument handling [#2969](https://github.com/sandboxie-plus/Sandboxie/issues/2969)
- fixed IPC issue introduced in 1.10.1 [#3132](https://github.com/sandboxie-plus/Sandboxie/issues/3132) [#3134](https://github.com/sandboxie-plus/Sandboxie/issues/3134)
- fixed issue with pinned run entry icons
- fixed UGlobalHotkey library not being compatible with Qt6

### Removed
- removed hardcoded support for LogAPI library
  - use the Add-On Manager and DLL injection settings



## [1.10.1 / 5.65.1] - 2023-07-24

### Added
- added UI option to select a custom text editor to open Sandboxie.ini [#3116](https://github.com/sandboxie-plus/Sandboxie/issues/3116)
- added separate protection against box removal and content deletion [#3104](https://github.com/sandboxie-plus/Sandboxie/issues/3104)
- added "auto scroll" feature in the Trace Log tab [#393](https://github.com/sandboxie-plus/Sandboxie/issues/393)

### Changed
- reworked handling of NT object handles
- "OpenClipboard=n" is also implemented in user mode now, thus enabling it for compartment boxes as well
- changed Delete V2 scheme to use drive letters in FilePaths.dat (backwards-compatibility with NT paths remains) [#3053](https://github.com/sandboxie-plus/Sandboxie/issues/3053)
- improved robustness of snapshot merging [#3017](https://github.com/sandboxie-plus/Sandboxie/issues/3017)

### Fixed
- fixed "Disable Security Isolation" causing a game to stop playing audio [#2893](https://github.com/sandboxie-plus/Sandboxie/issues/2893)
- fixed NtQueryDirectoryObject not implemented [#2734](https://github.com/sandboxie-plus/Sandboxie/issues/2734)
- fixed issue with working directory for Run Menu entries
- fixed improper global symlink in sandboxed namespace [#3112](https://github.com/sandboxie-plus/Sandboxie/issues/3112)
- fixed 'Addon already installed!' error when clicking 'Show Stack Trace' [#3114](https://github.com/sandboxie-plus/Sandboxie/issues/3114)
- fixed existing BoxNameTitle=process.exe,- removed when toggling other options [#3106](https://github.com/sandboxie-plus/Sandboxie/issues/3106)
- fixed asynchronously assigned PCA job not being properly detected [#1919](https://github.com/sandboxie-plus/Sandboxie/issues/1919)
- fixed incompatibility with first Windows 10 release [#3117](https://github.com/sandboxie-plus/Sandboxie/issues/3117)
- fixed remove sandbox only deletes the contents of the sandbox when an application is running in the sandbox [#3118](https://github.com/sandboxie-plus/Sandboxie/issues/3118)
- fixed crash issue with the script engine not terminating properly [#3120](https://github.com/sandboxie-plus/Sandboxie/issues/3120)
- fixed ImDisk under Sandboxie supervision causes SBIE2337 and sometimes BSoD [#1092](https://github.com/sandboxie-plus/Sandboxie/issues/1092)
- fixed snapshots not merging duplicate directory junctions [#3016](https://github.com/sandboxie-plus/Sandboxie/issues/3016)
- fixed snapshot-related issue when using Delete V2 rename functionality
- fixed issue with Delete V2 when using network shares
- fixed issue with accessing drive roots when using "UseVolumeSerialNumbers=y"
- fixed remove snapshot resurrects deleted files when using Delete V2 [#3015](https://github.com/sandboxie-plus/Sandboxie/issues/3015)

### Removed
- removed template support for Internet Download Manager [#991](https://github.com/sandboxie-plus/Sandboxie/issues/991#issuecomment-1646582375)



## [1.10.0 / 5.65.0] - 2023-07-12

### Added
- added box scripting engine to make SandMan more flexible
- added scriptable troubleshooting wizard [#1875](https://github.com/sandboxie-plus/Sandboxie/issues/1875)
- added Add-On Manager which helps to install additional and third-party components, available add-ons:
  - [ImDisk Toolkit](https://sourceforge.net/projects/imdisk-toolkit/) - used to create RAM drives and other virtual drives
  - [V4 Script Debugger](https://github.com/DavidXanatos/NeoScriptTools) - used to debug troubleshooting scripts
  - [Microsoft Debug Help Library](https://learn.microsoft.com/en-us/windows/win32/debug/debug-help-library) - used for the stack trace feature introduced in 1.9.6
  - [signcheck.exe](https://learn.microsoft.com/en-us/sysinternals/downloads/sigcheck) - used to scan files on VirusTotal before recovering them
  - [SbieHide.dll](https://github.com/VeroFess/SbieHide) - a third-party DLL to hide SbieDll.dll
  - [LogAPI.dll](https://bsa.isoftware.nl/) - an API logging library used for Buster Sandbox Analyzer
- added option to set the update interval to 1, 7, 14 or 30 days
- added `What's new in Sandboxie Plus` dialog in SbieCtrl.exe to highlight the new features of the Plus UI
  - Note: this is shown after the installation of Sandboxie Classic
- added "fixdacls" command to KmdUtil.exe, it repairs broken DACL entries on the Sandboxie folder to fix issues where SbieDll.dll fails to load
- added option to hide Sandboxie's own processes [#3054](https://github.com/sandboxie-plus/Sandboxie/issues/3054)
- added functionality to cache Sandboxie messages within the Plus UI [#2920](https://github.com/sandboxie-plus/Sandboxie/issues/2920)
- added button to invoke troubleshooting wizard directly from the SBIE message pop-up

### Changed
- the setup wizard now has a dedicated update configuration page
  - Note: this page will be shown once for all users who do not have updates enabled
- split the support page into Sandboxie Support and Sandboxie Updater tabs
- when the troubleshooting.7z file is available, the script engine will be used to match compatibility templates
  - Note: this allows a better granularity in template selection by using the AppCompatibility.js script
- reworked low-level code injection mechanism to improve flexibility and debugging
  - Note: the main injection detour code is now written in C instead of Assembler and can properly report SbieDll.dll loading errors as SBIE2181
- improved session agent startup to be more flexible
- improved SBIEMSG help handling, the link now contains message details allowing to point to a more exact document (if available)
- updated certificate validation code

### Fixed
- fixed uninstall issue in the Sandboxie Classic installer [d1863ff](https://github.com/sandboxie-plus/Sandboxie/commit/d1863ffadfe105c695de71c9e841c2fd568116fe)
- added workaround for Chrome not starting on Windows 11 with KB5027231 [#3040](https://github.com/sandboxie-plus/Sandboxie/issues/3040)
- improved compatibility with procmon/stack traces for debug builds
- fixed issue with non-standard command lines
- fixed online updater checking in daily instead of weekly intervals

### Removed
- cleaned up duplicate code (thanks lmou523) [#3067](https://github.com/sandboxie-plus/Sandboxie/pull/3067)



## [1.9.8 / 5.64.8] - 2023-06-21

### Changed
- improved uninstall process in the Sandboxie Classic installer (thanks sredna) [#3044](https://github.com/sandboxie-plus/Sandboxie/pull/3044)

### Fixed
- fixed a link error when the path was created with a symbolic link (thanks lmou523) [#3038](https://github.com/sandboxie-plus/Sandboxie/pull/3038)
- fixed recovery window focus after file overwrite dialog [#3048](https://github.com/sandboxie-plus/Sandboxie/issues/3048)
- fixed Sandboxie Plus crash with exception code c0000005 [#3024](https://github.com/sandboxie-plus/Sandboxie/issues/3024)



## [1.9.7 / 5.64.7] - 2023-06-09

### Added
- added configurable toolbar items (thanks typpos) [#2961](https://github.com/sandboxie-plus/Sandboxie/pull/2961)
- added new icons for Plus UI (thanks typpos) [#3010](https://github.com/sandboxie-plus/Sandboxie/pull/3010) [#3011](https://github.com/sandboxie-plus/Sandboxie/pull/3011)

### Changed
- minor update to the BlockSoftwareUpdaters template (thanks APMichael) [#2991](https://github.com/sandboxie-plus/Sandboxie/pull/2991)

### Fixed
- fixed empty lines when using "Copy Panel" in Plus UI [#2995](https://github.com/sandboxie-plus/Sandboxie/issues/2995)
- fixed Plus installer not honouring IniPath value when updating the current installed version (thanks offhub) [#2994](https://github.com/sandboxie-plus/Sandboxie/issues/2994)
- fixed timestamp on crash dumps
- fixed Firefox 114 crashes in Security Hardened boxes, if start restrictions are enabled (thanks offhub) [#3019](https://github.com/sandboxie-plus/Sandboxie/issues/3019)
- fixed a bug when there is no GUI proxy (thanks lmou523) [#3020](https://github.com/sandboxie-plus/Sandboxie/pull/3020)



## [1.9.6 / 5.64.6] - 2023-05-30

### Added
- added full stack trace to all trace messages
  - activating the Trace Log tab also turns on the Keep Terminated feature by default [#2988](https://github.com/sandboxie-plus/Sandboxie/issues/2988#issuecomment-1567787776)
- added option to launch the default notepad editor as non-admin [#2987](https://github.com/sandboxie-plus/Sandboxie/issues/2987)

### Fixed
- fixed an issue with token manipulation in the SbieDrv driver
- fixed "Reset all GUI options" not resetting all GUI sections as expected [#2967](https://github.com/sandboxie-plus/Sandboxie/issues/2967)
- fixed sbie:// links below the Box Type presets [#2959](https://github.com/sandboxie-plus/Sandboxie/issues/2959#issuecomment-1565264161)
- fixed "Reset all GUI options" causing all sandbox names to disappear [#2972](https://github.com/sandboxie-plus/Sandboxie/issues/2972)
- fixed A game can't be launched properly from "Run from Start Menu" [#2969](https://github.com/sandboxie-plus/Sandboxie/issues/2969)
- fixed drag and drop issue with Microsoft Excel data grid [9455e96](https://github.com/sandboxie-plus/Sandboxie/commit/9455e96a699cbc665f791e191f2a13bb40783ab0)
  - additional feedback for other drag and drop scenarios can be provided in [#856](https://github.com/sandboxie-plus/Sandboxie/issues/856)
- fixed regression: DLL loading problem (Entry Point Not Found) [#2980](https://github.com/sandboxie-plus/Sandboxie/issues/2980)
- fixed Sandboxie not marking deleted files or registry keys while virtualization scheme v2 is active (1.9.4/1.9.5) [#2984](https://github.com/sandboxie-plus/Sandboxie/issues/2984)
- fixed SandMan: Timestamp of the Sandboxie.ini [#2985](https://github.com/sandboxie-plus/Sandboxie/issues/2985)
- fixed crash after pressing the button "Show NT Object Tree" several times (Plus UI) [#2943](https://github.com/sandboxie-plus/Sandboxie/issues/2943)



## [1.9.5 / 5.64.5] - 2023-05-26

### Added
- added Turn SBIE message IDs into clickable links [#2953](https://github.com/sandboxie-plus/Sandboxie/issues/2953)
- added a limit to the number of identical log messages [#2856](https://github.com/sandboxie-plus/Sandboxie/issues/2856)

### Fixed
- fixed sbie:// links not working properly [#2959](https://github.com/sandboxie-plus/Sandboxie/issues/2959)
- fixed Sbie Messages, Trace Log, Recovery Log tabs should never hide some columns [#2940](https://github.com/sandboxie-plus/Sandboxie/issues/2940)
- fixed the RegisterDeviceNotificationW function DBT_DEVTYP_DEVICEINTERFACE [#2129](https://github.com/sandboxie-plus/Sandboxie/discussions/2129) [#1975](https://github.com/sandboxie-plus/Sandboxie/issues/1975)
- fixed a BSoD issue likely introduced in 1.9.4 (reported by 7starsseeker) [#2958](https://github.com/sandboxie-plus/Sandboxie/issues/2958)



## [1.9.4 / 5.64.4] - 2023-05-24

### Added
- added more documentation links to the Plus UI
- added tray menu option to dismiss a pending update notification
- added Pin/Favourite files to Tray [#2913](https://github.com/sandboxie-plus/Sandboxie/issues/2913)

### Changed
- improved compatibility template for Privacy Enhanced box types (thanks offhub) [#2899](https://github.com/sandboxie-plus/Sandboxie/pull/2899)
- improved support page in settings and reminder [#2896](https://github.com/sandboxie-plus/Sandboxie/issues/2896)
- improved signature error message [#2931](https://github.com/sandboxie-plus/Sandboxie/issues/2931)
- changed Don't show "No Inet" when exceptions exist [#2919](https://github.com/sandboxie-plus/Sandboxie/issues/2919)

### Fixed
- fixed Qt6 issues in ARM64 build
- fixed delete V2 bug when using box mounts without a drive letter
- fixed icon overlay issue with high DPI scaling
- fixed behaviour on multiple selection (thanks okrc) [#2903](https://github.com/sandboxie-plus/Sandboxie/pull/2903)
- fixed issue with default box grouping (thanks okrc) [#2910](https://github.com/sandboxie-plus/Sandboxie/pull/2910)
- fixed issue with sandbox renaming [#2912](https://github.com/sandboxie-plus/Sandboxie/issues/2912)
- fixed the checkbox for Open System Protected Storage appearing unchecked [#2866](https://github.com/sandboxie-plus/Sandboxie/issues/2866)
- fixed Firewall Rules' lack of colour contrast making text hard to read [#2900](https://github.com/sandboxie-plus/Sandboxie/issues/2900)
- fixed RecoverFolder shows GUID instead of folder name [#2918](https://github.com/sandboxie-plus/Sandboxie/issues/2918)
- fixed hourglass icon overlay gets stuck in the system tray when operation is stopped [#2869](https://github.com/sandboxie-plus/Sandboxie/issues/2869)
- fixed File Panel doesn't allow to adjust columns size in a permanent way [#2930](https://github.com/sandboxie-plus/Sandboxie/issues/2930)
- fixed renaming a box with sandboxed run entries can break those entries [#2921](https://github.com/sandboxie-plus/Sandboxie/issues/2921)
- fixed WFP not enabled after Setup Wizard and other issues [#2915](https://github.com/sandboxie-plus/Sandboxie/issues/2915)
- fixed Name column in the sandbox view should never be hidden [#2933](https://github.com/sandboxie-plus/Sandboxie/issues/2933)
- fixed File Panel does not share the right-click options provided in the sandbox view columns [#2934](https://github.com/sandboxie-plus/Sandboxie/issues/2934)
- fixed Checkboxes issue of NetworkEnableWFP and EnableObjectFiltering settings [#2935](https://github.com/sandboxie-plus/Sandboxie/issues/2935)
- fixed Unquoted service path [#537](https://github.com/sandboxie-plus/Sandboxie/issues/537)
- fixed Running the Insert chart in Microsoft Word in the sandbox fails [#2863](https://github.com/sandboxie-plus/Sandboxie/issues/2863)
- fixed An error occurred in the switch sandbox type preset configuration [#2941](https://github.com/sandboxie-plus/Sandboxie/issues/2941)
- fixed Unable to open Microsoft Excel (thanks lmou523) [#2890](https://github.com/sandboxie-plus/Sandboxie/issues/2890)



## [1.9.3 / 5.64.3] - 2023-05-08

### Added
- added global setting to disable overlay icons
- added ability to specify Sandboxie.ini location in registry
  - open "HKLM\SYSTEM\CurrentControlSet\Services\SbieDrv" and set "IniPath" [#2837](https://github.com/sandboxie-plus/Sandboxie/issues/2837)
  - use a REG_SZ string for the path, like "\\??\C:\my_path\my_sandboxie.ini"

### Changed
- improved handling of DLL injection failure
- updated Polish translation (thanks to user 7zip)

### Fixed
- fixed issue with command lines [#2858](https://github.com/sandboxie-plus/Sandboxie/issues/2858)
- fixed issue with always on top in the recovery window [#2885](https://github.com/sandboxie-plus/Sandboxie/issues/2885)



## [1.9.2 / 5.64.2] - 2023-04-24

### Added
- added box presets menu option to toggle 'DisableForceRules=y' (thanks offhub) [#2851](https://github.com/sandboxie-plus/Sandboxie/pull/2851)

### Fixed
- fixed issue with SBIE1305 message
- fixed encoding issue with Korean translation [#2833](https://github.com/sandboxie-plus/Sandboxie/issues/2833)



## [1.9.1 / 5.64.1] - 2023-04-23

### Added
- added option to disable all force rules of a given sandbox [#2797](https://github.com/sandboxie-plus/Sandboxie/issues/2797)

### Changed
- reworked configuration storage for box grouping

### Fixed
- fixed system check in Classic installer [#2812](https://github.com/sandboxie-plus/Sandboxie/pull/2812)
- fixed memory corruption issue with Normal[File/Key]Path [#2588](https://github.com/sandboxie-plus/Sandboxie/issues/2588)
- fixed DPI scaling in the snapshot manager window [#782](https://github.com/sandboxie-plus/Sandboxie/issues/782)
- fixed issue with arm64 DialogProc hook [#2838](https://github.com/sandboxie-plus/Sandboxie/issues/2838)
- fixed Qt6 compatibility issue with 7-Zip file system implementation [#2681](https://github.com/sandboxie-plus/Sandboxie/issues/2681)
- fixed issue with TLS support using Qt6 [#2682](https://github.com/sandboxie-plus/Sandboxie/issues/2682)
- added template preset for Spotify [#2673](https://github.com/sandboxie-plus/Sandboxie/issues/2673)
- added missing localisation to finder [#2845](https://github.com/sandboxie-plus/Sandboxie/issues/2845)



## [1.9.0 / 5.64.0] - 2023-04-17

### Added
- added local template editor to the global settings window
- added hidden message list
- added option to prevent processes located outside the sandbox from loading boxed DLLs
  - to enable it, use "ProtectHostImages=y"
- added option to block box intern programs but not box external ones
- added SbieLogon option to the new box wizard [#2823](https://github.com/sandboxie-plus/Sandboxie/issues/2823)
- added a few UI debug options [#2816](https://github.com/sandboxie-plus/Sandboxie/issues/2816)

### Changed
- Run Menu entries now can have custom icons
- the init edit page will automatically switch to edit when the user changes something
- improved change handling in the global settings window
- reorganized global options

### Fixed
- fixed issue with group renaming in the SandMan UI [#2804](https://github.com/sandboxie-plus/Sandboxie/issues/2804)



## [1.8.4 / 5.63.4] - 2023-04-07

### Added
- added installer icon [#2795](https://github.com/sandboxie-plus/Sandboxie/issues/2795)
- added token type indicator to process list

### Fixed
- fixed compatibility issue with Microsoft Edge 112.x
- fixed updater issue [#2790](https://github.com/sandboxie-plus/Sandboxie/issues/2790)
- fixed box naming issue in the new box wizard by making the box name static [#2792](https://github.com/sandboxie-plus/Sandboxie/issues/2792)
- fixed issue with Firefox/Thunderbird [#2799](https://github.com/sandboxie-plus/Sandboxie/issues/2799)



## [1.8.3 / 5.63.3] - 2023-04-05

### Fixed
- fixed issue with WFP support and driver verifier



## [1.8.2a / 5.63.2] - 2023-04-02

### Changed
- disabled token-based workarounds when in compartment mode

### Fixed
- fixed issue with the new SBIE2307 message being triggered on media removal
- excluded some old token hacks (for Firefox) from being disabled
- fixed long-standing ping issue with compartment type boxes [#1608](https://github.com/sandboxie-plus/Sandboxie/issues/1608)



## [1.8.2 / 5.63.2] - 2023-04-01

### Added
- reworked CreateAppContainerToken hook to return a restricted token for the issue [#2762](https://github.com/sandboxie-plus/Sandboxie/issues/2762)
  - Note: this behaviour can be disabled with 'FakeAppContainerToken=program.exe,n'
- enabled app container compatibility in App Compartment mode
  - Note: this should improve Microsoft Edge compatibility
- added web browser compatibility template wizard [#2761](https://github.com/sandboxie-plus/Sandboxie/issues/2761)
- added a mechanism to dynamically detect Chromium and Firefox based browsers
  - Note: the new mechanism can be disabled using 'DynamicImageDetection=program.exe,n'

### Changed
- renamed 'DropAppContainerTokens=program.exe,n' to 'DropAppContainerToken=program.exe,n'
- 'DropAppContainerToken=program.exe,y' can now be used in App Compartment boxes, however it is not recommended security-wise
- the desktop security workaround used for Chrome, Firefox and Acrobat is now enabled by default, you can disable it with "UseSbieDeskHack=n"
  - Note: this should allow Electron apps to run without 'SpecialImage=chrome,program.exe'
- disabled old token hacks, as these seem to be no longer required with the new App Container token
  - Note: in case of issues with Microsoft Edge, Chrome, Firefox or Acrobat, they can be re-enabled using 'DeprecatedTokenHacks=y'
- updated to Inno Setup 6.2.2 which also comes with a new installer icon

### Fixed
- fixed issue with Global Settings INI section editing
- fixed issue with 'UseRegDeleteV2=y' [#2756](https://github.com/sandboxie-plus/Sandboxie/issues/2756)
- autorun path now supports any length [#2769](https://github.com/sandboxie-plus/Sandboxie/pull/2769) (thanks Sapour)
- recovery window: delete confirmation dialog is no longer shown when no file/folder is selected [#2771](https://github.com/sandboxie-plus/Sandboxie/pull/2771) (thanks Sapour)
- fixed WeChat crash issue [#2772](https://github.com/sandboxie-plus/Sandboxie/pull/2772)

### Removed
- removed obsolete workaround for Maxthon 4



## [1.8.1 / 5.63.1] - 2023-03-12

### Added
- added certificate protection [#2722](https://github.com/sandboxie-plus/Sandboxie/issues/2722)

### Fixed
- fixed issue with finder search in trace log
- fixed issue with NT namespace virtualization [#2701](https://github.com/sandboxie-plus/Sandboxie/issues/2701)
- fixed issue with "Run Unsandboxed" [#2710](https://github.com/sandboxie-plus/Sandboxie/issues/2710)
- fixed issue with new box wizard [#2729](https://github.com/sandboxie-plus/Sandboxie/issues/2729)
- fixed issue with breakout processes and user-restricted sandboxes [#2732](https://github.com/sandboxie-plus/Sandboxie/issues/2732)
- fixed a couple of UI issues [#2733](https://github.com/sandboxie-plus/Sandboxie/issues/2733)
- fixed useless help button [#2748](https://github.com/sandboxie-plus/Sandboxie/issues/2748)



## [1.8.0 / 5.63.0] - 2023-02-27

### Added
- Run Menu now supports folders, to be used by entering foldername1\foldername2\entryname in the Name column of the UI
- added a tray indicator for pending updates
- added virtualization for CreateDirectoryObject(Ex) and OpenDirectoryObject (improves security, prevents name squatting)
  - Note: this can be disabled using 'NtNamespaceIsolation=n'

### Changed
- 'OpenProtectedStorage=y' has been replaced with a template
- moved all built-in access rules to a set of default templates
- moved WinInetCache control to a template OpenWinInetCache; 'CloseWinInetCache=y' is now obsolete
- added hook for CreateAppContainerToken, which should also improve compatibility with other apps [#1926](https://github.com/sandboxie-plus/Sandboxie/issues/1926)
  - Note: Template_Edge_Fix is no longer required
- replaced a few icons
- moved the "Support" global settings page above the "Advanced Config" page and renamed it to "Support & Updates"
- when dragging and dropping a file on the SandMan UI to run it, the currently selected box will be preselected in the box picker dialog
- improved access rule handling [#2633](https://github.com/sandboxie-plus/Sandboxie/discussions/2633)
- SbieCtrl now uses the new update format when checking for updates
- added prioritization of primary matches over auxiliary matches to rule specificity

### Fixed
- added AppContainer support for Compartment type boxes
- FIXED SECURITY ISSUE ID-22: NtCreateSectionEx was not filtered by the driver
- fixed issue starting services without a system token
- fixed issues with new file migration settings [#2700](https://github.com/sandboxie-plus/Sandboxie/issues/2700)
- fixed shell integration on ARM64 [#2685](https://github.com/sandboxie-plus/Sandboxie/issues/2685)
- fixed new issues with driver verifier [#2708](https://github.com/sandboxie-plus/Sandboxie/issues/2708)



## [1.7.2 / 5.62.2] - 2023-02-05

### Changed
- reorganized box options once more, the old box layout is default again
- SBIE2227 indicating volume without 8.3 name support is now disabled by default
  - Note: you can use "EnableVerboseChecks=y" to re-enable this check

### Fixed
- fixed delete v1/v2 display now respecting global presets
- avoid blocking Windows Explorer when dragging [#2660](https://github.com/sandboxie-plus/Sandboxie/pull/2660)
- fixed issue with QtSingleApp [#2659](https://github.com/sandboxie-plus/Sandboxie/issues/2659)
- fixed updater sometimes failing to create a temporary directory [#2615](https://github.com/sandboxie-plus/Sandboxie/issues/2615)
- fixed issue with snapshot removal [#2663](https://github.com/sandboxie-plus/Sandboxie/issues/2663)
- fixed issue with symbolic links [#2606](https://github.com/sandboxie-plus/Sandboxie/issues/2606)
- fixed issues with AppContainer isolation, app container tokens are now dropped by default
  - Note: this behaviour can be disabled with 'DropAppContainerTokens=program.exe,n'



## [1.7.1 / 5.62.1] - 2023-01-30

### Added
- added option to create a new sandbox to run from the box picker dialog
- added sandbox creation wizard (not available in Vintage View mode)
- added ability to open all COM classes [#2448](https://github.com/sandboxie-plus/Sandboxie/issues/2448)
  - Note: use OpenClsid={00000000-0000-0000-0000-000000000000} to open all
- the SandMan UI now indicates if a sandboxed process has an elevated (admin) or system token
- DropAdminRights can now be configured per process [#2293](https://github.com/sandboxie-plus/Sandboxie/issues/2293)
- added self-removing boxes [#1936](https://github.com/sandboxie-plus/Sandboxie/issues/1936)
- added Ctrl+F search filter to the box picker dialog
- added menu options to edit the Templates.ini and the Sandboxie-Plus.ini

### Changed
- refactored network blocking code in driver
- box options now show the expanded paths where appropriate
- made new box option layout the default (can be changed in appearance settings)

### Fixed
- fixed BlockNetworkFiles=y not working together with RestrictDevices=y [#2629](https://github.com/sandboxie-plus/Sandboxie/issues/2629)
- fixed SandMan crash issue introduced in 1.7.0
- fixed trace log filter being case-sensitive
- fixed performance issues with Delete V2
- fixed issue with NtQueryDirectoryFile data alignment [#2443](https://github.com/sandboxie-plus/Sandboxie/issues/2443)
- fixed issue with Microsoft Edge 111 dev build [#2631](https://github.com/sandboxie-plus/Sandboxie/issues/2631)
- fixed issue with mio sockets [#2617](https://github.com/sandboxie-plus/Sandboxie/issues/2617)
- fixed issue with Run Menu entries created from the options/settings window [#2610](https://github.com/sandboxie-plus/Sandboxie/issues/2610)
- fixed issues with the start menu when using snapshots [#2589](https://github.com/sandboxie-plus/Sandboxie/issues/2589)



## [1.7.0 / 5.62.0] - 2023-01-27

### Added
- added OnFileRecovery trigger allowing to check files before recovering them [#2202](https://github.com/sandboxie-plus/Sandboxie/issues/2202)
- added more presets to sandbox options
  - Note: these can only be changed when the box is empty
- added new file migration option page with additional settings
- added SBIE2113/SBIE2114/SBIE2115 message to indicate when files are not migrated due to presets
- added 'NoParallelLoading=y' to disable parallel loading of dlls to debug loader issues

### Changed
- moved SeparateUserFolders checkbox from global settings to per box options

### Fixed
- resolved SbieDll.dll incompatibility with shadow stack and enabled /CETCOMPAT for SbieDll.dll [#2559](https://github.com/sandboxie-plus/Sandboxie/issues/2559)
- added missing registry hooks to improve compatibility with newer applications
- fixed permission issue with registry entries in privacy mode boxes



## [1.6.7 / 5.61.7] - 2023-01-24

### Added
- added option to Sandboxie Classic to apply a supporter certificate

### Changed
- time-limited certificates now have 1 more month of validity to improve the renewal experience

### Fixed
- fixed issue with Hebrew language (Classic UI) [#2608](https://github.com/sandboxie-plus/Sandboxie/issues/2608)
- fixed issue with start menu integration and snapshots [#2589](https://github.com/sandboxie-plus/Sandboxie/issues/2589)



## [1.6.6 / 5.61.6] - 2023-01-16

### Changed
- reworked trace log retrieval for vastly improved performance
- improved list/tree finder
- improved trace logging
- enabled drop admin rights option for compartment boxes
  - Note: programs started unsandboxed can not have rights dropped, but UAC prompts and elevation attempts from within the sandbox are blocked

### Fixed
- fixed potential BSoD issue in the driver
- fixed crash in the trace log [#2599](https://github.com/sandboxie-plus/Sandboxie/issues/2599)
- fixed tray issue [#2600](https://github.com/sandboxie-plus/Sandboxie/pull/2600) (okrc)
- fixed issues with shortcuts [#2601](https://github.com/sandboxie-plus/Sandboxie/pull/2601) (okrc)



## [1.6.5 / 5.61.5] - 2023-01-10

### Added
- added user mode system call tracing, now syscalls can be logged in compartment boxes as well as all Win32k syscalls
- added nt object view to trace log

### Changed
- reworked trace log model, now it can load more than a million entries in less than one second
- ApiLog support is now hidden from the trace UI when the appropriate DLLs are missing

### Fixed
- fixed issue with Microsoft Edge in a Security Hardened box [#2571](https://github.com/sandboxie-plus/Sandboxie/issues/2571)
- opened OpenIpcPath=\\{BEC19D6F-D7B2-41A8-860C-8787BB964F2D} on ARM64 systems
- fixed incompatibility with Windows 11 21H2 ARM64 [#2431](https://github.com/sandboxie-plus/Sandboxie/issues/2431)



## [1.6.4 / 5.61.4] - 2022-12-31

### Added
- added option to disable sandbox clean-up on startup [#2553](https://github.com/sandboxie-plus/Sandboxie/issues/2553)
- added contribution guidelines in the Help menu for both Plus and Classic UIs [#2551](https://github.com/sandboxie-plus/Sandboxie/pull/2551)

### Fixed
- fixed issue with auto-delete sandbox on SandMan startup [#2555](https://github.com/sandboxie-plus/Sandboxie/issues/2555)
- fixed issue with Windows 11 context menu when SandMan was not already running [#2284](https://github.com/sandboxie-plus/Sandboxie/issues/2284)
- fixed issue with the display about cmbDefault [#2560](https://github.com/sandboxie-plus/Sandboxie/pull/2560) (okrc)
- fixed issue with colour inaccuracy [#2570](https://github.com/sandboxie-plus/Sandboxie/pull/2570) (okrc)
- fixed issue with Templates.ini loading on first start [#2574](https://github.com/sandboxie-plus/Sandboxie/issues/2574)
- fixed Cyberpunk 2077 not being able to load mods when sandboxed
- fixed performance issue in games
- fixed FFS hooking issue observed in Windows 11 ARM64 Build 22621.819



## [1.6.3 / 5.61.3] - 2022-12-21

### Changed
- improved Network Location Awareness template



## [1.6.2b / 5.61.2] - 2022-12-21

### Fixed
- fixed issue with update label when "Version Updates" drop-down menu was set to "Ignore"
- fixed issue with WoW64 thunking on 32-bit Windows 10 systems [#2546](https://github.com/sandboxie-plus/Sandboxie/issues/2546)
- fixed issue with auto delete option not working in sandboxes [#2531](https://github.com/sandboxie-plus/Sandboxie/issues/2531)
  - Note: a clean-up is now done after each reboot
- fixed issue with Network Location Awareness under Windows 11 [#2530](https://github.com/sandboxie-plus/Sandboxie/issues/2530)
- fixed issues with recovery window [#2458](https://github.com/sandboxie-plus/Sandboxie/issues/2458)



## [1.6.2a / 5.61.2] - 2022-12-19

### Fixed
- fixed crash issue in Vintage UI introduced in 1.6.1a



## [1.6.2 / 5.61.2] - 2022-12-19

### Added
- added template for Tencent TIM [#2516](https://github.com/sandboxie-plus/Sandboxie/pull/2516) (thanks TooYoungTooSimp)



## [1.6.1b / 5.61.1] - 2022-12-16

### Added
- added game/presentation mode [#2534](https://github.com/sandboxie-plus/Sandboxie/issues/2534)
- added option to pick a custom box icon

### Fixed
- fixed support setting page not showing version updates properly



## [1.6.1a / 5.61.1] - 2022-12-07

### Added
- added "Run Sandboxed" command to the sandbox menu
- DefaultBox can now be replaced with another sandbox [#2445](https://github.com/sandboxie-plus/Sandboxie/issues/2445)

### Fixed
- improved fix for [#2495](https://github.com/sandboxie-plus/Sandboxie/issues/2495)



## [1.6.1 / 5.61.1] - 2022-12-04

### Added
- added global option to introduce Run Menu entries to all boxes

### Changed
- reorganized command prompt entries [#2451](https://github.com/sandboxie-plus/Sandboxie/issues/2451)
- the "live" update channel is now part of the "preview" channel for which version updates can now be disabled

### Fixed
- fixed issue with support setting drop-down menu multiplying [#2502](https://github.com/sandboxie-plus/Sandboxie/pull/2502) (okrc)
- added translations folder as deprecated, to be removed upon installation [#2500](https://github.com/sandboxie-plus/Sandboxie/pull/2500) (lufog)
- fixed issue with disk usage of hidden items not counted [#2503](https://github.com/sandboxie-plus/Sandboxie/pull/2503) (okrc)
- fixed issue with boolean settings which can also be set per process [#2495](https://github.com/sandboxie-plus/Sandboxie/issues/2495)
- fixed issue with disk usage of hidden items not counted [#2503](https://github.com/sandboxie-plus/Sandboxie/pull/2503)



## [1.6.0 / 5.61.0] - 2022-11-26

### Added
- added ability to import/export boxes to .7z files
- added new update mechanism allowing for incremental updates
  - used for nightly test builds in the "live" update channel
  - used to update Templates.ini and translations after installation [#1105](https://github.com/sandboxie-plus/Sandboxie/issues/1105)

### Changed
- restructured the box context menu
- SandMan translations are now compressed and located in translations.7z

### Fixed
- fixed issue with recycle icon not updated in time [#2457](https://github.com/sandboxie-plus/Sandboxie/issues/2457) (thanks okrc)
- fixed issue with "index out of range" [#2470](https://github.com/sandboxie-plus/Sandboxie/pull/2470) (thanks okrc)
- fixed issue starting SandMan after install [#2284](https://github.com/sandboxie-plus/Sandboxie/issues/2284)
- fixed encoding issue with my_version.h [#2475](https://github.com/sandboxie-plus/Sandboxie/issues/2475)
- fixed issue with empty group blinking sequence [#2486](https://github.com/sandboxie-plus/Sandboxie/pull/2486) (thanks okrc)



## [1.5.3 / 5.60.3] - 2022-11-08

### Added
- boxes set to auto-delete are now marked with a small red recycle symbol
- added Vietnamese language on Plus UI [#2438](https://github.com/sandboxie-plus/Sandboxie/pull/2438)

### Fixed
- fixed issue with box options [#2400](https://github.com/sandboxie-plus/Sandboxie/issues/2400)
- fixed issue with Smart App Control [#2341](https://github.com/sandboxie-plus/Sandboxie/issues/2341)
- fixed issue with snapshots when using privacy boxes [#2427](https://github.com/sandboxie-plus/Sandboxie/issues/2427)
- fixed issue with m_pColorslider change not applied [#2433](https://github.com/sandboxie-plus/Sandboxie/pull/2433) (thanks okrc)
- fixed issue with switching snapshots when the file panel is open
- fixed issue with file panel when an empty box is selected [#2419](https://github.com/sandboxie-plus/Sandboxie/issues/2419)
- fixed issue with menu icon not being disabled [#2406](https://github.com/sandboxie-plus/Sandboxie/issues/2406)
- fixed compatibility issue with Microsoft Edge [#2312](https://github.com/sandboxie-plus/Sandboxie/issues/2312)
- fixed UsePrivacyMode=y compatibility with Windows 7 [#2423](https://github.com/sandboxie-plus/Sandboxie/issues/2423)
- fixed minor issue during Sandboxie Plus uninstall [#2421](https://github.com/sandboxie-plus/Sandboxie/issues/2421)
- fixed BSoD issue when driver initialization fails (introduced in 1.5.1) [#2431](https://github.com/sandboxie-plus/Sandboxie/issues/2431)
- fixed fake paths being listed even if true paths do not exist [#2403](https://github.com/sandboxie-plus/Sandboxie/issues/2403)
- fixed issue with Firefox 106.x requesting write access to plugin executables [#2391](https://github.com/sandboxie-plus/Sandboxie/issues/2391) [#2411](https://github.com/sandboxie-plus/Sandboxie/issues/2411)



## [1.5.2 / 5.60.2] - 2022-10-28

### Changed
- utility groups are now cleaned up automatically
- improved removal of leftovers [#2409](https://github.com/sandboxie-plus/Sandboxie/pull/2409)

### Fixed
- fixed issues with SBIE2227 on virtual drives
- fixed issues on native 32-bit Windows introduced with 1.5.0 [#2401](https://github.com/sandboxie-plus/Sandboxie/issues/2401)



## [1.5.1 / 5.60.1] - 2022-10-26

### Added
- added SBIE2227 warning when a sandbox is located on a volume without 8.3 name support
- added template for Tencent QQ [#2367](https://github.com/sandboxie-plus/Sandboxie/issues/2367)

### Fixed
- fixed issues when renaming a sandbox with a custom path [#2368](https://github.com/sandboxie-plus/Sandboxie/issues/2368)
- properly fixed the Firefox 106 issue [46e9979](https://github.com/sandboxie-plus/Sandboxie/commit/46e99799e2b753b85224dc921ec141a05025acfd)
- fixed issue with alternative UI modes [#2380](https://github.com/sandboxie-plus/Sandboxie/issues/2380)
- fixed command line corruption with breakout processes [#2377](https://github.com/sandboxie-plus/Sandboxie/issues/2377)
- fixed issues with Privacy Enhanced box types [#2342](https://github.com/sandboxie-plus/Sandboxie/issues/2342)
- fixed issue with boxed object directory initialization [#2342](https://github.com/sandboxie-plus/Sandboxie/issues/2342)
- Sandboxie no longer leaves behind permanent directory objects
- FIXED SECURITY ISSUE ID-21: AlpcConnectPortEx was not filtered by the driver [#2396](https://github.com/sandboxie-plus/Sandboxie/issues/2396)
- fixed issues with program control options [#2395](https://github.com/sandboxie-plus/Sandboxie/issues/2395)



## [1.5.0 / 5.60.0] - 2022-10-19

### Added
- added support for Windows on ARM64 [#1321](https://github.com/sandboxie-plus/Sandboxie/issues/1321) [#645](https://github.com/sandboxie-plus/Sandboxie/issues/645)
  - ported SbieDrv for ARM64
  - ported low-level injection mechanism for ARM64/ARM64EC
  - ported syscall hooks for ARM64/ARM64EC
  - ported SbieDll.dll to ARM64/ARM64EC
  - Note: ARM32 on ARM64 is not implemented and will terminate with message SBIE2338
  - Note: when Sandboxie is running, it disables the use of CHPE binaries for x86 processes globally - as required for the forced process functionality. This can be disabled by adding the global option "DisableCHPE=n" to the Sandboxie.ini, which will terminate x86 processes started outside the sandbox with message SBIE2338, instead of being forced

### Changed
- reworked API compatibility check
- breakout process is now available for all users

### Fixed
- fixed issue with Win32 hooks in x86 applications
- avoid window overlap when editing templates [#2339](https://github.com/sandboxie-plus/Sandboxie/pull/2339) (thanks okrc)
- fixed incorrect write of OpenWinClass UI setting [#2347](https://github.com/sandboxie-plus/Sandboxie/pull/2347) (thanks okrc)
- fixed issue with local template [#2338](https://github.com/sandboxie-plus/Sandboxie/pull/2338) (thanks okrc)
- fixed Edge WebView2 compatibility issue [#2350](https://github.com/sandboxie-plus/Sandboxie/issues/2350)
- added provisional workaround for Firefox 106 content process sandbox issue
- fixed bug with renaming sandboxes [#2358](https://github.com/sandboxie-plus/Sandboxie/pull/2358) (thanks okrc)



## [1.4.2 / 5.59.2] - 2022-10-10

### Added
- added tooltips to resource access modes [#2300](https://github.com/sandboxie-plus/Sandboxie/issues/2300)
- added UI option to control ApplyElevateCreateProcessFix [#2302](https://github.com/sandboxie-plus/Sandboxie/issues/2302)
- added message 2226 if a process needs 'ApplyElevateCreateProcessFix=y'

### Changed
- moved Restrictions tab from the new Security page back to the General page

### Fixed
- fixed wrong button captions in the breakout process page
- fixed issue with saving box recovery options
- fixed the display problem of SandMan [#2306](https://github.com/sandboxie-plus/Sandboxie/pull/2306) (thanks okrc)
- fixed theme not auto-changing [#2307](https://github.com/sandboxie-plus/Sandboxie/pull/2307)
- fixed issue with saving SandMan window state on Windows shutdown
- fixed miscellaneous minor issues [#2301](https://github.com/sandboxie-plus/Sandboxie/issues/2301)
- fixed issue with Microsoft Edge introduced in 106.x [#2325](https://github.com/sandboxie-plus/Sandboxie/issues/2325)
- fixed Vivaldi hooking issue, UseVivaldiWorkaround is no longer needed [#1783](https://github.com/sandboxie-plus/Sandboxie/issues/1783)
- fixed issues with miscellaneous tab on the advanced options page [#2315](https://github.com/sandboxie-plus/Sandboxie/issues/2315#issuecomment-1272544086)



## [1.4.1 / 5.59.1] - 2022-10-05

### Added
- added dark title bar support for Windows 11 [#2299](https://github.com/sandboxie-plus/Sandboxie/pull/2299)

### Changed
- in Sbie 5.28 and later WinInetCache is open, which breaks IE's source view, therefore it can now be disabled with 'CloseWinInetCache=y'

### Fixed
- fixed WarnProcess and WarnFolder not working with certain configurations



## [1.4.0 / 5.59.0] - 2022-09-30

### Added
- added integrated run from start menu [#1836](https://github.com/sandboxie-plus/Sandboxie/issues/1836)
- added start menu enumeration [#1570](https://github.com/sandboxie-plus/Sandboxie/issues/1570)
- added UI for breakout processes [#1904](https://github.com/sandboxie-plus/Sandboxie/issues/1904)
- added option to customize double-click action per sandbox
- added new miscellaneous tab in the advanced options tab allowing to configure specific processes and other advanced options
- added "SeparateUserFolders=y" and "SandboxieLogon=y" to the sandbox options dialog
- added icons to the section labels on the option pages

### Changed
- prepared for Qt 6.3.1
- restructured the general settings page
- restructured the sandbox options page, added a new tab and moved some of the advanced options there

### Fixed
- fixed menu bar issue in Plus UI [#2280](https://github.com/sandboxie-plus/Sandboxie/pull/2280) (thanks okrc)



## [1.3.5 / 5.58.5] - 2022-09-26

### Added
- added localization to Windows 11 shell menu [#2229](https://github.com/sandboxie-plus/Sandboxie/issues/2229)

### Changed
- improved recovery window behaviour in Plus UI [#2266](https://github.com/sandboxie-plus/Sandboxie/issues/2266)

### Fixed
- fixed issues with stale data in Sandboxie-Plus.ini [#2248](https://github.com/sandboxie-plus/Sandboxie/pull/2248) (thanks okrc)
- fixed issue with dummy manifests [#2252](https://github.com/sandboxie-plus/Sandboxie/issues/2252)
- fixed issue with XYplorer [#2230](https://github.com/sandboxie-plus/Sandboxie/issues/2230)
- fixed crash in Plus UI [e9e21c2](https://github.com/sandboxie-plus/Sandboxie/commit/e9e21c215ed87cf1d5aa999407ab2f99d5da9e4a)
- fixed m_pCleanUpButton is displayed empty when NoIcons=1 [#2273](https://github.com/sandboxie-plus/Sandboxie/pull/2273) (thanks okrc)



## [1.3.4 / 5.58.4] - 2022-09-19

### Added
- added NoRenameWinClass to the Plus UI
- added Windows.UI.* to the list of hardcoded well-known classes to resolve issues with WinUI apps [#2109](https://github.com/sandboxie-plus/Sandboxie/issues/2109)

### Changed
- NoRenameWinClass now supports wildcards

### Fixed
- fixed issue with default box not being detected on start [#2195](https://github.com/sandboxie-plus/Sandboxie/issues/2195)
- fixed move sandbox menu issue [#2225](https://github.com/sandboxie-plus/Sandboxie/issues/2225) (thanks okrc)
- fixed issues with stale data in Sandboxie-Plus.ini [#2234](https://github.com/sandboxie-plus/Sandboxie/pull/2234) (thanks okrc)
- fixed autostart issue [#2219](https://github.com/sandboxie-plus/Sandboxie/issues/2219)
- fixed firewall UI issue, all programs entries were missing *, prefix [#2247](https://github.com/sandboxie-plus/Sandboxie/issues/2247)
- fixed BlockPorts template with a missing *, prefix [4420ba4](https://github.com/sandboxie-plus/Sandboxie/commit/4420ba4448a797b7369917058c34e8a78c2ec9fc)
- fixed issues with various Electron apps [#2217](https://github.com/sandboxie-plus/Sandboxie/issues/2217) [#2235](https://github.com/sandboxie-plus/Sandboxie/issues/2235) [#2201](https://github.com/sandboxie-plus/Sandboxie/issues/2201) [#2166](https://github.com/sandboxie-plus/Sandboxie/issues/2166)
  - now the default behaviour is UseElectronWorkaround=n

### Removed
- removed obsolete VPNTunnel template



## [1.3.3 / 5.58.3] - 2022-09-12

### Added
- added domain\user notation when the LogFile registry setting is applied as workaround for [#2207](https://github.com/sandboxie-plus/Sandboxie/issues/2207)
  - Usage: in "HKLM\SYSTEM\CurrentControlSet\Services\SbieSvc" add REG_SZ "LogFile" with "3;[path]\Sandboxie.log"
- added option to block host processes from accessing sandboxed ones [#2132](https://github.com/sandboxie-plus/Sandboxie/issues/2132)
  - Usage: DenyHostAccess=Program.exe,y
  - Note: by default, this protection only applies for write access, that is, unsandboxed processes will still be able to obtain read-only access
  - to prevent host processes from obtaining read access, ConfidentialBox=y must also be set, which requires a supporter certificate
- added compatibility template for ReHIPS
- added creation of all default folders in privacy box type [#2218](https://github.com/sandboxie-plus/Sandboxie/issues/2218)

### Changed
- improved SandMan settings behaviour for non admin users [#2123](https://github.com/sandboxie-plus/Sandboxie/issues/2123)

### Fixed
- fixed issues with group moving via drag and drop
- approved more required syscalls [#2190](https://github.com/sandboxie-plus/Sandboxie/issues/2190)
- fixed issues when deleting box content and the file panel view is open
- fixed issue with config protection [#2206](https://github.com/sandboxie-plus/Sandboxie/issues/2206)
- fixed issue with default box [#2195](https://github.com/sandboxie-plus/Sandboxie/issues/2195)
- fixed issue with keyboard delete shortcut for process termination

### Removed
- removed obsolete Online Armor template



## [1.3.2 / 5.58.2] - 2022-08-30

### Added
- added icons to sub tabs in the box options dialog
- recovery and message pop-up menu options are not persisting across UI restarts any more
- added new box colour, a white box indicates that it's not really a sandbox and is displayed when the user specified OpenFilePath=* or alike

### Changed
- Sandboxie no longer issues message 1301 when forced processes are temporarily disabled
  - the message can be re-enabled with "NotifyForceProcessDisabled=y"
- reworked the "Open COM" checkbox mechanism in the plus UI
  - now it uses a template and it can also keep COM closed while OpenIpcPath=* is set

### Fixed
- fixed compatibility issue with Proxifier [#2163](https://github.com/sandboxie-plus/Sandboxie/issues/2163)
- fixed encoding issue with Korean translation [#2173](https://github.com/sandboxie-plus/Sandboxie/pull/2173)
- fixed issues with update available message



## [1.3.1 / 5.58.1] - 2022-08-20

### Added
- added ability to switch the fusion theme independently from the dark theme
- added ability to download updates from the support page
- added missing system calls to the hardened box type [88bc06a](https://github.com/sandboxie-plus/Sandboxie/commit/88bc06a0c7368a81c80a77d7a89ddc73455abb25) [b775264](https://github.com/sandboxie-plus/Sandboxie/commit/b775264a4824e49b554f1b776c377170e5f90797) [04b2377](https://github.com/sandboxie-plus/Sandboxie/commit/04b23770f53597c12eda9122c774ed5165129147) (thanks Mr.X)
- added search box to the Plus UI Settings and box options dialog [#2134](https://github.com/sandboxie-plus/Sandboxie/issues/2134)
- added Korean translation to the Plus UI [#2133](https://github.com/sandboxie-plus/Sandboxie/pull/2133) (thanks VenusGirl)
- added grouping to SandMan tray menu [#2148](https://github.com/sandboxie-plus/Sandboxie/issues/2148)

### Changed
- improved info label
- the vintage mode look is even more vintage now
- reloading the configuration with the SandMan command "Options -> Reload INI file" now updates the list of approved syscalls
- made rule specificity more specific, now a rule with less wildcards overrules a rule with more wildcards
  - Note: trailing wildcards are evaluated separately

### Fixed
- fixed issue with displaying sandbox configuration [#2111](https://github.com/sandboxie-plus/Sandboxie/issues/2111)
- fixed flashing issue when switching views [#2050](https://github.com/sandboxie-plus/Sandboxie/issues/2050)
- fixed inconsistencies with various checkboxes in the Plus UI [ef4ac1b](https://github.com/sandboxie-plus/Sandboxie/commit/ef4ac1b6b34d505e46515e9aabb98411a9b1751e) [06c89e3](https://github.com/sandboxie-plus/Sandboxie/commit/06c89e3f45036f593fed7a0d0d59d54313e8ca77)
- fixed a certificate validation issue [238cb44](https://github.com/sandboxie-plus/Sandboxie/commit/238cb44969923479148e210814ab91d2428ec4b0)
- fixed issue with "UseRuleSpecificity" setting [#2124](https://github.com/sandboxie-plus/Sandboxie/issues/2124) [file.c#L965-L966](https://github.com/sandboxie-plus/Sandboxie/blob/ff759692a222cf7c492cb4d5cfd76c79fbde1c2b/Sandboxie/core/drv/file.c#L965-L966)



## [1.3.0 / 5.58.0] - 2022-08-09

### Added
- added hook configuration for ntoskrnl/ntdll
  - individual ntdll hooks can be disabled using "DisableWinNtHook=..."
- added a new Security Hardened Box Mode; add "UseSecurityMode=y" to enable
  - when this setting is enabled, it combines "SysCallLockDown=y" which limits the use of NT system calls with "DropAdminRights=y" and "RestrictDevices=y"
  - only calls configured in the global section as "ApproveWinNtSysCall=..."/"ApproveWin32SysCall=..." will be executed with the original token
  - all non-approved NT syscalls will be executed with the sandboxed token, this may break compatibility in certain scenarios
  - additional syscalls may need to be allowed, this has to be done in the [GlobalSettings] and the driver must be restarted
  - Note: boxes created as Security Hardened with prior builds will be displayed as normal in the UI from now on
  - the Security Hardened icons are now repurposed for the new Super Extra Security Hardened Box Mode
  - Note: the new enhanced security features require a supporter certificate
- added browse option to the "force process" tab
- added support reminder dialog

### Changed
- replaced the "DeviceSecurity" template with a dedicated setting "RestrictDevices=y"
  - Note: when needed, more "NormalFilePath=..." entries can be added to open specific devices
- rule specificity is now even more specific, an exact rule now overrules those ending with a wildcard



## [1.2.8b / 5.57.7] - 2022-08-08

### Fixed
- fixed issue with context menu setup on Windows 11
- fixed column issue in vintage mode [#2103](https://github.com/sandboxie-plus/Sandboxie/issues/2103)



## [1.2.8 / 5.57.7] - 2022-08-05

### Fixed
- fixed missing uninstall routine in SandMan



## [1.2.7 / 5.57.7] - 2022-07-31

### Added
- added option for alternating row colours in all lists [#2073](https://github.com/sandboxie-plus/Sandboxie/issues/2073)

### Changed
- SandboxieLogon is now disabled by default as it wasn't compatible with third-party malware tools [#2025](https://github.com/sandboxie-plus/Sandboxie/issues/2025)
- the access view list now adds the trailing "*" to file and key paths the same way the driver does [2039](https://github.com/sandboxie-plus/Sandboxie/issues/2039)
- setup of shell integration is now done by SandMan, not the installer
- uninstaller can now remove the sandbox folders [#1235](https://github.com/sandboxie-plus/Sandboxie/pull/1235)



## [1.2.6 / 5.57.6] - 2022-07-25

### Changed
- reworked saving of global options

### Fixed
- fixed issue with the Delete Content option [#2043](https://github.com/sandboxie-plus/Sandboxie/issues/2043)
- fixed issue with box preferences [#2046](https://github.com/sandboxie-plus/Sandboxie/issues/2046)
- fixed issue with the Delete V2 registry



## [1.2.5 / 5.57.5] - 2022-07-22

### Changed
- improved a few icons

### Fixed
- fixed a certificate validation issue



## [1.2.4 / 5.57.4] - 2022-07-21

### Added
- added a delete button to the recovery window [#2024](https://github.com/sandboxie-plus/Sandboxie/issues/2024)

### Changed
- improved the tree selection display

### Fixed
- fixed issues with the file panel
- fixed issue with some key bindings [#2030](https://github.com/sandboxie-plus/Sandboxie/issues/2030)
- fixed issue with RemoveSidName when terminating SbieSvc
- fixed issue with the new hooking mechanism
- fixed BSoD issue with Win32k hooks introduced in 1.2.0 [#2035](https://github.com/sandboxie-plus/Sandboxie/issues/2035)
- fixed issue with Element 1.11 and Electron workaround [#2023](https://github.com/sandboxie-plus/Sandboxie/issues/2023)



## [1.2.3 / 5.57.3] - 2022-07-13

### Fixed
- fixed issues with the new menu code

### Changed
- reworked frame drawing



## [1.2.2 / 5.57.2] - 2022-07-13

### Fixed
- fixed issues with frame drawing
- fixed issues with the tray and box menu introduced in the last build
- removed focus rectangle from the tree list

### Changed
- refactored menu creation code



## [1.2.1 / 5.57.1] - 2022-07-11

### Added
- added Swedish translation to the Plus UI (thanks pb1)
- added Vintage View mode to make SandMan UI look like SbieCtrl
- added alternative tray menu mechanics
- added ability to auto-generate sandbox icons based on the border colour

### Changed
- changed box group icon to a dedicated one
- "browse content" is now available as a side panel in the main window
- animated hourglass icon overlay

### Fixed
- fixed DPI issue on Windows 7
- fixed issue with Software Compatibility tab
- fixed issue with OpenKeyPath introduced in build 1.1.1 [#2006](https://github.com/sandboxie-plus/Sandboxie/issues/2006)



## [1.2.0 / 5.57.0] - 2022-06-28

### Added
- re-engineered "SandboxieLogon=y"; it's on by default, as each sandbox gets its own SID now
  - Note: this enforces strict isolation of sandboxes from each other

### Changed
- reworked hook management, unloaded DLLs are properly unhooked now [#1243](https://github.com/sandboxie-plus/Sandboxie/issues/1243)
- the box order is now stored in Sandboxie-Plus.ini
- improved DPI scaling behaviour



## [1.1.3 / 5.56.3] - 2022-06-20

### Added
- added group-first sorting [#1922](https://github.com/sandboxie-plus/Sandboxie/issues/1922)

### Changed
- updated Classic UI Swedish translation (thanks pb1)
- restored Plus UI Turkish translation [#1419](https://github.com/sandboxie-plus/Sandboxie/issues/1419) (thanks fmbxnary)

### Fixed
- fixed issue with recovery window on delete [#1948](https://github.com/sandboxie-plus/Sandboxie/issues/1948)
- fixed double-click issue on path column [#1951](https://github.com/sandboxie-plus/Sandboxie/issues/1951)
- "AllowBoxedJobs=n" is back to the default behaviour as issues were reported [#1954](https://github.com/sandboxie-plus/Sandboxie/issues/1954)
- fixed issue with internet block [#1955](https://github.com/sandboxie-plus/Sandboxie/issues/1955)
- fixed grouping issue in the Plus UI [#1950](https://github.com/sandboxie-plus/Sandboxie/issues/1950)
- fixed issue with CredentialUIBroker.exe on Windows 11 with Win32k hooks [#1839](https://github.com/sandboxie-plus/Sandboxie/issues/1839)
- fixed issue with Delete V2 [#1939](https://github.com/sandboxie-plus/Sandboxie/issues/1939)



## [1.1.2 / 5.56.2] - 2022-06-14

### Added
- added missing file recovery log to SandMan [#425](https://github.com/sandboxie-plus/Sandboxie/issues/425)
- the immediate recovery window will now auto-close when all files have been recovered [#1498](https://github.com/sandboxie-plus/Sandboxie/issues/1498)
- the immediate recovery window of SandMan is always on top by default like in SbieCtrl; this can be disabled with "Options/RecoveryOnTop=n" [#1465](https://github.com/sandboxie-plus/Sandboxie/issues/1465)
- added option to toggle immediate recovery from the presets submenu [#1653](https://github.com/sandboxie-plus/Sandboxie/issues/1653)
- added option to disable file recovery and message pop-up globally
- added per box refresh option [#1945](https://github.com/sandboxie-plus/Sandboxie/issues/1945)

### Changed
- the desktop security workaround used for Chrome, Firefox and Acrobat can now be enabled for all processes using "UseSbieDeskHack=y"
- improved double-click behaviour [#1935](https://github.com/sandboxie-plus/Sandboxie/issues/1935)
- box size info is refreshed on file recovery

### Fixed
- fixed issue with unnecessary Sandboxie config reloads introduced in 1.1.1 [#1938](https://github.com/sandboxie-plus/Sandboxie/issues/1938)
- fixed issue with recovery window focus [#1374](https://github.com/sandboxie-plus/Sandboxie/issues/1374)
- fixed issues with desktop objects introduced in 1.1.1 [#1934](https://github.com/sandboxie-plus/Sandboxie/issues/1934)
- fixed issues with Edge startup boost using a GPO preset [#1913](https://github.com/sandboxie-plus/Sandboxie/issues/1913)



## [1.1.1 / 5.56.1] - 2022-06-07

### Added
- compatibility templates can now be viewed from the settings window [#1891](https://github.com/sandboxie-plus/Sandboxie/issues/1891)
- the refresh command is now bound to F5 [#1885](https://github.com/sandboxie-plus/Sandboxie/issues/1885)
- added more first start wizard options
- added option to permanently disable immediate recovery for any given box when it opens [#1478](https://github.com/sandboxie-plus/Sandboxie/issues/1478)
- double-click on the path column now opens the box root in Windows Explorer [#1924](https://github.com/sandboxie-plus/Sandboxie/issues/1924)

### Changed
- changed Move Box behaviour [#1879](https://github.com/sandboxie-plus/Sandboxie/issues/1879)
- improved implementation of the PreferExternalManifest option
- Win32k hooks are now by default only used for Edge and Chromium apps as they cause issues with other software [#1902](https://github.com/sandboxie-plus/Sandboxie/issues/1902) [#1912](https://github.com/sandboxie-plus/Sandboxie/issues/1912) [#1897](https://github.com/sandboxie-plus/Sandboxie/issues/1897)
- "AllowBoxedJobs=y" is now the default behaviour

### Fixed
- fixed Edge issue with Windows 11 after KB5014019
- fixed issues with the new Delete V2 mechanism when using "SeparateUserFolders=y" [#1885](https://github.com/sandboxie-plus/Sandboxie/issues/1885)
- fixed credential issue [#1770](https://github.com/sandboxie-plus/Sandboxie/pull/1770)
- fixed force process priorities [#1883](https://github.com/sandboxie-plus/Sandboxie/issues/1883)
- fixed issues with the new Delete V2 mechanism
- fixed issue with the Windows 11 menu on older Windows builds [1877](https://github.com/sandboxie-plus/Sandboxie/issues/1877)
- refresh now works without WatchBoxSize option [#1885](https://github.com/sandboxie-plus/Sandboxie/issues/1885)
- fixed crash issue with WatchBoxSize=true [#1885](https://github.com/sandboxie-plus/Sandboxie/issues/1885)
- fixed issue with recovery folder paths [#1840](https://github.com/sandboxie-plus/Sandboxie/issues/1840)
- fixed issues with Sbie desktop and wndStation affecting Acrobat Reader [#1863](https://github.com/sandboxie-plus/Sandboxie/issues/1863)
- fixed issues with box grouping [#1921](https://github.com/sandboxie-plus/Sandboxie/issues/1921) [#1920](https://github.com/sandboxie-plus/Sandboxie/issues/1920)
- fixed issues when changing language [#1914](https://github.com/sandboxie-plus/Sandboxie/issues/1914)
- fixed issue with BreakoutFolder [#1908](https://github.com/sandboxie-plus/Sandboxie/issues/1908)
- fixed issue with SbieDll.dll for x86 exception handling
- fixed issues with application-specific hives (RegLoadAppKey) affecting Visual Studio [#1576](https://github.com/sandboxie-plus/Sandboxie/issues/1576) [#1452](https://github.com/sandboxie-plus/Sandboxie/issues/1452)



## [1.1.0 / 5.56.0] - 2022-05-24

### Added
- added support for NtRenameKey (this requires UseRegDeleteV2=y) [#205](https://github.com/sandboxie-plus/Sandboxie/issues/205)
- added box size info [#1780](https://github.com/sandboxie-plus/Sandboxie/issues/1780)

### Changed
- reworked the mechanism Sandboxie uses to mark host files as deleted
  - under the new behaviour a data file (FilePaths.dat) is created in the box root instead of dummy files
  - it can be enabled with "UseFileDeleteV2=y" and also for the registry with "UseRegDeleteV2=y" which creates a reg file (RegPaths.dat)
- reworked the TlsNameBuffer mechanism to be more versatile and less error-prone
- significantly reduced the CPU usage of SandMan.exe

### Fixed
- fixed folder rename issues (this requires UseFileDeleteV2=y) [#71](https://github.com/sandboxie-plus/Sandboxie/issues/71)
- fixed issue with process access [#1603](https://github.com/sandboxie-plus/Sandboxie/issues/1603)
- fixed translation issue [#1864](https://github.com/sandboxie-plus/Sandboxie/issues/1864)
- fixed UI issue with the box selection window [#1867](https://github.com/sandboxie-plus/Sandboxie/issues/1867)
- fixed UI issue when switching languages [#1871](https://github.com/sandboxie-plus/Sandboxie/issues/1871)



## [1.0.22 / 5.55.22] - 2022-05-15

### Added
- added auto-update download and silent install option to SandMan.exe [#917](https://github.com/sandboxie-plus/Sandboxie/issues/917)
- trace monitor mode can save to file now [#1851](https://github.com/sandboxie-plus/Sandboxie/issues/1851)
- trace log now shows IPC object type information
- added support for Windows 11 context menus

### Fixed
- fixed SandMan crash issue [#1846](https://github.com/sandboxie-plus/Sandboxie/issues/1846)
- fixed issue with Windows Server 2022 build 20348
- fixed translation switching issues [#1852](https://github.com/sandboxie-plus/Sandboxie/issues/1852)



## [1.0.21 / 5.55.21] - 2022-05-10

### Added
- added "FuncSkipHook=FunctionName" option to selectively disable certain function hooks

### Changed
- improved the support certificate entry box
- changing the language no longer requires a restart on Plus UI
- fixed issue with high CPU load when using SbieCtrl to change settings

### Fixed
- fixed issue with Firefox/Chromium browsers that have been compiled with the MinGW toolchain [#538](https://github.com/sandboxie-plus/Sandboxie/issues/538)
- fixed issues with folder recovery on Plus UI [#1840](https://github.com/sandboxie-plus/Sandboxie/issues/1840) [#1380](https://github.com/sandboxie-plus/Sandboxie/issues/1380)



## [1.0.20 / 5.55.20] - 2022-05-02

### Fixed
- fixed issue with Firefox video playback introduced in the previous build [#1831](https://github.com/sandboxie-plus/Sandboxie/issues/1831)
- fixed driver-related BSoD [#1811](https://github.com/sandboxie-plus/Sandboxie/issues/1811)
- fixed issue with editing start restriction entries
- fixed issue with the network options tab [#1825](https://github.com/sandboxie-plus/Sandboxie/issues/1825)
- fixed portable mode issue if SandMan is run as admin [#1764](https://github.com/sandboxie-plus/Sandboxie/issues/1764)



## [1.0.19 / 5.55.19] - 2022-04-21

### Added
- added drag and drop support for groups [#1775](https://github.com/sandboxie-plus/Sandboxie/issues/1775)
- added Del key support to the box view for all entry types [#1779](https://github.com/sandboxie-plus/Sandboxie/issues/1779)
- added warning when trying to run explorer.exe in a box with COM open [#1716](https://github.com/sandboxie-plus/Sandboxie/issues/1716)

### Fixed
- fixed crash issue in the SandMan UI [#1772](https://github.com/sandboxie-plus/Sandboxie/issues/1772)
- fixed issue with some installers when EnableObjectFiltering is enabled [#1795](https://github.com/sandboxie-plus/Sandboxie/issues/1795)
- fixed allowing NtCreateSymbolicLinkObject to be safely used in the sandbox
- added workaround for a Vivaldi hooking issue [#1783](https://github.com/sandboxie-plus/Sandboxie/issues/1783)
  - Note: this fix is provisional, it can be disabled with UseVivaldiWorkaround=n
- fixed registry issue with snapshots [#1782](https://github.com/sandboxie-plus/Sandboxie/issues/1782)
- fixed issue with box grouping [#1778](https://github.com/sandboxie-plus/Sandboxie/issues/1778) [#1777](https://github.com/sandboxie-plus/Sandboxie/issues/1777) [#1776](https://github.com/sandboxie-plus/Sandboxie/issues/1776)
- fixed further issues with box grouping [#1698](https://github.com/sandboxie-plus/Sandboxie/issues/1698) [#1697](https://github.com/sandboxie-plus/Sandboxie/issues/1697)
- fixed issues with snapshot UI [#1696](https://github.com/sandboxie-plus/Sandboxie/issues/1696) [#1695](https://github.com/sandboxie-plus/Sandboxie/issues/1695)
- fixed issue with recovery dialog focus [#1374](https://github.com/sandboxie-plus/Sandboxie/issues/1374)



## [1.0.18 / 5.55.18] - 2022-04-13

### Added
- added minor browsers to the BlockSoftwareUpdaters template (by APMichael) [#1784](https://github.com/sandboxie-plus/Sandboxie/pull/1784)

### Changed
- failed memory read attempts to unboxed processes will no longer cause message 2111 by default
  - Note: the message can still be enabled in the settings with "NotifyProcessAccessDenied=y"
- reordered the BlockSoftwareUpdaters template (by APMichael) [#1785](https://github.com/sandboxie-plus/Sandboxie/pull/1785)

### Fixed
- fixed pipe impersonation in compartment mode
- fixed issue with box clean-up introduced in a recent build
- fixed missing trace log clean-up command [#1773](https://github.com/sandboxie-plus/Sandboxie/issues/1773)
- fixed inability to unpin programs that have been pinned to the Run Menu [#1694](https://github.com/sandboxie-plus/Sandboxie/issues/1694)



## [1.0.17 / 5.55.17] - 2022-04-02

### Added
- added checkbox for easy read access to memory of unsandboxed processes (old Sbie behaviour, not recommended)

### Changed
- improved OpenProcess/OpenThread logging

### Fixed
- fixed crash issue with the new monitor mode
- fixed issue with resource access entry parsing



## [1.0.16 / 5.55.16] - 2022-04-01

### Added
- FIXED SECURITY ISSUE ID-20: memory of unsandboxed processes can no longer be read, exceptions are possible
  - you can use ReadIpcPath=$:program.exe to allow read access to unsandboxed processes or processes in other boxes
- added "Monitor Mode" to the resource access trace, similar to the old monitor view of SbieCtrl.exe

### Changed
- EnableObjectFiltering is now set enabled by default, and replaces Sbie's old process/thread handle filter
- the `$:` syntax now accepts a wildcard `$:*` no more specialized wildcards though

### Fixed
- fixed NtGetNextProcess being fully disabled instead of properly filtered
- fixed reworked image name resolution when creating new processes in a sandbox
- fixed regression with HideOtherBoxes=y [#1743](https://github.com/sandboxie-plus/Sandboxie/issues/1743) [#1666](https://github.com/sandboxie-plus/Sandboxie/issues/1666)



## [1.0.15 / 5.55.15] - 2022-03-24

### Fixed
- fixed memory corruption introduced in the last build causing Chrome to crash sometimes
- FIXED SECURITY ISSUE ID-18: NtCreateSymbolicLinkObject was not filtered (thanks Diversenok)



## [1.0.14 / 5.55.14] - 2022-03-23

### Added
- added notification to warn that the default update checker is lagging behind the newest release on GitHub, to ensure that only bug-free builds are offered as updates [#1682](https://github.com/sandboxie-plus/Sandboxie/issues/1682)
- added main browsers to BlockSoftwareUpdaters template (by Dyras) [#1630](https://github.com/sandboxie-plus/Sandboxie/pull/1630)
- added a warning when Sandboxie-Plus.ini is not writeable [#1681](https://github.com/sandboxie-plus/Sandboxie/issues/1681)
- added clean-up for critical sections (by chunyou128) [#1686](https://github.com/sandboxie-plus/Sandboxie/pull/1686)

### Changed
- improved command line handling for breakout processes [#1655](https://github.com/sandboxie-plus/Sandboxie/issues/1655)
- disabled SBIE2193 notification (by isaak654) [#1690](https://github.com/sandboxie-plus/Sandboxie/pull/1690)
- improved error message 6004 [#1719](https://github.com/sandboxie-plus/Sandboxie/issues/1719)

### Fixed
- fixed dark mode issue with the new tray list
- fixed not showing a warning when Sandboxie-Plus.ini is not writeable [#1681](https://github.com/sandboxie-plus/Sandboxie/issues/1681)
- fixed issue with software compatibility checkbox (thanks MitchCapper) [#1678](https://github.com/sandboxie-plus/Sandboxie/issues/1678)
- fixed issue with events on box closure not always being executed [#1658](https://github.com/sandboxie-plus/Sandboxie/issues/1658)
- fixed memory leaks in key_merge.c
- fixed issue enumerating registry keys in privacy mode
- fixed settings issue introduced in 1.0.13 [#1684](https://github.com/sandboxie-plus/Sandboxie/issues/1684)
- fixed crash issue when parsing firewall port options
- FIXED SECURITY ISSUE ID-19: in certain cases a sandboxed process could obtain a handle on an unsandboxed thread with write privileges [#1714](https://github.com/sandboxie-plus/Sandboxie/issues/1714)



## [1.0.13 / 5.55.13] - 2022-03-08

### Fixed
- FIXED SECURITY ISSUE ID-17: Hard link creation was not properly filtered (thanks Diversenok)
- fixed issue with checking the certificate entry



## [1.0.12 / 5.55.12] - 2022-03-02

### Added
- added mini dump creation to SandMan.exe in case it crashes

### Changed
- disabled Chrome and Firefox phishing entries in new sandboxes (by isaak654) [#1616](https://github.com/sandboxie-plus/Sandboxie/pull/1616)
- updated Mozilla paths for the BlockSoftwareUpdaters template (by isaak654) [#1623](https://github.com/sandboxie-plus/Sandboxie/pull/1623)
- renamed "Pause Forced Programs Rules" command to "Pause Forcing Programs" (Plus only)
- reworked tray icon generation now using overlays, added busy overlay

### Fixed
- fixed issue with accessing network drives in privacy mode [#1617](https://github.com/sandboxie-plus/Sandboxie/issues/1617)
- fixed issue with ping in compartment mode [#1608](https://github.com/sandboxie-plus/Sandboxie/issues/1608)
- fixed SandMan UI freezing when a lot of processes are created and closed in a box [#1607](https://github.com/sandboxie-plus/Sandboxie/issues/1607)
- fixed editing of command line entry not being recognized in the Run Menu [#1648](https://github.com/sandboxie-plus/Sandboxie/issues/1648)
- fixed blue screen issue in driver (thanks Diversenok)
- fixed incompatibility with Windows 11 Insider build 22563.1 [#1654](https://github.com/sandboxie-plus/Sandboxie/issues/1654)



## [1.0.11 / 5.55.11] - 2022-02-13

### Added
- added optional tray notification when box content gets auto-deleted
- added FreeDownloadManager template
- added warning when opening unsandboxed regedit [#1606](https://github.com/sandboxie-plus/Sandboxie/issues/1606)
- added languages files that were missing in official Qt 5.15.2 (by DevSplash) [#1605](https://github.com/sandboxie-plus/Sandboxie/pull/1605)

### Changed
- the asynchronous box operations introduced in the last build are now disabled by default
- moved sys tray options from general to shell integration tab
- removed "AlwaysUseWin32kHooks", now these Win32 hooks are always enabled
  - Note: you can use "UseWin32kHooks=program.exe,n" to disable them for selected programs
- updated Listary template to v6 (by isaak654) [#1610](https://github.com/sandboxie-plus/Sandboxie/pull/1610)

### Fixed
- fixed compatibility issue with SECUROM [#1597](https://github.com/sandboxie-plus/Sandboxie/issues/1597)
- fixed modality issue [#1615](https://github.com/sandboxie-plus/Sandboxie/issues/1615)
- fixed special form of OpenWinClass in Templates.ini [d6d9588](https://github.com/sandboxie-plus/Sandboxie/commit/d6d95889a91d31dd55dd2b2d136d8f80c9a8ea71)



## [1.0.10 / 5.55.10] - 2022-02-06

### Added
- added option to show only boxes in tray with running processes [#1186](https://github.com/sandboxie-plus/Sandboxie/issues/1186)
  - additional option shows only pinned boxes, in box options a box can be set to be always shown in tray list (Pinned)
- added Options menu command to reset the GUI [#1589](https://github.com/sandboxie-plus/Sandboxie/issues/1589)
- added "Run Un-Sandboxed" context menu option
- added new trigger "OnBoxDelete" that allows to specify a command that is run UNBOXED just before the box content gets deleted
  - Note: this can be used as a replacement to "DeleteCommand" [#591](https://github.com/sandboxie-plus/Sandboxie/issues/591)
- selected box operations (deletion) no longer show the progress dialog [#1061](https://github.com/sandboxie-plus/Sandboxie/issues/1061)
  - if a box with a running operation shows a blinking hour glass icon, the context menu can be used to cancel the operation

### Changed
- "HideHostProcess=program.exe" can now be used to hide Sandboxie services [#1336](https://github.com/sandboxie-plus/Sandboxie/issues/1336)
- updater blocking is now done using a template called BlockSoftwareUpdaters
- enhanced "StartProgram=..." makes "StartCommand=..." obsolete
  - for same functionality as "StartCommand=...", use "StartProgram=%SbieHome%\Start.exe ..."
- merged "Auto Start" General tab with the "Auto Exec" Advanced tab into a universal "Triggers" Advanced tab

### Fixed
- fixed a couple issues with the new breakout process feature and improved security (thanks Diversenok)
- fixed issues with re-opening windows already open [#1584](https://github.com/sandboxie-plus/Sandboxie/issues/1584)
- fixed issue with desktop access [#1588](https://github.com/sandboxie-plus/Sandboxie/issues/1588)
- fixed issue about command line invocation handling [#1133](https://github.com/sandboxie-plus/Sandboxie/issues/1133)
- fixed UI issue with main window state when switching always on top attribute [#1169](https://github.com/sandboxie-plus/Sandboxie/issues/1169)
- fixed issue with box context menu in tray list [1106](https://github.com/sandboxie-plus/Sandboxie/issues/1106)
- fixed issue with "AutoExec=..."
- fixed issues where cancelling box deletion operations didn't work [#1061](https://github.com/sandboxie-plus/Sandboxie/issues/1061)
- fixed issue with DPI scaling and colour picker dialog [#803](https://github.com/sandboxie-plus/Sandboxie/issues/803)

### Removed
- removed "UseRpcMgmtSetComTimeout=AppXDeploymentClient.dll,y" used for Free Download Manager as it broke other things
  - only if you use Free Download Manager together with the setting "RpcMgmtSetComTimeout=n" in a sandbox, you have to add the line manually to your Sandboxie.ini



## [1.0.9 / 5.55.9] - 2022-01-31

### Added
- SandMan now causes all boxed processes to update their path settings in real time when access options were modified
- added new maintenance menu option "Uninstall All" to quickly remove all components when running in portable mode
- added version number to the title bar of Sandboxie Classic
- added option to return not to a snapshot but to an empty box state while keeping all snapshots
- Sandboxie-Plus.ini can now be placed in C:\ProgramData\Sandboxie-Plus\ folder and takes precedence (for business use)
- added support for AF_UNIX on Windows to resolve issues with OpenJDK17 and later [#1009](https://github.com/sandboxie-plus/Sandboxie/issues/1009) [#1520](https://github.com/sandboxie-plus/Sandboxie/issues/1520) [#1521](https://github.com/sandboxie-plus/Sandboxie/issues/1521)

### Changed
- reworked breakout mechanism to be service based and not allowing the parent process to access the broken out child process
- enabled creation of directory junctions for sandboxed processes [#1375](https://github.com/sandboxie-plus/Sandboxie/issues/1375)
- restored back "AutoRecover=y" on box creation [#1554](https://github.com/sandboxie-plus/Sandboxie/discussions/1554)
- improved snapshot support [#1220](https://github.com/sandboxie-plus/Sandboxie/issues/1220)
- renamed "Disable Forced Programs" command to "Pause Forced Programs Rules" (Plus only)

### Fixed
- fixed BreakoutProcess not working with "EnableObjectFiltering=y"
- FIXED SECURITY ISSUE ID-16: when starting *COMSRV* unboxed, the returned process handle had full access
- fixed issue with progress dialog [#1562](https://github.com/sandboxie-plus/Sandboxie/issues/1562)
- fixed issue with handling directory junctions in Sandboxie [#1396](https://github.com/sandboxie-plus/Sandboxie/issues/1396)
- fixed a handle leak in File_NtCloseImpl
- fixed border issues on maximized windows introduced in the last build [#1561](https://github.com/sandboxie-plus/Sandboxie/issues/1561)
- fixed a couple of index overruns (thanks 7eRoM) [#1571](https://github.com/sandboxie-plus/Sandboxie/pull/1571)
- fixed issues with sysnative directory [#1403](https://github.com/sandboxie-plus/Sandboxie/issues/1403)
- fixed issue with starting SandMan when running sandboxed from context menu [#1579](https://github.com/sandboxie-plus/Sandboxie/issues/1579)
- fixed dark mode flash issue with main window creation [#1231](https://github.com/sandboxie-plus/Sandboxie/issues/1231#issuecomment-1024469681)
- fixed issues with snapshot error handling [#350](https://github.com/sandboxie-plus/Sandboxie/issues/350)
- fixed issues with the always on top option (Plus only)



## [1.0.8 / 5.55.8] - 2022-01-18

### Added
- added Portuguese of Portugal on Plus UI (by JNylson, isaak654, mpheath) [#1497](https://github.com/sandboxie-plus/Sandboxie/pull/1497)
- added "BreakoutProcess=program.exe", with this option selected applications can be started unboxed from within a box [#1500](https://github.com/sandboxie-plus/Sandboxie/issues/1500)
  - the program image must be located outside the sandbox for this to work
  - if another sandbox has "ForceProcess=program.exe" configured, it will capture the process
  - use case: set up a box with a Web browser forced, when another box opens a website, this will happen in the dedicated browser box
  - Note: "BreakoutFolder=some\path" is also available
- added silent uninstall switch `/remove /S` for Classic installer (by sredna) [#1532](https://github.com/sandboxie-plus/Sandboxie/pull/1532)

### Changed
- the filename "sandman_pt" was changed to "sandman_pt_BR" (Brazilian Portuguese) [#1497](https://github.com/sandboxie-plus/Sandboxie/pull/1497)
- the filename "sandman_ua" was changed to "sandman_uk" (Ukrainian) [#1527](https://github.com/sandboxie-plus/Sandboxie/issues/1527)
  - Note: translators are encouraged to follow the [Localization notes and tips](https://github.com/sandboxie-plus/Sandboxie/discussions/1123#discussioncomment-1203489) before creating a new pull request
- updated Firefox update blocker (discovered by isaak654) [#1545](https://github.com/sandboxie-plus/Sandboxie/issues/1545#issuecomment-1013807831)

### Fixed
- fixed issue with opening all file access OpenFilePath=* [#971](https://github.com/sandboxie-plus/Sandboxie/issues/971)
- fixed issue with opening network shares [#1529](https://github.com/sandboxie-plus/Sandboxie/issues/1529)
- fixed possible upgrade issue with Classic installer (by isaak654) [130c43a](https://github.com/sandboxie-plus/Sandboxie/commit/130c43a62c9778b734fa625bf4f46b12d0701719)
- fixed minor issues with Classic installer (by sredna) [#1533](https://github.com/sandboxie-plus/Sandboxie/pull/1533)
- fixed issue with Ldr_FixImagePath_2 [#1507](https://github.com/sandboxie-plus/Sandboxie/issues/1507)
- when using "Run Sandboxed" with SandMan UI and the UI is off, it will stay off
- fixed issue with Util_GetProcessPidByName that should resolve the driver sometimes failing to start at boot [#1451](https://github.com/sandboxie-plus/Sandboxie/issues/1451)
- SandMan will now run in background like SbieCtrl when starting a boxed process [post506](https://forum.xanasoft.com/threads/direct-start-firefox-in-box.173/post-506)
- fixed taskbar not showing with persistent box border in full screen [post474](https://forum.xanasoft.com/threads/taskbar-problems-when-using-sandboxie-in-windows-11.224/post-474)
- fixed box border not spanning across multiple monitors [#1512](https://github.com/sandboxie-plus/Sandboxie/issues/1512)
- fixed issues with border when using DPI scaling [#1506](https://github.com/sandboxie-plus/Sandboxie/issues/1506)
- fixed DPI issues with Qt [#1368](https://github.com/sandboxie-plus/Sandboxie/issues/1368)
- fixed issue with bright flashing on window creation when in dark mode [#1231](https://github.com/sandboxie-plus/Sandboxie/issues/1231)
- fixed issues with the PortableRootDir setting [#1509](https://github.com/sandboxie-plus/Sandboxie/issues/1509)
- fixed issue with the settings window crashing when the driver was not connected
- fixed DPI issues with Finder Tool [#912](https://github.com/sandboxie-plus/Sandboxie/issues/912)
- fixed another issue with reused process IDs [#1547](https://github.com/sandboxie-plus/Sandboxie/issues/1547)
- fixed issue introduced in 1.0.6 related to SeAccessCheckByType [#1548](https://github.com/sandboxie-plus/Sandboxie/issues/1548)



## [1.0.7 / 5.55.7] - 2022-01-06

### Added
- added experimental option "CreateToken=y" to create a new token instead of repurposing an existing one
- added option "DisableRTBlacklist=y" allowing to disable the hardcoded runtime class blacklist
- added new template "DeviceSecurity" to lock down access to device drivers on the system
  - Note: this template requires RuleSpecificity being available to work properly
- added option to set a custom INI editor in the Plus UI [#1475](https://github.com/sandboxie-plus/Sandboxie/issues/1475)
- added option "LingerLeniency=n" to solve issue [#997](https://github.com/sandboxie-plus/Sandboxie/issues/997)

### Changed
- reworked syscall invocation code in the driver
  - Win32k hooking is now compatible with HVCI [#1483](https://github.com/sandboxie-plus/Sandboxie/issues/1483)

### Fixed
- fixed memory leak in driver (conf_user.c)
- fixed issue with file renaming in open paths introduced in 1.0.6
- fixed issue causing Chromium browsers not closing properly [#1496](https://github.com/sandboxie-plus/Sandboxie/issues/1496)
- fixed issue with Start.exe [#1517](https://github.com/sandboxie-plus/Sandboxie/issues/1517) [#1516](https://github.com/sandboxie-plus/Sandboxie/issues/1516)
- fixed SandMan issue with reused process IDs
- fixed KmdUtil sometimes not properly terminating the driver [#1493](https://github.com/sandboxie-plus/Sandboxie/issues/1493)

### Removed
- removed OpenToken as it is only a shorthand for UnrestrictedToken=y and UnfilteredToken=y set together



## [1.0.6 / 5.55.6] - 2021-12-31

### Added
- replaced "Open with" with a Sandboxie dialog to work on Windows 10 [#1138](https://github.com/sandboxie-plus/Sandboxie/issues/1138)
- added ability to run Win32 store apps in Application Compartment mode (requires COM to be open in Windows 11)
  - Note: this does not mean UWP store apps, just regular Win32 apps packaged to be deployed via the store
- added new debug options "UnstrippedToken=y" and "KeepUserGroup=y"
- added double-click to recover files and folders in recovery window [#1466](https://github.com/sandboxie-plus/Sandboxie/issues/1466)
- added Ukrainian language on Plus UI (by SuperMaxusa) [#1488](https://github.com/sandboxie-plus/Sandboxie/pull/1488)

### Changed
- "UseSbieWndStation=y" is now the default behaviour [#1442](https://github.com/sandboxie-plus/Sandboxie/issues/1442)
- disabled Win32k hooking when HVCI is enabled due to an incompatibility (BSoD) [#1483](https://github.com/sandboxie-plus/Sandboxie/issues/1483)

### Fixed
- fixed box initialization issue in privacy mode [#1469](https://github.com/sandboxie-plus/Sandboxie/issues/1469)
- fixed issue with shortcuts creation introduced in a recent build [#1471](https://github.com/sandboxie-plus/Sandboxie/issues/1471)
- fixed various issues in Privacy Enhanced boxes and rule specificity
- fixed issue with SeAccessCheckByType and alike
- fixed issues with Win32k hooking on 32-bit Windows [#1479](https://github.com/sandboxie-plus/Sandboxie/issues/1479)

### Removed
- removed obsolete SkyNet rootkit detection from 32-bit build



## [1.0.5 / 5.55.5] - 2021-12-25

### Added
- sandbox top level exception handler to create crash dumps
  - it can be enabled per process or globally using "EnableMiniDump=process.exe,y" or "EnableMiniDump=y" respectively
  - the dump flags can be set as hex with "MiniDumpFlags=0xAABBCCDD"
  - a preselected flag set for a verbose dump can be set with "MiniDumpFlags=Extended"
  - Note: dump files created with the EnableMiniDump option are located at: `C:\Sandbox\%USER%\%SANDBOX%`
- added template support for Osiris and Slimjet browsers (by Dyras) [#1454](https://github.com/sandboxie-plus/Sandboxie/pull/1454)

### Changed
- improved SbieDll initialization
- doubled size of Name_Buffer_Depth [#1342](https://github.com/sandboxie-plus/Sandboxie/issues/1342)
- improved text filter in the templates view [#1456](https://github.com/sandboxie-plus/Sandboxie/issues/1456)

### Fixed
- fixed issue with forced process display [#1447](https://github.com/sandboxie-plus/Sandboxie/issues/1447)
- fixed crash issue with GetClassName [#1448](https://github.com/sandboxie-plus/Sandboxie/issues/1448)
- fixed minor UI issue [#1382](https://github.com/sandboxie-plus/Sandboxie/issues/1382)
- fixed UI language preset issue [#1348](https://github.com/sandboxie-plus/Sandboxie/issues/1348)
- fixed grouping issues in SandMan UI [#1358](https://github.com/sandboxie-plus/Sandboxie/issues/1358)
- fixed issue with EnableWin32kHooks [#1458](https://github.com/sandboxie-plus/Sandboxie/issues/1458)

### Installers re-released with the following fix:
- fixed regression when launching Office apps [#1468](https://github.com/sandboxie-plus/Sandboxie/issues/1468)



## [1.0.4 / 5.55.4] - 2021-12-20

### Added
- mechanism to hook Win32 system calls now also works for 32-bit applications running under WoW64
- added customization to Win32k hooking mechanism, as by default only GdiDdDDI* hooks are installed
  - you can force the installation of other hooks by specifying them with "EnableWin32Hook=..."
  - or disable the installation of the default hooks with "DisableWin32Hook=..."
  - please note that some Win32k hooks may cause BSoDs or undefined behaviour (!)
  - the most obviously problematic Win32k hooks are blacklisted, this can be bypassed with "IgnoreWin32HookBlacklist=y"
- added debug option "AdjustBoxedSystem=n" to disable the adjustment of service ACLs running with a system token
- added "NoUACProxy=y" option together with the accompanying template, in order to disable UAC proxy
  - Note: boxes configured in compartment mode activate this template by default
- added UI option to change default RpcMgmtSetComTimeout preset
- added Plus installer option to start the default browser under Sandboxie through a desktop shortcut
- added more entries to the Plus installer (current translations on [Languages.iss](https://github.com/sandboxie-plus/Sandboxie/blob/master/Installer/Languages.iss) file need to be updated)

### Changed
- "EnableWin32kHooks=y" is now enabled by default, as no issues were reported in 1.0.3
  - Note: currently only the GdiDdDDI* hooks are applied, required for Chromium HW acceleration
- cleaned up low-level hooking code
- "RunRpcssAsSystem=y" is now auto applied for sandboxes in Application Compartment mode when "RunServicesAsSystem=y" or "MsiInstallerExemptions=y" are present

### Fixed
- fixed RPC handling in case a requested open service is not running [#1443](https://github.com/sandboxie-plus/Sandboxie/issues/1443)
- fixed a hooking issue with NdrClientCall2 in 32-bit applications
- fixed issue with start directory to run sandboxed when using SandMan [#1436](https://github.com/sandboxie-plus/Sandboxie/issues/1436)
- fixed issue with recovering from network share locations [#1435](https://github.com/sandboxie-plus/Sandboxie/issues/1435)



## [1.0.3 / 5.55.3] - 2021-12-12

### Added
- added mechanism to hook Win32k system calls on Windows 10 and later, this should resolve the issue with Chromium HW acceleration
  - Note: this mechanism does not, yet, work for 32-bit applications running under WoW64
  - to enable it, add "EnableWin32kHooks=y" to the global INI section, this feature is highly experimental (!)
  - the hooks will be automatically applied to Chromium GPU processes
  - to force Win32k hooks for all processes in a selected box, add "AlwaysUseWin32kHooks=program.exe,y" [#1261](https://github.com/sandboxie-plus/Sandboxie/issues/1261) [#1395](https://github.com/sandboxie-plus/Sandboxie/issues/1395)

### Fixed
- fixed bug in GetVersionExW making "OverrideOsBuild=..." not working [#605](https://github.com/sandboxie-plus/Sandboxie/issues/605) [#1426](https://github.com/sandboxie-plus/Sandboxie/issues/1426)
- fixed issue with some UTF-8 characters when used in the INI file
- fixed isolation issue with Virtual Network Editor [#1102](https://github.com/sandboxie-plus/Sandboxie/issues/1102)



## [1.0.2 / 5.55.2] - 2021-12-08

### Fixed
- fixed recovery window not refreshing count on reload [#1402](https://github.com/sandboxie-plus/Sandboxie/issues/1402)
- fixed printing issue introduced in 1.0.0 [#1397](https://github.com/sandboxie-plus/Sandboxie/issues/1397)
- fixed issues with CreateProcess function [#1408](https://github.com/sandboxie-plus/Sandboxie/issues/1408)



## [1.0.1 / 5.55.1] - 2021-12-06

### Added
- added checkboxes to most major box options lists
- added SumatraPDF templates (by Dyras) [#1391](https://github.com/sandboxie-plus/Sandboxie/pull/1391)

### Changed
- rolled back change to "OpenClsid=..." handling
- made all major lists in the box options editable

### Fixed
- fixed issue with read only paths introduced in 1.0.0
- fixed BSoD issue introduced in the 1.0.0 build [#1389](https://github.com/sandboxie-plus/Sandboxie/issues/1389)
- fixed multiple BITS notifications while running sandboxed Chromium browsers (by isaak654) [ca320ec](https://github.com/sandboxie-plus/Sandboxie/commit/ca320ecc17180ff09a67bdefc524b30cf3540c08) [#1081](https://github.com/sandboxie-plus/Sandboxie/issues/1081)
- fixed executables selection for Run Menu entries (by isaak654) [#1379](https://github.com/sandboxie-plus/Sandboxie/issues/1379)
- fixed SetCursorPos and ClipCursor ignoring DPI awareness (by alvinhochun) [#1394](https://github.com/sandboxie-plus/Sandboxie/pull/1394)

### Removed
- removed Virtual Desktop Manager template (by isaak654) [d775807](https://github.com/sandboxie-plus/Sandboxie/commit/d7758071f6930539c4e1f236297b4cfa332346ad) [#1326](https://github.com/sandboxie-plus/Sandboxie/discussions/1326)



## [1.0.0 / 5.55.0] - 2021-11-17

### Added
- added Privacy Enhanced mode, sandboxes with "UsePrivacyMode=y" will not allow read access to locations containing user data
  - all locations except generic Windows system paths will need to be opened explicitly for read and/or write access
  - using "NormalFilePath=...", "NormalKeyPath=...", "NormalIpcPath=..." allows to open locations to be readable and sandboxed
- added new Application Compartment mode of operation, it is enabled by adding "NoSecurityIsolation=y" to the sandbox config
  - in this mode, security is traded in for compatibility, it should not be used for untrusted applications
  - Note: in this mode, file and registry filtering are still in place, hence processes run without administrative privileges
  - it is reasonably safe, all filtering can be disabled with "NoSecurityFiltering=y"
- added experimental use of ObRegisterCallbacks to filter object creation and duplication
  - this filtering is independent from the regular SbieDrv's syscall-based filtering, hence it also applies to Application Compartments
  - with it enabled, an application running in a compartment will not be able to manipulate processes running outside the sandbox
  - Note: this feature improves the security of non-isolated Application Compartment sandboxes
  - to enable this feature, set "EnableObjectFiltering=y" in the global section and reload the driver
  - when globally activated, the filtering can be disabled for individual boxes with "DisableObjectFilter=y"
- added "DontOpenForBoxed=n", this option disables the discrimination of boxed processes for open file and open key directives
  - this behaviour does not really improve security anyway, but may be annoying, also Application Compartments always disable this
- added setting to entirely open access to the COM infrastructure

### Changed
- reworked the resource access path matching mechanism to optionally apply more specific rules over less specific ones
  - for example "OpenFilePath=C:\User\Me\AppData\Firefox takes precedence over "WriteFilePath=C:\User\Me\"
  - to enable this new behaviour, add "UseRuleSpecificity=y" to your Sandboxie.ini, this behaviour is always enabled in Privacy Enhanced mode
  - added "NormalFilePath=..." to restore default Sandboxie behaviour on a given path
  - added "OpenConfPath=...", which similarly to "OpenPipePath=..." is a "OpenKeyPath=..." variant which applies to executables located in the sandbox
- removed option to copy a box during creation, instead the box context menu offers a duplication option
- reworked the box creation dialog to offer new box types

### Fixed
- fixed SBIE1401 notification during Sandboxie Plus uninstall (by mpheath) [68fa37d](https://github.com/sandboxie-plus/Sandboxie/commit/68fa37d45be2be3565917d0de097709b7aa009e0)
- fixed memory leak in driver handling FLT_FILE_NAME_INFORMATION (by Therzok) [#1371](https://github.com/sandboxie-plus/Sandboxie/pull/1371)



## [0.9.8d / 5.53.3] - 2021-11-01

### Added
- added checkbox if the user wants SandMan.exe to be started after installation [#1318](https://github.com/sandboxie-plus/Sandboxie/issues/1318)
- added template for Windows 10 virtual desktop manager [#1326](https://github.com/sandboxie-plus/Sandboxie/discussions/1326)

### Changed
- "OpenClsid=..." is no longer restricted to CLSCTX_LOCAL_SERVER execution contexts only
  - this allows to run objects with the CLSCTX_INPROC_SERVER flag in the COM helper service
- in the trace view, now multiple types can be selected at once
- a few Plus UI entries were made translatable (by gexgd0419) [#1320](https://github.com/sandboxie-plus/Sandboxie/pull/1320)
- changed default "terminate all boxed processes" key to Shift+Pause (by isaak654) [#1337](https://github.com/sandboxie-plus/Sandboxie/issues/1337)

### Fixed
- fixed INI writing issue with SbieCtrl and the new INI handling mechanism [#1331](https://github.com/sandboxie-plus/Sandboxie/issues/1331)
- fixed issue with trace log filtering
- fixed space issue about German language on Plus installer (by mpheath) [#1333](https://github.com/sandboxie-plus/Sandboxie/issues/1333)
- restored Waterfox phishing template entries with a proper fix (by APMichael) [#1334](https://github.com/sandboxie-plus/Sandboxie/issues/1334)



## [0.9.8c / 5.53.2] - 2021-10-24

### Added
- added explicit lines on Plus installer to delete empty shell registry keys at uninstall time (by mpheath) [3f661a8](https://github.com/sandboxie-plus/Sandboxie/commit/3f661a8d49137b6d2c3e00757952c71b0df11e4d)

### Fixed
- fixed template sections not showing in editor [#1287](https://github.com/sandboxie-plus/Sandboxie/issues/1287)
- fixed autodelete box content broken in the previous build [#1296](https://github.com/sandboxie-plus/Sandboxie/issues/1296) [#1324](https://github.com/sandboxie-plus/Sandboxie/issues/1324)
- fixed crash in "Browse Content" window [#1313](https://github.com/sandboxie-plus/Sandboxie/issues/1313)
- fixed issue with icon resolution [#1310](https://github.com/sandboxie-plus/Sandboxie/issues/1310)
- fixed invalid "No Inet" status in the status column [#1312](https://github.com/sandboxie-plus/Sandboxie/issues/1312)
- fixed Windows Explorer search box not working (by isaak654) [#1002](https://github.com/sandboxie-plus/Sandboxie/issues/1002)
- fixed Waterfox phishing template (by Dyras) [#1309](https://github.com/sandboxie-plus/Sandboxie/pull/1309)
- fixed issue with Chinese translation files on Plus installer (by mpheath) [#1317](https://github.com/sandboxie-plus/Sandboxie/issues/1317)
- fixed autorun registry key path on Plus installer (by mpheath) [abd2d44](https://github.com/sandboxie-plus/Sandboxie/commit/abd2d44cd6f305da956ad70c7481cb1256efff24)
- fixed memory corruption in SbieSvc.exe



## [0.9.8b / 5.53.1] - 2021-10-19

### Added
- added ability to save trace log to file on Plus UI
- added French language on Plus UI (by clexanis) [#1155](https://github.com/sandboxie-plus/Sandboxie/issues/1155)

### Changed
- network traffic trace is now properly logged to the driver log instead of to the kernel debug log
- Plus installer will autostart SandMan.exe after install to fix a taskbar icon issue [#post-3040211](https://www.wilderssecurity.com/threads/sandboxie-plus-0-9-7-test-build.440906/page-4#post-3040211)
- Classic installer will show the license agreement when updating [#1187](https://github.com/sandboxie-plus/Sandboxie/issues/1187)

### Fixed
- fixed template sections not showing in editor [#1287](https://github.com/sandboxie-plus/Sandboxie/issues/1287)
- fixed issue with app ID resulting in some apps showing two button groups in the taskbar [#1101](https://github.com/sandboxie-plus/Sandboxie/issues/1101)
- fixed issue with maximum INI value length on Plus UI [#1293](https://github.com/sandboxie-plus/Sandboxie/issues/1293)
- fixed issue handling an empty Sandboxie.ini that got introduced recently [#1292](https://github.com/sandboxie-plus/Sandboxie/issues/1292)
- fixed issue with "SpecialImages" template (by Coverlin) [#1288](https://github.com/sandboxie-plus/Sandboxie/issues/1288) [#1289](https://github.com/sandboxie-plus/Sandboxie/issues/1289)
- fixed issue with box emptying [#1296](https://github.com/sandboxie-plus/Sandboxie/issues/1296)
- fixed issues with some languages [#1304](https://github.com/sandboxie-plus/Sandboxie/issues/1304)
- fixed issue with mounted directories [#1302](https://github.com/sandboxie-plus/Sandboxie/issues/1302)
- added missing translation for Qt libraries [#1305](https://github.com/sandboxie-plus/Sandboxie/issues/1305)
- fixed issue with Windows compatibility assistant [#1265](https://github.com/sandboxie-plus/Sandboxie/issues/1265)
- fixed issue with specific process image settings [#1307](https://github.com/sandboxie-plus/Sandboxie/issues/1307)



## [0.9.8 / 5.53.0] - 2021-10-15

### Added
- added debug switch to disable Sbie console redirection "NoSandboxieConsole=y"
  - Note: this was previously part of "NoSandboxieDesktop=y"
- added Sbie+ version to the log [#1277](https://github.com/sandboxie-plus/Sandboxie/issues/1277)
- added uninstall clean-up of extra files for the Plus installer (by mpheath) [#1235](https://github.com/sandboxie-plus/Sandboxie/pull/1235)
- added set language for SandMan for the Plus installer (by mpheath) [#1241](https://github.com/sandboxie-plus/Sandboxie/issues/1241)
- added EventLog messages with SbieMsg.dll for the Plus installer (by mpheath)
- group expansion state is now saved
- added additional filters to the trace tab
- added a new section [DefaultTemplates] in Templates.ini which contains mandatory templates that are always applied [0c9ecb0](https://github.com/sandboxie-plus/Sandboxie/commit/0c9ecb084286821c0db7436c41ef99e3b9daca76#diff-965721e9c3f2350b16f4acb47d3fb75654976f0dbb4da3c507d0eaff16a4f5f2)

### Changed
- reworked and extended RPC logging
- reintroduced the "UseRpcMgmtSetComTimeout=some.dll,n" setting to be used when no "RpcPortBinding" entry is specified
  - this allows to enable/disable out of box RPC binding independently from the timeout setting
- the "BoxNameTitle" value can now be set explicitly on a per image name basis [#1190](https://github.com/sandboxie-plus/Sandboxie/issues/1190)

### Fixed
- fixed inability to delete read-only files from the sandboxed Windows Explorer [#1237](https://github.com/sandboxie-plus/Sandboxie/issues/1237)
- fixed wrong recovery target in Plus UI [#1274](https://github.com/sandboxie-plus/Sandboxie/issues/1274)
- fixed SBIE2101 issue introduced with 0.9.7a [#1279](https://github.com/sandboxie-plus/Sandboxie/issues/1279)
- fixed sorting in the box picker window [#1269](https://github.com/sandboxie-plus/Sandboxie/issues/1269)
- fixed tray refresh issue [#1250](https://github.com/sandboxie-plus/Sandboxie/issues/1250)
- fixed tray activity display [#1221](https://github.com/sandboxie-plus/Sandboxie/issues/1221)
- fixed recovery window not displaying in taskbar [#1195](https://github.com/sandboxie-plus/Sandboxie/issues/1195)
- fixed dark theme preset not updating in real time [#1270](https://github.com/sandboxie-plus/Sandboxie/issues/1270)
- fixed Microsoft Edge complaining about "FakeAdminRights=y" [#1271](https://github.com/sandboxie-plus/Sandboxie/issues/1271)
- fixed issue with using local template in the global section [#1212](https://github.com/sandboxie-plus/Sandboxie/issues/1212)
- fixed issue with git.exe from MinGW freezing [#1238](https://github.com/sandboxie-plus/Sandboxie/issues/1238)
- fixed issue with search highlighting in dark mode

### Removed
- removed the ability to sort the trace log as it took too much CPU



## [0.9.7e / 5.52.5] - 2021-10-09

### Changed
- reworked the settings handling once again, now the driver maintains the order when enumerating, but for good performance there is a Hash Map held in parallel for quick exact lookups



## [0.9.7d / 5.52.4] - 2021-10-06

### Fixed
- fixed yet another INI issue with the SbieCtrl



## [0.9.7c / 5.52.3] - 2021-10-05

### Fixed
- fixed yet another handling bug with SbieApi_EnumBoxesEx



## [0.9.7b / 5.52.2] - 2021-10-04

### Fixed
- fixed issue about loading a non-Unicode Sandboxie.ini that was introduced in the previous build



## [0.9.7 / 5.52.1] - 2021-10-02

### Added
- added forced process indicator to process status column [#1174](https://github.com/sandboxie-plus/Sandboxie/issues/1174)
- added "SbieTrace=y" option to trace the interaction between Sandboxie processes and Sandboxie core components
- when initializing an empty sandbox, MSI debug keys are set to generate the debug output of MSI installer service
- added "DisableComProxy=y" allowing to disable COM proxying through the service
- added "ProcessLimit=..." which allows limiting the maximum number of processes in a sandbox [#1230](https://github.com/sandboxie-plus/Sandboxie/issues/1230)
- added missing IPC logging

### Changed
- reworked SbieSvc INI server to allow settings caching and greatly improve performance
  - Now comments in the Sandboxie.ini are being preserved as well as the order of all entries
- enabled configuration section list replacement with a hash map to improve configuration performance
- improved progress and status messages for the Plus installer (by mpheath) [#1168](https://github.com/sandboxie-plus/Sandboxie/pull/1168)
- reworked RpcSs start mechanics, sandboxed RpcSs and DcomLaunch can now be run as system, use "RunRpcssAsSystem=y"
  - Note: this is generally not recommended for security reasons but may be needed for compatibility in some scenarios
- reworked WTSQueryUserToken handling to work properly in all scenarios
- reworked configuration value list to use a hash table for better performance

### Fixed
- fixed Plus upgrade install in Windows 7 (by mpheath) [#1194](https://github.com/sandboxie-plus/Sandboxie/pull/1194)
- fixed custom autoexec commands being executed on each box start instead of only during the initialization
- fixed a design issue limiting the maximum amount of processes per sandbox to 511
- fixed handle leaks in the lingering process monitor mechanism
- fixed issue with opening device paths like "\\??\\FltMgr"
- fixed build issue with an explicit FileDigestAlgorithm option for driver sign (by isaak654) [#1210](https://github.com/sandboxie-plus/Sandboxie/pull/1210)
- fixed issue with resource access log sometimes getting corrupted
- fixed issue with Microsoft Office Click-to-Run [#428](https://github.com/sandboxie-plus/Sandboxie/issues/428) [#882](https://github.com/sandboxie-plus/Sandboxie/issues/882)

### Removed
- removed support for Microsoft EMET (Enhanced Mitigation Experience Toolkit), as it was EOL in 2018
- removed support for Messenger Plus! Live, as MSN Messenger is EOL since 2013
- disabled Turkish language on Plus UI for inactivity (by isaak654) [#1215](https://github.com/sandboxie-plus/Sandboxie/pull/1215)



## [0.9.6 / 5.51.6] - 2021-09-12

### Added
- added ability to rename groups [#1152](https://github.com/sandboxie-plus/Sandboxie/issues/1152)
- added ability to define a custom order for the sandboxes, they can be moved by using the move context menu, or holding Alt + Arrow Key
- added recovery to list to the recovery window: [#988](https://github.com/sandboxie-plus/Sandboxie/issues/988)
- added finder to the recovery window

### Changed
- updated the BlockPort rule inside Template_BlockPorts to the new NetworkAccess format (by isaak654) [#1162](https://github.com/sandboxie-plus/Sandboxie/pull/1162)
- default for immediate recovery behaviour is now to show the recovery window instead of using the notifications window [#988](https://github.com/sandboxie-plus/Sandboxie/issues/988)
- the new run dialog now requires a double-click [#1171](https://github.com/sandboxie-plus/Sandboxie/issues/1171)
- reworked the recovery window

### Fixed
- fixed issue with create group menu [#1151](https://github.com/sandboxie-plus/Sandboxie/issues/1151)
- fixed issue that caused a box to lose its group association when renaming
- fixed issue with Thunderbird 91+ [#1156](https://github.com/sandboxie-plus/Sandboxie/issues/1156)
- fixed an issue with file disposition handling [#1161](https://github.com/sandboxie-plus/Sandboxie/issues/1161)
- fixed issue with Windows 11 22449.1000 [#1164](https://github.com/sandboxie-plus/Sandboxie/issues/1164)
- fixed SRWare Iron template (by Dyras) [#1146](https://github.com/sandboxie-plus/Sandboxie/pull/1146)
- fixed label positioning in Classic UI (by isaak654) [#1088](https://github.com/sandboxie-plus/Sandboxie/issues/1088)
- fixed an old issue that occurred when only an asterisk was set as path [#971](https://github.com/sandboxie-plus/Sandboxie/issues/971)



## [0.9.5 / 5.51.5] - 2021-08-30

### Added
- added option to run a sandbox in [session 0](https://techcommunity.microsoft.com/t5/ask-the-performance-team/application-compatibility-session-0-isolation/ba-p/372361)
  - Note: the processes then have a system token, hence it's recommended to enable "DropAdminRights=y"
- if the UI is run with admin privileges, it can terminate sandboxed processes in other sessions now
- added "StartSystemBox=" option to auto-run a box on Sbie start/system boot in session 0
  - Note: box start is done by issuing Start.exe /box:[name] auto_run
- add Start.exe auto_run command to start all sandboxed auto-start locations
- add Start.exe /keep_alive command line switch which keeps a process running in the box until it gracefully terminates
- added "StartCommand=" which starts a complex command through Start.exe on box startup
- added menu option to start regedit and load the box's registry key
- added system tray option in the Plus UI to show Classic icon [#963](https://github.com/sandboxie-plus/Sandboxie/issues/963#issuecomment-903933535)

### Changed
- changed command prompt icon and string from "Terminal" to "Command Prompt" [#1135](https://github.com/sandboxie-plus/Sandboxie/issues/1135)
- reworked box menu layout

### Fixed
- fixed driver compatibility with Windows Server 2022 (build 20348) [#1143](https://github.com/sandboxie-plus/Sandboxie/issues/1143)
- fixed issue with creating shortcuts [#1134](https://github.com/sandboxie-plus/Sandboxie/issues/1134)

### Installers re-released on 2021-08-31 with the following fix:
- fixed KmdUtil warning 1061 after Plus upgrade (by mpheath) [#968](https://github.com/sandboxie-plus/Sandboxie/issues/968) [#1139](https://github.com/sandboxie-plus/Sandboxie/issues/1139)



## [0.9.4 / 5.51.4] - 2021-08-22

### Added
- added clear commands to log submenus [#391](https://github.com/sandboxie-plus/Sandboxie/issues/391)
- added option to disable process termination prompt [#514](https://github.com/sandboxie-plus/Sandboxie/issues/514)
- added "Options/InstantRecovery" setting to Sandboxie-Plus.ini to use the recovery window instead of the notification pop-up [#988](https://github.com/sandboxie-plus/Sandboxie/issues/988)
- added ability to rename a non-empty sandbox [#1100](https://github.com/sandboxie-plus/Sandboxie/issues/1100)
- added ability to remove a non-empty sandbox
- added file browser window to SandMan UI to cover the file-view functionality of SbieCtrl [#578](https://github.com/sandboxie-plus/Sandboxie/issues/578)

### Changed
- generic errors in Sbie UI now show the status code as hex and provide a string description when available

### Fixed
- fixed "del" shortcut to terminate a process not always working
- fixed group display issue [#1094](https://github.com/sandboxie-plus/Sandboxie/issues/1094)
- fixed issue when using "Run Sandboxed" on a file that is already located in a sandbox [#1099](https://github.com/sandboxie-plus/Sandboxie/issues/1099)



## [0.9.3 / 5.51.3] - 2021-08-08

> Read the developer's notes about the new [WFP functionality](https://github.com/sandboxie-plus/Sandboxie/releases/tag/0.9.3).

### Added
- ability to use the "Run Unsandboxed" option with Sandboxie links [#614](https://github.com/sandboxie-plus/Sandboxie/issues/614)

### Fixed
- fixed "Run Outside Sandbox" issue on Classic builds [#614](https://github.com/sandboxie-plus/Sandboxie/issues/614#issuecomment-894710466)
- fixed open template does not load the edit tab [#1054](https://github.com/sandboxie-plus/Sandboxie/issues/1054#issuecomment-893001316)
- fixed issue with "explore sandboxed" [#972](https://github.com/sandboxie-plus/Sandboxie/issues/972)
- fixed start directory for sandboxed processes [#1071](https://github.com/sandboxie-plus/Sandboxie/issues/1071)
- fixed issue with language auto-detection [#1018](https://github.com/sandboxie-plus/Sandboxie/issues/1018)
- fixed issue with multiple files with the same name, by always showing the extension [#1041](https://github.com/sandboxie-plus/Sandboxie/issues/1041)
- fixed multiple program grouping issues with the SandMan UI [#1054](https://github.com/sandboxie-plus/Sandboxie/issues/1054)
- fixed "no disk" error [#966](https://github.com/sandboxie-plus/Sandboxie/issues/966)
- fixed issue with 32-bit build using qMake, the -O2 option resulted in a crash in the QSbieAPI.dll [#995](https://github.com/sandboxie-plus/Sandboxie/issues/995)
- fixed issue with UserSettings introduced in a recent build [#1054](https://github.com/sandboxie-plus/Sandboxie/issues/1054)



## [0.9.2 / 5.51.2] - 2021-08-07 (pre-release)

### Added
- added ability to reconfigure the driver, which allows enabling/disabling WFP and other features without a reload/reboot

### Changed
- reorganised and improved the settings window
- improved the tray icon, the sand colour is more yellow now

### Fixed
- fixed issue with process start handling introduced in 5.51.0 [#1063](https://github.com/sandboxie-plus/Sandboxie/issues/1063)
- fixed issue with quick recovery introduced in 5.51.0
- fixed incompatibility with CET Hardware-enforced Stack Protection on Intel 11th gen and AMD Ryzen 5XXX CPUs [#1067](https://github.com/sandboxie-plus/Sandboxie/issues/1067) [#1012](https://github.com/sandboxie-plus/Sandboxie/issues/1012)

### Removed
- commented out all Windows XP-specific support code from the driver



## [0.9.1 / 5.51.1] - 2021-07-31 (pre-release)

### Added
- added tray icon indicating broken connection to the driver if it happens
- added option to customize the tray icon
- added "DllSkipHook=some.dll" option to disable installation of hooks into selected DLLs
- added localization support for Plus installer (by yfdyh000 and mpheath) [#923](https://github.com/sandboxie-plus/Sandboxie/pull/923)

### Changed
- reworked NtClose handling for better performance and extendibility
- improved tray box menu and list

### Fixed
- fixed issue with fake admin and some NSIS installers [#1052](https://github.com/sandboxie-plus/Sandboxie/issues/1052)
- fixed more issued with FileDispositionInformation behaviour, which resulted in bogus file deletion handling
- fixed issue with checking WFP status
- fixed issue WFP failing to initialize at boot
- fixed issue with tray sandbox options not being available just after boot
- fixed issue access changed flag not being properly set in box options [#1065](https://github.com/sandboxie-plus/Sandboxie/issues/1065)



## [0.9.0 / 5.51.0] - 2021-07-29 (pre-release)

### Added
- added support for Windows Filtering Platform (WFP) to be used instead of the device-based network blocking scheme
  - to enable this support, add 'NetworkEnableWFP=y' to the global section and reboot or reload the driver
  - to use WFP for a specific sandbox, add 'AllowNetworkAccess=n'
  - you can allow certain processes by using 'AllowNetworkAccess=program.exe,y'
  - you can also enable this policy globally by adding 'AllowNetworkAccess=n' to the global section
  - in this case you can exempt entire sandboxes by adding 'AllowNetworkAccess=y' to specific boxes
  - you can block certain processes by using 'AllowNetworkAccess=program.exe,n'
  - Note: WFP is less absolute than the old approach, using WFP will filter only TCP/UDP communication
  - restricted boxed processes will still be able to resolve domain names using the system service
  - however, they will not be able to send or receive data packets directly
  - the advantages of WFP is that filter rules can be implemented by restricting communication only to specified addresses or selected ports using "NetworkAccess=..."
- added fully functional rule-based packet filter in user mode for the case when "NetworkEnableWFP=y" is not set
  - the mechanism replaces the old "BlockPort=..." functionality
  - Note: this filter applies only to outgoing connections/traffic, for incoming traffic either the WFP mode or a third-party firewall is needed
  - like the old user mode based mechanism, malicious applications can bypass it by unhooking certain functions
  - hence it's recommended to use the kernel mode WFP-based mechanism when reliable isolation is required
- added new trace option "NetFwTrace=*" to trace the actions of the firewall components
  - please note that the driver only trace logs the kernel debug output, use DbgView.exe to log
- API_QUERY_PROCESS_INFO can now be used to get the impersonation token of a sandboxed thread
  - Note: this capability is used by TaskExplorer to allow inspecting sandbox-internal tokens
  - Note: a process must have administrative privileges to be able to use this API
- added a UI option to switch "MsiInstallerExemptions=y" on and off
  - just in case a future Windows build breaks something in the systemless mode
- added sample code for ObRegisterCallbacks to the driver
- added new debug options "DisableFileFilter=y" and "DisableKeyFilter=y" that allow to disable file and registry filtering
  - Note: these options are for testing only and disable core parts of the sandbox isolation
- added a few command line options to SandMan.exe

### Changed
- greatly improved the performance of the trace log, but it's no longer possible to log to both SandMan and SbieCtrl at the same time
- reworked process creation code to use PsSetCreateProcessNotifyRoutineEx and improved process termination

### Fixed
- added missing hook for ConnectEx function



## [0.8.9 / 5.50.9] - 2021-07-28 HotFix 2

### Fixed
- fixed issue with registering session leader



## [0.8.9 / 5.50.9] - 2021-07-28 HotFix 1

### Fixed
- fixed issue with Windows 7



## [0.8.9 / 5.50.9] - 2021-07-27

### Changed
- updated a few icons
- updated GitHub build action to use Qt 5.15.2
- improved the "full" tray icon to be more distinguishable from the "empty" one
- changed code integrity verification policies [#1003](https://github.com/sandboxie-plus/Sandboxie/issues/1003)
  - code signature is no longer required to change config, to protect presets use the existing "EditAdminOnly=y"

### Fixed
- fixed issue with systemless MSI mode introduced in the last build
- fixed MSI installer not being able to create the action server mechanism on Windows 11
- fixed MSI installer not working in systemless mode on Windows 11
- fixed Inno Setup script not being able to remove shell integration keys during Sandboxie Plus uninstall (by mpheath) [#1037](https://github.com/sandboxie-plus/Sandboxie/pull/1037)



## [0.8.8 / 5.50.8] - 2021-07-13

### Changed
- MSIServer no longer requires being run as system; this completes the move to not use system tokens in a sandbox by default
  - the security-enhanced option "MsiInstallerExemptions=n" is now the default behaviour

### Fixed
- fixed issue with the "explore sandboxed" command [#972](https://github.com/sandboxie-plus/Sandboxie/issues/972)
- rolled back the switch from using NtQueryKey to NtQueryObject as it seems to break Windows 10 version 1803 and older [#984](https://github.com/sandboxie-plus/Sandboxie/issues/984)
  - this change was introduced to fix [#951](https://github.com/sandboxie-plus/Sandboxie/issues/951)
  - to use NtQueryObject the option "UseObjectNameForKeys=y" can be added to Sandboxie.ini



## [0.8.7b / 5.50.7] - 2021-07-11

### Fixed
- fixed issue with boxes that had auto-delete activated introduced in the previous build [#986](https://github.com/sandboxie-plus/Sandboxie/issues/986)



## [0.8.7 / 5.50.7] - 2021-07-10

### Added
- added option to always auto-pick the DefaultBox [#959](https://github.com/sandboxie-plus/Sandboxie/issues/959)
  - when this option is enabled, the normal behaviour with a box selection dialog can be brought up by holding down CTRL
- added option to hide a sandbox from the "run in box" dialog
  - useful to avoid listing insecure compatibility test boxes for example
- added box options to system tray [#439](https://github.com/sandboxie-plus/Sandboxie/issues/439) [#272](https://github.com/sandboxie-plus/Sandboxie/issues/272)

### Changed
- changed default "terminate all boxed processes" key from Ctrl+Pause to Ctrl+Alt+Pause [#974](https://github.com/sandboxie-plus/Sandboxie/issues/974)
- Start.exe no longer links in unused MFC code, which reduced its file size from over 2.5 MB to below 250 KB
- updated the main SandMan and tray icon [#963](https://github.com/sandboxie-plus/Sandboxie/issues/963)
- improved the box tree-style view

### Fixed
- added additional delay and retries to KmdUtil.exe to mitigate issues when unloading the driver [#968](https://github.com/sandboxie-plus/Sandboxie/issues/968)
- fixed issue with SbieCtrl not being properly started after setup [#969](https://github.com/sandboxie-plus/Sandboxie/issues/969)
- fixed issue with "explore sandboxed" shell option [#972](https://github.com/sandboxie-plus/Sandboxie/issues/972)
- fixed issue when running SandMan elevated [#932](https://github.com/sandboxie-plus/Sandboxie/issues/932)
- fixed new box selection dialog showing disabled boxes
- fixed issue updating box active status

### Removed
- removed Online Armor support as this product is deprecated since 2016



## [0.8.6 / 5.50.6] - 2021-07-07

### Added
- added LibreWolf template (by Dyras) [#929](https://github.com/sandboxie-plus/Sandboxie/pull/929)

### Fixed
- fixed performance bug introduced in 0.8.5



## [0.8.5 / 5.50.5] - 2021-07-06

### Added
- added global hotkey to terminate all sandboxed processes (default: Ctrl+Pause)
- the "Run Sandboxed" dialog can now be handled by the SandMan UI
- added "AllowBoxedJobs=y" allowing boxed processes to use nested jobs on Windows 8 and later
  - Note: this allows Chrome and other programs to use the job system for additional isolation
- added Librewolf.exe to the list of Firefox derivatives [#927](https://github.com/sandboxie-plus/Sandboxie/issues/927)
- added run regedit sandboxed menu command
- added new support settings tab to SandMan UI for updates and news
- added code integrity verification to Sbie service and UI
- added template for Vivaldi Notes (by isaak654) [#948](https://github.com/sandboxie-plus/Sandboxie/issues/948)

### Changed
- replaced the Process List used by the driver with a much faster Hash Map implementation
  - Note: this change provides an almost static system call speed of 1.2µs regardless of the running process count
  - the old list, with 100 programs running required 4.5µs; with 200: 12µs; and with 300: 18µs per syscall
  - Note: some of the slowdown was also affecting non-sandboxed applications due to how the driver handles certain callbacks
- replaced the per-process Thread List used by the driver with a much faster Hash Map implementation
- replaced configuration section list with a hash map to improve configuration performance, and increased line limit to 100000
  - not yet enabled in production build
- the presence of the default box is only checked on connect
- the portable directory dialog now shows the directory [#924](https://github.com/sandboxie-plus/Sandboxie/issues/924)
- when terminated, boxed processes now try terminating the job object first
- the driver now can terminate problematic processes by default without the help of the service
- the box delete routine now retries up to 10 times, see [#954](https://github.com/sandboxie-plus/Sandboxie/issues/954)
- replaced the Process List used by the service with a much faster Hash Map implementation
- replaced the per-process Thread List used by the service with a much faster Hash Map implementation

### Fixed
- fixed faulty initialization in SetServiceStatus (by flamencist) [#921](https://github.com/sandboxie-plus/Sandboxie/issues/921)
- fixed buttons position in Classic UI settings (by isaak654) [#914](https://github.com/sandboxie-plus/Sandboxie/issues/914)
- fixed missing password length check in the SandMan UI [#925](https://github.com/sandboxie-plus/Sandboxie/issues/925)
- fixed issues opening job objects by name
- fixed missing permission check when reopening job object handles (thanks Diversenok)
- fixed issue with some Chromium 90+ hooks affecting the display of PDFs in derived browsers [#930](https://github.com/sandboxie-plus/Sandboxie/issues/930) [#817](https://github.com/sandboxie-plus/Sandboxie/issues/817)
- fixed issues with reconnecting broken LPC ports used for communication with SbieSvc
- fixed minor setting issue [#957](https://github.com/sandboxie-plus/Sandboxie/issues/957)
- fixed minor UI issue with resource access COM settings [#958](https://github.com/sandboxie-plus/Sandboxie/issues/958)
- fixed an issue with NtQueryKey using NtQueryObject instead [#951](https://github.com/sandboxie-plus/Sandboxie/issues/951)
- fixed crash in key.c when failing to resolve key paths
- added workaround for topmost modality issue [#873](https://github.com/sandboxie-plus/Sandboxie/issues/873)
  - the notification window is not only topmost for 5 seconds
- fixed an issue deleting directories introduced in 5.49.5
- fixed an issue when creating box copies

### Removed
- removed switch for "BlockPassword=n" as it does not seem to be working [#938](https://github.com/sandboxie-plus/Sandboxie/issues/938)
  - it's recommended to use "OpenSamEndpoint=y" to allow password changes in Windows 10



## [0.8.2 / 5.50.2] - 2021-06-15

### Changed
- split anti-phishing rules per browser (by isaak654) [#910](https://github.com/sandboxie-plus/Sandboxie/pull/910)

### Fixed
- properly fixed an issue with Driver Verifier and user handles [#906](https://github.com/sandboxie-plus/Sandboxie/issues/906)
- fixed an issue with CreateWindow function introduced with 0.8.0
- fixed issue with outdated BoxDisplayOrder entries being retained [#900](https://github.com/sandboxie-plus/Sandboxie/issues/900)



## [0.8.1 / 5.50.1] - 2021-06-14

### Fixed
- fixed an issue with Driver Verifier and user handles
- fixed driver memory leak of FLT_FILE_NAME_INFORMATION objects
- fixed broken clipboard introduced in 5.50.0 [#899](https://github.com/sandboxie-plus/Sandboxie/issues/899)
- fixed DcomLaunch issue on Windows 7 32-bit introduced in 5.50.0 [#898](https://github.com/sandboxie-plus/Sandboxie/issues/898)



## [0.8.0 / 5.50.0] - 2021-06-13

### Added
- normally Sandboxie applies "Close...=!<program>,..." directives to non-excluded images if they are located in a sandbox
  - added 'AlwaysCloseForBoxed=n' to disable this behaviour as it may not be always desired, and it doesn't provide extra security
- added process image information to SandMan UI
- localized template categories in the Plus UI [#727](https://github.com/sandboxie-plus/Sandboxie/issues/727)
- added "DisableResourceMonitor=y" to disable resource access monitor for selected boxes [#886](https://github.com/sandboxie-plus/Sandboxie/issues/886)
- added option to show trace entries only for the selected sandbox [#886](https://github.com/sandboxie-plus/Sandboxie/issues/886)
- added "UseVolumeSerialNumbers=y" that allows drive letters to be suffixed with the volume SN in the \drive\ sandbox location
  - it helps to avoid files mixed together on multiple pendrives using the same letter
  - Note: this option is not compatible with the recovery function of the Classic UI, only SandMan UI is fully compatible
- added "ForceRestart=PicoTorrent.exe" to the PicoTorrent template in order to fix a compatibility issue [#720](https://github.com/sandboxie-plus/Sandboxie/issues/720)
- added localization support for RPC templates (by isaak654) [#736](https://github.com/sandboxie-plus/Sandboxie/issues/736)

### Changed
- portable clean-up message now has yes/no/cancel options [#874](https://github.com/sandboxie-plus/Sandboxie/issues/874)
- consolidated Proc_CreateProcessInternalW and Proc_CreateProcessInternalW_RS5 to remove duplicate code
- the ElevateCreateProcess fix, as sometimes applied by the Program Compatibility Assistant, will no longer be emulated by default [#858](https://github.com/sandboxie-plus/Sandboxie/issues/858)
  - use 'ApplyElevateCreateProcessFix=y' or 'ApplyElevateCreateProcessFix=program.exe,y' to enable it
- trace log gets disabled only when it has no entries and the logging is stopped

### Fixed
- fixed APC issue with the new global hook emulation mechanism and WoW64 processes [#780](https://github.com/sandboxie-plus/Sandboxie/issues/780) [#779](https://github.com/sandboxie-plus/Sandboxie/issues/779)
- fixed IPv6 issues with BlockPort options
- fixed an issue with CheatEngine when "OpenWinClass=*" was specified [#786](https://github.com/sandboxie-plus/Sandboxie/issues/786)
- fixed memory corruption in SbieDrv [#838](https://github.com/sandboxie-plus/Sandboxie/issues/838)
- fixed crash issue with process elevation on CreateProcess calls [#858](https://github.com/sandboxie-plus/Sandboxie/issues/858)
- fixed process elevation when running in the built-in administrator account [#3](https://github.com/sandboxie-plus/Sandboxie/issues/3)
- fixed template preview resetting unsaved entries in box options window [#621](https://github.com/sandboxie-plus/Sandboxie/issues/621)



## [0.7.5 / 5.49.8] - 2021-06-05

### Added
- clipboard access for a sandbox can now be disabled with "OpenClipboard=n" [#794](https://github.com/sandboxie-plus/Sandboxie/issues/794)

### Changed
- now the OpenBluetooth template is enabled by default for compatibility with Unity games [#799](https://github.com/sandboxie-plus/Sandboxie/issues/799)
- "PreferExternalManifest=program.exe,y" can now be set on a per-process basis

### Fixed
- fixed compiler issues with the most recent VS2019 update
- fixed issue with Vivaldi browser [#821](https://github.com/sandboxie-plus/Sandboxie/issues/821)
- fixed some issues with box options in the Plus UI [#879](https://github.com/sandboxie-plus/Sandboxie/issues/879)
- fixed some issues with hardware acceleration in Chromium based browsers [#795](https://github.com/sandboxie-plus/Sandboxie/issues/795)
- the "Stop All" command now issues "KmdUtil scandll" first to solve issues when the SbieDll.dll is in use
- workaround for Electron apps, by forcing an additional command line argument on the GPU renderer process [#547](https://github.com/sandboxie-plus/Sandboxie/issues/547) [#310](https://github.com/sandboxie-plus/Sandboxie/issues/310) [#215](https://github.com/sandboxie-plus/Sandboxie/issues/215)
- fixed issue with Software Compatibility tab that doesn't always show template names correctly [#774](https://github.com/sandboxie-plus/Sandboxie/issues/774)



## [0.7.4 / 5.49.7] - 2021-04-11

### Added
- added option to disable file migration prompt in the Plus UI with PromptForFileMigration=n [#643](https://github.com/sandboxie-plus/Sandboxie/issues/643)
- added UI options for various security isolation features
- added missing functionality to set template values in the Plus UI
- added templates for Popcorn-Time, Clementine Music Player, Strawberry Music Player, 32-bit MPC-HC (by Dyras) [#726](https://github.com/sandboxie-plus/Sandboxie/pull/726) [#737](https://github.com/sandboxie-plus/Sandboxie/pull/737)

### Changed
- align default settings of AutoRecover and Favourites to the Plus version (thanks isaak654) [#747](https://github.com/sandboxie-plus/Sandboxie/pull/747)
- list of email clients and browsers is now centralized in Dll_GetImageType
- localstore.rdf reference in Templates.ini was replaced with xulstore.json (by isaak654) [#751](https://github.com/sandboxie-plus/Sandboxie/pull/751)

### Fixed
- fixed minor issue with logging internet blocks
- fixed issue with file recovery when located on a network share [#711](https://github.com/sandboxie-plus/Sandboxie/issues/711)
- fixed UI issue with CallTrace [#769](https://github.com/sandboxie-plus/Sandboxie/issues/769)
- fixed sandbox shortcuts receiving double extension upon creation [#770](https://github.com/sandboxie-plus/Sandboxie/issues/770)
- fixed misplaced labels in the Classic UI (thanks isaak654) [#759](https://github.com/sandboxie-plus/Sandboxie/pull/759)
- fixed separator line in SbieCtrl (thanks isaak654) [#761](https://github.com/sandboxie-plus/Sandboxie/pull/761)
- fixed broken paths in The Bat! template (by isaak654) [#756](https://github.com/sandboxie-plus/Sandboxie/pull/756)
- fixed issue about media players that attempt to write unneeded media files inside the box (by Dyras) [#743](https://github.com/sandboxie-plus/Sandboxie/pull/743) [#536](https://github.com/sandboxie-plus/Sandboxie/issues/536)



## [0.7.3 / 5.49.5] - 2021-03-27

### Added
- added "UseSbieWndStation=y" to emulate CreateDesktop for selected processes, not only Firefox and Chrome [#635](https://github.com/sandboxie-plus/Sandboxie/issues/635)
- added option to drop the console host process integrity, now you can use "DropConHostIntegrity=y" [#678](https://github.com/sandboxie-plus/Sandboxie/issues/678)
- added option to easily add local templates
- added new torrent clients and media players templates (by Dyras) [#719](https://github.com/sandboxie-plus/Sandboxie/pull/719)

### Changed
- reworked window hooking mechanism to improve performance [#697](https://github.com/sandboxie-plus/Sandboxie/issues/697) [#519](https://github.com/sandboxie-plus/Sandboxie/issues/519) [#662](https://github.com/sandboxie-plus/Sandboxie/issues/662) [#69](https://github.com/sandboxie-plus/Sandboxie/issues/69) [#109](https://github.com/sandboxie-plus/Sandboxie/issues/109) [#193](https://github.com/sandboxie-plus/Sandboxie/issues/193)
  - resolves issues with file save dialogs taking 30+ seconds to open
  - this fix greatly improves the Win32 GUI performance of sandboxed processes
- reworked RPC resolver to be ini-configurable
  - the following options are now deprecated:
    - "UseRpcMgmtSetComTimeout=some.dll,n", so use "RpcPortBinding=some.dll,*,TimeOut=y"
    - "OpenUPnP=y", "OpenBluetooth=y", "OpenSmartCard=n", so use the new RPC templates instead
  - see Templates.ini for usage examples

### Fixed
- fixed process-specific hooks being applied to all processes in a given sandbox
- fixed issue with messages and templates sometimes not being properly displayed in the SandMan UI
- fixed issue with compatibility settings not being applied properly
- fixed auto delete issue that got introduced with 0.7.1 [#637](https://github.com/sandboxie-plus/Sandboxie/issues/637)
- fixed issue with NtSetInformationFile, FileDispositionInformation resulting in Opera installer failing
- fixed issue with MacType introduced in the 0.7.2 build [#676](https://github.com/sandboxie-plus/Sandboxie/issues/676)
- fixed global sandboxed windows hooks not working when window rename option is disabled
- fixed issue with saving local templates
- fixed issue when using runas to start a process that was created outside of the Sandboxie supervision [#688](https://github.com/sandboxie-plus/Sandboxie/issues/688)
  - since the runas facility is not accessible by default, this did not constitute a security issue
  - to enable runas functionality, add "OpenIpcPath=\RPC Control\SECLOGON" to your Sandboxie.ini
  - please take note that doing so may open other yet unknown issues
- fixed a driver compatibility issue with Windows 10 32-bit Insider Preview Build 21337
- fixed issues with driver signature for Windows 7



## [0.7.2 / 5.49.0] - 2021-03-04

### Added
- added option to alter reported Windows version "OverrideOsBuild=7601" for Windows 7 SP1 [#605](https://github.com/sandboxie-plus/Sandboxie/issues/605)
- the trace log can now be structured like a tree with processes as root items and threads as branches

### Changed
- SandboxieCrypto now always migrates the CatRoot2 files in order to prevent locking of real files
- greatly improved trace log performance
- MSI Server can now run with the "FakeAdminRights=y" and "DropAdminRights=y" options [#600](https://github.com/sandboxie-plus/Sandboxie/issues/600)
  - special service allowance for the MSI Server can be disabled with "MsiInstallerExemptions=n"
- changed SCM access check behaviour; non elevated users can now start services with a user token
  - elevation is now only required to start services with a system token
- reworked the trace log mechanism to be more verbose
- reworked RPC mechanism to be more flexible

### Fixed
- fixed issues with some installers introduced in 5.48.0 [#595](https://github.com/sandboxie-plus/Sandboxie/issues/595)
- fixed "add user to sandbox" in the Plus UI [#597](https://github.com/sandboxie-plus/Sandboxie/issues/597)
- FIXED SECURITY ISSUE ID-15: the HostInjectDll mechanism allowed for local privilege escalation (thanks hg421)
- Classic UI no longer allows to create a sandbox with an invalid or reserved device name [#649](https://github.com/sandboxie-plus/Sandboxie/issues/649)



## [0.7.1 / 5.48.5] - 2021-02-21

### Added
- enhanced RpcMgmtSetComTimeout handling with "UseRpcMgmtSetComTimeout=some.dll,n"
  - this option allows to specify if RpcMgmtSetComTimeout should be used or not for each individual dll
  - this setting takes precedence over hard-coded and per-process presets
  - "UseRpcMgmtSetComTimeout=some.dll" and "UseRpcMgmtSetComTimeout=some.dll,y" are equivalent
- added "FakeAdminRights=y" option that makes processes think they have admin permissions in a given box
  - this option is recommended to be used in combination with "DropAdminRights=y" to improve security
  - with "FakeAdminRights=y" and "DropAdminRights=y" installers should still work
- added RPC support for SSDP API (the Simple Service Discovery Protocol), you can enable it with "OpenUPnP=y"

### Changed
- SbieCrypto no longer triggers message 1313
- changed enum process API; now more than 511 processes per box can be enumerated (no limit)
- reorganised box settings
- made COM tracing more verbose
- "RpcMgmtSetComTimeout=y" is now again the default behaviour, it seems to cause less issues overall

### Fixed
- fixed issues with webcam access when the DevCMApi filtering is in place
- fixed issue with free download manager for 'AppXDeploymentClient.dll', so RpcMgmtSetComTimeout=y will be used by default for this one [#573](https://github.com/sandboxie-plus/Sandboxie/issues/573)
- fixed not all WinRM files were blocked by the driver, with "BlockWinRM=n" this file block can be disabled
- fixed Sandboxie Classic crash when saving any option in Sandbox Settings -> Appearance (by typpos) [#586](https://github.com/sandboxie-plus/Sandboxie/issues/586)



## [0.7.0 / 5.48.0] - 2021-02-14

### Added
- sandboxed indicator for tray icons, the tooltip now contains [#] if enabled
- the trace log buffer can now be adjusted with "TraceBufferPages=2560"
  - the value denotes the count of 4K large pages to be used; here for a total of 10 MB
- new functionality for the list finder

### Changed
- improved RPC debugging
- improved IPC handling around RpcMgmtSetComTimeout; "RpcMgmtSetComTimeout=n" is now the default behaviour
  - required exceptions have been hard-coded for specific calling DLLs
- the LogAPI library is now using Sandboxie's tracing facility to log events instead of its own pipe server

### Fixed
- FIXED SECURITY ISSUE ID-11: elevated sandboxed processes could access volumes/disks for reading (thanks hg421)
  - this protection option can be disabled by using "AllowRawDiskRead=y"
- fixed crash issue around SetCurrentProcessExplicitAppUserModelID observed with GoogleUpdate.exe
- fixed issue with Resource Monitor sort by timestamp
- fixed invalid Opera bookmarks path (by isaak654) [#542](https://github.com/sandboxie-plus/Sandboxie/pull/542)
- FIXED SECURITY ISSUE ID-12: a race condition in the driver allowed to obtain an elevated rights handle to a process (thanks typpos) [#549](https://github.com/sandboxie-plus/Sandboxie/pull/549)
- FIXED SECURITY ISSUE ID-13: "\RPC Control\samss lpc" is now filtered by the driver (thanks hg421) [#553](https://github.com/sandboxie-plus/Sandboxie/issues/553)
  - this allowed elevated processes to change passwords, delete users and alike; to disable filtering use "OpenSamEndpoint=y"
- FIXED SECURITY ISSUE ID-14: "\Device\DeviceApi\CMApi" is now filtered by the driver (thanks hg421) [#552](https://github.com/sandboxie-plus/Sandboxie/issues/552)
  - this allowed elevated processes to change hardware configuration; to disable filtering use "OpenDevCMApi=y"



## [0.6.7 / 5.47.1] - 2021-02-01

### Added
- added UI language auto-detection

### Fixed
- fixed Brave.exe now being properly recognized as Chrome-, not Firefox-based
- fixed issue introduced in 0.6.5 with recent Edge builds
  - the 0.6.5 behaviour can be set on a per-process basis using "RpcMgmtSetComTimeout=POPPeeper.exe,n"
- fixed grouping issues [#445](https://github.com/sandboxie-plus/Sandboxie/issues/445)
- fixed main window restore state from tray [#288](https://github.com/sandboxie-plus/Sandboxie/issues/288)



## [0.6.5 / 5.47.0] - 2021-01-31

### Added
- added detection for Waterfox.exe, Palemoon.exe and Basilisk.exe Firefox forks as well as Brave.exe [#468](https://github.com/sandboxie-plus/Sandboxie/issues/468)
- added Bluetooth API support, IPC port can be opened with "OpenBluetooth=y" [#319](https://github.com/sandboxie-plus/Sandboxie/issues/319)
  - this should resolve issues with many Unity games hanging on startup for a long time
- added enhanced RPC/IPC interface tracing
- when DefaultBox is not found by the SandMan UI, it will be recreated
- "Disable Forced Programs" time is now saved and reloaded

### Changed
- reduced SandMan CPU usage
- Sandboxie.ini and Templates.ini can now be UTF-8 encoded [#461](https://github.com/sandboxie-plus/Sandboxie/issues/461) [#197](https://github.com/sandboxie-plus/Sandboxie/issues/197)
  - this feature is experimental, files without a UTF-8 Signature should be recognized also
  - "ByteOrderMark=yes" is obsolete, Sandboxie.ini is now always saved with a BOM/Signature
- legacy language files can now be UTF-8 encoded
- reworked file migration behaviour, removed hardcoded lists in favour of templates [#441](https://github.com/sandboxie-plus/Sandboxie/issues/441)
  - you can now use "CopyAlways=", "DontCopy=" and "CopyEmpty=" that support the same syntax as "OpenFilePath="
  - "CopyBlockDenyWrite=program.exe,y" makes a write open call to a file that won't be copied fail instead of turning it read-only
- removed hardcoded SkipHook list in favour of templates

### Fixed
- fixed old memory pool leak in the Sbie driver [#444](https://github.com/sandboxie-plus/Sandboxie/issues/444)
- fixed issue with item selection in the access restrictions UI
- fixed updater crash in SbieCtrl.exe [#450](https://github.com/sandboxie-plus/Sandboxie/issues/450)
- fixed issues with RPC calls introduced in Sbie 5.33.1
- fixed recently broken 'terminate all' command
- fixed a couple minor UI issues with SandMan UI
- fixed IPC issue with Windows 7 and 8 resulting in process termination
- fixed "recover to" functionality



## [0.6.0 / 5.46.5] - 2021-01-25

### Added
- added confirmation prompts to terminate all commands
- added window title to boxed process info [#360](https://github.com/sandboxie-plus/Sandboxie/issues/360)
- added WinSpy based sandboxed window finder [#351](https://github.com/sandboxie-plus/Sandboxie/issues/351)
- added option to view disabled boxes and double-click on box to enable it

### Changed
- "Reset Columns" now resizes them to fit the content, and it can now be localized [#426](https://github.com/sandboxie-plus/Sandboxie/issues/426)
- modal windows are now centered to the parent [#417](https://github.com/sandboxie-plus/Sandboxie/issues/417)
- improved new box window [#417](https://github.com/sandboxie-plus/Sandboxie/issues/417)

### Fixed
- fixed issues with window modality [#409](https://github.com/sandboxie-plus/Sandboxie/issues/409)
- fixed issues when main window was set to be always on top [#417](https://github.com/sandboxie-plus/Sandboxie/issues/417)
- fixed a driver issue with Windows 10 Insider build 21286
- fixed issues with snapshot dialog [#416](https://github.com/sandboxie-plus/Sandboxie/issues/416)
- fixed an issue when writing to a path that already exists in the snapshot but not outside [#415](https://github.com/sandboxie-plus/Sandboxie/issues/415)



## [0.5.5 / 5.46.4] - 2021-01-17

### Added
- added "SandboxService=..." to force selected services to be started in the sandbox
- added template clean-up functionality to Plus UI
- added internet prompt to now also allow internet access permanently
- added browse button for box root folder in the SandMan UI [#382](https://github.com/sandboxie-plus/Sandboxie/issues/382)
- added Windows Explorer info message
- added option to keep the SandMan UI always on top [#352](https://github.com/sandboxie-plus/Sandboxie/issues/352)
- allow drag and drop file onto SandMan.exe to run it sandboxed [#355](https://github.com/sandboxie-plus/Sandboxie/issues/355)
- added start SandMan UI when a sandboxed application starts [#367](https://github.com/sandboxie-plus/Sandboxie/issues/367)
- recovery window can now list all files
- added file counter to recovery window
- when "NoAddProcessToJob=y" is specified, Chrome and related browsers now can fully use the job system
  - Note: "NoAddProcessToJob=y" reduces the box isolation, but the affected functions are mostly covered by UIPI anyway
- added optimized default column widths to Sbie view
- added template support for Yandex and Ungoogled Chromium browsers (by isaak654)

### Changed
- updated templates with multiple browsers fixes (thanks isaak654)
- when trying to take a snapshot of an empty sandbox a proper error message is displayed [#381](https://github.com/sandboxie-plus/Sandboxie/issues/381)
- new layout for the recovery window
- Sbie view sorting is now case insensitive

### Fixed
- fixed issue child window closing terminating application when main was hidden [#349](https://github.com/sandboxie-plus/Sandboxie/issues/349)
- fixed issues with non modal windows [#349](https://github.com/sandboxie-plus/Sandboxie/issues/349)
- fixed issues connecting to driver in portable mode
- fixed minor issues with snapshot window
- fixed missing error message when attempting to create an already existing sandbox [#359](https://github.com/sandboxie-plus/Sandboxie/issues/359)
- fixed issue allowing to save setting when a sandbox was already deleted [#359](https://github.com/sandboxie-plus/Sandboxie/issues/359)
- fixed issues with disabled items in dark mode [#359](https://github.com/sandboxie-plus/Sandboxie/issues/359)
- fixed some dialogs not closing when pressing Esc [#359](https://github.com/sandboxie-plus/Sandboxie/issues/359)
- fixed tab stops on many windows



## [0.5.4d / 5.46.3] - 2021-01-11

### Changed
- improved access tracing, removed redundant entries
- OpenIpcPath=\BaseNamedObjects\[CoreUI]-* is now hardcoded in the driver no need for the template entry
- WindowsFontCache is now open by default
- refactored some IPC code in the driver

### Fixed
- FIXED SECURITY ISSUE ID-10: the registry isolation could be bypassed, present since Windows 10 Creators Update
- fixed creation time not always being properly updated in the SandMan UI



## [0.5.4c / 5.46.2] - 2021-01-10

### Added
- added "CallTrace=*" to log all system calls to the access log

### Changed
- improved IPC logging code
- improved MSG_2101 logging

### Fixed
- fixed more issues with IPC tracing
- fixed SBIE2101 issue with Chrome and derivatives



## [0.5.4b / 5.46.1] - 2021-01-08

### Added
- added "RunServiceAsSystem=..." allows specific named services to be run as system

### Changed
- refactored some code around SCM access

### Fixed
- fixed a crash issue in SbieSvc.exe introduced with the last build
- fixed issue with SandMan UI update check
- FIXED SECURITY ISSUE ID-9: a Sandboxed process could start sandboxed as system even with DropAdminRights in place

### Removed
- removed "ProtectRpcSs=y" due to incompatibility with new isolation defaults



## [0.5.4 / 5.46.0] - 2021-01-06

### Added
- FIXED SECURITY ISSUE ID-4: Sandboxie now strips particularly problematic privileges from sandboxed system tokens
  - with those a process could attempt to bypass the sandbox isolation (thanks Diversenok)
  - old legacy behaviour can be enabled with "StripSystemPrivileges=n" (absolutely NOT Recommended)
- added new isolation options "ClosePrintSpooler=y" and "OpenSmartCard=n"
  - those resources are open by default, but for a hardened box it is desired to close them
- FIXED SECURITY ISSUE ID-5: added print spooler filter to prevent printers from being set up outside the sandbox
  - the filter can be disabled with "OpenPrintSpooler=y"
- added overwrite prompt when recovering an already existing file
- added "StartProgram=", "StartService=" and "AutoExec=" options to the SandMan UI
- added more compatibility templates (thanks isaak654) [#294](https://github.com/sandboxie-plus/Sandboxie/pull/294)

### Changed
- changed Emulated SCM behaviour, boxed services are no longer by default started as boxed system
  - use "RunServicesAsSystem=y" to enable the old legacy behaviour
  - Note: sandboxed services with a system token are still sandboxed and restricted
  - however not granting them a system token in the first place removes possible exploit vectors
  - Note: this option is not compatible with "ProtectRpcSs=y" and takes precedence!
- reworked dynamic IPC port handling
- improved Resource Monitor status strings

### Fixed
- FIXED SECURITY ISSUE ID-6: processes could spawn processes outside the sandbox (thanks Diversenok)
- FIXED SECURITY ISSUE ID-7: bug in the dynamic IPC port handling allowed to bypass IPC isolation
- fixed issue with IPC tracing
- FIXED SECURITY ISSUE ID-8: CVE-2019-13502 "\RPC Control\LSARPC_ENDPOINT" is now filtered by the driver (thanks Diversenok)
  - this allowed some system options to be changed, to disable filtering use "OpenLsaEndpoint=y"
- fixed hooking issues SBIE2303 with Chrome, Edge and possibly others [#68](https://github.com/sandboxie-plus/Sandboxie/issues/68) [#166](https://github.com/sandboxie-plus/Sandboxie/issues/166)
- fixed failed check for running processes when performing snapshot operations
- fixed some box options checkboxes were not properly initialized
- fixed unavailable options are not properly disabled when SandMan is not connected to the driver
- fixed MSI installer not being able to create "C:\Config.msi" folder on Windows 20H2 [#219](https://github.com/sandboxie-plus/Sandboxie/issues/219)
- added missing localization to generic list commands
- fixed issue with "iconcache_*" when running sandboxed Windows Explorer
- fixed more issues with groups



## [0.5.3b / 5.45.2] - 2021-01-02

### Added
- added settings for the portable boxed root folder option
- added process name to resource log
- added command line column to the process view in the SandMan UI

### Fixed
- fixed a few issues with group handling [#262](https://github.com/sandboxie-plus/Sandboxie/issues/262)
- fixed issue with GetRawInputDeviceInfo when running a 32-bit program on a 64-bit system
- fixed issue when pressing apply in the "Resource Access" tab; the last edited value was not always applied
- fixed issue merging entries in Resource Access Monitor



## [0.5.3a / 5.45.2] - 2020-12-29

### Added
- added prompt to choose if links in the SandMan UI should be opened in a sandboxed or unsandboxed browser [#273](https://github.com/sandboxie-plus/Sandboxie/issues/273)
- added more recovery options
- added "ClosedClsid=" to block COM objects from being used when they cause compatibility issues
- added "ClsidTrace=*" option to trace COM usage
- added "ClosedRT=" option to block access to problematic Windows RT interfaces
- added option to make a link for any selected process to SandMan UI
- added option to reset all hidden messages
- added more process presets "force program" and "allow internet access"
- added "SpecialImage=chrome,some_electron_app.exe" option to Sandboxie.ini, valid image types "chrome", "firefox"
  - with this option you can enable special hardcoded workarounds to new obscure forks of those browsers
- added German translation (thanks bastik-1001) to the SandMan UI
- added Russian translation (thanks lufog) to the SandMan UI
- added Portuguese translation (thanks JNylson ) to the SandMan UI

### Changed
- changed docs and update URLs to the new sandboxie-plus.com domain
- greatly improved the setup script (thanks mpheath)
- "OpenClsid=" and "ClosedClsid=" now support specifying a program or group name
- by default, when started in portable mode, the sandbox folder will be located in the parent directory of the Sandboxie instance

### Fixed
- grouping menu not fully working in the new SandMan UI [#277](https://github.com/sandboxie-plus/Sandboxie/issues/277)
- fixed not being able to set quick recovery in SandMan UI
- fixed resource leak when loading process icons in SandMan UI
- fixed issue with OpenToken debug options
- fixed Chrome crashing on websites that cause the invocation of "FindAppUriHandlersAsync" [#198](https://github.com/sandboxie-plus/Sandboxie/issues/198)
- fixed issue connecting to the driver when starting in portable mode
- fixed missing template setup when creating new boxes

### removed
- removed obsolete "OpenDefaultClsid=n" use "ClosedClsid=" with the appropriate values instead
- removed suspend/resume menu entry, pooling that state wastes substantial CPU cycles; use TaskExplorer for that functionality



## [0.5.2a / 5.45.1] - 2020-12-23

### Fixed
- fixed translation support in the SandMan UI
- fixed sandboxed Windows Explorer issue [#289](https://github.com/sandboxie-plus/Sandboxie/issues/289)
- fixed simplified Chinese localization



## [0.5.2 / 5.45.1] - 2020-12-23

### Added
- added advanced new box creation dialog to SandMan UI
- added show/hide tray context menu entry
- added refresh button to file recovery dialog
- added mechanism to load icons from {install-dir}/Icons/{icon}.png for UI customization
- added tray indicator to show disabled forced program status in the SandMan UI
- added program name suggestions to box options in SandMan UI
- added saving of column sizes in the options window

### Changed
- reorganised the advanced box options
- changed icons (thanks Valinwolf for picking the new ones) [#235](https://github.com/sandboxie-plus/Sandboxie/issues/235)
- updated Templates.ini (thanks isaak654) [#256](https://github.com/sandboxie-plus/Sandboxie/pull/256) [#258](https://github.com/sandboxie-plus/Sandboxie/pull/258)
- increased max value for disable forced process time in SandMan UI

### Fixed
- fixed BSoD introduced in 5.45.0 when using Windows 10 "core isolation" [#221](https://github.com/sandboxie-plus/Sandboxie/issues/221)
- fixed minor issue with lingering/leader processes
- fixed menu issue in SandMan UI
- fixed issue with stop behaviour page in SandMan UI
- fixed issue with Plus installer not displaying KmdUtil window
- fixed SandMan UI saving UI settings on Windows shutdown
- fixed issue with Plus installer autorun [#247](https://github.com/sandboxie-plus/Sandboxie/issues/247)
- fixed issue with legacy installer not removing all files
- fixed a driver compatibility issue with Windows 20H1 and later [#228](https://github.com/sandboxie-plus/Sandboxie/issues/228)
  - this solves "stop pending", LINE messenger hanging and other issues...
- fixed quick recovery issue in SbieCtrl.exe introduced in 5.45.0 [#224](https://github.com/sandboxie-plus/Sandboxie/issues/224)
- fixed issue advanced hide process settings not saving
- fixed some typos in the UI (thanks isaak654) [#252](https://github.com/sandboxie-plus/Sandboxie/pull/252) [#253](https://github.com/sandboxie-plus/Sandboxie/pull/253) [#254](https://github.com/sandboxie-plus/Sandboxie/pull/254)
- fixed issue with GetRawInputDeviceInfo failing when boxed processes are put in a job object [#176](https://github.com/sandboxie-plus/Sandboxie/issues/176) [#233](https://github.com/sandboxie-plus/Sandboxie/issues/233)
  - this fix resolves issues with CP2077 and other games not getting keyboard input (thanks Rostok)
- fixed failing ClipCursor won't longer span the message log
- fixed issue with adding recovery folders in SandMan UI
- fixed issue with Office 2019 template when using a non-default Sbie install location
- fixed issue setting last access attribute on sandboxed folders [#218](https://github.com/sandboxie-plus/Sandboxie/issues/218)
- fixed issue with process start signal



## [0.5.1 / 5.45.0] - 2020-12-12

### Added
- added simple view mode

### Changed
- updated SandMan UI to use Qt 5.15.1

### Fixed
- fixed crash issue with progress dialog
- fixed progress dialog cancel button not working for update checker
- fixed issue around NtQueryDirectoryFile when deleting sandbox content
- fixed dark theme in the notification window
- fixed issue with disable force programs tray menu



## [0.5.0 / 5.45.0] - 2020-12-06

### Added
- added new notification window
- added user interactive control mechanism when using the new SandMan UI
  - when a file exceeds the copy limit instead of failing, the user is prompted if the file should be copied or not
  - when internet access is blocked it now can be exempted in real time by the user
- added missing file recovery and auto/quick recovery functionality [#188](https://github.com/sandboxie-plus/Sandboxie/issues/188) [#178](https://github.com/sandboxie-plus/Sandboxie/issues/178)
- added silent MSG_1399 boxed process start notification to keep track of short lived boxed processes
- added ability to prevent system wide process starts, Sandboxie can now instead of just alerting also block processed on the alert list
  - set "StartRunAlertDenied=y" to enable process blocking
- the process start alert/block mechanism can now also handle folders use "AlertFolder=..."
- added ability to merge snapshots [#151](https://github.com/sandboxie-plus/Sandboxie/issues/151)
- added icons to the sandbox context menu in the new UI
- added more advanced options to the sandbox options window
- added file migration progress indicator
- added more run commands and custom run commands per sandbox
  - users can now specify programs to be available from the Run Menu of the sandbox
  - also processes can be pinned to that list from the presets menu
- added more Windows 10 specific template presets
- added ability to create desktop shortcuts to sandboxed items
- added icons to box option tabs
- added box grouping
- added new debug option "DebugTrace=y" to log debug output to the trace log
- added check for updates to the new SandMan UI
- added check for updates to the legacy SbieCtrl UI

### Changed
- file migration limit can now be disabled by specifying "CopyLimitKb=-1" [#526](https://github.com/sandboxie-plus/Sandboxie/issues/526)
- improved and refactored message logging mechanism, reducing memory usage by factor of 2
- terminated boxed processes are now kept listed for a couple of seconds
- reworked sandbox deletion mechanism of the new UI
- restructured sandbox options window
- SbieDLL.dll can now be compiled with an up to date ntdll.lib (Thanks to TechLord from Team-IRA for help)
- improved automated driver self repair

### Fixed
- fixed issues migrating files > 4GB
- fixed an issue that would allow a malicious application to bypass the internet blockade
- fixed issue when logging messages from a non-sandboxed process, added process_id parameter to API_LOG_MESSAGE_ARGS
- fixed issues with localization
- fixed issue using file recovery in legacy UI SbieCtrl.exe when "SeparateUserFolders=n" is set
- when a program is blocked from starting due to restrictions no redundant messages are issued any more
- fixed UI not properly displaying async errors
- fixed issues when a snapshot operation failed
- fixed some special cases of IpcPath and WinClass in the new UI
- fixed driver issues with WHQL passing compatibility testing
- fixed issues with Classic installer



## [0.4.5 / 5.44.1] - 2020-11-16

### Added
- added "Terminate all processes" and "disable forced programs" commands to tray menu in SandMan UI
- program start restrictions settings now can be switched between a whitelist and a blacklist
  - programs can be terminated and blacklisted from the context menu
- added additional process context menu options, lingering and leader process can be now set from menu
- added option to view template presets for any given box
- added text filter to templates view
- added new compatibility templates:
  - Windows 10 core UI component: OpenIpcPath=\BaseNamedObjects\[CoreUI]-* solving issues with Chinese Input and Emojis [#120](https://github.com/sandboxie-plus/Sandboxie/issues/120) [#88](https://github.com/sandboxie-plus/Sandboxie/issues/88)
  - Firefox Quantum, access to Windows's FontCachePort for compatibility with Windows 7
- added experimental debug option "OriginalToken=y" which allows sandboxed processes to retain their original unrestricted token
  - this option is comparable with "OpenToken=y" and is intended only for testing and debugging, as it breaks most security measures (!)
- added debug option "NoSandboxieDesktop=y" it disables the desktop proxy mechanism
  - Note: without an unrestricted token with this option applications won't be able to start
- added debug option "NoSysCallHooks=y" it disables the sys call processing by the driver
  - Note: without an unrestricted token with this option applications won't be able to start
- added ability to record verbose access traces to the Resource Monitor
  - use INI options "FileTrace=*", "PipeTrace=*", "KeyTrace=*", "IpcTrace=*", "GuiTrace=*" to record all events
  - replace "*" to log only: "A" - allowed, "D" - denied, or "I" - ignore events
- added ability to record debug output strings to the Resource Monitor
  - use INI option DebugTrace=y to enable

### Changed
- AppUserModelID string no longer contains Sandboxie version string
- now by default Sbie's application manifest hack is disabled, as it causes problems with version checking on Windows 10
  - to enable old behaviour add "PreferExternalManifest=y" to the global or the box specific INI section
- the resource log mechanism can now handle multiple strings to reduce on string copy operations

### Fixed
- fixed issue with disabling some restriction settings failed
- fixed disabling of internet block from the presets menu sometimes failed
- the software compatibility list in the SandMan UI now shows the proper template names
- fixed use of freed memory in the driver
- replaced swprintf with snwprintf to prevent potential buffer overflow in SbieDll.dll
- fixed bad list performance with resource log and API log in SandMan UI



## [0.4.4 / 5.44.0] - 2020-11-03

### Added
- added SbieLdr (experimental)

### Changed
- moved code injection mechanism from SbieSvc to SbieDll
- moved function hooking mechanism from SbieDrv to SbieDll
- introduced a new driverless method to resolve wow64 ntdll base address

### Removed
- removed support for Windows Vista x64



## [0.4.3 / 5.43.7] - 2020-11-03

### Added
- added disable forced programs menu command to the SandMan UI

### Fixed
- fixed file rename bug introduced with an earlier Driver Verifier fix [#174](https://github.com/sandboxie-plus/Sandboxie/issues/174) [#153](https://github.com/sandboxie-plus/Sandboxie/issues/153)
- fixed issue saving access lists
- fixed issue with program groups parsing in the SandMan UI
- fixed issue with internet access restriction options [#177](https://github.com/sandboxie-plus/Sandboxie/issues/177) [#185](https://github.com/sandboxie-plus/Sandboxie/issues/185)
- fixed issue deleting sandbox when located on a drive directly [#139](https://github.com/sandboxie-plus/Sandboxie/issues/139)



## [0.4.2 / 5.43.6] - 2020-10-10

### Added
- added "explore box" content menu option

### Fixed
- fixed thread handle leak in SbieSvc and other components [#144](https://github.com/sandboxie-plus/Sandboxie/issues/144)
- msedge.exe is now categorized as a Chromium derivate
- fixed Chrome 86+ compatibility bug with Chrome's own sandbox [#149](https://github.com/sandboxie-plus/Sandboxie/issues/149)



## [0.4.1 / 5.43.5] - 2020-09-12

### Added
- added core version compatibility check to SandMan UI
- added shell integration options to SbiePlus

### Changed
- SbieCtrl no longer auto-shows the tutorial on first start
- when hooking to the trampoline, the migrated section of the original function is no longer noped out due to causing issues with Unity games

### Fixed
- fixed colour issue with vertical tabs in dark mode
- fixed wrong path separators when adding new forced folders
- fixed directory listing bug introduced in 5.43
- fixed issues with settings window when not being connected to driver
- fixed issue when starting SandMan UI as admin
- fixed auto-content-delete not working with SandMan UI



## [0.4.0 / 5.43] - 2020-09-05

### Added
- added a proper custom installer to the Plus release
- added sandbox snapshot functionality to Sbie core
  - filesystem is saved incrementally, the snapshots built upon each other
  - each snapshot gets a full copy of the box registry for now
  - each snapshot can have multiple children snapshots
- added access status to Resource Monitor
- added setting to change border width [#113](https://github.com/sandboxie-plus/Sandboxie/issues/113)
- added snapshot manager UI to SandMan
- added template to enable authentication with an Yubikey or comparable 2FA device
- added UI for program alert
- added software compatibility options to the UI

### Changed
- SandMan UI now handles deletion of sandbox content on its own
- no longer adding redundant resource accesses as new events

### Fixed
- fixed issues when hooking functions from delay loaded libraries
- fixed issues when hooking an already hooked function
- fixed issues with the new box settings editor

### Removed
- removed deprecated workaround in the hooking mechanism for an obsolete anti-malware product



## [0.3.5 / 5.42.1] - 2020-07-19

### Added
- added settings window
- added translation support
- added dark theme
- added auto start option
- added sandbox options
- added debug option "NoAddProcessToJob=y"

### Changed
- improved empty sandbox tray icon
- improved message parsing
- updated homepage links

### Fixed
- fixed INI issue with SandMan.exe when renaming sandboxes
- fixed INI auto reload bug introduced in the last build
- fixed issue when hooking delayed loaded libraries



## [0.3 / 5.42] - 2020-07-04

### Added
- API_QUERY_PROCESS_INFO can be now used to get the original process token of sandboxed processes
  - Note: this capability is used by TaskExplorer to allow inspecting sandbox internal tokens
- added option "KeepTokenIntegrity=y" to make the Sbie token keep its initial integrity level (debug option)
  - Note: do not use Debug Options if you don't know their security implications (!)
- added process id to log messages very useful for debugging
- added finder to resource log
- added option "HideHostProcess=program.exe" to hide unsandboxed host processes
  - Note: Sbie hides by default processes from other boxes, this behaviour can now be controlled with "HideOtherBoxes=n"
- sandboxed RpcSs and DcomLaunch can now be run as system with the option "ProtectRpcSs=y" however this breaks the sandboxed Windows Explorer and others
- built-in Clsid whitelist can now be disabled with "OpenDefaultClsid=n"
- processes can be now terminated with the del key, and require a confirmation
- added sandboxed window border display to SandMan.exe
- added notification for Sbie log messages
- added Sandbox Presets submenu to quickly change some settings
  - Enable/Disable API logging; LogAPI DLLs are now distributed with Sandboxie Plus
  - Drop admin rights
  - Block/Allow internet access
  - Block/Allow access to files on the network
- added more info to the sandbox status column
- added path column to SbieModel
- added info tooltips in SbieView

### Changed
- reworked ApiLog, added PID and PID filter
- auto config reload on change is now delayed by 500ms to prevent reloading multiple times on incremental changes
- Sandbox names now replace "_" with " " thus enabling names that consist of separate words

### Fixed
- added missing PreferExternalManifest initialization to portable mode
- FIXED SECURITY ISSUE ID-2: fixed permission issues with sandboxed system processes
  - Note: you can use "ExposeBoxedSystem=y" for the old behaviour (debug option)
- FIXED SECURITY ISSUE ID-3: fixed missing SCM access check for sandboxed services (thanks Diversenok)
  - Note: to disable the access check use "UnrestrictedSCM=y" (debug option)
- fixed missing initialization in service server that caused sandboxed programs to crash when querying service status
- fixed many bugs that caused the SbieDrv.sys to BSoD when running with Driver Verifier enabled [#57](https://github.com/sandboxie-plus/Sandboxie/issues/57)
  - 0xF6 in GetThreadTokenOwnerPid and File_Api_Rename
  - missing non optional parameter for FltGetFileNameInformation in File_PreOperation
  - 0xE3 in Key_StoreValue and Key_PreDataInject



## [0.2.2 / 5.41.2] - 2020-06-19

### Added
- added option "SeparateUserFolders=n" to no longer have the user profile files stored separately in the sandbox
- added "SandboxieLogon=y" - it makes processes run under the SID of the "Sandboxie" user instead of the Anonymous user
  - Note: the global option "AllowSandboxieLogon=y" must be enabled, the "Sandboxie" user account must be manually created first and the driver reloaded, else process start will fail
- improved debugging around process creation errors in the driver

### Fixed
- fixed log messages getting lost after driver reload
- fixed MSI installer issue, see Proc_CreateProcessInternalW_RS5



## [0.2.1 / 5.41.1] - 2020-06-18

### Added
- added different sandbox icons for different types
  - Red LogAPI/BSA enabled
  - more to come :D
- added progress window for async operations that take time
- added DPI awareness [#56](https://github.com/sandboxie-plus/Sandboxie/issues/56)
- the driver file is now obfuscated to avoid false positives
- additional debug option for Sandboxie.ini named OpenToken=y which combines UnrestrictedToken=y and UnfilteredToken=y
  - Note: using these options weakens the sandboxing, they are intended for debugging and may be used for better application virtualization later

### Changed
- SbieDll.dll when processing InjectDll now looks in the SbieHome folder for the DLLs if the entered path starts with a backslash
  - i.e. "InjectDll=\LogAPI\i386\logapi32v.dll" or "InjectDll64=\LogAPI\amd64\logapi64v.dll"

### Fixed
- IniWatcher did not work in portable mode
- service path fix broke other services
- workaround for the MSI installer issue



## [0.2 / 5.41.0] - 2020-06-08

### Added
- IniWatcher, the .ini is now reloaded automatically every time it changes
- added Maintenance menu to the Sandbox menu, allowing to install/uninstall and start/stop Sandboxie driver, service
- SandMan.exe now is packed with Sbie files and when no Sbie is installed acts as a portable installation
- added option to clean-up logs

### Changed
- Sbie driver now first checks the home path for the configuration file Sandboxie.ini before checking SystemRoot

### Fixed
- FIXED SECURITY ISSUE ID-1: sandboxed processes could obtain a write handle on non sandboxed processes (thanks Diversenok)
  - this allowed to inject code in non sandboxed processes
- fixed issue boxed services not starting when the path contained a space
- NtQueryInformationProcess now returns the proper sandboxed path for sandboxed processes



## [0.1 / 5.40.2] - 2020-06-01

### Added
- created a new Qt-based UI named SandMan (Sandboxie Manager)
- Resource Monitor now shows the PID
- added basic API call log using updated BSA LogAPI library

### Changed
- reworked Resource Monitor to work with multiple event consumers
- reworked log to work with multiple event consumers



## [5.40.1] - 2020-04-10

### Added
- added the new "Other" type for the Resource Access Monitor
- added call to StartService to the logged Resources

### Fixed
- fixed "Windows Installer Service could not be accessed" that got introduced with Windows 1903
