# Changelog
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).


## [0.4.2 / 5.43.6] - 2020-10-10

### Added
- added explore box content menu option

### Fixed
- fixed thread handle leak in SbieSvc and other components
- msedge.exe is now categorized as a chromium derivate
- fixed Chrome 86+ compatibility bug with Chrome's own sandbox



## [0.4.1 / 5.43.5] - 2020-09-12

### Added
- added core version compatibility check to sandman UI
- added shell integration options to SbiePlus

### Changed
- SbieCtrl does no longer auto-show the tutorial on first start
- when hooking, the trampoline migrated section of the original function is no longer noped out
- it caused issues with unity games, will be investigated and re-enabled later

### Fixed
- fixed color issue with vertical tabs in dark mode
- fixed wrong path separators when adding new forced folders
- fixed directory listing bug introduced in 5.43
- fixed issues with settings window when not being connected to driver
- fixed issue when starting Sandman UI as admin
- fixed auto content delete not working with Sandman UI



## [0.4.0 / 5.43] - 2020-09-05

### Added
- added a proper custom installer to the the Plus release
- added sandbox snapshot functionality to SBIE core
- filesystem is saved incrementally, the snapshots build upon each other
- each snapshot gets a full copy of the box registry for now
- each snapshot can have multiple children snapshots
- added access status to resource monitor
- added setting to change border width
- added snapshot manager UI to SandMan
- added template to enable authentication with an Yubikey or comparable 2FA device
- added UI for program allert
- added software compatibility options to the UI

### Changed
- SandMan UI now handles deletion of sandboxe content on its own
- no longer adding redundant resource accesses as new events

### Fixed
- fixed issues when hooking functions from delay loaded libraries
- fixed issues when hooking an already hooked function 
- fixed issues with the new box settings editor

### Removed
- removes deprecated workaround in the hooking mechanism for an obsolete antimalware product



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
- fixed ini issue with sandman.exe when renaming sandboxes
- fixed ini auto reload bug introduced in the last build
- fixed issue when hooking delayed loaded libraries



## [0.3 / 5.42] - 2020-07-04

### Added
- API_QUERY_PROCESS_INFO can be now used to get the original process token of sandboxed processes
- Note: this capability is used by TaskExplorer to allow inspecting sandbox internal tokens
- Added option "KeepTokenIntegrity=y" to make the sbie token keep its initial integrity level (debug option)
- Note: Do NOT USE Debug Options if you don't know their security implications (!) 
- Added process ID to log messages very usefull for debugging
- Added finder to resource log
- Added option to hide host processes "HideHostProcess=[name]"
- Note: Sbie hides by default processes from other boxes, this behaviour can now be controlled with "HideOtherBoxes=n"
- Sandboxed RpcSs and DcomLaunch can now be run as system with the option "ProtectRpcSs=y" howeever tht breaks sandboxed explorer and other
- BuiltIn Clsid whitelist can now be disabled with "OpenDefaultClsid=n"
- Processes can be now terminated with the del key, and require a confirmation
- Added sandboxed window border display to SandMan.exe
- Added notification for sbie log messages
- Added Sandbox Presets sub menu allowing to quickly change some settings
- Enable/Disable API logging, logapi_dll's are now distributed with SbiePlus 
- And other: Drop admin rights; Block/Allow internet access; Block/Allow access to files on te network
- Added more info to the sandbox status column
- Added path column to SbieModel
- Added info tooltips in SbieView

### Changed
- Reworked ApiLog, added pid and pid filter
- Auto config reload on in change is now delayed by 500ms to not reload multiple times on incremental changes
- Sandbox names now replace "_" witn " " for display allowing to use names that are build of separated words

### Fixed
- added mising PreferExternalManifest itialization to portable mode
- fixed permission issues with sandboxed system processes
- Note: you can use "ExposeBoxedSystem=y" for the old behaviour (debug option)
- fixed missing SCM access check for sandboxed services
- Note: to disable the access check use "UnrestrictedSCM=y" (debug option)
- fixed missing initialization in serviceserver that caused sandboxed programs to crash when querying service status
- fixed many bugs that caused the SbieDrv.sys to BSOD when run with MSFT Driver Verifier active
- 0xF6 in GetThreadTokenOwnerPid and File_Api_Rename
- missing non-optional parameter for FltGetFileNameInformation in File_PreOperation
- 0xE3 in Key_StoreValue and Key_PreDataInject



## [0.2.2 / 5.41.2] - 2020-06-19

### Added
- added option SeparateUserFolders=n to no longer have the user profile files stored separately in the sandbox
- added SandboxieLogon=y it makes processes run under the SID of the "Sandboxie" user instead of the Anonymous user
- Note: the global option AllowSandboxieLogon=y must be enabled, the "Sandboxie" user account must be manually created first and the driver reloaded, else process start will fail
- improved debugging around process creation errors in the driver

### Fixed
- fixed some log messages going lost after driver reload
- found a workable fix for the MSI installer issue, see Proc_CreateProcessInternalW_RS5



## [0.2.1 / 5.41.1] - 2020-06-18

### Added
- added different sandbox icons for different types
- Red LogAPI/BSA enabled
- More to come :D
- Added progress window for async operations that take time
- added DPI awareness
- the driver file is now obfuscated to avoid false positives
- additional debug options to sandboxie.ini OpenToken=y that combines UnrestrictedToken=y and UnfilteredToken=y
- Note: using these options weakens the sandboxing, they are intended for debugging and may be used for better application virtualization later

### Changed
- SbieDll.dll when processing InjectDll now looks in the SbieHome folder for the Dll's if the entered path starts with a backslash
- i.e. "InjectDll=\LogAPI\i386\logapi32v.dll" or "InjectDll64=\LogAPI\amd64\logapi64v.dll"

### Fixed
- IniWatcher did not work in portable mode
- service path fix broke other services, now properly fixed, maybe
- found workaround for the msi installer issue



## [0.2 / 5.41.0] - 2020-06-08

### Added
- IniWatcher, no more clicking reload, the ini is now reloaded automatically every time it changes
- Added Mainanance menu to the Sandbox menu, allowing to install/uninstall and start/stop sandboxie driver, service
- SandMan.exe now is packed with Sbie files and when no sbie is installed acts as a portable installation
- Added option to clean up logs

### Changed
- sbie driver now first checks the home path for the sbie ini before checking SystemRoot

### Fixed
- Fixed a resource leak when running sandboxed
- Fixed issue boxed services not starting when the path contained a space
- NtQueryInformationProcess now returns the proper sandboxed path for sandboxed processes



## [0.1 / 5.40.2] - 2020-06-01

### Added
- Created a new Qt based UI names SandMan (Sandboxie Manager)
- Resource monitor now shows the PID
- Added basic API call log using updated BSA LogApiDll


### Changed
- reworked resource monitor to work with multiple event consumers
- reworked log to work with multiple event consumers



## [5.40.1] - 2020-04-10

### Added
- "Other" type for the Resource Access Monitor
- added call to StartService to the logged Resources

### Fixed
- fixed "Windows Installer Service could not be accessed" that got introduced with Windows 1903

