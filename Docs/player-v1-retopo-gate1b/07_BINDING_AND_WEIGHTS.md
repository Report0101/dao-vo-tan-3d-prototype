# Player V1 Retopology Gate 1B — Identity & Deformation Recovery

Starting revision: e17d912ea653dc48f0e52141f8b792f1cfd10ed6. Branch: `feat/player-v1-retopo-gate1b`.
Experimental candidate remains `/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/`; production player is unchanged.

Evidence: `outputs/PLAYER_V1_RETOPO_GATE1B` in the task workspace; deliverable archive `PLAYER_V1_RETOPO_GATE1B_EVIDENCE.zip`.
Images of deformed meshes are inspection renders of UE-imported LOD0 vertices skinned from the accepted clips. They are **not live PIE captures**, runtime Animation Blueprint playback, final shaders, cloth or collision certification. Seven frames per clip are measured; three are exported/rendered. CPU linear-blend skinning is used. The reference pose is also inspected separately in Blender.

Existing `SK_Mannequin` reference skeleton, **89 UE reference bones** on all 15 imported assets. Blender has 88 deform bones; importer/reference root accounts for the reported count difference. No second gameplay skeleton. All modules reference-compatible, zero unweighted imported vertices; authoring weights normalized, maximum four influences. Exact transform errors and slot/section data are in `engine/engine-deformation.json`.

Body/upper robe: continuous analytic spine/arm/leg weights. Hands: Manny barycentric surface transfer, geometry adjusted locally. Panels: pelvis/thigh gradient. Hair: head/spine. Rigid accessories: existing binding. Boots/Belt/Sash/Jade asset binaries preserved unchanged.

`-ComponentPose` evaluates the accepted animation using a temporary Manny SkeletalMeshComponent single-node instance, then skins imported LOD0 vertices. Read-only `-BeforeRoot` mounts the accepted commit's 15 mesh packages exported using `git show`; no old checkout/reset or extra project is created. Temporary actors are destroyed. Original raw-track method remains for historical cross-check. Largest scalar stretch difference on the same candidate between raw and component modes was below 0.000001; original failures are valid, not dismissed as retargeting artifacts.

OBJ exports include real section material labels and frames at 1/6, 1/2, 5/6. Neutral inspection normals are recomputed only for render presentation; measured positions and geometry are not altered. Full runtime graph/physics/cloth not simulated by this validator.
