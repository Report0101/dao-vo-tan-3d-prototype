param(
    [string]$SimulationRepository,
    [string]$EngineRoot='C:\Program Files\Epic Games\UE_5.8',
    [string]$BridgeDirectory
)
$ErrorActionPreference='Stop'
$previewRepoRoot=(Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$previewProject=Join-Path $previewRepoRoot 'Unreal/DaoVoTan3D.uproject'
if(!$SimulationRepository){$SimulationRepository=Join-Path (Split-Path -Parent $previewRepoRoot) 'tu-tien'}
$previewCli=Join-Path (Resolve-Path -LiteralPath $SimulationRepository).Path 'dist-p1/cli/cultivation-bridge.js'
$previewEditor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe'
$previewNode=(Get-Command node -ErrorAction Stop).Source
foreach($previewFile in @($previewProject,$previewCli,$previewEditor)) {
    if(!(Test-Path -LiteralPath $previewFile -PathType Leaf)){throw "Missing existing dependency: $previewFile"}
}
if(!$BridgeDirectory){$BridgeDirectory=Join-Path $previewRepoRoot 'Unreal/Saved/PlayerV1HumanPreviewBridge'}
$BridgeDirectory=[IO.Path]::GetFullPath($BridgeDirectory)
$previewSession=Join-Path $BridgeDirectory 'session.json'
if(!(Test-Path -LiteralPath $previewSession)) {
    & $previewNode $previewCli init $BridgeDirectory
    if($LASTEXITCODE -ne 0){throw 'Existing simulation CLI init failed'}
}
$previewLock=Join-Path $BridgeDirectory 'worker.lock'
$previewWorker=$null
$previewOwned=$false
if(Test-Path -LiteralPath $previewLock) {
    $previewLockData=Get-Content -LiteralPath $previewLock -Raw|ConvertFrom-Json
    $previewExisting=Get-CimInstance Win32_Process -Filter "ProcessId=$($previewLockData.pid)"
    if(!$previewExisting -or !$previewExisting.CommandLine.Contains($BridgeDirectory) -or !$previewExisting.CommandLine.Contains('cultivation-bridge.js')) {
        throw "Existing worker lock cannot be verified. Choose a fresh -BridgeDirectory or inspect $previewLock. No lock was removed."
    }
    Write-Host "Using existing worker PID $($previewExisting.ProcessId)"
} else {
    $previewWorker=Start-Process $previewNode -ArgumentList @(('"'+$previewCli+'"'),'watch',('"'+$BridgeDirectory+'"')) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $BridgeDirectory 'preview-worker-stdout.log') -RedirectStandardError (Join-Path $BridgeDirectory 'preview-worker-stderr.log')
    $previewOwned=$true
}
try {
    Write-Host 'Current Player is the default. In PIE click the viewport and press F7 to toggle Player V1.'
    Write-Host 'Use E/M for real Cultivate. Stop/Play keeps the worker alive; each PIE starts on Current Player.'
    # This launcher intentionally opens the interactive Editor requested for human testing.
    $previewEditorProcess=Start-Process $previewEditor -ArgumentList @(('"'+$previewProject+'"'),('-CultivationBridgeDir="'+$BridgeDirectory+'"')) -WindowStyle Normal -PassThru
    $previewEditorProcess.WaitForExit()
} finally {
    if($previewOwned -and $previewWorker) {
        $previewRunning=Get-CimInstance Win32_Process -Filter "ProcessId=$($previewWorker.Id)"
        if($previewRunning -and $previewRunning.CommandLine.Contains($BridgeDirectory) -and $previewRunning.CommandLine.Contains('cultivation-bridge.js')) {Stop-Process -Id $previewWorker.Id}
        if(Test-Path -LiteralPath $previewLock) {
            $previewLockData=Get-Content -LiteralPath $previewLock -Raw|ConvertFrom-Json
            if($previewLockData.pid -eq $previewWorker.Id){Remove-Item -LiteralPath $previewLock}
        }
    }
}
