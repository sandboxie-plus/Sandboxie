# AGENTS.md

Guidance for AI agents (Copilot cloud agent and other AI assistants) working on this repository.

## Project overview

Sandboxie is a sandbox-based isolation tool for Windows NT systems, letting apps run/install without permanently changing the host OS. Community fork since April 2020 (not the official continuation of the original project). Two editions, same core:

- **Plus** — modern Qt UI, gets all new features.
- **Classic** — legacy MFC UI, no longer actively developed.

## Repository structure

- `Sandboxie/core` — kernel driver (SbieDrv), service (SbieSvc), hook DLL (SbieDll). C/C++/Assembly. **Highest risk area.**
- `Sandboxie/{common,apps,SboxHostDll}` — shared code, utilities.
- `Sandboxie/install` — Sandboxie Classic NSIS installer, config/templates, shared build utilities.
- `Sandboxie/msgs` — localized strings, message parser, translation report tooling.
- `SandboxiePlus` — Qt UI ("SandMan") and supporting libraries (QSbieAPI, MiscHelpers, SbieShell, UGlobalHotkey). C++, Sandboxie Plus only.
- `SandboxieTools` — auxiliary tools (ImBox, ImDisk, MiniDump, UpdUtil), Sandboxie Plus only.
- `Installer` — Inno Setup packaging scripts and build/setup helper scripts, Sandboxie Plus only.

## Licensing

`LICENSE.Classic` (GPLv3) and `LICENSE.Plus` (custom) — dual licensing at repository root. Duplicate/per-component licenses also exist in `Sandboxie/COPYING` (GPLv3), `Sandboxie/install/LICENSE.TXT` (GPLv3), `Installer/license.txt` (custom), `SandboxiePlus/LICENSE` (component summary), `SandboxiePlus/QSbieAPI/LICENSE` (LGPL), `SandboxiePlus/MiscHelpers/LICENSE` (LGPL), `SandboxiePlus/SandMan/LICENSE` (custom), inline in `SandboxiePlus/UGlobalHotkey/README.md` (Public Domain), inline in `SandboxiePlus/QtSingleApp/README.TXT` (LGPL), and `SandboxieTools/ImBox/dc/tools/yasm/COPYING` (third-party, BSD/Artistic/GPL/LGPL). Do not mix, remove, or alter copyright/license headers.

## Toolchain

- Visual Studio (`.sln`/`.props`) for core/driver; `qmake` (`qmake_plus.cmd`, `SandboxiePlus.pro`) for the Qt UI.
- Windows SDK/WDK required for kernel-mode components.

## Known documentation gaps

- `Sandboxie/ReadMe.md`, `SandboxiePlus/ReadMe.md`, `Installer/ReadMe.md` — build instructions may lag behind the actual toolchain (VS/WDK/Qt versions) and the [CI workflow](./.github/workflows/main.yml); verify against CI before trusting them.
- `Installer/ReadMe.md` — explicitly incomplete (marked "please open a new pull request" for both installer environment setup and installer build steps). Do not treat as authoritative; cross-check with `Installer/Sandboxie-Plus.iss` and CI.
- [sandboxie-plus.com](https://sandboxie-plus.com/) (content sourced, often manually and irregularly, from [sandboxie-plus/sandboxie-docs](https://github.com/sandboxie-plus/sandboxie-docs)) may lag behind actual releases — e.g. new versions can be published/announced before the static [download page](https://sandboxie-plus.com/downloads/) and documentation are updated. Don't assume the sandboxie-plus.com site reflects the latest release state.
- When toolchain versions change (Qt, WDK, VS), update these ReadMe files alongside the code change to prevent further drift.

## Code change guidelines

- Treat `Sandboxie/core` with extreme caution: bugs can cause BSODs or sandbox-escape security holes. Prefer minimal, surgical patches there over refactors.
- Don't silently break the `Sandboxie.ini` format or the driver/service/DLL IPC protocol.
- Keep Windows 7+ (64-bit) support unless told otherwise.
- Match the existing style of the file/module you're editing.
- Reflect significant changes in `CHANGELOG.md`.

## Testing

- No automated test suite for kernel-mode code — validation is manual (build + run on Windows).
- Verify builds for Win32 and x64 (ARM64 where applicable).
- Test sandbox start/stop, sandboxed process creation, and file recovery when touching the driver or hook DLL.

## Security

- Never report vulnerabilities in public issues/PRs — follow `SECURITY.md`.
- Justify explicitly any change that loosens default security restrictions.

## Code review

- Review the entire diff, not just the first files/issues found.
- Flag every occurrence of a repeated problem, not just one example.
- Give extra scrutiny to kernel-mode code, IPC/protocol changes, and sandbox security boundaries.
- Flag PRs that bundle multiple unrelated changes and suggest splitting.

## Pull requests

- Commit messages and PR descriptions must be in **English**.
- Translators: see notes at https://git.io/J9G19
- **One PR = one logical change.** Don't mix unrelated fixes/features in a single PR (see [#5313](https://github.com/sandboxie-plus/Sandboxie/pull/5313) as an example to avoid).
- Link to an existing issue when possible; state impact on Plus, Classic, or both.

## Resources

- [Documentation](https://sandboxie-plus.github.io/sandboxie-docs) · [CHANGELOG.md](./CHANGELOG.md) · [SECURITY.md](./SECURITY.md) · [CONTRIBUTING.md](./CONTRIBUTING.md)
