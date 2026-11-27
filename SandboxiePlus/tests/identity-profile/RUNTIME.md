# Runtime check of the volume serial profile

This is the manual, real-driver check that the simulated tests and the build
cannot replace. It was run once for commit `6ef86f5f` on a lab VM; the steps
are here so that anyone can repeat it. Nothing in this file is executed by
CTest.

## Lab conditions

- Windows 10 x64 VM with the official Sandboxie-Plus 1.18.4 installer, then
  `SandMan.exe` and `QSbieAPI.dll` replaced by the build from this branch.
- The official driver only accepts a signed manager as session agent
  (`STATUS_INVALID_SIGNATURE` otherwise), so the VM ran in Windows
  test-signing mode, where `MyIsCallerSigned` short-circuits. This is a
  lab condition for running a development manager; it is not a requirement of
  the feature and says nothing about production security.
- Two volumes: the system volume (`HarddiskVolumeN`, N from
  `QueryDosDevice`) and a small fixed VHD mounted as `D:`. Two empty
  sandboxes `BoxA` and `BoxB`, one host control process outside any box.

## Steps

1. Note the host serials: `powershell -File volume_probe.ps1 -PathList C:\,D:\`.
2. Map letters to device names (`QueryDosDevice C:`), for example
   `C: -> \Device\HarddiskVolume3`, `D: -> \Device\HarddiskVolume4`.
3. In SandMan: BoxA > Sandbox Options > Advanced Options > Privacy >
   **Identity Profiles...** > **New**. Name it, set the two rows to the device
   names from step 2 with synthetic serials (`A1A1-0001`, `A1A1-0002`), Save,
   Close, choose the profile in the combo box, OK. No error dialog may appear.
4. Repeat for BoxB with a second profile (`B2B2-0001`, `B2B2-0002`).
5. Check `Sandboxie.ini`: each box section has `HideDiskSerialNumber=y`,
   `IdentityProfile=<id>`, `IdentityProfileRevision=1` and one
   `DiskSerialNumber=<device>,<serial>` per row; the JSON files are under the
   SandMan data directory in `IdentityProfiles`.
6. Run the probe inside each box through `Start.exe /box:<box> /wait` and on
   the host; repeat in fresh processes, then reboot, reattach the VHD (a VHD
   does not reattach by itself) and repeat again without touching the profiles.

## Result for 6ef86f5f (anonymised)

| Process | C: by handle | D: by handle | Notes |
| --- | --- | --- | --- |
| BoxA (profile A), 4 fresh processes + after reboot | `A1A1-0001` | `A1A1-0002` | D: only after the VHD was reattached |
| BoxB (profile B), 2 fresh processes + after reboot | `B2B2-0001` | `B2B2-0002` | while BoxB was briefly bound to profile A it showed the A1A1 values |
| Host control, before/between/after | real serial, unchanged | real serial, unchanged | not reproduced here |

Two boxes present two different identities, a shared profile presents the
same identity in both, and the host is untouched. Values survived process
restarts and a reboot for both volumes. `GetVolumeInformationW` returned the
same substituted value inside the boxes on this build; that is an observation,
not part of the covered surface. Physical disk serials, WMI and the native
query were not checked and are not claimed.

An earlier build (06af52de) reported "configuration read back does not match
the profile" from this exact flow although the INI was correct after commit;
that is why the binding now commits before reading back.
