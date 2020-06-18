# Changelog
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).


## [0.2.1 / 5.41.1] - 2020-06-18

### Added
- added different sandbox icons for different types
-- Red LogAPI/BSA enabled
-- More to come :D
- Added progress window for async operations that take time
- added DPI awareness
- the driver file is now obfuscated to avoid false positives
- additional debug options to sandboxie.ini OpenToken=y that combines UnrestrictedToken=y and UnfilteredToken=y
-- Note: using these options weekens the sandboxing, they are intended for debugging and may be used for better application virtualization later

### Changed
- SbieDll.dll when processinh InjectDll now looks in the SbieHome folder for the Dll's if the entered path starts with a backslash
-- i.e. "InjectDll=\LogAPI\i386\logapi32v.dll" or "InjectDll64=\LogAPI\amd64\logapi64v.dll"

### Fixed
- IniWatcher did not work in portable mode
- service path fix broke other services, now properly fixed, may be
- found workaround for the msi installer issue



## [0.2 / 5.41.0] - 2020-06-08

### Added
- IniWatcher, no more clicking reload, the ini is now reloaded automatically every time it changes
- Added Mainanance menu to the Sandbox menu, allowing to install/uninstall and start/stop sandboxie driver, service
- SandMan.exe now is packed with Sbie files and when no sbie is installed acts as a portable instalation
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
-- added call to StartService to the logged Resources

### Fixed
- fixed "Windows Installer Service could not be accessed" that got introduced with Windows 1903

