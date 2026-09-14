# INI value trimming tests

These tests compile the complete production `Sandboxie/common/ini.cpp`, which
is shared by SbieSvc and QSbieAPI. No methods are extracted or replaced.

From the repository root, with CMake and Visual Studio C++ Build Tools installed:

```powershell
cmake -S SandboxieTools/Tests/IniFile -B build/ini-file-x64 -A x64
cmake --build build/ini-file-x64 --config Release
cmake -E chdir build/ini-file-x64 ctest -C Release --output-on-failure
```

Use a separate build directory and `-A Win32` to test x86. To test the same cases
against an older checkout, configure with `-DSANDBOXIE_SOURCE_ROOT=<checkout>`.

The executable runs 25 cases. The #5552 regression loads an INI containing a
trailing U+0003, reads the service value, trims its edges using an explicit model
of the driver's `<=32` rule, and asks `RemoveValue` to delete that result. It then
saves and reloads the file to verify that the entry is gone. This is checked with
UTF-8 BOM, UTF-16 LE, and UTF-16 BE input.

Other cases cover value edges in parsing, AddValue, SetValue, and whole-section
replacement; Classic's LF-separated lists, including empty items and CRLF input;
truncation before trimming and skipping oversized list segments; null and empty
delete-all arguments versus nonempty whitespace arguments; empty and normal
same-name entries; duplicate entries; ordinary whitespace; Unicode; interior
controls; comments; ordering; and the existing CR/LF rejection in AddValue.

Each run creates fresh fixtures below the build directory's `test-data` folder.
The tests never read an installed configuration, access the registry, connect to
Sandboxie services, or delete files. Exceptions are reported per case and any
failed expectation makes the executable return a nonzero exit code.

This tests CIniFile, not service IPC, authorization, driver cache publication,
or the GUI. The driver-side trimming rule is a small model, not a running driver.
