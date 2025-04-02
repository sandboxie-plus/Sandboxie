# Security Policy

## Reporting a vulnerability

Please report any found security vulnerability directly to me at xanatosdavid[at]gmail.com or through the [Report a vulnerability](https://github.com/sandboxie-plus/Sandboxie/security/advisories/new) form provided by GitHub. Please describe the issue in full detail and, if possible, include a proof of concept exploit.

## Fixed security issues

### SECURITY ISSUE ID-25 (thanks hg421)
A new method of exploiting HostInjectDll mechanism for local privilege escalation

fixed in: 1.15.9 / 5.70.9

### SECURITY ISSUE ID-24
Files stored inside a sandbox folder were accessible to all users on a system, resulting in security issues in multi-user scenarios; see [CVE-2024-49360](https://github.com/sandboxie-plus/Sandboxie/security/advisories/GHSA-4chj-3c28-gvmp)

fixed in: 1.15.0 / 5.70.0

### SECURITY ISSUE ID-23 (thanks Diversenok)
A sandboxed process with administrative privileges could enable SeManageVolumePrivilege, this allowed it to read MFT data, in case of files smaller than 1 cluster that allowed to read the file payload

fixed in: 1.12.3 / 5.67.3

### SECURITY ISSUE ID-22
NtCreateSectionEx was not filtered by the driver

fixed in: 1.8.0 / 5.63.0

### SECURITY ISSUE ID-21
AlpcConnectPortEx was not filtered by the driver

fixed in: 1.5.1 / 5.60.1

### SECURITY ISSUE ID-20
Sandboxed programs could read the memory of host processes, presumably this was an intentional design decision by the old developers, but it's not required and it's better for privacy not to allow this. Note: You can use ReadIpcPath=$:program.exe to allow read access to unsandboxed processes or processes in other boxes

fixed in: 1.0.16 / 5.55.16

### SECURITY ISSUE ID-19 [#1714](https://github.com/sandboxie-plus/Sandboxie/issues/1714)
NtGetNextThread was not properly filtered by the Sandboxie driver, hence a sandboxed process could obtain a handle on an unsandboxed thread with write privileges. The issue can be remedied on older Sandboxie versions by enabling EnableObjectFiltering=y

fixed in: 1.0.14 / 5.55.14

### SECURITY ISSUE ID-18 (thanks Diversenok)
NtCreateSymbolicLinkObject was not filtered

fixed in: 1.0.15 / 5.55.15

### SECURITY ISSUE ID-17 (thanks Diversenok)
Hard link creation was not properly filtered

fixed in: 1.0.13 / 5.55.13

### SECURITY ISSUE ID-16
When starting *COMSRV* unboxed, the returned process handle had full access

fixed in: 1.0.9 / 5.55.9

### SECURITY ISSUE ID-15 (thanks hg421)
The HostInjectDll mechanism allowed for local privilege escalation

fixed in: 0.7.2 / 5.49.0

### SECURITY ISSUE ID-14 (thanks hg421) [#552](https://github.com/sandboxie-plus/Sandboxie/issues/552)
"\Device\DeviceApi\CMApi" is now filtered by the driver, this allowed elevated processes to change hardware configuration

fixed in: 0.7.0 / 5.48.0

### SECURITY ISSUE ID-13 (thanks hg421) [#553](https://github.com/sandboxie-plus/Sandboxie/issues/553)
"\RPC Control\samss lpc" is now filtered by the driver, this allowed elevated processes to change passwords, delete users and alike

fixed in: 0.7.0 / 5.48.0

### SECURITY ISSUE ID-12 (thanks typpos) [#549](https://github.com/sandboxie-plus/Sandboxie/pull/549)
A race condition in the driver allowed to obtain an elevated rights handle to an unsandboxed process

fixed in: 0.7.0 / 5.48.0

### SECURITY ISSUE ID-11 (thanks hg421)
Elevated sandboxed processes could access volumes/disks for reading

fixed in: 0.7.0 / 5.48.0

### SECURITY ISSUE ID-10
The registry isolation could be bypassed, present since Windows 10 Creators Update

fixed in: 0.5.4d / 5.46.3

### SECURITY ISSUE ID-9
A sandboxed process could start sandboxed as system, even with DropAdminRights in place

fixed in: 0.5.4b / 5.46.1

### SECURITY ISSUE ID-8 (thanks Diversenok)
CVE-2019-13502 "\RPC Control\LSARPC_ENDPOINT" is now filtered by the driver, this allowed some system options to be changed

fixed in: 0.5.4 / 5.46.0

### SECURITY ISSUE ID-7
A bug in the dynamic IPC port handling allowed to bypass IPC isolation

fixed in: 0.5.4 / 5.46.0

### SECURITY ISSUE ID-6 (thanks Diversenok)
Processes could spawn processes outside the sandbox

fixed in: 0.5.4 / 5.46.0

### SECURITY ISSUE ID-5
Added print spooler filter to prevent printers from being set up outside the sandbox

fixed in: 0.5.4 / 5.46.0

### SECURITY ISSUE ID-4 (thanks Diversenok)
Sandboxie now strips particularly problematic privileges from sandboxed system tokens. With those, a process could attempt to bypass the sandbox isolation

fixed in: 0.5.4 / 5.46.0

### SECURITY ISSUE ID-3 (thanks Diversenok)
Fixed missing SCM access check for sandboxed services

fixed in: 0.3 / 5.42

### SECURITY ISSUE ID-2
Fixed permission issues with sandboxed system processes

fixed in: 0.3 / 5.42

### SECURITY ISSUE ID-1 (thanks Diversenok)
Sandboxed processes could obtain a write handle on non-sandboxed processes

fixed in: 0.2 / 5.41.0
