# Reproduce Gate 1 validation from the canonical checkout.
param([switch]$Stress)
$ErrorActionPreference = 'Stop'
$engineRoot = 'C:\Program Files\Epic Games\UE_5.8'
$repoRoot = Split-Path $PSScriptRoot -Parent
if (-not (Test-Path 'R:\')) {
    subst.exe R: $repoRoot
    if ($LASTEXITCODE -ne 0) { throw 'Cannot establish R: alias' }
} else {
    $canonicalProject = Join-Path $repoRoot 'Unreal\DaoVoTan3D.uproject'
    if ((Get-FileHash $canonicalProject).Hash -ne (Get-FileHash 'R:\Unreal\DaoVoTan3D.uproject').Hash) { throw 'R: does not match this project; inspect alias manually' }
}
$argsForEditor = @('R:\Unreal\DaoVoTan3D.uproject','-unattended','-nosplash','-nop4','-ExecCmds="Automation RunTests DaoVoTan.Gate1.PlayableSlice"','-TestExit="Automation Test Queue Empty"')
if ($Stress) { $argsForEditor += @('-GateNPCs=10','-GateStress','-ReportExportPath=R:\Unreal\Saved\Gate1Automation-Stress','-abslog=R:\Unreal\Saved\Logs\Gate1PIEStress.log') }
else { $argsForEditor += @('-ReportExportPath=R:\Unreal\Saved\Gate1Automation','-abslog=R:\Unreal\Saved\Logs\Gate1PIEFinal.log') }
$process = Start-Process "$engineRoot\Engine\Binaries\Win64\UnrealEditor.exe" -ArgumentList $argsForEditor -PassThru -Wait
exit $process.ExitCode
