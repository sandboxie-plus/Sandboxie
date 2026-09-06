# Native proxy profiles and tunnel review prototype

**Status: experimental review prototype, not a completed feature.**
The normal build exposes a native Qt profile panel, but deliberately refuses tunnel
activation and sandbox reassignment. Do not present this code as leak protection.

Baseline reviewed: `00ae8850d681962cf1864055c551faaf3ba77770` in
`Kizuno18/sbxie` / `sandboxie-plus/Sandboxie`, September 6, 2026.
The root `AGENTS.md`, contribution/build instructions, relevant SandMan/QSbieAPI,
network DLL hooks, DNS hooks, templates, dependency/add-on and build code were read.
The reference Go implementation was inspected, not copied or executed.

## Components and intended data path

```text
SandMan (Qt widgets)
  -> CProxyManager (profiles, independent runtime state, batches, ownership lock)
  -> CProxyIntegration (QSbieAPI / permissions / certificates / add-ons)
  -> CProxyTunnel worker (Windows laboratory backend, disabled by default)

Box A ---- BindAdapter=SbieProxy-<UUID-A> ---- virtual adapter A ---- proxy A
Box B --+-- BindAdapter=SbieProxy-<UUID-B> ---- virtual adapter B ---- proxy B
Box C --+
```

`ProxyProfile` validates imports and stores name, protocol, server, port, UUID and a
current-user DPAPI-protected authentication blob. Usernames and passwords are both
inside that blob. Endpoint equality never deduplicates profiles: even identical
server/port/authentication inputs receive independent UUIDs. The adapter alias is
stable across profile edits; deleting an associated profile is rejected.

`ProxyProfiles.json` is in the existing SandMan configuration directory, not a new
registry/INI backend. `QSaveFile` replaces the whole profile set atomically, without
direct-write fallback. Invalid/corrupt files and unavailable locks make the store
read-only; the original file is retained. A `QLockFile` prevents two controllers
using the same profile store concurrently. No PID, live adapter observation, exit
IP or running status is persisted. SandMan restart starts all profiles as stopped.
It does not adopt processes or adapters from persisted names/PIDs.

The window provides add/edit/remove, a validated batch import with line-specific
errors, selected/all start/stop commands, an adapter-bound exit check command and
both directions of the profile/sandbox mapping. The controller is testable with a
worker factory and explicit integration callbacks. Production callbacks always use
SandMan, never a localhost HTTP service. Closing the panel does not close the
controller. SandMan shutdown cancels and joins its workers.

Assignments use existing `BindAdapter` values as the authority, not a second
independently persisted association map. Multiple boxes may reference one UUID.
Existing process-specific, inherited, built-in proxy or direct-IP bindings are not
silently overwritten. Changing a box requires it to have no running processes and
uses the existing authenticated configuration API. Every write status is checked;
partial writes leave restrictive rules and display an error, rather than rolling
back to unprotected direct access. This is not a transaction against concurrent
launchers or external INI edits; those races require Windows qualification.

## Scope and release gate

No driver change, new privileged service, local HTTP server, HWID spoofing, process
whitelisting or IP-reputation feature is included. Two small companion changes are
present: retain an unavailable NIC in the existing network-options combo, and
propagate `bind()` errors before marking the socket bound. The latter touches the
user-mode hook DLL and still requires its Windows regression/build tests.

`SBIE_PROXY_TUNNELS_LAB` is an intentionally absent compile-time definition. It is not
an INI setting or a UI preference. A normal build cannot launch a tunnel or change
box associations through this panel. The source behind that laboratory gate is a
proposal for review on an isolated Windows test machine, not a supported opt-in
feature. Do not ship with the gate enabled until the blockers below are resolved.

### Blockers before activation can be enabled

1. **Host routing invariant.** The proposed backend adds an on-link `0.0.0.0/0`
   route on the TUN interface, with a high metric and a `/32` SkipAsSource address.
   This is still a route in the global Windows table. Checking that the selected
   route to `1.1.1.1` remains unchanged does not prove every host destination,
   protocol, compartment or uplink-failure path remains unaffected. It must not be
   described as satisfying a strict “never change host output” requirement. Prove
   an adequate source/interface-specific design, or redesign this part before
   enabling it. The prototype never rewrites the host's routes to “fix” selection.
2. **Adapter ownership.** The official child creates Wintun. Observing a new unique
   alias, recording its LUID/GUID and rejecting a pre-existing alias does not prove
   ownership in a concurrent creation race. A stronger ownership handshake/native
   helper contract is needed before configuring that adapter. Cleanup never calls
   `Remove-NetAdapter`, removes by name prefix, or deletes the Wintun driver.
3. **Named-pipe configuration.** The official helper reads a path with Go
   `os.ReadFile`. The proposed current-user-only, PID-checked Windows named pipe
   keeps credentials out of disk files and the command line, but actual Go/Qt
   pipe open, EOF, cancellation and peer validation behavior has not been tested.
   Do not fall back to a plaintext temporary YAML file or command-line secrets.
4. **DNS and non-hooked traffic.** The restrictive template is not a complete
   transparent DNS implementation. See the coverage matrix. License/WFP changes,
   cached socket state, raw APIs and brokered services require explicit tests.
5. **Distribution and compatibility.** No official `ProxyTunnels` add-on entry has
   been published by this change. MSVC/Qt6, Win32/x64/ARM64 and actual helper OS
   requirements remain unqualified. Do not drop the main application's Windows 7
   compatibility just to support a dependency built with a newer Go toolchain.

## Coverage matrix

The statuses describe design intent behind the laboratory gate, not proven
end-to-end guarantees. In a normal build the backend is disabled for all traffic.

| Traffic/path | Laboratory design | Actual validation |
|---|---|---|
| IPv4 TCP via intercepted Winsock | Strict BindAdapter to an IPv4 TUN, then SOCKS5 or HTTP CONNECT | Code inspected; no Windows packets tested |
| Shared profile across boxes | One runtime UUID/adapter/worker; many BindAdapter references | Controller mapping/lifecycle tested using fake workers |
| Public exit-IP check | No system proxy; bind TUN source plus IP_UNICAST_IF; numeric HTTPS endpoint; verify TLS | Source implemented; actual route and TLS test not executed |
| Ordinary DNS / port 53 | Wildcard NetworkDnsFilter, DNSResolver IPC closure, TCP/UDP port-53 block | Template/source analysis only; not universal DNS interception |
| App-owned DoH/DoT over TCP | May traverse the same tunnel with a suitable bootstrap configuration | Not validated; application-specific setup still required |
| UDP, including QUIC and normal UDP DNS | Intended block through existing WFP NetworkAccess rules, even for SOCKS5 | Real filtering and IPv6 variants not tested |
| IPv6 application traffic | No usable IPv6 TUN address; StrictBindIP should reject covered Winsock paths | Not a kernel-wide IPv6 guarantee; untested |
| IPv6-only proxy ingress | Profiles parse and persist; laboratory backend requires an IPv4-reachable endpoint | Not supported for execution |
| ICMP / raw sockets / non-Winsock APIs | ICMP block proposed; no full raw/bypass interception claim | Unvalidated; release blocker |
| Brokered/out-of-box services | Not automatically attributable to or routed with the box | Outside the demonstrated coverage |
| Host DNS for proxy ingress hostname | Explicit host-side bootstrap lookup for the proxy server itself | Intentional; distinct from sandbox application DNS |

`BindAdapter` is a user-mode hook mechanism, with positive adapter-validity caching
and existing socket-state behavior. A passed IP check is only a bounded IPv4 TCP
sample at a timestamp. It proves neither DNS/IPv6 coverage nor an enforcement
boundary against a hostile sandbox process, external privileged actor, policy edit
or certificate expiry. Periodic health polling is not a kernel kill switch.

## Runtime failure handling in the proposal

Only handles returned by the manager's own suspended `CreateProcessW` call may be
terminated. The child is assigned to a kill-on-job-close job before being resumed;
only an explicit NUL handle is inherited. No process enumeration, process-name kill,
PID-file adoption or port eviction exists. Child output is discarded because it
could include the proxy URL. UI/log errors are stage names and numeric Windows
codes, never command lines, input lines, usernames or passwords.

The laboratory worker checks child liveness, adapter/address state, the sampled
host route, and a bounded HTTPS probe; probe failure stops the owned runtime.
Configuration delivery/startup are bounded, and cancellation is cooperative.
SandMan does not clear `BindAdapter` or the guard on stop, timeout or child failure.
Tunnels are never silently restarted as direct host connections.

Normal cleanup tracks exactly the address/route it created, with LUID/GUID checks,
not every object on an interface. The helper owns the adapter lifetime. Abrupt
process termination and driver cleanup behavior have not been qualified. A stale
alias is refused instead of adopted/deleted; automatic stale-adapter recovery is
therefore **not complete**. Host interfaces changing underneath a tunnel are also
unqualified. These are reasons to retain the release gate, not just UI warnings.

## Credentials

DPAPI is current-user scoped with the profile UUID as entropy. There is no
plaintext fallback on Linux, decryption failure or a different Windows identity.
Copying the JSON to another user/machine does not provide a working password store;
the edit dialog permits explicit credential replacement. DPAPI does not protect
against a process already running as the same user, an administrator, process
memory inspection, Qt implicit-sharing copies, paging or crash dumps. Best-effort
buffer wiping is not a claim of complete memory erasure. SOCKS5 and HTTP proxy
credentials are not inherently encrypted on the network; use trusted transport.

## Build and tests

The existing qmake and Visual Studio projects include the new sources and Crypt32.
Main-app dependency versions are unchanged. These project edits and the native
QSbieAPI bridge were not compiled with MSVC in this environment.

For the isolated profile/controller/widget tests, with a supported Qt SDK:

```sh
cmake -S SandboxiePlus/SandMan/Tests/ProxyTunnels -B build/proxy-tests
cmake --build build/proxy-tests --config Release
ctest --test-dir build/proxy-tests -C Release --output-on-failure
```

The CMake recipe is supplied for reproduction. Here the same sources were compiled
with GCC, the downloaded Debian Qt 5.15.15 development headers/tools and installed
Qt runtime, using `-Wall -Wextra -Werror`. The Qt widget smoke test ran offscreen.
Test evidence is in the delivery package. See `VALIDATION.md` for what was and was
not executed. The test factory is in-process; it does not pretend to exercise
Wintun or a real proxy. CI dependency/tool acquisition success is not a SandMan
build result.
