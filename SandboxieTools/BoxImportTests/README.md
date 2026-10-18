# Box import configuration tests

Run in a Visual Studio developer PowerShell with the matching MSVC Qt kit:

```powershell
./SandboxieTools/BoxImportTests/run-tests.ps1 -QtRoot C:/Qt/6.8.3/msvc2022_64
```

The script creates a new temporary build directory, extracts the production
`ExtractIniSection`, `GetImportRootSetting`, and `PrepareImportedBoxConfig`
functions from `BoxTransfer.cpp`, and compiles them with Qt Core and the real
`Sandboxie/common/ini.cpp`. The configuration replacements run in memory; the
test does not connect to Sandboxie, change its configuration, mount images, or
create real sandboxes. Build files are retained at the printed path.

The cases cover a new custom destination, the destination machine's default,
an existing box's root, replacement of global settings, legacy archives,
case/spacing/duplicate root entries, CRLF, variables, an image-backed box's
configuration, and empty/missing sections. Effective-setting capture uses a
small adapter to check that template/global lookup is requested without
expanding variables. It does not emulate the driver's configuration engine.

The import retains the destination's effective root pattern as a per-box value,
including an inherited/default pattern. This prevents imported global settings
or templates from replacing that root pattern through inheritance. Variables
remain unexpanded, and changes to their values can still change the resolved
location. The expanded extraction directory and image mount path are never used
as settings. The imported box no longer follows later changes to the global
`FileRootPath` value.

To reproduce the original failure against a Git revision:

```powershell
./SandboxieTools/BoxImportTests/run-tests.ps1 -QtRoot C:/Qt/6.8.3/msvc2022_64 -SourceRef 5313bfa91
```

For the baseline, the script extracts the original section-processing statements
instead of the new helper. Destination-persistence assertions are expected to
fail. This is a regression test for configuration handling, not an end-to-end
archive extraction or GUI test; plain and encrypted imports should also be
checked in an isolated Sandboxie installation before release.
