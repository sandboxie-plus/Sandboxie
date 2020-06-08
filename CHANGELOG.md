# Changelog
All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](http://semver.org/).


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

