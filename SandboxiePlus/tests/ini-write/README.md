# INI write error regression tests

The list-update and safe-boolean helpers must return a failed mutation instead
of continuing and reporting success. SandMan's options dialog must also stop
Apply/OK after a reported save error rather than reloading and closing over the
pending edits.

This target extracts the production `CSbieIni::UpdateTextList`, `SetBoolSafe`,
and the relevant `COptionsWindow` write, save, Apply and OK methods at build time.
It uses real Qt containers and widgets with injected storage, status payloads,
and surrounding dialog callbacks. It does not compile the entire real classes,
connect to SbieSvc, exercise credentials, or emulate the driver's configuration
cache. The fixture error code and all setting values are synthetic.

```sh
cmake -S SandboxiePlus/tests/ini-write -B build/ini-write -DCMAKE_BUILD_TYPE=Release
cmake --build build/ini-write --config Release
ctest --test-dir build/ini-write -C Release --output-on-failure
```

Python 3, a C++17 compiler, and Qt 5 or Qt 6 development packages with Widgets are
required. CTest selects the offscreen platform. Pass `CMAKE_PREFIX_PATH` when Qt
is not on the default search path. On Windows, the selected Qt DLL directory
must also be on `PATH` when running tests.

The ten cases cover failed deletion, failed append, partial progress and retry,
unchanged/duplicate list values, safe-boolean failure and success, failed checkbox
default removal, and options-dialog failure/success/retry. A separate injected
save-stage failure tests Apply/OK independently of the list helper. Checks remain
enabled in Release builds. The successful list path intentionally retains the
existing multiset-diff behavior rather than introducing ordering changes.

`generate_fixture.py --source-root <other-SandboxiePlus-directory> <output>`
extracts the same methods from another revision for a before/after reproduction.
The generated includes are build products and should not be committed.

These changes are not a transaction: mutations completed before an error can
remain applied, and the existing final commit/refresh path still runs. They do
not add rollback, compare-and-swap, concurrent-writer protection, checked read
enumeration, or propagation of errors hidden by other void-returning helpers
(such as `CommitIniChanges` and `SetTextMap`). Retaining pending edits is not a
promise that retrying every partially completed operation is safe. Actual
service-denial and disk-write failure tests, plus the full SandMan build, remain
separate from these injected regression tests.

## Raw INI editor regressions

A second executable, `raw_ini_test`, adds thirteen cases for the raw editor in
both the sandbox options and global settings dialogs. Save, Apply and OK must
retain the text, cursor selection, undo history and edit controls on a reported
write failure. They must not reload or close the dialog. Reconnecting and retrying
must submit the same Unicode text, comments, duplicate keys and ordering.
A disconnected global editor rejects the save without a write attempt. The
sandbox's structured-view dirty flag changes only after a successful raw write.
Explicit Cancel and successful empty-section writes keep their existing behavior.
Each case runs with tab navigation and tree navigation.

`generate_raw_fixture.py --source-root <other-SandboxiePlus-directory> <output>`
extracts both dialogs' raw save methods and their callers for before/after runs.
The fixture uses real Qt text-edit, cursor and navigation widgets, but storage,
message reporting and surrounding load callbacks are injected. It does not run
CCodeEdit's completion/highlighting logic, the complete SandMan dialogs or SbieSvc.
The fixtures model a reported failure, not a real disk fault or service outage.

The raw helpers now report failure to every Save/Apply/OK caller. No raw section
parser, format conversion, automatic retry, rollback or native publication API is
added. This does not fix errors swallowed by the structured global
`CSettingsWindow::SaveSettings` path, or guarantee that a service-reported failure
left its cached or on-disk configuration unchanged. Window-manager close and the
explicit Cancel operation are not recovery storage for unsaved text.
