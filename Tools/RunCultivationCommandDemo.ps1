param(
    [string]$SimulationRepository = (Join-Path $PSScriptRoot '..\..\tu-tien'),
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string]$BridgeDirectory = (Join-Path $PSScriptRoot '..\Unreal\Saved\CultivationCommandDemo')
)
$ErrorActionPreference = 'Stop'
$simulationRoot = (Resolve-Path -LiteralPath $SimulationRepository).Path
$project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\Unreal\DaoVoTan3D.uproject')).Path
$bridge = [IO.Path]::GetFullPath($BridgeDirectory)
$entry = Join-Path $simulationRoot 'dist-p1\cli\cultivation-bridge.js'
$editorExe = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not (Test-Path -LiteralPath $entry)) { throw 'Build the simulation first: npm run build:p1' }
if (-not (Test-Path -LiteralPath $editorExe)) { throw "Unreal Editor not found: $editorExe" }
$node = (Get-Command node -ErrorAction Stop).Source
$checkpoint = Join-Path $bridge 'session.json'
$lock = Join-Path $bridge 'worker.lock'
if (Test-Path -LiteralPath $lock) { throw "Bridge writer lock exists. Verify its PID before removing any stale lock: $lock" }
if (-not (Test-Path -LiteralPath $checkpoint)) {
    & $node $entry init $bridge
    if ($LASTEXITCODE -ne 0) { throw 'Could not initialize the deterministic cultivation demo' }
}
$worker = $null
try {
    $worker = Start-Process -FilePath $node -ArgumentList @(('"' + $entry + '"'), 'watch', ('"' + $bridge + '"')) -WorkingDirectory $simulationRoot -WindowStyle Hidden -RedirectStandardOutput (Join-Path $bridge 'worker-stdout.log') -RedirectStandardError (Join-Path $bridge 'worker-stderr.log') -PassThru
    Write-Host "Simulation worker PID: $($worker.Id). In Unreal, press Play, then E/M to Cultivate. +/- selects 1..30 strategic days."
    # This window is intentionally interactive: the user controls the existing Unreal project.
    $editor = Start-Process -FilePath $editorExe -ArgumentList @(('"' + $project + '"'), ('-CultivationBridgeDir="' + $bridge + '"')) -WindowStyle Normal -PassThru
    $editor.WaitForExit()
}
finally {
    if ($worker) {
        if (-not $worker.HasExited) { Stop-Process -Id $worker.Id; $worker.WaitForExit() }
        # Only the exact lock of this launched worker may be removed. Never delete saves or directories.
        if (Test-Path -LiteralPath $lock) {
            $owner = Get-Content -Raw -LiteralPath $lock | ConvertFrom-Json
            if ($owner.pid -eq $worker.Id) { Remove-Item -LiteralPath $lock }
        }
    }
}
