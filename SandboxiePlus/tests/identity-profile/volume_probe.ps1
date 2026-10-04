# Read-only volume serial probe for the identity profile runtime check.
# Prints JSON per root path: the by-handle serial (GetVolumeInformationByHandleW,
# the hook covered by identity profiles), the path-based serial
# (GetVolumeInformationW, recorded for comparison only) and whether SbieDll is
# loaded in this process. Run it on the host and inside a box, for example:
#   powershell -ExecutionPolicy Bypass -File volume_probe.ps1 -PathList C:\,D:\
#   Start.exe /box:BoxA /wait cmd /c "powershell -ExecutionPolicy Bypass -File volume_probe.ps1 -PathList C:\,D:\ > out.txt"
# It prints only the observed serials and error codes, no other identifiers.
param([string]$PathList = "C:\")
$Paths = $PathList -split ","

Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class VolProbe {
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern IntPtr CreateFileW(string name, uint access, uint share, IntPtr sec, uint disp, uint flags, IntPtr tmpl);
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool CloseHandle(IntPtr h);
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool GetVolumeInformationByHandleW(IntPtr h, StringBuilder name, uint nameSize, out uint serial, out uint maxLen, out uint flags, StringBuilder fs, uint fsSize);
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    public static extern bool GetVolumeInformationW(string root, StringBuilder name, uint nameSize, out uint serial, out uint maxLen, out uint flags, StringBuilder fs, uint fsSize);
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
    public static extern IntPtr GetModuleHandleW(string name);
}
"@

$result = [ordered]@{
    pid = $PID
    sbieDllLoaded = ([VolProbe]::GetModuleHandleW("SbieDll.dll") -ne [IntPtr]::Zero)
    boxName = $env:SBIE_BOX_NAME
    volumes = @()
}
foreach ($p in $Paths) {
    $entry = [ordered]@{ path = $p; byHandle = $null; byHandleError = 0; byPath = $null; byPathError = 0 }
    $h = [VolProbe]::CreateFileW($p, 0, 7, [IntPtr]::Zero, 3, 0x02000000, [IntPtr]::Zero)
    if ($h -ne [IntPtr]-1) {
        $serial = [uint32]0; $ml = [uint32]0; $fl = [uint32]0
        $name = New-Object System.Text.StringBuilder 261; $fs = New-Object System.Text.StringBuilder 261
        if ([VolProbe]::GetVolumeInformationByHandleW($h, $name, 261, [ref]$serial, [ref]$ml, [ref]$fl, $fs, 261)) {
            $entry.byHandle = ('{0:X4}-{1:X4}' -f ($serial -shr 16), ($serial -band 0xFFFF))
        } else { $entry.byHandleError = [Runtime.InteropServices.Marshal]::GetLastWin32Error() }
        [VolProbe]::CloseHandle($h) | Out-Null
    } else { $entry.byHandleError = [Runtime.InteropServices.Marshal]::GetLastWin32Error() }
    $serial2 = [uint32]0; $ml2 = [uint32]0; $fl2 = [uint32]0
    $name2 = New-Object System.Text.StringBuilder 261; $fs2 = New-Object System.Text.StringBuilder 261
    if ([VolProbe]::GetVolumeInformationW($p, $name2, 261, [ref]$serial2, [ref]$ml2, [ref]$fl2, $fs2, 261)) {
        $entry.byPath = ('{0:X4}-{1:X4}' -f ($serial2 -shr 16), ($serial2 -band 0xFFFF))
    } else { $entry.byPathError = [Runtime.InteropServices.Marshal]::GetLastWin32Error() }
    $result.volumes += $entry
}
$result | ConvertTo-Json -Compress -Depth 4
