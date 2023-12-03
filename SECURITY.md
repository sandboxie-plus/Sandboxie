# Security Policy

## Reporting a vulnerability

Please report any found security vulnerability directly to me at xanatosdavid[at]gmail.com or through the [Report a vulnerability](https://github.com/sandboxie-plus/Sandboxie/security/advisories/new) form provided by GitHub. Please describe the issue in full detail and, if possible, include a proof of concept exploit.


## Fixed security issues

### SECURITY ISSUE ID-23 (thanks Diversenok)
A sandboxed process with administrative privileges could enable SeManageVolumePrivilege,
this allowed it to read MFT data, in case of files smaller then 1 cluster that allowed to read the file payload

### SECURITY ISSUE ID-22 
NtCreateSectionEx was not filtered by the driver

### SECURITY ISSUE ID-21 
AlpcConnectPortEx was not filtered by the driver

### SECURITY ISSUE ID-20
Sandboxed programs could read the memory of host processes,
presumably this was an intentional design decision by the old devs, but its not required and its better fpr privacy to not allow this.
Note: You can use ReadIpcPath=$:program.exe to allow read access to unsandboxed processes or processes in other boxes
  
### SECURITY ISSUE ID-19 [#1714](https://github.com/sandboxie-plus/Sandboxie/issues/1714)
NtGetNextThread was not properly filtered by the sbie driver, hence a sandboxed process could obtain a handle on an unsandboxed thread with write privileges 
The issue can be remedied on older sbie versions by enabling EnableObjectFiltering=y

### SECURITY ISSUE ID-18 (thanks Diversenok)
NtCreateSymbolicLinkObject was not filtered

### SECURITY ISSUE ID-17 (thanks Diversenok)
Hard link creation was not properly filtered

### SECURITY ISSUE ID-16
when starting *COMSRV* unboxed, the returned process handle had full access

### SECURITY ISSUE ID-15 (thanks hg421)
the HostInjectDll mechanism allowed for local privilege escalation

### SECURITY ISSUE ID-14 (thanks hg421) [#552](https://github.com/sandboxie-plus/Sandboxie/issues/552)
"\Device\DeviceApi\CMApi" is now filtered by the driver 
this allowed elevated processes to change hardware configuration

### SECURITY ISSUE ID-13 (thanks hg421) [#553](https://github.com/sandboxie-plus/Sandboxie/issues/553)
"\RPC Control\samss lpc" is now filtered by the driver
this allowed elevated processes to change passwords, delete users and alike

### SECURITY ISSUE ID-12 (thanks typpos) [#549](https://github.com/sandboxie-plus/Sandboxie/pull/549)
a race condition in the driver allowed to obtain an elevated rights handle to a unsandboxed process

### SECURITY ISSUE ID-11 (thanks hg421)
elevated sandboxed processes could access volumes/disks for reading

### SECURITY ISSUE ID-10
the registry isolation could be bypassed, present since Windows 10 Creators Update

### SECURITY ISSUE ID-9
a Sandboxed process could start sandboxed as system even with DropAdminRights in place

### SECURITY ISSUE ID-8 (thanks Diversenok)
CVE-2019-13502 "\RPC Control\LSARPC_ENDPOINT" is now filtered by the driver,
this allowed some system options to be changed.

### SECURITY ISSUE ID-7
bug in the dynamic IPC port handling allowed to bypass IPC isolation

### SECURITY ISSUE ID-6 (thanks Diversenok)
processes could spawn processes outside the sandbox

### SECURITY ISSUE ID-5
added print spooler filter to prevent printers from being set up outside the sandbox

### SECURITY ISSUE ID-4 (thanks Diversenok)
Sandboxie now strips particularly problematic privileges from sandboxed system tokens
with those a process could attempt to bypass the sandbox isolation 

### SECURITY ISSUE ID-3 (thanks Diversenok)
fixed missing SCM access check for sandboxed services

### SECURITY ISSUE ID-2
fixed permission issues with sandboxed system processes

### SECURITY ISSUE ID-1 (thanks Diversenok)
sandboxed processes could obtain a write handle on non sandboxed processes
