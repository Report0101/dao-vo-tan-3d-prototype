$ErrorActionPreference='Stop'
$editorExe='C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$entry=(Resolve-Path '.\work\tu-tien\dist-p1\cli\cultivation-bridge.js').Path
$node=(Get-Command node).Source
function Run-Editor([string]$name,[string[]]$extra) {
 $editorArgs=@('R:\Unreal\DaoVoTan3D.uproject','-unattended','-nosplash','-nop4','-TestExit="Automation Test Queue Empty"',"-ReportExportPath=R:\Unreal\Saved\${name}Automation","-abslog=R:\Unreal\Saved\Logs\${name}PIE.log")+$extra
 $p=Start-Process $editorExe -ArgumentList $editorArgs -WindowStyle Hidden -PassThru
 $p.Id|Set-Content ".\work\$name-pid.txt"
 Write-Output "$name Editor PID $($p.Id)"
 $p.WaitForExit()
 $r=Get-Content "R:\Unreal\Saved\${name}Automation\index.json" -Raw|ConvertFrom-Json
 if($r.failed -ne 0){throw "$name FAILED tests=$($r.failed)"}
 Write-Output "$name PASS success=$($r.succeeded) warningSuccess=$($r.succeededWithWarnings)"
}
function Fixture([string]$name,[string[]]$extra) {
 $dir="R:\Unreal\Saved\${name}Bridge"
 if(Test-Path -LiteralPath $dir){throw "Refusing to reuse old fixture: $dir"}
 & $node $entry init $dir
 if($LASTEXITCODE -ne 0){throw 'init failed'}
 $worker=Start-Process $node -ArgumentList @(('"'+$entry+'"'),'watch',$dir) -WindowStyle Hidden -PassThru -RedirectStandardOutput "$dir\stdout.log" -RedirectStandardError "$dir\stderr.log"
 $worker.Id|Set-Content ".\work\$name-worker-pid.txt"
 try {
  Run-Editor $name ($extra+@("-CultivationBridgeDir=$dir"))
  if($worker.HasExited){throw 'Worker died during PIE lifecycle'}
  "workerPid=$($worker.Id) aliveAfterEditor=true"|Set-Content "$dir\worker-lifecycle-proof.txt"
 } finally {
  $proc=Get-CimInstance Win32_Process -Filter "ProcessId=$($worker.Id)"
  if($proc -and $proc.CommandLine -like "*${name}Bridge*"){Stop-Process -Id $worker.Id}
  $lock="$dir\worker.lock"
  if((Test-Path -LiteralPath $lock) -and ((Get-Content -LiteralPath $lock -Raw|ConvertFrom-Json).pid -eq $worker.Id)){Remove-Item -LiteralPath $lock}
 }
}
Fixture 'RetopoGate1BFinalFull' @('-CultivationGate2Test','-CorrectionLabel=retopo-gate1b-final','-ExecCmds="Automation RunTests DaoVoTan"')


Fixture 'RetopoGate1BFinalReconnect' @('-StabilizationTest','-ExecCmds="Automation RunTests DaoVoTan.Cultivation.StabilizationPIE"')
