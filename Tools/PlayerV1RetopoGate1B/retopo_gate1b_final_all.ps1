$ErrorActionPreference='Stop'
$engineRoot='C:\Program Files\Epic Games\UE_5.8'
& "$engineRoot\Engine\Build\BatchFiles\Build.bat" DaoVoTan3DEditor Win64 Development '-Project=R:\Unreal\DaoVoTan3D.uproject' -WaitMutex -NoHotReloadFromIDE *> .\work\retopo-gate1b-final-editor.log
if($LASTEXITCODE -ne 0){throw 'Editor build failed'}
& "$engineRoot\Engine\Build\BatchFiles\Build.bat" DaoVoTan3D Win64 Development '-Project=R:\Unreal\DaoVoTan3D.uproject' -WaitMutex -NoHotReloadFromIDE *> .\work\retopo-gate1b-final-game.log
if($LASTEXITCODE -ne 0){throw 'Game build failed'}
& (Join-Path $PSScriptRoot 'run_retopo_gate1b_final.ps1')
