# Validation record — September 6, 2026

## Packaged Qt result, not the current branch commit

Environment: Debian 13, x86-64, GCC 14.2.0, Qt/QtTest 5.15.15. Qt development tools
were acquired as official Debian packages using the fork's read-only Actions
preparation workflow; existing runtime libraries were used locally.

Compiled the actual profile, controller, default-gated tunnel and widget sources
with `-std=c++17 -fPIC -Wall -Wextra -Werror`, plus Qt meta-object output. Ran the
Qt test executable with the offscreen widget platform.

The source-review package applied the patch to
`00ae8850d681962cf1864055c551faaf3ba77770` before this branch added direct Qt
includes and recovery/UI updates. Its result was **44 passed, 0 failed, 1
skipped.** Qt's total includes setup and cleanup, data-driven parser rows and
controller/widget cases; it is not a count
of 44 independent Windows network experiments.

Covered by executed tests:

- SOCKS5/HTTP and legacy import parsing, bracketed IPv6, IDN, URL encoding, invalid
  ports/hosts/protocols, wildcard/multicast/broadcast addresses, credential/control
  limits, line-specific errors, partial import and the 1000-profile cap.
- Different sessions/endpoints are not deduplicated by host. Separately created
  equal endpoints also receive distinct persistent UUIDs and adapter names.
- Atomic profile round trips, rejected duplicate IDs/invalid JSON fields, failed
  writes, corruption preservation, exclusive store lock and absence of persisted
  running/PID/password fields.
- Refusal of authenticated storage on Linux, without a plaintext fallback.
- One fake worker shared by two sandbox associations, duplicate-start rejection,
  edits/deletes rejected while in use, simulated worker failure, immediate stop,
  retry after failure, policy revocation, batch stop and joined shutdown.
- Manager reconstruction retains profiles and external association fixtures but
  does not restore running state or adopt a worker.
- Default real worker implementation refuses activation; no exit result is emitted.
- Native Qt window construction and its profile-to-two-box display.

The single skipped case is the **Windows DPAPI round trip/tamper test**. Its Linux
refusal assertion ran before the skip. Do not record DPAPI encryption/decryption
as executed successfully on Windows.

Static checks executed: `git diff --check`; XML parsing of both Visual Studio
project files; resolution of all ten new source/header entries in each; comparison
of all six compiled dependency file-hash pins with the actual downloaded official
artifacts. The patch was also checked for application to its clean baseline.

The standalone CMake target does not compile `ProxyIntegration.cpp`, and it does
not define `SBIE_PROXY_TUNNELS_LAB`; a successful run therefore does not compile
the SandMan integration or the laboratory backend. Those paths require the full
SandMan build and a separately enabled laboratory build.

## Executed using Windows Actions

The dependency inspection job downloaded official tun2socks v2.7.0 artifacts and
Wintun 0.14.1. It checked archive SHA-256 values, computed extracted hashes and
validated Wintun Authenticode signatures. See `DEPENDENCIES.md` and the included
inspection JSON for provenance. **This did not compile the feature or run a tunnel.**

Source/toolchain archival jobs also completed. A green preparation workflow is not
a successful Sandboxie build or successful packet-flow test.

## Not executed / required before removing the release gate

| Required experiment | Acceptance criteria | Current result |
|---|---|---|
| Full SandMan Qt6/MSVC build | Clean qmake/Jom and VS builds, x64/Win32/ARM64 | Not executed |
| Core hook DLL build/regression | Win32/x64/ARM64/ARM64EC as applicable; sandbox start/process/file recovery unaffected | Not executed |
| Real DPAPI lifecycle | Same-user round trip; tamper, other-user/computer refusal; no plaintext on disk/logs/CLI | Not executed on Windows |
| Dependency failure | Missing/wrong-arch/replaced binary and bad hash refuse launch before side effects | Source reviewed only |
| Private configuration pipe | Correct Go/Qt EOF, genuine peer PID, competing peer denied, cancellation and timeouts | Not executed |
| Concurrent adapter creation | No unowned adapter may be adopted, configured, or removed | Ownership proof remains incomplete |
| Two proxies + shared box pair | A exits via A; B/C share B; stopping A never alters B/C or unrelated processes | Fake-controller portion only; no packets |
| Proxy refusal/auth failure/drop | Useful sanitized errors; no direct-host traffic in full packet capture | Not executed |
| Owned child killed externally | Preserved BindAdapter, other tunnels unaffected, no unowned-process kill | Fake-controller portion only |
| Adapter removal/replacement | Stop cleanly without touching the replacement/unrelated adapters | Source reviewed only |
| Manager exit/crash/restart | Job cleanup, no orphan adoption, persistent association and no direct fallback | Orderly fake-controller portion only |
| Host route / uplink changes | Host output must not switch to any managed tunnel, even during route churn | Unproven; architecture blocker |
| DNS matrix | GetAddrInfo, DNS APIs, resolver service, browser DoH/DoT, TCP/UDP 53 packet capture | Not executed |
| IPv4/IPv6/UDP/QUIC/ICMP/raw | Explicit support/block coverage, including pre-existing and nonblocking sockets | Not executed |
| Cert/WFP/admin/settings changes | Respect existing gates without a temporary direct-network escape | Source/fake-policy review only |
| Offline adapter in options | Saving unrelated settings retains BindAdapter, including inherited/process cases | Code reviewed; Windows UI not run |
| Stale adapter recovery | Automatic, ownership-proven recovery without manual adapter management | Not implemented/qualified |

None of these pending rows may be inferred from an IP-check result or the unit-test
count. The runtime stays disabled in ordinary builds until the required architecture
and Windows qualification work is complete.
