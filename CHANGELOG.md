# Changelog
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).



## [1.0.6 / 5.55.6] - 2021-12-??

### Added
- replaced "Open with" with a Sandboxie dialog to work on Windows 10 [#1138](https://github.com/sandboxie-plus/Sandboxie/issues/1138)
- added ability to run Store apps in App Compartment mode (on Windows 11 requires COM to be open)
- added new debug options "UnstrippedToken=y" and "KeepUserGroup=y"
- added double click to recover files and folders in recovery window [#1466](https://github.com/sandboxie-plus/Sandboxie/issues/1466)
- added Ukrainian language on Plus UI (by SuperMaxusa) [#1488](https://github.com/sandboxie-plus/Sandboxie/pull/1488)

### Changed
- "UseSbieWndStation=y" is now the default behaviour [#1442](https://github.com/sandboxie-plus/Sandboxie/issues/1442)
- disabled Win32k hooking when HVCI is enabled due to an incompatibility (BSOD) [#1483](https://github.com/sandboxie-plus/Sandboxie/issues/1483)

### Fixed
- fixed box initialization issue in Privacy mode [#1469](https://github.com/sandboxie-plus/Sandboxie/issues/1469)
- fixed issue with shortcuts creation introduced in a recent build [#1471](https://github.com/sandboxie-plus/Sandboxie/issues/1471)
- fixed various issues in Privacy Enhanced boxes and rule specificity
- fixed issue with SeAccessCheckByType and alike
- fixed issues with Win32k hooking on 32 bit Windows [#1479](https://github.com/sandboxie-plus/Sandboxie/issues/1479)

### Removed
- removed obsolete SkyNet rootkit detection from 32 bit build




## [1.0.5 / 5.55.5] - 2021-12-25

### Added
- sandbox top level exception handler to create crash dumps
-- it can be enabled per process or globally using "EnableMiniDump=process.exe,y" or "EnableMiniDump=y" respectively
-- the dump flags can be set as hex with "MiniDumpFlags=0xAABBCCDD"
-- a preselected flag set for a verbose dump can be set with "MiniDumpFlags=Extended"
-- Note: created dump files are located at: `C:\Sandbox\%SANDBOX%`
- added template support for Osiris and Slimjet browsers (by Dyras) [#1454](https://github.com/sandboxie-plus/Sandboxie/pull/1454)

### Changed
- improved SbieDll initialization a bit
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
- Mechanism to hook Win32 system calls now also works for 32 bit applications running under WoW64
- Added customization to Win32k hooking mechanism, as by default only GdiDdDDI* hooks are installed
-- You can force the installation of other hooks by specifying them with "EnableWin32Hook=..." 
-- or disable the installation of the default hooks with "DisableWin32Hook=..."
-- Please note that some Win32k hooks may cause BSODs or undefined behaviour. (!)
-- The most obviously problematic Win32k hooks are blacklisted, this can be bypassed with "IgnoreWin32HookBlacklist=y"
- added debug option "AdjustBoxedSystem=n" to disable the adjustment of service ACLs running with a system token
- added "NoUACProxy=y" option together with the accompanying template, in order to disable UAC proxy
-- Note: Boxes configured in compartment mode activate this template by default
- added UI option to change default RpcMgmtSetComTimeout preset
- added Plus installer option to start the default browser under Sandboxie through a desktop shortcut
- added more entries to the Plus installer (current translations on [Languages.iss](https://github.com/sandboxie-plus/Sandboxie/blob/master/Installer/Languages.iss) file need to be updated)

### Changed
- "EnableWin32kHooks=y" is now enabled by default, as no issues were reported in 1.0.3
-- Note: currently only the GdiDdDDI* hooks are applied, required for Chromium HW acceleration
- Cleaned up low level hooking code a bit
- "RunRpcssAsSystem=y" is now auto applied for boxes in "App Compartment" mode when "RunServicesAsSystem=y" or "MsiInstallerExemptions=y" are present

### Fixed
- fixed RPC handling in case a requested open service is not running [#1443](https://github.com/sandboxie-plus/Sandboxie/issues/1443)
- fixed a hooking issue with NdrClientCall2 in 32 bit applications
- fixed issue with start directory to run sandboxed when using SandMan [#1436](https://github.com/sandboxie-plus/Sandboxie/issues/1436)
- fixed issue with recovering from network share locations [#1435](https://github.com/sandboxie-plus/Sandboxie/issues/1435)



## [1.0.3 / 5.55.3] - 2021-12-12

### Added
- added mechanism to hook Win32k system calls on Windows 10 and later, this should resolve the issue with Chromium HW acceleration
-- Note: this mechanism does not, yet, work for 32 bit applications running under WoW64
-- to enable it, add "EnableWin32kHooks=y" to the global ini section, this feature is highly experimental (!)
-- the hooks will be automatically applied to Chromium GPU processes
-- to force Win32k hooks for all processes in a selected box, add "AlwaysUseWin32kHooks=program.exe,y" [#1261](https://github.com/sandboxie-plus/Sandboxie/issues/1261) [#1395](https://github.com/sandboxie-plus/Sandboxie/issues/1395)

### Fixed
- fixed bug in GetVersionExW making "OverrideOsBuild=..." not working [#605](https://github.com/sandboxie-plus/Sandboxie/issues/605) [#1426](https://github.com/sandboxie-plus/Sandboxie/issues/1426)
- fixed issue with some UTF-8 characters when used in the ini file
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
- fixed BSOD issue introduced in the 1.0.0 build [#1389](https://github.com/sandboxie-plus/Sandboxie/issues/1389)
- fixed multiple BITS notifications while running sandboxed Chromium browsers (by isaak654) [ca320ec](https://github.com/sandboxie-plus/Sandboxie/commit/ca320ecc17180ff09a67bdefc524b30cf3540c08) [#1081](https://github.com/sandboxie-plus/Sandboxie/issues/1081)
- fixed executables selection for "Run Menu" entries (by isaak654) [#1379](https://github.com/sandboxie-plus/Sandboxie/issues/1379)
- fixed SetCursorPos and ClipCursor ignoring DPI awareness (by alvinhochun) [#1394](https://github.com/sandboxie-plus/Sandboxie/pull/1394)

### Removed
- removed Virtual Desktop Manager template (by isaak654) [d775807](https://github.com/sandboxie-plus/Sandboxie/commit/d7758071f6930539c4e1f236297b4cfa332346ad) [#1326](https://github.com/sandboxie-plus/Sandboxie/discussions/1326)



## [1.0.0 / 5.55.0] - 2021-11-17

### Added 
- added Privacy enhanced mode, sandboxes with "UsePrivacyMode=y" will not allow read access to locations containing user data
-- all locations except generic Windows system paths will need to be opened explicitly for read and/or write access
-- using "NormalFilePath=...", "NormalKeyPath=...", "NormalIpcPath=..." allows to open locations to be readable and sandboxed

- added new "App Compartment" mode of operation, it is enabled by adding "NoSecurityIsolation=y" to the box configuration
-- in this mode, security is traded in for compatibility, it should not be used for untrusted applications
-- Note: in this mode, file and registry filtering are still in place, hence processes run without administrative privileges
-- it is reasonably safe, all filtering can be disabled with "NoSecurityFiltering=y"

- added experimental use of ObRegisterCallbacks to filter object creation and duplication 
-- this filtering is independent from the regular SbieDrv's syscall-based filtering, hence it also applies to App Compartments
-- with it enabled, an application running in a compartment will not be able to manipulate processes running outside the sandbox
-- Note: this feature improves the security of unisolated App Compartment boxes
-- to enable this feature, set "EnableObjectFiltering=y" in the global section and reload the driver
-- when globally activated, the filtering can be disabled for individual boxes with "DisableObjectFilter=y"

- added "DontOpenForBoxed=n", this option disables the discrimination of boxed processes for open file and open key directives
-- this behaviour does not really improve security anyways, but may be annoying, also app compartments always disable this

- added setting to entirely open access to the COM infrastructure

### Changed
- reworked the resource access path matching mechanism to optionally apply more specific rules over less specific ones
-- for example "OpenFilePath=C:\User\Me\AppData\Firefox takes precedence over "WriteFilePath=C:\User\Me\"
-- to enable this new behaviour, add "UseRuleSpecificity=y" to your Sandboxie.ini, this behaviour is always enabled in Privacy enhanced mode
-- added "NormalFilePath=..." to restore default Sandboxie behaviour on a given path
-- added "OpenConfPath=...", which similarly to "OpenPipePath=..." is a "OpenKeyPath=..." variant which applies to executables located in the sandbox
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
-- this allows to run objects with the CLSCTX_INPROC_SERVER flag in the COM helper service
- in the trace view, now multiple types can be selected at once
- a few Plus UI entries were made translatable (by gexgd0419) [#1320](https://github.com/sandboxie-plus/Sandboxie/pull/1320)
- changed default "terminate all boxed processes" key to Shift+Pause (by isaak654) [#1337](https://github.com/sandboxie-plus/Sandboxie/issues/1337)

### Fixed
- fixed ini writing issue with SbieCtrl and the new ini handling mechanism [#1331](https://github.com/sandboxie-plus/Sandboxie/issues/1331)
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
- Plus installer will autostart SandMan.exe after install to fix a taskbar icon issue [#3040211](https://www.wilderssecurity.com/threads/sandboxie-plus-0-9-7-test-build.440906/page-4#post-3040211)
- Classic installer will show the license agreement when updating [#1187](https://github.com/sandboxie-plus/Sandboxie/issues/1187)

### Fixed
- fixed template sections not showing in editor [#1287](https://github.com/sandboxie-plus/Sandboxie/issues/1287)
- fixed issue with app ID resulting in some apps showing two button groups in the taskbar [#1101](https://github.com/sandboxie-plus/Sandboxie/issues/1101)
- fixed issue with maximum ini value length on Plus UI [#1293](https://github.com/sandboxie-plus/Sandboxie/issues/1293)
- fixed issue handling an empty Sandboxie.ini that got introduced recently [#1292](https://github.com/sandboxie-plus/Sandboxie/issues/1292)
- fixed issue with "SpecialImages" template (by Coverlin) [#1288](https://github.com/sandboxie-plus/Sandboxie/issues/1288) [#1289](https://github.com/sandboxie-plus/Sandboxie/issues/1289)
- fixed issue with box emptying [#1296](https://github.com/sandboxie-plus/Sandboxie/issues/1296)
- fixed issues with some languages [#1304](https://github.com/sandboxie-plus/Sandboxie/issues/1304)
- fixed issue with mounted directories [#1302](https://github.com/sandboxie-plus/Sandboxie/issues/1302)
- added missing translation for qt libraries [#1305](https://github.com/sandboxie-plus/Sandboxie/issues/1305)
- fixed issue with Windows compatibility assistant [#1265](https://github.com/sandboxie-plus/Sandboxie/issues/1265)
- fixed issue with process image specific settigns [#1307](https://github.com/sandboxie-plus/Sandboxie/issues/1307)





## [0.9.8 / 5.53.0] - 2021-10-15

### Added 
- added debug switch to disable Sbie console redirection "NoSandboxieConsole=y" 
-- Note: this was previously part of "NoSandboxieDesktop=y"
- added Sbie+ version to the log [#1277](https://github.com/sandboxie-plus/Sandboxie/issues/1277)
- added uninstall clean-up of extra files for the Plus installer (by mpheath) [#1235](https://github.com/sandboxie-plus/Sandboxie/pull/1235)
- added set language for Sandman for the Plus installer (by mpheath) [#1241](https://github.com/sandboxie-plus/Sandboxie/issues/1241)
- added EventLog messages with SbieMsg.dll for the Plus installer (by mpheath)
- group expansion state is now saved
- added additional filters to the trace tab
- added a new section [DefaultTemplates] in Templates.ini which contains mandatory templates that are always applied [0c9ecb0](https://github.com/sandboxie-plus/Sandboxie/commit/0c9ecb084286821c0db7436c41ef99e3b9daca76#diff-965721e9c3f2350b16f4acb47d3fb75654976f0dbb4da3c507d0eaff16a4f5f2)

### Changed
- reworked and extended RPC logging
- reintroduced the "UseRpcMgmtSetComTimeout=some.dll,n" setting to be used when no "RpcPortBinding" entry is specified
--- this allows to enable/disable out of box RPC binding independently from the timeout setting
- the "BoxNameTitle" value can now be set explicitly on a per image name basis [#1190](https://github.com/sandboxie-plus/Sandboxie/issues/1190)

### Fixed
- fixed inability to delete read-only files from sandboxed explorer [#1237](https://github.com/sandboxie-plus/Sandboxie/issues/1237)
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
- fixed yet another ini issue with the SbieCtrl


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
- reworked SbieSvc ini server to allow settings caching and greatly improve performance
-- Now comments in the Sandboxie.ini are being preserved as well as the order of all entries
- enabled configuration section list replacement with a hash map to improve configuration performance
- improved progress and status messages for the Plus installer (by mpheath) [#1168](https://github.com/sandboxie-plus/Sandboxie/pull/1168)
- reworked RpcSs start mechanics, sandboxed RpcSs and DcomLaunch can now be run as system, use "RunRpcssAsSystem=y"
-- Note: this is generally not recommended for security reasons but may be needed for compatibility in some scenarios
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
-- Note: the processes then have a system token, hence it's recommended to enable "DropAdminRights=y"
- if the UI is run with admin privileges, it can terminate sandboxed processes in other sessions now
- added "StartSystemBox=" option to auto-run a box on Sbie start/system boot in session 0
-- Note: box start is done by issuing Start.exe /box:[name] auto_run
- add Start.exe auto_run command to start all sandboxed auto-start locations
- add Start.exe /keep_alive command line switch which keeps a process running in the box until it gracefully terminates
- added "StartCommand=" which starts a complex command through Start.exe on box startup
- added menu option to start regedit and load the box's registry key
- added system tray option in the Plus UI to show Classic icon [#963](https://github.com/sandboxie-plus/Sandboxie/issues/963#issuecomment-903933535)

### Changed
- changed command prompt icon and string from "Terminal" to "Command Prompt" [#1135](https://github.com/sandboxie-plus/Sandboxie/issues/1135)
- reworked box menu layout a bit

### Fixed
- fixed driver compatibility with Windows Server 2022 (build 20348) [#1143](https://github.com/sandboxie-plus/Sandboxie/issues/1143)
- fixed issue with creating shortcuts [#1134](https://github.com/sandboxie-plus/Sandboxie/issues/1134)

### Installers re-released on 2021-08-31 with the following fix:
- fixed KmdUtil warning 1061 after Plus upgrade (by mpheath) [#968](https://github.com/sandboxie-plus/Sandboxie/issues/968) [#1139](https://github.com/sandboxie-plus/Sandboxie/issues/1139)



## [0.9.4 / 5.51.4] - 2021-08-22

### Added
- added clear commands to log submenus [#391](https://github.com/sandboxie-plus/Sandboxie/issues/391)
- added option to disable process termination prompt [#514](https://github.com/sandboxie-plus/Sandboxie/issues/514)
- added "Options/InstantRecovery" setting to sandboxie-plus.ini to use the recovery window instead of the notification pop-up [#988](https://github.com/sandboxie-plus/Sandboxie/issues/988)
- added ability to rename a non-empty sandbox [#1100](https://github.com/sandboxie-plus/Sandboxie/issues/1100)
- added ability to remove a non-empty sandbox 
- added file browser window to SandMan UI to cover the file-view functionality of SbieCtrl [#578](https://github.com/sandboxie-plus/Sandboxie/issues/578)

### Changed
- generic errors in Sbie UI now show the status code as hex and provide a string description when available

### Fixed
- fixed "del" shortcut to terminate a process not always working
- fixed group display issue [#1094](https://github.com/sandboxie-plus/Sandboxie/issues/1094)
- fixed issue when using "run sandboxed" on a file that is already located in a sandbox [#1099](https://github.com/sandboxie-plus/Sandboxie/issues/1099)



## [0.9.3 / 5.51.3] - 2021-08-08

> Read the developer's notes about the new [WFP functionality](https://github.com/sandboxie-plus/Sandboxie/releases/tag/0.9.3).

### Added
- ability to use the "run unsandboxed" option with Sandboxie links [#614](https://github.com/sandboxie-plus/Sandboxie/issues/614)

### Fixed
- fixed "run outside sandbox" issue on Classic build [#614](https://github.com/sandboxie-plus/Sandboxie/issues/614#issuecomment-894710466)
- fixed open template does not load the edit tab [#1054](https://github.com/sandboxie-plus/Sandboxie/issues/1054#issuecomment-893001316)
- fixed issue with "explore sandboxed" [#972](https://github.com/sandboxie-plus/Sandboxie/issues/972)
- fixed start directory for sandboxed processes [#1071](https://github.com/sandboxie-plus/Sandboxie/issues/1071)
- fixed issue with language auto-detection [#1018](https://github.com/sandboxie-plus/Sandboxie/issues/1018)
- fixed issue with multiple files with the same name, by always showing the extension [#1041](https://github.com/sandboxie-plus/Sandboxie/issues/1041)
- fixed multiple program grouping issues with the SandMan UI [#1054](https://github.com/sandboxie-plus/Sandboxie/issues/1054)
- fixed "no disk" error [#966](https://github.com/sandboxie-plus/Sandboxie/issues/966)
- fixed issue with 32bit build using qMake, the -O2 option resulted in a crash in the QSbieAPI.dll [#995](https://github.com/sandboxie-plus/Sandboxie/issues/995)
- fixed issue with UserSettings introduced in a recent build [#1054](https://github.com/sandboxie-plus/Sandboxie/issues/1054)



## [0.9.2 / 5.51.2] - 2021-08-07 (pre-release)

### Added
- added ability to reconfigure the driver, which allows enabling/disabling WFP and other features without a reload/reboot

### Changed
- reorganized and improved the settings window
- improved the tray icon a bit, the sand is now more yellow

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
-- to enable this support, add 'NetworkEnableWFP=y' to the global section and reboot or reload the driver
-- to use WFP for a specific sandbox, add 'AllowNetworkAccess=n'
-- you can allow certain processes by using 'AllowNetworkAccess=program.exe,y'
-- you can also enable this policy globally by adding 'AllowNetworkAccess=n' to the global section
-- in this case you can exempt entire sandboxes by adding 'AllowNetworkAccess=y' to specific boxes
 -- you can block certain processes by using 'AllowNetworkAccess=program.exe,n'
 -- Note: WFP is less absolute than the old approach, using WFP will filter only TCP/UDP communication
--	restricted boxed processes will still be able to resolve domain names using the system service
--  however, they will not be able to send or receive data packets directly
-- the advantages of WFP is that filter rules can be implemented by restricting communication only to specified addresses or selected ports using "NetworkAccess=..."
- added fully functional rule-based packet filter in user mode for the case when "NetworkEnableWFP=y" is not set
-- the mechanism replaces the old "BlockPort=..." functionality
-- Note: this filter applies only to outgoing connections/traffic, for incoming traffic either the WFP mode or a third-party firewall is needed
-- like the old user mode based mechanism, malicious applications can bypass it by unhooking certain functions
-- hence it's recommended to use the kernel mode WFP-based mechanism when reliable isolation is required
- added new trace option "NetFwTrace=*" to trace the actions of the firewall components
-- please note that the driver only trace logs the kernel debug output, use DbgView.exe to log
- API_QUERY_PROCESS_INFO can now be used to get the impersonation token of a sandboxed thread
-- Note: this capability is used by TaskExplorer to allow inspecting sandbox-internal tokens
-- Note: a process must have administrative privileges to be able to use this API
- added a UI option to switch "MsiInstallerExemptions=y" on and off
-- just in case a future Windows build breaks something in the systemless mode
- added sample code for ObRegisterCallbacks to the driver
- added new debug options "DisableFileFilter=y" and "DisableKeyFilter=y" that allow to disable file and registry filtering
-- Note: these options are for testing only and disable core parts of the sandbox isolation
- added a few command line options to SandMan.exe

### Changed
- greatly improved the performance of the trace log, but it's no longer possible to log to both SandMan and SbieCtrl at the same time
- reworked process creation code to use PsSetCreateProcessNotifyRoutineEx and improved process termination

### Fixed
- added missing hook for ConnectEx function



## [0.8.9 / 5.50.9] - 2021-07-28 HotFix 2

### Fixed
Fixed issue with registering session leader



## [0.8.9 / 5.50.9] - 2021-07-28 HotFix 1

### Fixed
Fixed issue with Windows 7



## [0.8.9 / 5.50.9] - 2021-07-27

### Changed
- updated a few icons
- updated GitHub build action to use Qt 5.15.2
- improved the "full" tray icon to be more distinguishable from the "empty" one
- changed code integrity verification policies [#1003](https://github.com/sandboxie-plus/Sandboxie/issues/1003)
-- code signature is no longer required to change config, to protect presets use the existing "EditAdminOnly=y"

### Fixed
- fixed issue with systemless MSI mode introduced in the last build
- fixed MSI installer not being able to create the action server mechanism on Windows 11
- fixed MSI installer not working in systemless mode on Windows 11
- fixed Inno Setup script not being able to remove shell integration keys during Sandboxie Plus uninstall (by mpheath) [#1037](https://github.com/sandboxie-plus/Sandboxie/pull/1037)



## [0.8.8 / 5.50.8] - 2021-07-13

### Changed
- MSIServer no longer requires being run as system; this completes the move to not use system tokens in a sandbox by default
-- the security-enhanced option "MsiInstallerExemptions=n" is now the default behaviour

### Fixed
- fixed issue with the "Explore Sandboxed" command [#972](https://github.com/sandboxie-plus/Sandboxie/issues/972)
- rolled back the switch from using NtQueryKey to NtQueryObject as it seems to break some older Windows 10 versions like 1803 [#984](https://github.com/sandboxie-plus/Sandboxie/issues/984)
-- this change was introduced to fix [#951](https://github.com/sandboxie-plus/Sandboxie/issues/951)
-- to use NtQueryObject the option "UseObjectNameForKeys=y" can be added to Sandboxie.ini



## [0.8.7b / 5.50.7] - 2021-07-11

### Fixed
- fixed issue with boxes that had auto-delete activated introduced in the previous build [#986](https://github.com/sandboxie-plus/Sandboxie/issues/986)



## [0.8.7 / 5.50.7] - 2021-07-10

### Added
- added option to always auto-pick the DefaultBox [#959](https://github.com/sandboxie-plus/Sandboxie/issues/959)
-- when this option is enabled, the normal behaviour with a box selection dialog can be brought up by holding down CTRL
- added option to hide a sandbox from the "run in box" dialog
-- useful to avoid listing insecure compatibility test boxes for example
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
- the Run Sandboxed dialog can now be handled by the SandMan UI
- added "AllowBoxedJobs=y" allowing boxed processes to use nested jobs on Windows 8 and later
-- Note: this allows Chrome and other programs to use the job system for additional isolation
- added Librewolf.exe to the list of Firefox derivatives [#927](https://github.com/sandboxie-plus/Sandboxie/issues/927)
- added run regedit sandboxed menu command
- added new support settings tab to SandMan UI for updates and news
- added code integrity verification to Sbie service and UI
- added template for Vivaldi Notes (by isaak654) [#948](https://github.com/sandboxie-plus/Sandboxie/issues/948)

### Changed
- replaced the Process List used by the driver with a much faster Hash Map implementation
-- Note: this change provides an almost static system call speed of 1.2µs regardless of the running process count
-- The old list, with 100 programs running required 4.5µs; with 200: 12µs; and with 300: 18µs per syscall
-- Note: some of the slowdown was also affecting non-sandboxed applications due to how the driver handles certain callbacks
- replaced the per-process Thread List used by the driver with a much faster Hash Map implementation
- replaced configuration section list with a hash map to improve configuration performance, and increased line limit to 100000
-- not yet enabled in production build
- the presence of the default box is only checked on connect
- the portable directory dialog now shows the directory [#924](https://github.com/sandboxie-plus/Sandboxie/issues/924)
- when terminated, boxed processes now first try doing that by terminating the job object
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
-- the notification window is not only topmost for 5 seconds
- fixed an issue deleting directories introduced in 5.49.5
- fixed an issue when creating box copies

### Removed
- removed switch for "BlockPassword=n" as it does not seem to be working [#938](https://github.com/sandboxie-plus/Sandboxie/issues/938)
-- it's recommended to use "OpenSamEndpoint=y" to allow password changes in Windows 10



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
- fixed DcomLaunch issue on Windows 7 32 bit introduced in 5.50.0 [#898](https://github.com/sandboxie-plus/Sandboxie/issues/898)



## [0.8.0 / 5.50.0] - 2021-06-13

### Added
- Normally Sandboxie applies "Close...=!<program>,..." directives to non-excluded images if they are located in a sandbox
-- added 'AlwaysCloseForBoxed=n' to disable this behaviour as it may not be always desired, and it doesn't provide extra security
- added process image information to SandMan UI
- localized template categories in the Plus UI [#727](https://github.com/sandboxie-plus/Sandboxie/issues/727)
- added "DisableResourceMonitor=y" to disable resource access monitor for selected boxes [#886](https://github.com/sandboxie-plus/Sandboxie/issues/886)
- added option to show trace entries only for the selected sandbox [#886](https://github.com/sandboxie-plus/Sandboxie/issues/886)
- added "UseVolumeSerialNumbers=y" that allows drive letters to be suffixed with the volume SN in the \drive\ sandbox location
-- it helps to avoid files mixed together on multiple pendrives using the same letter
-- Note: this option is not compatible with the recovery function of the Classic UI, only SandMan UI is fully compatible
- added "ForceRestart=PicoTorrent.exe" to the PicoTorrent template in order to fix a compatibility issue [#720](https://github.com/sandboxie-plus/Sandboxie/issues/720)
- added localization support for RPC templates (by isaak654) [#736](https://github.com/sandboxie-plus/Sandboxie/issues/736)

### Changed
- portable clean-up message now has yes/no/cancel options [#874](https://github.com/sandboxie-plus/Sandboxie/issues/874)
- consolidated Proc_CreateProcessInternalW and Proc_CreateProcessInternalW_RS5 to remove duplicate code
- the ElevateCreateProcess fix, as sometimes applied by the Program Compatibility Assistant, will no longer be emulated by default [#858](https://github.com/sandboxie-plus/Sandboxie/issues/858)
-- use 'ApplyElevateCreateProcessFix=y' or 'ApplyElevateCreateProcessFix=program.exe,y' to enable it
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
-- resolves issues with file save dialogs taking 30+ seconds to open
-- this fix greatly improves the win32 GUI performance of sandboxed processes
- reworked RPC resolver to be ini-configurable
-- the following options are now deprecated:
--- "UseRpcMgmtSetComTimeout=some.dll,n", so use "RpcPortBinding=some.dll,*,TimeOut=y"
--- "OpenUPnP=y", "OpenBluetooth=y", "OpenSmartCard=n", so use the new RPC templates instead
-- See Templates.ini for usage examples

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
-- since the runas facility is not accessible by default, this did not constitute a security issue
-- to enable runas functionality, add "OpenIpcPath=\RPC Control\SECLOGON" to your Sandboxie.ini
-- please take note that doing so may open other yet unknown issues
- fixed a driver compatibility issue with Windows 10 32 bit Insider Preview Build 21337
- fixed issues with driver signature for Windows 7



## [0.7.2 / 5.49.0] - 2021-03-04

### Added
- added option to alter reported Windows version "OverrideOsBuild=7601" for Windows 7 SP1 [#605](https://github.com/sandboxie-plus/Sandboxie/issues/605)
- the trace log can now be structured like a tree with processes as root items and threads as branches

### Changed
- SandboxieCrypto now always migrates the CatRoot2 files in order to prevent locking of real files
- greatly improved trace log performance
- MSI Server can now run with the "FakeAdminRights=y" and "DropAdminRights=y" options [#600](https://github.com/sandboxie-plus/Sandboxie/issues/600)
-- special service allowance for the MSI Server can be disabled with "MsiInstallerExemptions=n"
- changed SCM access check behaviour; non elevated users can now start services with a user token
-- elevation is now only required to start services with a system token
- reworked the trace log mechanism to be more verbose
- reworked RPC mechanism to be more flexible

### Fixed
- fixed issues with some installers introduced in 5.48.0 [#595](https://github.com/sandboxie-plus/Sandboxie/issues/595)
- fixed "add user to sandbox" in the Plus UI [#597](https://github.com/sandboxie-plus/Sandboxie/issues/597)
- FIXED SECURITY ISSUE: the HostInjectDll mechanism allowed for local privilege escalation (thanks hg421)
- Classic UI no longer allows to create a sandbox with an invalid or reserved device name [#649](https://github.com/sandboxie-plus/Sandboxie/issues/649)



## [0.7.1 / 5.48.5] - 2021-02-21

### Added
- enhanced RpcMgmtSetComTimeout handling with "UseRpcMgmtSetComTimeout=some.dll,n"
-- this option allows to specify if RpcMgmtSetComTimeout should be used or not for each individual dll
-- this setting takes precedence over hard-coded and per-process presets
-- "UseRpcMgmtSetComTimeout=some.dll" and "UseRpcMgmtSetComTimeout=some.dll,y" are equivalent
- added "FakeAdminRights=y" option that makes processes think they have admin permissions in a given box
-- this option is recommended to be used in combination with "DropAdminRights=y" to improve security
-- with "FakeAdminRights=y" and "DropAdminRights=y" installers should still work
- added RPC support for SSDP API (the Simple Service Discovery Protocol), you can enable it with "OpenUPnP=y"


### Changed
- SbieCrypto no longer triggers message 1313
- changed enum process API; now more than 511 processes per box can be enumerated (no limit)
- reorganized box settings a bit
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
-- the value denotes the count of 4K large pages to be used; here for a total of 10 MB
- new functionality for the list finder

### Changed
- improved RPC debugging
- improved IPC handling around RpcMgmtSetComTimeout; "RpcMgmtSetComTimeout=n" is now the default behaviour
-- required exceptions have been hard-coded for specific calling DLLs
- the LogApi dll is now using Sbie's tracing facility to log events instead of its own pipe server

### Fixed
- FIXED SECURITY ISSUE: elevated sandboxed processes could access volumes/disks for reading (thanks hg421)
-- this protection option can be disabled by using "AllowRawDiskRead=y"
- fixed crash issue around SetCurrentProcessExplicitAppUserModelID observed with GoogleUpdate.exe
- fixed issue with Resource Monitor sort by timestamp
- fixed invalid Opera bookmarks path (by isaak654) [#542](https://github.com/sandboxie-plus/Sandboxie/pull/542)
- FIXED SECURITY ISSUE: a race condition in the driver allowed to obtain an elevated rights handle to a process (thanks typpos) [#549](https://github.com/sandboxie-plus/Sandboxie/pull/549)
- FIXED SECURITY ISSUE: "\RPC Control\samss lpc" is now filtered by the driver (thanks hg421) [#553](https://github.com/sandboxie-plus/Sandboxie/issues/553)
-- this allowed elevated processes to change passwords, delete users and alike; to disable filtering use "OpenSamEndpoint=y"
- FIXED SECURITY ISSUE: "\Device\DeviceApi\CMApi" is now filtered by the driver (thanks hg421) [#552](https://github.com/sandboxie-plus/Sandboxie/issues/552)
-- this allowed elevated processes to change hardware configuration; to disable filtering use "OpenDevCMApi=y"



## [0.6.7 / 5.47.1] - 2021-02-01

### Added
- added UI language auto-detection

### Fixed
- fixed Brave.exe now being properly recognized as Chrome-, not Firefox-based 
- fixed issue introduced in 0.6.5 with recent Edge builds
-- the 0.6.5 behaviour can be set on a per-process basis using "RpcMgmtSetComTimeout=POPPeeper.exe,n"
- fixed grouping issues [#445](https://github.com/sandboxie-plus/Sandboxie/issues/445)
- fixed main window restore state from tray [#288](https://github.com/sandboxie-plus/Sandboxie/issues/288)



## [0.6.5 / 5.47.0] - 2021-01-31

### Added
- added detection for Waterfox.exe, Palemoon.exe and Basilisk.exe Firefox forks as well as Brave.exe [#468](https://github.com/sandboxie-plus/Sandboxie/issues/468)
- added Bluetooth API support, IPC port can be opened with "OpenBluetooth=y" [#319](https://github.com/sandboxie-plus/Sandboxie/issues/319)
-- this should resolve issues with many Unity games hanging on startup for a long time
- added enhanced RPC/IPC interface tracing
- when DefaultBox is not found by the SandMan UI, it will be recreated
- "Disable Forced Programs" time is now saved and reloaded

### Changed
- reduced SandMan CPU usage
- Sandboxie.ini and Templates.ini can now be UTF-8 encoded [#461](https://github.com/sandboxie-plus/Sandboxie/issues/461) [#197](https://github.com/sandboxie-plus/Sandboxie/issues/197)
-- this feature is experimental, files without a UTF-8 Signature should be recognized also
-- "ByteOrderMark=yes" is obsolete, Sandboxie.ini is now always saved with a BOM/Signature
- legacy language files can now be UTF-8 encoded
- reworked file migration behaviour, removed hardcoded lists in favour of templates [#441](https://github.com/sandboxie-plus/Sandboxie/issues/441)
-- you can now use "CopyAlways=", "DontCopy=" and "CopyEmpty=" that support the same syntax as "OpenFilePath="
-- "CopyBlockDenyWrite=program.exe,y" makes a write open call to a file that won't be copied fail instead of turning it read-only
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
- added option to view disabled boxes and double click on box to enable it

### Changed
- "Reset Columns" now resizes them to fit the content, and it can now be localized [#426](https://github.com/sandboxie-plus/Sandboxie/issues/426)
- modal windows are now centered to the parent [#417](https://github.com/sandboxie-plus/Sandboxie/issues/417)
- improved new box window [#417](https://github.com/sandboxie-plus/Sandboxie/issues/417)

### Fixed
- fixed issues with window modality [#409](https://github.com/sandboxie-plus/Sandboxie/issues/409)
- fixed issues when main window was set to be always on top [#417](https://github.com/sandboxie-plus/Sandboxie/issues/417)
- fixed a driver issue with Windows 10 insider build 21286
- fixed issues with snapshot dialog [#416](https://github.com/sandboxie-plus/Sandboxie/issues/416)
- fixed an issue when writing to a path that already exists in the snapshot but not outside [#415](https://github.com/sandboxie-plus/Sandboxie/issues/415)



## [0.5.5 / 5.46.4] - 2021-01-17

### Added
- added "SandboxService=..." to force selected services to be started in the sandbox
- added template clean-up functionality to Plus UI
- added internet prompt to now also allow internet access permanently
- added browse button for box root folder in the SandMan UI [#382](https://github.com/sandboxie-plus/Sandboxie/issues/382)
- added explorer info message [#352](https://github.com/sandboxie-plus/Sandboxie/issues/352)
- added option to keep the SandMan UI always on top
- allow drag and drop file onto SandMan.exe to run it sandboxed [#355](https://github.com/sandboxie-plus/Sandboxie/issues/355)
- added start SandMan UI when a sandboxed application starts [#367](https://github.com/sandboxie-plus/Sandboxie/issues/367)
- recovery window can now list all files
- added file counter to recovery window
- when "NoAddProcessToJob=y" is specified, Chrome and related browsers now can fully use the job system
-- Note: "NoAddProcessToJob=y" reduces the box isolation, but the affected functions are mostly covered by UIPI anyway
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
- FIXED SECURITY ISSUE: the registry isolation could be bypassed, present since Windows 10 Creators Update
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
- FIXED SECURITY ISSUE: a Sandboxed process could start sandboxed as system even with DropAdminRights in place

### Removed
- removed "ProtectRpcSs=y" due to incompatibility with new isolation defaults



## [0.5.4 / 5.46.0] - 2021-01-06

### Added
- FIXED SECURITY ISSUE: Sandboxie now strips particularly problematic privileges from sandboxed system tokens
-- with those a process could attempt to bypass the sandbox isolation (thanks Diversenok)
-- old legacy behaviour can be enabled with "StripSystemPrivileges=n" (absolutely NOT Recommended) 
- added new isolation options "ClosePrintSpooler=y" and "OpenSmartCard=n" 
-- those resources are open by default, but for a hardened box it is desired to close them
- FIXED SECURITY ISSUE: added print spooler filter to prevent printers from being set up outside the sandbox
-- the filter can be disabled with "OpenPrintSpooler=y"
- added overwrite prompt when recovering an already existing file
- added "StartProgram=", "StartService=" and "AutoExec=" options to the SandMan UI
- added more compatibility templates (thanks isaak654) [#294](https://github.com/sandboxie-plus/Sandboxie/pull/294)

### Changed
- Changed Emulated SCM behaviour, boxed services are no longer by default started as boxed system
-- use "RunServicesAsSystem=y" to enable the old legacy behaviour
-- Note: sandboxed services with a system token are still sandboxed and restricted 
-- However not granting them a system token in the first place removes possible exploit vectors
-- Note: this option is not compatible with "ProtectRpcSs=y" and takes precedence!
- reworked dynamic IPC port handling
- improved Resource Monitor status strings

### Fixed
- FIXED SECURITY ISSUE: processes could spawn processes outside the sandbox (thanks Diversenok)
- FIXED SECURITY ISSUE: bug in the dynamic IPC port handling allowed to bypass IPC isolation
- fixed issue with IPC tracing
- FIXED SECURITY ISSUE: CVE-2019-13502 "\RPC Control\LSARPC_ENDPOINT" is now filtered by the driver (thanks Diversenok)
-- this allowed some system options to be changed, to disable filtering use "OpenLsaEndpoint=y"
- fixed hooking issues SBIE2303 with Chrome, Edge and possibly others [#68](https://github.com/sandboxie-plus/Sandboxie/issues/68) [#166](https://github.com/sandboxie-plus/Sandboxie/issues/166)
- fixed failed check for running processes when performing snapshot operations
- fixed some box option checkboxes were not properly initialized
- fixed unavailable options are not properly disabled when SandMan is not connected to the driver
- fixed MSI installer issue, not being able to create "C:\Config.msi" folder on Windows 20H2 [#219](https://github.com/sandboxie-plus/Sandboxie/issues/219)
- added missing localization to generic list commands
- fixed issue with "iconcache_*" when running sandboxed explorer
- fixed more issues with groups



## [0.5.3b / 5.45.2] - 2021-01-02

### Added
- added settings for the portable boxed root folder option
- added process name to resource log
- added command line column to the process view in the SandMan UI

### Fixed
- fixed a few issues with group handling [#262](https://github.com/sandboxie-plus/Sandboxie/issues/262) 
- fixed issue with GetRawInputDeviceInfo when running a 32 bit program on a 64 bit system
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
-- with this option you can enable special hardcoded workarounds to new obscure forks of those browsers
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
- removed suspend/resume menu entry, pooling that state wastes substantial CPU cycles; use task explorer for that functionality



## [0.5.2a / 5.45.1] - 2020-12-23

### Fixed
- fixed translation support in the SandMan UI
- fixed sandboxed explorer issue [#289](https://github.com/sandboxie-plus/Sandboxie/issues/289)
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
- reorganized the advanced box options a bit
- changed icons (thanks Valinwolf for picking the new ones) [#235](https://github.com/sandboxie-plus/Sandboxie/issues/235)
- updated Templates.ini (thanks isaak654) [#256](https://github.com/sandboxie-plus/Sandboxie/pull/256) [#258](https://github.com/sandboxie-plus/Sandboxie/pull/258)
- increased max value for disable forced process time in SandMan UI

### Fixed
- fixed BSOD introduced in 5.45.0 when using Windows 10 "core isolation" [#221](https://github.com/sandboxie-plus/Sandboxie/issues/221)
- fixed minor issue with lingering/leader processes
- fixed menu issue in SandMan UI
- fixed issue with stop behaviour page in SandMan UI
- fixed issue with Plus installer not displaying KmdUtil window
- fixed SandMan UI saving UI settings on Windows shutdown
- fixed issue with Plus installer autorun [#247](https://github.com/sandboxie-plus/Sandboxie/issues/247)
- fixed issue with legacy installer not removing all files
- fixed a driver compatibility issue with Windows 20H1 and later [#228](https://github.com/sandboxie-plus/Sandboxie/issues/228)
-- this solves "stop pending", LINE messenger hanging and other issues...
- fixed quick recovery issue in SbieCtrl.exe introduced in 5.45.0 [#224](https://github.com/sandboxie-plus/Sandboxie/issues/224)
- fixed issue advanced hide process settings not saving
- fixed some typos in the UI (thanks isaak654) [#252](https://github.com/sandboxie-plus/Sandboxie/pull/252) [#253](https://github.com/sandboxie-plus/Sandboxie/pull/253) [#254](https://github.com/sandboxie-plus/Sandboxie/pull/254)
- fixed issue with GetRawInputDeviceInfo failing when boxed processes are put in a job object [#176](https://github.com/sandboxie-plus/Sandboxie/issues/176) [#233](https://github.com/sandboxie-plus/Sandboxie/issues/233)
-- this fix resolves issues with CP2077 and other games not getting keyboard input (thanks Rostok)
- fixed failing ClipCursor won't longer span the message log
- fixed issue with adding recovery folders in SandMan UI
- fixed issue with Office 2019 template when using a non-default Sbie install location
- fixed issue setting last access attribute on sandboxed folders [#218](https://github.com/sandboxie-plus/Sandboxie/issues/218)
- fixed issue with process start signal



## [0.5.1 / 5.45.0] - 2020-12-12

### Added
- added simple view mode

### Changed
- updated SandMan UI to use Qt5.15.1

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
-- when a file exceeds the copy limit instead of failing, the user is prompted if the file should be copied or not
-- when internet access is blocked it now can be exempted in real time by the user
- added missing file recovery and auto/quick recovery functionality [#188](https://github.com/sandboxie-plus/Sandboxie/issues/188) [#178](https://github.com/sandboxie-plus/Sandboxie/issues/178)
- added silent MSG_1399 boxed process start notification to keep track of short lived boxed processes
- added ability to prevent system wide process starts, Sandboxie can now instead of just alerting also block processed on the alert list
-- set "StartRunAlertDenied=y" to enable process blocking
- the process start alert/block mechanism can now also handle folders use "AlertFolder=..." 
- added ability to merge snapshots [#151](https://github.com/sandboxie-plus/Sandboxie/issues/151)
- added icons to the sandbox context menu in the new UI
- added more advanced options to the sandbox options window
- added file migration progress indicator
- added more run commands and custom run commands per sandbox
-- the box settings users can now specify programs to be available from the box run menu
-- also processes can be pinned to that list from the presets menu
- added more Windows 10 specific template presets
- added ability to create desktop shortcuts to sandboxed items
- added icons to box option tabs
- added box grouping
- added new debug option "DebugTrace=y" to log debug output to the trace log
- added check for updates to the new SandMan UI
- added check for updates to the legacy SbieCtrl UI

### Changed
- File migration limit can now be disabled by specifying "CopyLimitKb=-1" [#526](https://github.com/sandboxie-plus/Sandboxie/issues/526)
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
- when a program is blocked from starting due to restrictions no redundant messages are issues anymore
- fixed UI not properly displaying async errors
- fixed issues when a snapshot operation failed
- fixed some special cases of IpcPath and WinClass in the new UI
- fixed driver issues with WHQL passing compatibility testing
- fixed issues with Classic installer



## [0.4.5 / 5.44.1] - 2020-11-16

### Added
- added "Terminate all processes" and "disable forced programs" commands to tray menu in SandMan UI
- program start restrictions settings now can be switched between a white list and a black list
-- programs can be terminated and blacklisted from the context menu
- added additional process context menu options, lingering and leader process can be now set from menu
- added option to view template presets for any given box
- added text filter to templates view
- added new compatibility templates:
-- Windows 10 core UI component: OpenIpcPath=\BaseNamedObjects\[CoreUI]-* solving issues with Chinese Input and Emojis [#120](https://github.com/sandboxie-plus/Sandboxie/issues/120) [#88](https://github.com/sandboxie-plus/Sandboxie/issues/88)
-- Firefox Quantum, access to Windows’ FontCachePort for compatibility with Windows 7
- added experimental debug option "OriginalToken=y" which lets sandboxed processes retain their original unrestricted token
-- This option is comparable with "OpenToken=y" and is intended only for testing and debugging, it BREAKS most SECURITY guarantees (!)
- added debug option "NoSandboxieDesktop=y" it disables the desktop proxy mechanism
-- Note: without an unrestricted token with this option applications won't be able to start
- added debug option "NoSysCallHooks=y" it disables the sys call processing by the driver
-- Note: without an unrestricted token with this option applications won't be able to start
- added ability to record verbose access traces to the Resource Monitor
-- use ini options "FileTrace=*", "PipeTrace=*", "KeyTrace=*", "IpcTrace=*", "GuiTrace=*" to record all events
-- replace "*" to log only: "A" - allowed, "D" - denied, or "I" - ignore events
- added ability to record debug output strings to the Resource Monitor
-- use ini option DebugTrace=y to enable

### Changed
- AppUserModelID string no longer contains Sandboxie version string
- now by default Sbie's application manifest hack is disabled, as it causes problems with version checking on Windows 10
-- to enable old behaviour add "PreferExternalManifest=y" to the global or the box specific ini section
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

### removed
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
- added explore box content menu option

### Fixed
- fixed thread handle leak in SbieSvc and other components [#144](https://github.com/sandboxie-plus/Sandboxie/issues/144)
- msedge.exe is now categorized as a Chromium derivate
- fixed Chrome 86+ compatibility bug with Chrome's own sandbox [#149](https://github.com/sandboxie-plus/Sandboxie/issues/149)


 
## [0.4.1 / 5.43.5] - 2020-09-12

### Added
- added core version compatibility check to SandMan UI
- added shell integration options to SbiePlus

### Changed
- SbieCtrl does not longer auto show the tutorial on first start
- when hooking to the trampoline, the migrated section of the original function is no longer noped out
-- it caused issues with unity games, will be investigated and re-enabled later

### Fixed
- fixed colour issue with vertical tabs in dark mode
- fixed wrong path separators when adding new forced folders
- fixed directory listing bug introduced in 5.43
- fixed issues with settings window when not being connected to driver
- fixed issue when starting SandMan UI as admin
- fixed auto content delete not working with SandMan UI



## [0.4.0 / 5.43] - 2020-09-05

### Added
- added a proper custom installer to the Plus release
- added sandbox snapshot functionality to Sbie core
-- filesystem is saved incrementally, the snapshots built upon each other
-- each snapshot gets a full copy of the box registry for now
-- each snapshot can have multiple children snapshots
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
- removes deprecated workaround in the hooking mechanism for an obsolete anti-malware product



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
- fixed ini issue with SandMan.exe when renaming sandboxes
- fixed ini auto reload bug introduced in the last build
- fixed issue when hooking delayed loaded libraries



## [0.3 / 5.42] - 2020-07-04

### Added
- API_QUERY_PROCESS_INFO can be now used to get the original process token of sandboxed processes
-- Note: this capability is used by TaskExplorer to allow inspecting sandbox internal tokens
- added option "KeepTokenIntegrity=y" to make the Sbie token keep its initial integrity level (debug option)
-- Note: Do NOT USE Debug Options if you don't know their security implications (!)
- added process id to log messages very useful for debugging
- added finder to resource log
- added option to hide host processes "HideHostProcess=[name]"
-- Note: Sbie hides by default processes from other boxes, this behaviour can now be controlled with "HideOtherBoxes=n"
- Sandboxed RpcSs and DcomLaunch can now be run as system with the option "ProtectRpcSs=y" however this breaks sandboxed explorer and other
- Built In Clsid whitelist can now be disabled with "OpenDefaultClsid=n"
- Processes can be now terminated with the del key, and require a confirmation
- added sandboxed window border display to SandMan.exe
- added notification for Sbie log messages
- added Sandbox Presets sub menu allowing to quickly change some settings
-- Enable/Disable API logging, logapi_dll's are now distributed with SbiePlus
-- And other: Drop admin rights; Block/Allow internet access; Block/Allow access to files on the network
- added more info to the sandbox status column
- added path column to SbieModel
- added info tooltips in SbieView

### Changed
- reworked ApiLog, added PID and PID filter
- auto config reload on in change is now delayed by 500ms to not reload multiple times on incremental changes
- Sandbox names now replace "_" with " " for display allowing to use names that are made of separated words

### Fixed
- added missing PreferExternalManifest initialization to portable mode
- FIXED SECURITY ISSUE: fixed permission issues with sandboxed system processes
-- Note: you can use "ExposeBoxedSystem=y" for the old behaviour (debug option)
- FIXED SECURITY ISSUE: fixed missing SCM access check for sandboxed services (thanks Diversenok)
-- Note: to disable the access check use "UnrestrictedSCM=y" (debug option)
- fixed missing initialization in service server that caused sandboxed programs to crash when querying service status
- fixed many bugs that caused the SbieDrv.sys to BSOD when running with Driver Verifier enabled [#57](https://github.com/sandboxie-plus/Sandboxie/issues/57)
-- 0xF6 in GetThreadTokenOwnerPid and File_Api_Rename
-- missing non optional parameter for FltGetFileNameInformation in File_PreOperation
-- 0xE3 in Key_StoreValue and Key_PreDataInject



## [0.2.2 / 5.41.2] - 2020-06-19

### Added
- added option SeparateUserFolders=n to no longer have the user profile files stored separately in the sandbox
- added SandboxieLogon=y it makes processes run under the SID of the "Sandboxie" user instead of the Anonymous user
-- Note: the global option AllowSandboxieLogon=y must be enabled, the "Sandboxie" user account must be manually created first and the driver reloaded, else process start will fail
- improved debugging around process creation errors in the driver

### Fixed
- fixed some log messages going lost after driver reload
- found a workable fix for the MSI installer issue, see Proc_CreateProcessInternalW_RS5



## [0.2.1 / 5.41.1] - 2020-06-18

### Added
- added different sandbox icons for different types
-- Red LogAPI/BSA enabled
-- more to come :D
- added progress window for async operations that take time
- added DPI awareness [#56](https://github.com/sandboxie-plus/Sandboxie/issues/56)
- the driver file is now obfuscated to avoid false positives
- additional debug options to Sandboxie.ini OpenToken=y that combines UnrestrictedToken=y and UnfilteredToken=y
-- Note: using these options weakens the sandboxing, they are intended for debugging and may be used for better application virtualization later

### Changed
- SbieDll.dll when processing InjectDll now looks in the SbieHome folder for the DLLs if the entered path starts with a backslash
-- i.e. "InjectDll=\LogAPI\i386\logapi32v.dll" or "InjectDll64=\LogAPI\amd64\logapi64v.dll"

### Fixed
- IniWatcher did not work in portable mode
- service path fix broke other services, now properly fixed, maybe
- found workaround for the MSI installer issue



## [0.2 / 5.41.0] - 2020-06-08

### Added
- IniWatcher, no more clicking reload, the ini is now reloaded automatically every time it changes
- added Maintenance menu to the Sandbox menu, allowing to install/uninstall and start/stop Sandboxie driver, service
- SandMan.exe now is packed with Sbie files and when no Sbie is installed acts as a portable installation
- added option to clean-up logs

### Changed
- Sbie driver now first checks the home path for the Sbie ini before checking SystemRoot

### Fixed
- FIXED SECURITY ISSUE: sandboxed processes could obtain a write handle on non sandboxed processes (thanks Diversenok)
-- this allowed to inject code in non sandboxed processes
- fixed issue boxed services not starting when the path contained a space
- NtQueryInformationProcess now returns the proper sandboxed path for sandboxed processes



## [0.1 / 5.40.2] - 2020-06-01

### Added
- created a new Qt based UI names SandMan (Sandboxie Manager)
- Resource Monitor now shows the PID
- added basic API call log using updated BSA LogApiDll


### Changed
- reworked Resource Monitor to work with multiple event consumers
- reworked log to work with multiple event consumers



## [5.40.1] - 2020-04-10

### Added
- "Other" type for the Resource Access Monitor
-- added call to StartService to the logged Resources

### Fixed
- fixed "Windows Installer Service could not be accessed" that got introduced with Windows 1903

