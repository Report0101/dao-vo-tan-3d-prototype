# Player V1 Retopology Gate 1B — Identity & Deformation Recovery

Starting revision: e17d912ea653dc48f0e52141f8b792f1cfd10ed6. Branch: `feat/player-v1-retopo-gate1b`.
Experimental candidate remains `/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/`; production player is unchanged.

Evidence: `outputs/PLAYER_V1_RETOPO_GATE1B` in the task workspace; deliverable archive `PLAYER_V1_RETOPO_GATE1B_EVIDENCE.zip`.
Images of deformed meshes are inspection renders of UE-imported LOD0 vertices skinned from the accepted clips. They are **not live PIE captures**, runtime Animation Blueprint playback, final shaders, cloth or collision certification. Seven frames per clip are measured; three are exported/rendered. CPU linear-blend skinning is used. The reference pose is also inspected separately in Blender.

## Baseline
UE 5.8.3 CL58210709 at `C:/Program Files/Epic Games/UE_5.8`. MSVC directory 14.44.35207, actual cl.exe ProductVersion 14.44.35229.0; UBT selects this family. SDK 10.0.26100.0. No banned 14.50 compiler used.
Baseline Editor/Game builds and Full/Offline/Live fixtures passed before editing. Initial tracked tree was clean. Only two intentional untracked inputs existed: Meshy source FBX and `ArtSource/T-pose character 2D.png`. Both preserved; the PNG was not used in this audit.

## Boundary
Only candidate geometry/material instances/isolated validation map, authoring source and validation code/docs changed. No simulation, protocol, Motion Matching, production sword/socket/transform, camera, production appearance defaults, GameMode or DefaultPawn edits. No new gameplay skeleton/character/locomotion/appearance owner. No cloth, Groom, physics or QCBH assets. No push/merge and no next gate.

Raw Meshy FBX is visual/design reference only. Its topology was not reused. New surfaces are internal experimental reconstructions; source rights are not newly certified by this gate.
