param(
    [string]$ProjectPath = "Unreal/DaoVoTan3D.uproject",
    [string]$Target = "DaoVoTan3DEditor",
    [string]$Configuration = "Development",
    [switch]$SkipGenerateProjectFiles
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

function Resolve-UnrealEngineRoot {
    if ($env:UE_ROOT -and (Test-Path $env:UE_ROOT)) {
        return (Resolve-Path $env:UE_ROOT).Path
    }

    $registryCandidates = @(
        "HKLM:\SOFTWARE\EpicGames\Unreal Engine",
        "HKCU:\SOFTWARE\Epic Games\Unreal Engine\Builds"
    )

    foreach ($root in $registryCandidates) {
        if (Test-Path $root) {
            try {
                $children = Get-ChildItem $root -ErrorAction SilentlyContinue
                foreach ($child in $children) {
                    $props = Get-ItemProperty $child.PSPath -ErrorAction SilentlyContinue
                    foreach ($name in @("InstalledDirectory", "(default)")) {
                        $candidate = $props.$name
                        if ($candidate -and (Test-Path $candidate)) {
                            return (Resolve-Path $candidate).Path
                        }
                    }
                }
            } catch {
            }
        }
    }

    $roots = @(
        "C:\Program Files\Epic Games",
        "D:\Program Files\Epic Games",
        "D:\Epic Games",
        "E:\Epic Games"
    )

    foreach ($root in $roots) {
        if (-not (Test-Path $root)) { continue }
        $candidate = Get-ChildItem $root -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like "UE_*" } |
            Sort-Object Name -Descending |
            Select-Object -First 1
        if ($candidate) { return $candidate.FullName }
    }

    throw "Unreal Engine installation not found. Set UE_ROOT to the engine root, e.g. C:\Program Files\Epic Games\UE_5.5"
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$project = (Resolve-Path (Join-Path $repoRoot $ProjectPath)).Path
$engineRoot = Resolve-UnrealEngineRoot

$buildBat = Join-Path $engineRoot "Engine\Build\BatchFiles\Build.bat"
$generateBat = Join-Path $engineRoot "Engine\Build\BatchFiles\GenerateProjectFiles.bat"
$editorCmd = Join-Path $engineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

Write-Host "UE_ROOT=$engineRoot"
Write-Host "PROJECT=$project"

if (-not (Test-Path $buildBat)) { throw "Build.bat not found: $buildBat" }

if (-not $SkipGenerateProjectFiles) {
    if (-not (Test-Path $generateBat)) { throw "GenerateProjectFiles.bat not found: $generateBat" }
    & $generateBat -project="$project" -game -engine
    if ($LASTEXITCODE -ne 0) { throw "GenerateProjectFiles failed with exit code $LASTEXITCODE" }
}

& $buildBat $Target Win64 $Configuration -Project="$project" -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) { throw "Unreal build failed with exit code $LASTEXITCODE" }

if (-not (Test-Path $editorCmd)) { throw "UnrealEditor-Cmd.exe not found: $editorCmd" }

Write-Host "Build succeeded. Running headless boot smoke test..."
& $editorCmd $project -game -nullrhi -unattended -nosplash -nosound -NoPause -ExecCmds="quit"
if ($LASTEXITCODE -ne 0) { throw "Headless boot smoke test failed with exit code $LASTEXITCODE" }

Write-Host "VS1 native build + headless boot smoke test PASSED"
