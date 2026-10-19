param(
    [string]$SimulationRepository,
    [string]$EngineRoot='C:\Program Files\Epic Games\UE_5.8',
    [string]$EvidenceDirectory
)
$ErrorActionPreference='Stop'
$gateRoot=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
if(!$SimulationRepository){$SimulationRepository=Join-Path (Split-Path -Parent $gateRoot) 'tu-tien'}
$gateSimulation=(Resolve-Path -LiteralPath $SimulationRepository).Path
$gateRun=Join-Path $gateRoot ('Unreal/Saved/G1A-'+[guid]::NewGuid().ToString('N'))
$gateFixtures=Join-Path $gateRun 'fixtures'
$gateBridge=Join-Path $gateRun 'bridge'
$gateReport=Join-Path $gateRun 'automation'
if(!$EvidenceDirectory){$EvidenceDirectory=Join-Path $gateRun 'evidence'}
[IO.Directory]::CreateDirectory($EvidenceDirectory)|Out-Null
$gateNode=(Get-Command node -ErrorAction Stop).Source
$gateCli=Join-Path $gateSimulation 'dist-p1/cli/cultivation-bridge.js'
& $gateNode (Join-Path $gateSimulation 'dist-p1/cli/export-dynamic-agent-scenario.js') $gateFixtures $gateBridge
if($LASTEXITCODE -ne 0){throw 'G1A source/worker fixture creation failed'}
$gateWorker=Start-Process $gateNode -ArgumentList @(('"'+$gateCli+'"'),'watch',('"'+$gateBridge+'"')) -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $gateRun 'worker-stdout.log') -RedirectStandardError (Join-Path $gateRun 'worker-stderr.log')
$gateEditorProcess=$null
try {
    $gateArgs=@(('"'+(Join-Path $gateRoot 'Unreal/DaoVoTan3D.uproject')+'"'),'-unattended','-nosplash','-nop4',
        '-DynamicAgentBridgeEvidence',('-CultivationBridgeDir="'+$gateBridge+'"'),('-G1AFixtureDir="'+$gateFixtures+'"'),
        ('-G1AEvidenceDir="'+$EvidenceDirectory+'"'),'-ExecCmds="Automation RunTests DaoVoTan.Integration.DynamicAgents"',
        '-TestExit="Automation Test Queue Empty"',('-ReportExportPath="'+$gateReport+'"'),('-abslog="'+(Join-Path $gateRun 'editor.log')+'"'))
    $gateEditorProcess=Start-Process (Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor.exe') -ArgumentList $gateArgs -WindowStyle Hidden -PassThru
    if(!$gateEditorProcess.WaitForExit(300000)){throw 'G1A editor fixture timed out'}
    if($gateEditorProcess.ExitCode -ne 0){throw "G1A editor exit code $($gateEditorProcess.ExitCode); see $gateRun"}
    $gateResult=Get-Content -LiteralPath (Join-Path $gateReport 'index.json') -Raw|ConvertFrom-Json
    if($gateResult.failed -ne 0 -or $gateResult.notRun -ne 0){throw "G1A automation failed or did not run: $gateReport"}
    foreach($gateRequired in @('DaoVoTan.Integration.DynamicAgents.Protocol','DaoVoTan.Integration.DynamicAgents.PIE')){
        $gateEntry=@($gateResult.tests|Where-Object fullTestPath -eq $gateRequired)
        if($gateEntry.Count -ne 1 -or $gateEntry[0].state -notin @('Success','SuccessWithWarnings')){throw "Required G1A test missing/failed: $gateRequired"}
    }
    if($gateWorker.HasExited){throw 'Owned worker exited before runtime test finished'}
    $gateProof=Get-Content -LiteralPath (Join-Path $EvidenceDirectory 'runtime-proof.json') -Raw|ConvertFrom-Json
    if($gateProof.controlledPlayerCount -ne 1 -or $gateProof.npcProxyCount -ne 9 -or $gateProof.uniqueAgentIds -ne 10 -or $gateProof.duplicateBindings -ne 0){throw 'Runtime identity evidence mismatch'}
    Copy-Item -LiteralPath (Join-Path $gateReport 'index.json') -Destination (Join-Path $EvidenceDirectory 'automation-index.json')
    Copy-Item -LiteralPath (Join-Path $gateFixtures 'identity-proof.json') -Destination (Join-Path $EvidenceDirectory 'simulation-identity-proof.json')
    Write-Output "G1A LIVE WORKER + PIE PASS: $gateRun; evidence: $EvidenceDirectory"
} finally {
    if($gateEditorProcess -and !$gateEditorProcess.HasExited){Stop-Process -Id $gateEditorProcess.Id}
    $gateOwned=Get-CimInstance Win32_Process -Filter "ProcessId=$($gateWorker.Id)"
    if($gateOwned -and $gateOwned.CommandLine.Contains($gateBridge) -and $gateOwned.CommandLine.Contains('cultivation-bridge.js')){Stop-Process -Id $gateWorker.Id}
    $gateLock=Join-Path $gateBridge 'worker.lock'
    if(Test-Path -LiteralPath $gateLock){$gateLockData=Get-Content -LiteralPath $gateLock -Raw|ConvertFrom-Json;if($gateLockData.pid -eq $gateWorker.Id){Remove-Item -LiteralPath $gateLock}}
}
