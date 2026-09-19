# Identity and privacy coverage inventory

"Existing" names a code path in the repository; it is not a claim that every
related API is covered or runtime-tested. "Not in this slice" means the identity
profile feature does not implement the category; it does not assert that no
compatibility code exists elsewhere. No category counts as complete from a
declaration, a checkbox or a passing unit test alone.

The current slice is the persistent volume serial profile described in
[IdentityProfiles.md](IdentityProfiles.md). It changes SandMan only; SbieDll,
SbieSvc and the driver are unchanged.

| # / category | Existing evidence and limits | This slice | Platforms and dependencies | Acceptance tests still needed |
| --- | --- | --- | --- | --- |
| 1. Generation, persistence and authority | `QSbieAPI/Sandboxie/SbieIni.cpp` writes box sections through SbieSvc; `sbieiniserver.cpp::RefreshConf` is backup/overwrite/reload, not a transaction | **New:** versioned JSON profile in the SandMan data dir, `QSaveFile` atomic save with read-back, explicit regenerate, clone and import with new ids; binding recorded as `IdentityProfile` + `IdentityProfileRevision` | SandMan Qt/C++, existing INI authorization | Service-side atomic publication, concurrent launcher lock, startup rejection of an invalid reference |
| 2. Machine and derived identifiers | `core/dll/custom.c::Custom_ProductID` (`RandomRegUID`) writes shared boxed registry values | Not in this slice | Windows registry, SbieDll/SbieSvc | Typed base/derived ids; A/W/native and registry-view agreement |
| 3. Computer name, user and directories | File/registry virtualization, `File_CreateBaseFolders` | Not in this slice | Windows name APIs, environment, virtual filesystem | Host name must stay resolvable; Unicode, children, buffer negotiation |
| 4. CPU identity/topology | `dllmain.c` reads `CpuAffinityMask` (resource policy, not identity) | Not in this slice, real CPU preserved | Architecture-specific CPU APIs | Logical/physical counts, groups, masks, native agreement |
| 5. Direct CPUID and hypervisor information | No instruction-level mechanism in the reviewed DLL paths | Not in this slice, real information preserved | x86/x64 vs ARM64/ARM64EC semantics | Feasibility analysis before any synthetic mode |
| 6. BIOS/firmware/system/board/chassis | `sysinfo.c` `HideFirmwareInfo`, `Template_BlockAccessWMI` | Not in this slice | SMBIOS/ACPI layouts, WMI, certificate policy | Independent SMBIOS/ACPI/registry/WMI mapping |
| 7. Registry interfaces | `key*.c` virtualization; `Custom_ProductID` shared values | Reused, no new registry writes | ANSI/Unicode wrappers, WOW64 views | Type/error/size/enumeration semantics, covered vs excluded processes |
| 8. Disks, volumes and capacity | `kernel.c::Kernel_GetVolumeInformationByHandleW` reads `DiskSerialNumber` when `HideDiskSerialNumber=y`; the cache-key fix on `fix/volume-serial-cache-20260906` is a separate change | **New, partial:** profile values become the `DiskSerialNumber` entries of the box; by-handle query only, volume serial not physical serial, capacity untouched | Existing hook, `HarddiskVolumeN` names | Runtime query in two boxes and on the host, `GetVolumeInformationW/A`, native and WMI paths compared independently |
| 9. Memory | Service job limits only | Not in this slice, real information preserved | Windows memory APIs | Totals and bounds across APIs |
| 10. MAC and adapters | `custom.c::Nsi_Init` (`HideNetworkAdapterMAC`/`NetworkAdapterMAC`), `net.c` policies | Not in this slice | NSI, adapter enumeration | Stable adapter authority, hot-plug, loopback |
| 11. Uptime, boot and clocks | `kernel.c` `UseChangeSpeed` timer adjustments | Not in this slice | Monotonic/wall clock semantics | Boot-session boundaries, resume/restart |
| 12. OS and language | `userenv.c` `OverrideOsBuild`, `kernel.c` `CustomLCID` | Not in this slice | Build/ABI and locale formats | System/app locale distinction |
| 13. GPU and graphics APIs | No generic GPU identity layer | Not in this slice, real information preserved | OpenGL, DXGI, D3D, Vulkan | Per-API getters and real rendering |
| 14. Window/presentation/audio preferences | Application behavior, not hardware identity | Not applicable | Application contracts | Not a sandbox identity surface |
| 15. Environment, credentials and startup | Configuration and process-launch infrastructure | **Profile authority ignores the environment:** only public synthetic fields are stored; no seed or secret material | SbieSvc authorization, process environment | Children, diagnostics, malformed input |
| 16. Process/module/resource visibility | `sysinfo.c` `HideOtherBoxes`, `HideSbieProcesses`, `HideHostProcess` | Reused as is | Native enumeration, existing boundaries | Other-box/host views |
| 17. Linux identity files and environment preparation | Linux `/proc`, sysfs, DMI, cgroups are Linux-specific | Not applicable; each requirement maps to a reviewed Windows interface | Windows APIs/registry/firmware | Native equivalents, not Linux behavior |
| 18. Files and handles | `File_GetName` and handle virtualization underpin the volume query | Reused; no synthetic filesystem | Name resolution, duplicated handles | Aliases, oversized names, handle reuse |
| 19. Initialization, children and reapplication | `Kernel_Init`, `dllmain.c` loader infrastructure | **Reuse:** no injector or hook library; new editor registered in `SandMan.vcxproj`; tests compile the shipped sources rather than a rewritten model | Architecture-specific SbieDll loading | Early queries, children, repeated initialization |
| 20. Capabilities and diagnostics | SandMan error reporting for configuration operations | **New, partial:** binding state (unbound/manual/applied/stale/diverged/missing), coverage text, validation errors; no runtime active/real indicator, no fail-closed launch | Qt UI, service errors | Missing/tampered profiles, partial initialization, diagnostics on failure |
| 21. Wine/compatibility environments | No Wine-specific identity implementation | Not applicable | Wine vs native Windows discovery | Separate native and compatibility results |
| 22. Application-specific behavior | Internal getters, telemetry filtering, binary patches are not generic sandbox identity interfaces | Explicitly excluded | Specific application contracts | Only owned/documented test applications |

## Follow-up sequence

1. Runtime evidence for category 8: run a read-only probe inside two bound
   sandboxes and on the host, compare the by-handle, `GetVolumeInformationW/A`,
   native and WMI paths independently, repeat after restarts.
2. Service-side atomic configuration publication and a launch/configuration
   lock, so a binding cannot race an external launcher.
3. Typed machine/firmware identity that integrates the existing registry and
   firmware options instead of duplicating them.
4. Per-process capability reporting and an explicitly opt-in fail-closed
   launch policy whose failure keeps SandMan and diagnostics usable.

Each stage needs separate source review, compilation and runtime evidence for
the relevant Win32, x64, ARM64 and ARM64EC targets. No anonymity claim follows
from a passing unit test or a successful build.
