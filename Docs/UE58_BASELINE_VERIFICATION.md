# Unreal 5.8 baseline verification

Status: READY. Project generation, Development Editor build, and project-open checks passed. No Gate 1 gameplay has been added.

## Engine and project

- Engine: Unreal Engine 5.8.3, changelist 58210709; ++UE5+Release-5.8.
- Engine installation: C:\Program Files\Epic Games\UE_5.8
- Branch: feat/ue58-gate1-baseline
- Canonical checkout: C:\Users\Le Anh Hoang\Documents\Codex\2026-10-15\files-pasted-by-the-user-you\work\dao-vo-tan-3d-prototype
- Project: C:\Users\Le Anh Hoang\Documents\Codex\2026-10-15\files-pasted-by-the-user-you\work\dao-vo-tan-3d-prototype\Unreal\DaoVoTan3D.uproject

## Installed MSVC toolchains

Inventory used Visual Studio Installer's vswhere across all registered installations and inspected each MSVC directory's Hostx64/x64/cl.exe ProductVersion.

Selected, supported:
- Visual Studio 2022 Enterprise 17.14.37806.7.
- Toolchain directory: C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.44.35207
- cl.exe: C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe
- Actual ProductVersion: 14.44.35229.0.
- UBT family: 14.44.35207; UBT version: 14.44.35229; Error=False.

Present, rejected by UBT:
- Visual Studio Community 2026 18.2.11408.102.
- Toolchain directory: C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.50.35717
- cl.exe: C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64\cl.exe
- Actual ProductVersion: 14.50.35722.0.
- UBT family: 14.50.35717; UBT version: 14.50.35722; Error=True.

These were the two MSVC toolchain directories found. The previously observed 14.34 directory is no longer present. The toolset folder name and compiler ProductVersion differ; the actual compiler is 14.44.35229, not 14.44.35207.

UBT chose 14.44 automatically for both generation and build. No compiler override, engine patch or ban bypass was used.

## Validation results

1. Project generation: PASS, exit 0, Result: Succeeded. Generated .sln and .slnx.
2. Editor target: PASS, exit 0, Result: Succeeded, seven actions completed in 107.73 seconds total. UnrealEditor-DaoVoTan3D.dll and module metadata were produced. UBT explicitly reports Visual Studio 14.44.35229 toolchain and Windows SDK 10.0.26100.0.
3. Editor open: PASS. Launched using the original absolute .uproject path. Process 13580 loaded the compiled project module, logged Engine is initialized, loaded the engine's default OpenWorld template into the untitled editor world, and logged Total Editor Startup Time (55.515 seconds). A nonzero window handle and responsive window titled DaoVoTan3D - Unreal Editor were verified after startup. The Editor remains open. This is startup/module/map/window verification, not Gate 1 visual acceptance.

No missing SDK or required project plugin blocks the baseline. Incredibuild is absent but optional; local compilation succeeded. Low available memory reduced compile concurrency to one. UBT emitted a nonblocking include-order upgrade notice (current Unreal5_6, suggested Unreal5_8).

## Long-path correction

The requested build at the original long checkout path first failed on a generated action path at the 260-character limit. The same existing checkout was mapped to R: with Windows subst. No copy or second repository was created. After clearing only the generated Development/Makefile.bin cache, project generation and the build succeeded through R:\Unreal\DaoVoTan3D.uproject.

The R: alias remains mapped in this Windows session for subsequent builds. It is temporary and may need recreating after sign-out/restart. If R: is already occupied later, inspect it before changing any mapping. Do not remove an unrelated mapping.

Reproduction in PowerShell (R: must map to the canonical checkout):

```powershell
$repoRoot = 'C:\Users\Le Anh Hoang\Documents\Codex\2026-10-15\files-pasted-by-the-user-you\work\dao-vo-tan-3d-prototype'
# Only if R: is unused:
subst.exe R: $repoRoot
$engineRoot = 'C:\Program Files\Epic Games\UE_5.8'
$projectPath = 'R:\Unreal\DaoVoTan3D.uproject'
& "$engineRoot\Engine\Build\BatchFiles\Build.bat" -projectfiles "-project=$projectPath" -game -engine
& "$engineRoot\Engine\Build\BatchFiles\Build.bat" DaoVoTan3DEditor Win64 Development "-Project=$projectPath" -WaitMutex -NoHotReloadFromIDE
# Launch only after a successful build:
& "$engineRoot\Engine\Binaries\Win64\UnrealEditor.exe" (Join-Path $repoRoot 'Unreal\DaoVoTan3D.uproject')
```

## Preservation and scope

Existing tracked documentation/contracts remain unchanged. Prior untracked documentation/contracts and fixtures remain present. No gameplay source, simulation rules or proprietary assets were added. No toolchain or engine was installed by the agent. No commits or remote pushes were made. This report supersedes the earlier compiler-blocked baseline report.

Evidence beside the output copy of this report: project-generation.log, build.log, build-long-path.log, editor-launch.log, toolchains.json, editor-window.json.
