param(
    [Parameter(Mandatory=$true)]
    [string]$RegistrationToken,
    [string]$RunnerRoot = "C:\actions-runner-dao-vo-tan-3d",
    [string]$RunnerName = "dao-vo-tan-3d-01"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$repoUrl = "https://github.com/Report0101/dao-vo-tan-3d-prototype"
$labels = "dao-vo-tan-3d,unreal"

if (Test-Path (Join-Path $RunnerRoot ".runner")) {
    throw "A runner is already configured at $RunnerRoot. Refusing to overwrite it."
}

New-Item -ItemType Directory -Force -Path $RunnerRoot | Out-Null

Write-Host "Resolving latest GitHub Actions runner release..."
$release = Invoke-RestMethod -Uri "https://api.github.com/repos/actions/runner/releases/latest" -Headers @{"User-Agent"="dao-vo-tan-runner-setup"}
$asset = $release.assets | Where-Object { $_.name -match '^actions-runner-win-x64-.*\.zip$' } | Select-Object -First 1
if (-not $asset) { throw "Could not find Windows x64 runner asset in latest release." }

$zip = Join-Path $env:TEMP $asset.name
Write-Host "Downloading $($asset.name)..."
Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $zip

Write-Host "Extracting runner to $RunnerRoot..."
Expand-Archive -Path $zip -DestinationPath $RunnerRoot -Force
Remove-Item $zip -Force -ErrorAction SilentlyContinue

Push-Location $RunnerRoot
try {
    Write-Host "Configuring repository-scoped runner..."
    & .\config.cmd --unattended --url $repoUrl --token $RegistrationToken --name $RunnerName --labels $labels --work _work --replace
    if ($LASTEXITCODE -ne 0) { throw "Runner configuration failed with exit code $LASTEXITCODE" }

    if (Test-Path .\svc.cmd) {
        Write-Host "Installing Windows service..."
        & .\svc.cmd install
        if ($LASTEXITCODE -ne 0) {
            Write-Warning "Service install failed. The runner is configured; run .\\run.cmd manually or rerun this script from an elevated PowerShell."
            exit 0
        }

        & .\svc.cmd start
        if ($LASTEXITCODE -ne 0) { throw "Runner service failed to start with exit code $LASTEXITCODE" }
        Write-Host "Runner service started."
    } else {
        Write-Warning "svc.cmd not found. Start the runner manually with .\\run.cmd"
    }
} finally {
    Pop-Location
}

Write-Host "Runner ready for $repoUrl"
Write-Host "Labels: self-hosted, Windows, X64, dao-vo-tan-3d, unreal"
