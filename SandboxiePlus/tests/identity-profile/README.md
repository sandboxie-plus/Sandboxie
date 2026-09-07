# Identity profile tests

This target compiles the shipped `SandMan/Helpers/IdentityProfile.cpp` and
`SandMan/Windows/IdentityProfilesDialog.cpp` unchanged, together with a test
driver. Storage is a `QTemporaryDir`; the box section is a synthetic in-memory
target implementing `CIdentityIniTarget`. It does not load SbieDll, talk to
SbieSvc, read `Sandboxie.ini` or query any real volume.

```sh
cmake -S SandboxiePlus/tests/identity-profile -B build/identity-profile -DCMAKE_BUILD_TYPE=Release
cmake --build build/identity-profile --config Release
ctest --test-dir build/identity-profile -C Release --output-on-failure
```

Python is not needed. Qt 5 or Qt 6 with Widgets is required; CTest selects
the offscreen platform. On Windows the selected Qt DLL directory must be on
`PATH` when running the tests.

## Three kinds of evidence

| Evidence | What it is | What it proves |
| --- | --- | --- |
| Simulated tests (this target) | 27 CTest cases over the real model, store, binding and dialog code with injected storage and box | Persistence, validation, atomic save, revision/staleness, two boxes, templates untouched, host untouched, dialog flows |
| Build | `SandMan.exe` and `QSbieAPI.dll` built with MSVC from the same commit | The Advanced page integration, `.ui` widgets and project registration compile and link |
| Runtime | Not part of this change | What a process inside a bound sandbox actually observes from `GetVolumeInformationByHandleW`, and that the host is unchanged, needs a probe run on Windows with Sandboxie installed |

The cases: `normalize-device`, `normalize-serial`, `validate-invalid`,
`json-corrupt`; `store-roundtrip` (second store instance stands in for a new
process), `store-atomic-failure` (commit blocked, previous file intact, no
temporaries left), `store-list-skips-bad`, `store-remove`;
`regenerate-explicit` (edit+save never changes serials, only `Regenerate`
does), `clone-new-id`, `import-new-id`, `export-atomic`; `bind-apply`,
`bind-two-boxes` (two profiles, three boxes, regeneration only reaches
re-applied boxes), `bind-refuses-running`, `bind-refuses-manual`,
`bind-stale-after-regenerate`, `bind-diverged`, `bind-missing`, `bind-unbind`,
`bind-keeps-templates`, `bind-write-failure`, `host-untouched` (only the
profile directory and the box target change); `dialog-new-edit`,
`dialog-clone-regenerate`, `dialog-import-export`, `dialog-delete-guard`.

A passing run here is not runtime validation of the hook, of SbieSvc
configuration writes, or of Windows file semantics.
