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
        $candidate = Get-ChildItem $root -ErrorAction SilentlyContinue |
            Where-Object { $_.PSIsContainer -and $_.Name -like "UE_*" } |
            Sort-Object Name -Descending |
            Select-Object -First 1
        if ($candidate) { return $candidate.FullName }
    }

    throw "Unreal Engine installation not found. Set UE_ROOT to the engine root, e.g. C:\Program Files\Epic Games\UE_5.5"
}

function Invoke-ProjectFileGeneration {
    param(
        [Parameter(Mandatory=$true)][string]$EngineRoot,
        [Parameter(Mandatory=$true)][string]$Project
    )

    $generateBat = Join-Path $EngineRoot "Engine\Build\BatchFiles\GenerateProjectFiles.bat"
    if (Test-Path $generateBat) {
        Write-Host "Generating project files with GenerateProjectFiles.bat..."
        & $generateBat -project="$Project" -game -engine
        if ($LASTEXITCODE -ne 0) { throw "GenerateProjectFiles failed with exit code $LASTEXITCODE" }
        return
    }

    $ubtExe = Join-Path $EngineRoot "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"
    if (Test-Path $ubtExe) {
        Write-Host "GenerateProjectFiles.bat not present. Using UnrealBuildTool.exe -ProjectFiles..."
        & $ubtExe -ProjectFiles -Project="$Project" -Game -Rocket -Progress
        if ($LASTEXITCODE -ne 0) { throw "UnrealBuildTool project generation failed with exit code $LASTEXITCODE" }
        return
    }

    $ubtDll = Join-Path $EngineRoot "Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll"
    if (Test-Path $ubtDll) {
        Write-Host "Using dotnet UnrealBuildTool.dll -ProjectFiles..."
        & dotnet $ubtDll -ProjectFiles -Project="$Project" -Game -Rocket -Progress
        if ($LASTEXITCODE -ne 0) { throw "UnrealBuildTool.dll project generation failed with exit code $LASTEXITCODE" }
        return
    }

    throw "No supported project-file generator found under $EngineRoot"
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$project = (Resolve-Path (Join-Path $repoRoot $ProjectPath)).Path
$engineRoot = Resolve-UnrealEngineRoot

$buildBat = Join-Path $engineRoot "Engine\Build\BatchFiles\Build.bat"
$editorCmd = Join-Path $engineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

Write-Host "UE_ROOT=$engineRoot"
Write-Host "PROJECT=$project"

if (-not (Test-Path $buildBat)) { throw "Build.bat not found: $buildBat" }

if (-not $SkipGenerateProjectFiles) {
    Invoke-ProjectFileGeneration -EngineRoot $engineRoot -Project $project
}

Write-Host "Building $Target ($Configuration, Win64)..."
& $buildBat $Target Win64 $Configuration -Project="$project" -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) { throw "Unreal build failed with exit code $LASTEXITCODE" }

if (-not (Test-Path $editorCmd)) { throw "UnrealEditor-Cmd.exe not found: $editorCmd" }

Write-Host "Build succeeded. Running headless boot smoke test..."
& $editorCmd $project -game -nullrhi -unattended -nosplash -nosound -NoPause -ExecCmds="quit"
if ($LASTEXITCODE -ne 0) { throw "Headless boot smoke test failed with exit code $LASTEXITCODE" }

Write-Host "VS1 native build + headless boot smoke test PASSED"
