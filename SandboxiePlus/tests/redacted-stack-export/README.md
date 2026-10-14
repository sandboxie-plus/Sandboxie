# Redacted stack export tests

This target compiles the shipped `SandMan/Helpers/RedactedStackExport.cpp`
unchanged and drives it with a synthetic capture. Storage is a
`QTemporaryDir`; there is no live diagnostic path, no SbieDll and no
SbieSvc. The dialog (`SandMan/Windows/StackExportDialog.cpp`) is a thin
consumer of the same transformer and is exercised in the runtime pass, not
here.

```sh
cmake -S SandboxiePlus/tests/redacted-stack-export -B build/redacted-stack -DCMAKE_BUILD_TYPE=Release
cmake --build build/redacted-stack --config Release
ctest --test-dir build/redacted-stack -C Release --output-on-failure
```

Python is not needed. Qt 5 or Qt 6 Core is enough (no Widgets); CTest
selects the offscreen platform.

## What each case proves

| Case | What it proves |
| --- | --- |
| `allow-list-privacy` | Every synthetic private token stays out of the text, the JSON, the on-disk file and the error message. |
| `allow-list-fields` | The JSON has exactly the documented top-level keys and per-frame keys; nothing else. |
| `opaque-ids-per-report` | Ids are 1-based order-of-appearance, distinct modules get distinct ids, the same key inside one report reuses its id. |
| `same-name-distinct-modules` | Two frames sharing a display name but using different private module keys never collide. |
| `module-name-shared` | When the caller passes the display name as the key (SandMan integration) they do collide, which is the documented limit of that integration. |
| `frame-order-preserved` | Order and count of the output match the input; indices are dense. |
| `unresolved-frame-marked` | An unresolved frame becomes `[unresolved]`, a module-only symbol becomes `[module-NN]![unresolved]`; neither is filled by a guess. |
| `partial-capture-preserved` | `state == ePartial` propagates to both the JSON and the text header. |
| `unicode-and-punctuation` | Unicode display names never appear in the output; different Unicode keys still get different ids. |
| `text-and-json-fidelity` | Text has one line per frame plus one header line; JSON round-trips through `QJsonDocument`. |
| `save-atomic-happy` | The written file is redacted, no temporary sibling survives. |
| `save-open-failure` | Missing parent directory: `false` is returned, no file is created, the error string does not leak private tokens. |
| `save-commit-failure` | A directory sitting on the destination path fails the commit; `directWriteFallback` is off so nothing is written. |
| `save-existing-file` | A previous file at the destination is replaced atomically, no rogue temp file, previous content is gone. |
| `save-does-not-touch-input` | The `SStackCapture` handed to the transformer is byte-for-byte the same after the export. |
| `concurrent-transforms-do-not-share-ids` | Two independent reports each start their id space at 1, so the opaque ids are truly per-report. |

## Three kinds of evidence, kept apart

| Evidence | What it is | What it proves |
| --- | --- | --- |
| Simulated tests (this target) | 16 CTest cases over the real transformer with a `QTemporaryDir` store and a synthetic capture. | Privacy allow-list, fidelity, atomic write, opaque ids. |
| Build | `SandMan.exe` and `QSbieAPI.dll` built with MSVC from the same commit. | The dialog integration, `.h`/`.cpp` registration and toolchain compile and link. |
| Runtime | Manual on a Windows host with Sandboxie installed. | That the "Export Redacted..." action in the stack view context menu produces a share view matching this transformer's output, without altering the underlying capture or the observed process. |

A passing run here is not runtime validation of the DbgHelp symbol
resolver, of the real capture path, or of Windows clipboard semantics.
