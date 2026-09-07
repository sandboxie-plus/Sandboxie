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
