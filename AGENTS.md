# AGENTS.md

Guidance for AI agents (Copilot cloud agent and other AI assistants) working on this repository.

## Project overview

Sandboxie is a sandbox-based isolation tool for Windows NT systems, letting apps run/install without permanently changing the host OS. Community fork since April 2020 (not the official continuation of the original project). Two editions, same core:

- **Plus** — modern Qt UI, gets all new features.
- **Classic** — legacy MFC UI, no longer actively developed.

## Repository structure

### Sandboxie core and Classic

- `Sandboxie/core/drv` — SbieDrv kernel-mode driver. Highest-risk code; changes can cause system crashes or weaken the sandbox boundary.
- `Sandboxie/core/svc` — SbieSvc service, process creation, service IPC, RPC, policy, and privileged operations.
- `Sandboxie/core/dll` — SbieDll injected into sandboxed processes; API hooks, redirection, networking, and compatibility behavior.
- `Sandboxie/core/low` — LowLevel DLL used by the injection path and embedded in SbieDll.dll.
- `Sandboxie/common` — shared low-level C/C++ utilities, native structures, helpers, and third-party components.
- `Sandboxie/apps` — user-mode applications and system-service wrappers, including `apps/common`, Control, Start, SbieIni, and COM helpers.
- `Sandboxie/msgs` — message definitions, parser/build projects, localized text, and generated language reports.
- `Sandboxie/SboxHostDll` — host-process injection component used where host processes must participate in sandbox redirection.
- `Sandboxie/install` — Classic NSIS installer, templates, configuration, and installer support projects.

### Sandboxie Plus and tools

- `SandboxiePlus/SandMan` — Qt-based Plus UI and its models, views, wizards, scripts, and translations.
- `SandboxiePlus/QSbieAPI` — user-mode API layer used by the Plus UI.
- `SandboxiePlus/MiscHelpers` — shared Qt/helper library.
- `SandboxiePlus/UGlobalHotkey` and `SandboxiePlus/QtSingleApp` — supporting libraries with their own projects and licensing.
- `SandboxiePlus/SbieShell` — Windows Explorer shell integration built from a separate Visual Studio solution.
- `SandboxieTools` — UpdUtil, ImBox, ImDisk, MiniDump, common helpers, and targeted test/probe utilities.
- `Installer` — Plus Inno Setup packaging, build variables, dependency acquisition, asset collection, and build merging.

Components are independently built. Communication between the driver, service, DLLs, tools, and UI may use exported APIs, shared wire structures, IPC, RPC, or dynamically loaded entry points. Do not assume that adding a function in one component makes it available to another.

## Licensing

- `LICENSE.Classic` (GPLv3) and `LICENSE.Plus` (custom) — dual licensing at repository root. Duplicate/per-component licenses also exist in `Sandboxie/COPYING` (GPLv3), `Sandboxie/install/LICENSE.TXT` (GPLv3), `Installer/license.txt` (custom), `SandboxiePlus/LICENSE` (component summary), `SandboxiePlus/QSbieAPI/LICENSE` (LGPL), `SandboxiePlus/MiscHelpers/LICENSE` (LGPL), `SandboxiePlus/SandMan/LICENSE` (custom), inline in `SandboxiePlus/UGlobalHotkey/README.md` (Public Domain), inline in `SandboxiePlus/QtSingleApp/README.TXT` (LGPL), and `SandboxieTools/ImBox/dc/tools/yasm/COPYING` (third-party, BSD/Artistic/GPL/LGPL).
- Do not mix, remove, or alter copyright/license headers.

## Build and toolchain

- Core and tools use Visual Studio/MSBuild project files, the Windows SDK, and the WDK. Qt components use qmake and Jom through `SandboxiePlus/qmake_plus.cmd`.
- Treat `.github/workflows/main.yml`, project files, and build scripts as the authoritative build reference.
- Read `Installer/buildVariables.cmd` for current Qt and dependency versions. Do not hard-code versions in this file.
- The core build has separate entry points: `Sandboxie/SandboxDll.sln`, `Sandboxie/Sandbox.sln`, and `Sandboxie/SandboxDrv.sln`. The CI build compiles Win32 DLL/service components before the x64 core package.
- The Plus build also includes `SandboxiePlus/qmake_plus.cmd`, `SandboxiePlus/SbieShell/SbieShell.sln`, and `SandboxieTools/SandboxieTools.sln`.
- Solution and project configurations include Win32, x64, ARM64, and ARM64EC in different combinations. Check the affected project before claiming a platform is supported.
- Classic packaging uses `Sandboxie/install/SandboxieVS.nsi`. Plus packaging uses `Installer/Sandboxie-Plus.iss`. Packaging is separate from compiling the runtime.

## Known documentation gaps

- `Sandboxie/ReadMe.md`, `SandboxiePlus/ReadMe.md`, and `Installer/ReadMe.md` may lag behind the current toolchain and `.github/workflows/main.yml`.
- `Installer/ReadMe.md` is incomplete. Cross-check installer setup and build instructions against `Installer/Sandboxie-Plus.iss`, `Installer/buildVariables.cmd`, and CI.
- The public website and documentation copied from the separate `sandboxie-docs` project may lag behind the source and current releases. Verify claims against source files and CI before relying on them.
- When toolchain versions or build procedures change, update the relevant build documentation so it does not drift further.

## Code change guidelines

- Treat `Sandboxie/core` with extreme caution: bugs can cause BSODs or sandbox-escape security holes. Prefer minimal, surgical patches there over refactors.
- Verify builds, branches, and properties for all architectures (Win32/x64/ARM64/ARM64EC). Ensure builds are free of pointer-size, packing, calling-convention, and signed/unsigned assumptions.
- Don't silently break the `Sandboxie.ini`, `Sandboxie-Plus.ini`, `Templates.ini` formats or the driver/service/DLL IPC protocol.
- Keep Windows 7+ (64-bit) support unless told otherwise.
- Match the existing style of the file you're editing, surrounding code, and `.editorconfig`, avoiding any reformatting of unrelated lines, except for repository-wide style and formatting overhauls isolated in a dedicated pull request.
- Never commit, create or delete branches, reset files, clean the worktree, or push changes unless the user explicitly requests that exact operation.
- Reflect significant changes in `CHANGELOG.md`.
- Link to any related issue when possible.

## Testing

- No automated test suite for kernel-mode code — validation is manual (build + run on Windows).
- Test sandbox start/stop, sandboxed process creation, and file recovery when touching the driver or hook DLL.

## Security

- Treat user-mode requests, IPC/RPC messages, handles, paths, registry data, configuration values, and sandbox contents as untrusted input.
- Check lengths, structure versions, pointer validity, lifetime, privilege boundaries, path normalization, object namespaces, and cleanup before using external data.
- Do not loosen default isolation, access checks, token restrictions, or breakout protections without documenting the security impact and testing both sandboxed and unsandboxed paths.
- Do not publish vulnerability details in public issues or pull requests. Follow `SECURITY.md` for private email or GitHub advisory reporting, and do not submit unverified AI-generated security reports.

## Code review

- Review the entire diff, not just the first files/issues found.
- Flag every occurrence of a repeated problem, not just one example.
- If repository code and documentation disagree, explain the discrepancy instead of silently choosing one interpretation.
- Give extra scrutiny to kernel-mode code, IPC/protocol changes, and sandbox security boundaries.
- Flag pull requests that bundle multiple unrelated changes and suggest splitting.

## Pull requests

- Commit messages and pull request descriptions must be in **English** language.
- Translators: see notes at https://github.com/sandboxie-plus/Sandboxie/discussions/1123
- **One PR = one logical change.** Don't mix unrelated fixes/features in a single pull request.
- Link to any existing issue when possible; state impact on Plus, Classic, or both.

## Resources

- [Documentation](https://sandboxie-plus.github.io/sandboxie-docs) · [CHANGELOG.md](./CHANGELOG.md) · [SECURITY.md](./SECURITY.md) · [CONTRIBUTING.md](./CONTRIBUTING.md)
