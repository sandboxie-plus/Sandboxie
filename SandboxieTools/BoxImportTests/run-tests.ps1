#Requires -Version 7.0
param(
    [Parameter(Mandatory = $true)][string]$QtRoot,
    [string]$SourceRef,
    [string]$BuildRoot = (Join-Path $env:TEMP ('Sandboxie-BoxImportTests-' + [guid]::NewGuid().ToString('N')))
)
$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
if (Test-Path -LiteralPath $BuildRoot) { throw 'Choose a new build directory.' }
[void](New-Item -ItemType Directory -Path $BuildRoot)
$BuildRoot = (Resolve-Path -LiteralPath $BuildRoot).Path
$Utf8 = [Text.UTF8Encoding]::new($false)
$Source = if ($SourceRef) {
    $Result = git -c "safe.directory=$RepoRoot" -C $RepoRoot show "${SourceRef}:SandboxiePlus/SandMan/BoxTransfer.cpp"
    if ($LASTEXITCODE -ne 0) { throw 'Cannot read the requested revision.' }
    $Result -join "`n"
} else { [IO.File]::ReadAllText((Join-Path $RepoRoot 'SandboxiePlus/SandMan/BoxTransfer.cpp')) }
$Source = $Source.Replace("`r`n", "`n")
$Extract = [regex]::Match($Source, '(?ms)^static QString ExtractIniSection\(.*?^\}')
if (!$Extract.Success) { throw 'Cannot locate ExtractIniSection.' }
$Prepare = [regex]::Match($Source, '(?ms)^static QString PrepareImportedBoxConfig\(.*?^\}')
if ($Prepare.Success) { $PrepareCode = $Prepare.Value } else {
    # Exercise the original transformation verbatim when checking the baseline.
    $Old = [regex]::Match($Source, '(?s)QString configContent = ExtractIniSection\(rawContent, entry.ArchiveBoxName\);.*?configContent.remove\(QRegularExpression\([^;]+;')
    if (!$Old.Success) { throw 'Cannot locate the original import transformation.' }
    $PrepareCode = "static QString PrepareImportedBoxConfig(const QString& rawContent, const QString& archiveName, const QString&) {`nstruct { QString ArchiveBoxName; } entry{archiveName};`n" + $Old.Value + "`nreturn configContent;`n}"
}
$Capture = [regex]::Match($Source, '(?ms)^static QString GetImportRootSetting\(.*?^\}')
$CaptureCode = if ($Capture.Success) { "#define HAVE_IMPORT_ROOT_SETTING`n" + $Capture.Value + "`n" } else { '' }
[IO.File]::WriteAllText((Join-Path $BuildRoot 'import_config.generated.h'), $Extract.Value + "`n" + $CaptureCode + $PrepareCode, $Utf8)
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'tests.cpp') -Destination $BuildRoot
$RepoForQmake = $RepoRoot.Replace('\', '/')
$Project = "QT = core`nCONFIG += console c++17 release`nCONFIG -= debug app_bundle`nTEMPLATE = app`nTARGET = box_import_tests`nINCLUDEPATH += `"$RepoForQmake/Sandboxie`"`nSOURCES += tests.cpp `"$RepoForQmake/Sandboxie/common/ini.cpp`"`n"
[IO.File]::WriteAllText((Join-Path $BuildRoot 'tests.pro'), $Project, $Utf8)

function Invoke-TestProcess([string]$Program, [string[]]$Arguments) {
    $Info = [Diagnostics.ProcessStartInfo]::new()
    $Info.FileName = $Program
    $Info.WorkingDirectory = $BuildRoot
    $Info.UseShellExecute = $false
    foreach ($Argument in $Arguments) { [void]$Info.ArgumentList.Add($Argument) }
    $Environment = [Environment]::GetEnvironmentVariables('Process')
    $Info.Environment.Clear()
    foreach ($Key in $Environment.Keys) {
        if ($Key -ine 'Path') { $Info.Environment[$Key] = $Environment[$Key] }
    }
    $Info.Environment['Path'] = (Join-Path $QtRoot 'bin') + ';' + $env:Path
    $Process = [Diagnostics.Process]::Start($Info)
    $Process.WaitForExit()
    if ($Process.ExitCode -ne 0) { throw "$Program failed with exit code $($Process.ExitCode). Build: $BuildRoot" }
}
Invoke-TestProcess (Join-Path $QtRoot 'bin/qmake.exe') @('tests.pro')
Invoke-TestProcess (Get-Command nmake.exe -ErrorAction Stop).Source @('/nologo')
Invoke-TestProcess (Join-Path $BuildRoot 'release/box_import_tests.exe') @()
Write-Output "Tests passed. Build: $BuildRoot"
