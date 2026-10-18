param(
    [switch]$VerboseReport
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
Push-Location $repoRoot
try {
    $required = @(
        'AGENTS.md',
        'Docs/ARCHITECTURE.md',
        'Docs/PLAYABLE_FIRST_V2.md',
        'Docs/CHARACTER_QUALITY_GATE.md',
        'Docs/V1_EXECUTION_GATE.md',
        'Docs/V1_GATE_MANIFEST.json',
        'Bridge/contracts/README.md',
        'Unreal/DaoVoTan3D.uproject',
        'Unreal/Source/DaoVoTan3D/DaoVoTan3D.Build.cs'
    )

    $missing = @($required | Where-Object { -not (Test-Path $_) })
    if ($missing.Count -gt 0) {
        throw "V1 gate is missing required files: $($missing -join ', ')"
    }

    try {
        $manifest = Get-Content 'Docs/V1_GATE_MANIFEST.json' -Raw | ConvertFrom-Json
    }
    catch {
        throw "V1 gate manifest is not valid JSON: $($_.Exception.Message)"
    }

    if ($manifest.gate -ne 'VS-V1-PLAYER-Q1') {
        throw "Unexpected V1 gate id: $($manifest.gate)"
    }

    $generatedDirs = @(
        'Unreal/Binaries',
        'Unreal/DerivedDataCache',
        'Unreal/Intermediate',
        'Unreal/Saved'
    )
    $committedGenerated = @()
    foreach ($dir in $generatedDirs) {
        if (Test-Path $dir) {
            $tracked = @(git ls-files -- $dir 2>$null)
            if ($tracked.Count -gt 0) {
                $committedGenerated += $dir
            }
        }
    }
    if ($committedGenerated.Count -gt 0) {
        throw "Generated Unreal output is tracked: $($committedGenerated -join ', ')"
    }

    $scanFiles = @(
        Get-ChildItem 'Unreal/Source','Unreal/Config' -Recurse -File -ErrorAction SilentlyContinue |
            Where-Object { $_.Extension -in @('.h','.hpp','.cpp','.cs','.ini') }
    )

    $forbidden = @(
        'Report0101/tu-ti-n',
        '\bHistoricalSimulator\b',
        '\bCultivationStore\b',
        '\bBodyConditionStore\b',
        '\bFactionStore\b',
        '\bMaterialHoldingsStore\b',
        '\bOpportunityStore\b',
        '\bDeathStore\b'
    )

    $boundaryHits = @()
    foreach ($file in $scanFiles) {
        $text = Get-Content $file.FullName -Raw
        foreach ($pattern in $forbidden) {
            if ($text -match $pattern) {
                $boundaryHits += "$($file.FullName.Substring($repoRoot.Length + 1)) :: $pattern"
            }
        }
    }
    if ($boundaryHits.Count -gt 0) {
        throw "Simulation-boundary leak detected:`n$($boundaryHits -join [Environment]::NewLine)"
    }

    $largeSourceWarnings = @()
    foreach ($file in $scanFiles | Where-Object { $_.Extension -in @('.h','.hpp','.cpp') }) {
        $lineCount = @(Get-Content $file.FullName).Count
        if ($lineCount -gt 900) {
            $largeSourceWarnings += "$($file.FullName.Substring($repoRoot.Length + 1)) ($lineCount lines)"
        }
    }

    $managerWarnings = @()
    foreach ($file in $scanFiles | Where-Object { $_.Extension -in @('.h','.hpp','.cpp') }) {
        if ($file.BaseName -match '(?i)GameManager|GlobalManager|MasterManager') {
            $managerWarnings += $file.FullName.Substring($repoRoot.Length + 1)
        }
    }

    Write-Host 'V1 STATIC GUARD: PASS'
    Write-Host "Gate: $($manifest.gate)"
    Write-Host "Scanned source/config files: $($scanFiles.Count)"
    Write-Host 'Authority boundary: clean'
    Write-Host 'Generated-output tracking: clean'

    if ($largeSourceWarnings.Count -gt 0) {
        Write-Warning "Large source files need anti-spaghetti review: $($largeSourceWarnings -join '; ')"
    }
    if ($managerWarnings.Count -gt 0) {
        Write-Warning "Manager-style names need ownership review: $($managerWarnings -join '; ')"
    }

    if ($VerboseReport) {
        Write-Host 'NOTE: This script verifies structural/authority guardrails only.'
        Write-Host 'Full V1 PASS still requires Player Q1 visual evidence and playable PIE/dev-build evidence.'
    }
}
finally {
    Pop-Location
}
