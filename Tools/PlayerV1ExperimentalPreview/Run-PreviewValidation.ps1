param(
    [string]$SimulationRepository,
    [string]$EngineRoot='C:\Program Files\Epic Games\UE_5.8',
    [switch]$InspectionCamera
)
$ErrorActionPreference='Stop'
$previewRoot=(Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$previewName='PlayerV1PreviewVerify-'+[DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')
$previewSaved=Join-Path $previewRoot 'Unreal/Saved'
$previewBridge=Join-Path $previewSaved ($previewName+'Bridge')
$previewReport=Join-Path $previewSaved ($previewName+'Automation')
if(!$SimulationRepository){$SimulationRepository=Join-Path (Split-Path -Parent $previewRoot) 'tu-tien'}
$previewCli=Join-Path (Resolve-Path -LiteralPath $SimulationRepository).Path 'dist-p1/cli/cultivation-bridge.js'
$previewNode=(Get-Command node -ErrorAction Stop).Source
$previewEditor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe'
if(Test-Path -LiteralPath $previewBridge){throw "Refusing stale fixture $previewBridge"}
& $previewNode $previewCli init $previewBridge
if($LASTEXITCODE -ne 0){throw 'Fixture init failed'}
$previewWorker=Start-Process $previewNode -ArgumentList @(('"'+$previewCli+'"'),'watch',('"'+$previewBridge+'"')) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $previewBridge 'stdout.log') -RedirectStandardError (Join-Path $previewBridge 'stderr.log')
try {
    $previewArgs=@(('"'+(Join-Path $previewRoot 'Unreal/DaoVoTan3D.uproject')+'"'),'-unattended','-nosplash','-nop4','-PlayerV1PreviewTest',('-CultivationBridgeDir="'+$previewBridge+'"'),'-ExecCmds="Automation RunTests DaoVoTan.Preview"','-TestExit="Automation Test Queue Empty"',('-ReportExportPath="'+$previewReport+'"'),('-abslog="'+(Join-Path $previewSaved ('Logs/'+$previewName+'.log'))+'"'))
    if($InspectionCamera){$previewArgs+='-PreviewInspectionCamera'}
    $previewProcess=Start-Process $previewEditor -ArgumentList $previewArgs -WindowStyle Hidden -PassThru
    $previewProcess.WaitForExit()
    if($previewProcess.ExitCode -ne 0){throw "Preview editor exited with code $($previewProcess.ExitCode)"}
    $previewResult=Get-Content -LiteralPath (Join-Path $previewReport 'index.json') -Raw|ConvertFrom-Json
    if($previewResult.failed -ne 0){throw "Preview tests failed: $previewReport"}
    foreach($previewRequired in @('DaoVoTan.Preview.AssetContract','DaoVoTan.Preview.LivePIE')){
        $previewEntry=@($previewResult.tests|Where-Object fullTestPath -eq $previewRequired)
        if($previewEntry.Count -ne 1 -or $previewEntry[0].state -notin @('Success','SuccessWithWarnings')){
            throw "Required preview test did not execute successfully: $previewRequired"
        }
    }
    if($previewWorker.HasExited){throw 'Worker exited before PIE lifecycle finished'}
    $previewCaptureFolder=if($InspectionCamera){'PlayerV1PreviewWideEvidence'}else{'PlayerV1PreviewEvidence'}
    Write-Output "PASS: $previewReport; live screenshots: $previewSaved/$previewCaptureFolder"
} finally {
    $previewOwned=Get-CimInstance Win32_Process -Filter "ProcessId=$($previewWorker.Id)"
    if($previewOwned -and $previewOwned.CommandLine.Contains($previewBridge) -and $previewOwned.CommandLine.Contains('cultivation-bridge.js')){Stop-Process -Id $previewWorker.Id}
    $previewLock=Join-Path $previewBridge 'worker.lock'
    if(Test-Path -LiteralPath $previewLock){$previewLockData=Get-Content -LiteralPath $previewLock -Raw|ConvertFrom-Json;if($previewLockData.pid -eq $previewWorker.Id){Remove-Item -LiteralPath $previewLock}}
}
