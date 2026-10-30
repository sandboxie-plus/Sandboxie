# CLI-only fork notes

This fork intentionally keeps the Sandboxie backend and CLI launcher, while omitting the SandMan Qt GUI and other non-essential Plus UI components.

## Scope

Preserved:
- `Sandboxie/core/drv`
- `Sandboxie/core/svc`
- `Sandboxie/core/dll`
- `Sandboxie/apps/start`
- `Sandboxie/apps/common`
- `Sandboxie/apps/ini`

Removed from the default fork build path:
- `SandboxiePlus/SandMan`
- `SandboxiePlus/UGlobalHotkey`
- `SandboxiePlus/QtSingleApp`
- GUI-heavy Plus frontend modules and translations

## License/support reminder

This project still follows the upstream behavior: some advanced functionality may require a valid supporter certificate. If no supporter license is present, the runtime may show the standard warning/reminder flow instead of the unrestricted Plus feature set.

## Intended usage

Use the command-line launchers and backend components in the `Sandboxie/apps` and `Sandboxie/core` trees. The console-first path is designed for headless automation and lightweight operation, without the Qt desktop manager.
