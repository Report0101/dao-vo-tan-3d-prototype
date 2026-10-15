# Gate 1 discovery before changes

Date: 2026-10-15 (Europe/Budapest). Repository: `Report0101/dao-vo-tan-3d-prototype`; local branch `main`, starting commit `9fa034e82fe9fd1538174f11f9e1e16314e23c40`. **Prepared specification/contracts only; Unreal runtime unbuilt and unvalidated.**

| Area | Status at discovery | Evidence |
|---|---|---|
| Architecture and visual direction | IMPLEMENTED as documentation | README, Docs/ARCHITECTURE.md, VERTICAL_SLICE_V0_1.md, VISUAL_DIRECTION.md |
| Unreal project/version | MISSING / version UNKNOWN | Repository has only six tracked text files; no .uproject/Source/Content/Config |
| Camera, locomotion, sword presentation | MISSING | No code/Blueprint/animation assets |
| Valley/map | MISSING | No .umap or environment assets |
| Character/rig/modular appearance | MISSING | No meshes, skeletons, textures or appearance implementation |
| Cultivation/aura/UI/portrait | MISSING | Direction exists in docs; runtime implementation absent |
| Bridge | PARTIAL, designed only | Bridge/contracts/README.md lists intended families; no implementation or transport |
| Simulation integration | MISSING | Core is separately owned; no live adapter |
| Plugins | MISSING project configuration | No .uproject plugin list; no installed dependency claim |
| Build | MISSING, not attempted | No Unreal project/compiler available |

Environment discovery: UnrealEditor/UnrealBuildTool not on PATH; common C:/D: Epic locations and engine registry entries did not identify an engine. No .uproject was found in searched Documents/OneDrive Documents/Desktop roots. This is not proof no engine exists anywhere. Blender 5.2 directory exists, but Blender was not launched. `game-dev --version` fails: command not found. No software installed or credentials requested.

Selected repository is the existing presentation repository, not the simulation repo. No duplicate Unreal project was fabricated. Existing architecture allows offline mocks and separates presentation from simulation; this gate narrows the larger v0.1 scope to the user's player+2 NPC+valley target. Do not add the older spec's cave/combat-win/economy features to Gate 1.

Changes prepared in this pass: versioned presentation-only schemas; three synthetic appearance fixtures; three aura parameter variants; mock action-presentation requests; pure contract checks; art/rig/animation/camera/performance/provenance documents. These are not .uasset/.umap content, a rig, an animation library or a playable build.
