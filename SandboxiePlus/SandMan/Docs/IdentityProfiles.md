# Identity profiles (version 1: volume serials)

This is the first functional slice of per-sandbox identity profiles. A profile
is a small, versioned record owned by SandMan that holds synthetic **volume
serial numbers** for native volumes. Binding a profile to a sandbox writes those
values into the sandbox section as the existing `DiskSerialNumber` entries and
sets `HideDiskSerialNumber=y`, so the existing `GetVolumeInformationByHandleW`
hook in SbieDll serves them. Nothing else is covered by this version.

Three things are deliberately kept apart:

| Layer | Where it lives | What it proves |
| --- | --- | --- |
| Saved profile | `<SandMan data dir>/IdentityProfiles/<id>.json` | The values exist and validate. Nothing is configured yet. |
| Applied configuration | `IdentityProfile`, `IdentityProfileRevision`, `HideDiskSerialNumber`, `DiskSerialNumber` in the box section of `Sandboxie.ini` | SandMan wrote the values and read them back. Running processes are not affected. |
| Verified protection | Not provided by this version | Only a runtime query inside the box proves what an application sees. |

## Using a profile

Sandbox Options > Advanced > Privacy has a profile selector next to
**Hide Disk Serial Number** and an **Identity Profiles...** button.

- **New / Edit** open the editor: a name and a table of `HarddiskVolumeN` device
  names with `XXXX-XXXX` serials. `\Device\HarddiskVolume1` is accepted and
  stored as `HarddiskVolume1`. Drive letters, `PhysicalDriveN` and wildcards are
  rejected. New rows get a random serial; the editor never reads the host.
- **Regenerate Serials** (editor) replaces every serial in the draft. It is an
  explicit action; nothing changes until Save.
- **Regenerate** (list) does the same to the stored profile in one step, after a
  confirmation that lists the sandboxes using it.
- **Clone** creates a new profile id with the same serials.
- **Import / Export** exchange one profile as JSON. Import always creates a new
  profile id and keeps the serials; it never overwrites an existing record.
- **Delete** refuses while any sandbox still references the profile.

Selecting a profile in the combo box and pressing Apply/OK binds it. SandMan
refuses to apply or remove a binding while the sandbox has running processes,
and refuses to bind a sandbox that already has manual `DiskSerialNumber`
entries; remove those first. While a profile is bound the **Hide Disk Serial
Number** checkbox is managed by the profile.

Selecting **No identity profile** removes `IdentityProfile`,
`IdentityProfileRevision` and the profile's `DiskSerialNumber` entries and leaves
`HideDiskSerialNumber` as it is, so the pre-existing checkbox keeps its meaning.

## Stability and lifecycle

- Values are stable across SandMan restarts and sandbox restarts because the
  profile file and the box section both persist them. Two sandboxes bound to
  the same profile present the same serials; two profiles distinguish two
  sandboxes; two volumes inside one profile keep distinct serials.
- Every save increments the profile `revision`; the box records the revision it
  applied. After **Regenerate** the box shows *stale* until it is re-applied
  while stopped. The stored configuration never changes silently.
- Raw INI edits that change the box entries show as *diverged*; re-applying the
  profile restores its values. A deleted profile shows as *missing* and the box
  keeps its configured values until it is unbound.
- Saves use `QSaveFile`, so a failed write leaves the previous file intact and
  a read-back mismatch is reported instead of a success.
- Templates are untouched: the binding only reads and writes the box section,
  and `Template=` references stay as they are. If a template also contributes
  `DiskSerialNumber` values they are listed separately in the binding state.
- No environment variable, seed or external process is an authority for a
  profile. The host configuration outside the box section is not written.

## What is not covered

- Only the by-handle volume query is substituted. `GetVolumeInformationW/A`,
  `NtQueryVolumeInformationFile`, WMI and physical storage device serials are
  separate surfaces and are not claimed.
- This is the **volume** serial (the value `vol` prints), not the physical
  disk serial number.
- `HarddiskVolumeN` names can be reassigned by Windows when storage changes;
  the profile binds names, not physical volumes.
- Already running processes keep their current values; a re-apply needs a
  stopped sandbox. There is no runtime indicator of what a process actually
  observes and no fail-closed launch policy.
- `SbieIniServer` configuration writes are not a crash-atomic transaction; the
  profile file is atomic, the INI update is verified by read-back only.

See [IdentityCoverage.md](IdentityCoverage.md) for the full category matrix and
the tests README under `SandboxiePlus/tests/identity-profile` for what the
simulated tests, the build and the runtime checks each prove.
