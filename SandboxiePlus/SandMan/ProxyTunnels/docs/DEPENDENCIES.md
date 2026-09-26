# Dependency provenance and packaging proposal

No executable from `Artjosh/tun2proxy-bindsandboxie` was executed, reused or trusted.
The reference has no provenance sufficient to treat its bundled EXE/DLL as our
release dependency. This patch contains no tunnel binary.

## Inspected official artifacts

- tun2socks **v2.7.0**, official release published July 12, 2026:
  https://github.com/xjasonlyu/tun2socks/releases/tag/v2.7.0
- The matching official source archive was inspected (`main.go`, engine/device
  paths, key parsing and license). The source archive directory identifies commit
  prefix `8dda19e`. Source-to-binary reproducibility was not established.
- Wintun **0.14.1**, https://www.wintun.net/ and
  https://www.wintun.net/builds/wintun-0.14.1.zip
- Wintun archive SHA-256:
  `07c256185d6ee3652e09fa55c0b673e2624b565e02c4b9091c79ca7d2f24ef51`

The official versioned downloads were inspected by this Windows Actions run:
https://github.com/Kizuno18/sbxie/actions/runs/34013729680

That job compared each tun2socks ZIP to the official release asset digest, checked
the Wintun ZIP hash, hashed the extracted executables/DLLs and used Windows
`Get-AuthenticodeSignature` on the Wintun DLLs. All four Wintun architecture DLLs
reported `Valid` with WireGuard LLC as signer. The job did not execute tun2socks,
install Wintun, configure an adapter or run traffic through a tunnel.

| Artifact | ZIP SHA-256 | Executable SHA-256 |
|---|---|---|
| tun2socks Windows amd64 | `c5d46e9452f6c9cc7c15ab9158d6d6a0169ceecd6bca019ce476b49337d2be43` | `076b3c3d6a372bae3f49f2b415a4105f70c30a3ed3caaed7979390e649892559` |
| tun2socks Windows arm64 | `74497771068da13f42921adfc540f2abb9ac822404582c8cbe34d30e8c0ea1f5` | `559eb3ee0935f628a6ab9952976b30ca7c3706dfe5fd9f1095fbe5692f3a13fa` |
| tun2socks Windows 386 | `ad870a2ca99d617e004ec09f8f5cca2c134b4d16ae12b4eac0ff6d9fe05d9044` | `bd964c932ca9969026f3ff47790d614f10186536c2e1330dc5cb4352f589cfa5` |

| Wintun DLL architecture | SHA-256 |
|---|---|
| amd64 | `e5da8447dc2c320edc0fc52fa01885c103de8c118481f683643cacc3220dafce` |
| arm64 | `f7ba89005544be9d85231a9e0d5f23b2d15b3311667e2dad0debd344918a3f80` |
| x86 | `d694fa46ab4cfebcb2632d094c7aa97278eef2f8052438621766d863ae98a931` |

The laboratory backend pins the extracted file hashes in compiled source and holds
non-write-sharing read handles across their use. It does not accept an arbitrary
executable path, current-directory fallback, renamed reference binary or a new
hash from a user-editable manifest. Hash/signature verification proves artifact
identity under those trust assumptions, not the absence of upstream defects.

## Licenses and integration

The official tun2socks license is MIT (Jason Lyu); package that notice with the
binary and consider transitive dependency notices. SandMan retains its own custom
license; this feature is not a relicensing of SandMan or the core.

Wintun source is GPLv2, but the **official prebuilt archive** contains its own
`LICENSE.txt` redistribution terms. Do not replace those terms with a generic
“MIT/GPL” label. Review and preserve that exact notice and the unmodified DLLs, use
only the permitted API, and package alongside the application using the driver.
This inspection does not constitute a complete legal review of a distributable
add-on. No Wintun binary or copied source is bundled in the patch.

The intended package ID is `ProxyTunnels`; the proposed installation directory is
`Addons/ProxyTunnels` below the application directory, containing architecture-
matched `tun2socks.exe`, `wintun.dll` and notices. This is a proposal, **not an
existing published add-on**. The button delegates to `CAddonManager` and its normal
updater/consent/progress path only when the catalog actually contains the package;
otherwise it reports the missing package without downloading anything.

Before publication, maintainers must approve packaging, catalog/update trust,
architecture selection, supported Windows versions, license notices and hash
updates. A privileged installation directory and pinned files should not replace
an actual installer security review. No unsigned reference executable is a fallback.
