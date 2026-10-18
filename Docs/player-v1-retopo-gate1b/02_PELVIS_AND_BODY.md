# Player V1 Retopology Gate 1B — Identity & Deformation Recovery

Starting revision: e17d912ea653dc48f0e52141f8b792f1cfd10ed6. Branch: `feat/player-v1-retopo-gate1b`.
Experimental candidate remains `/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/`; production player is unchanged.

Evidence: `outputs/PLAYER_V1_RETOPO_GATE1B` in the task workspace; deliverable archive `PLAYER_V1_RETOPO_GATE1B_EVIDENCE.zip`.
Images of deformed meshes are inspection renders of UE-imported LOD0 vertices skinned from the accepted clips. They are **not live PIE captures**, runtime Animation Blueprint playback, final shaders, cloth or collision certification. Seven frames per clip are measured; three are exported/rendered. CPU linear-blend skinning is used. The reference pose is also inspected separately in Blender.

## Geometry
Reduced crotch saddle depth from 4.5 cm to 1.5 cm, rebuilt upper-thigh entry rings and hip transition, kept a shared groin bridge with separated legs. Arm cross-sections now follow neighboring center tangents through the elbow. No extra hidden anatomy was added.

## Weights and result
Continuous analytic pelvis/spine/limb blends replace nearest-surface transfer on the underbody. A trial closest-segment switch near the elbow produced discontinuities; it was replaced by a continuous joint-bisector projection. A hard arm/torso mask in an earlier trial was also removed. Failed iterations are retained in the evidence rather than reported as final.

Underbody worst sampled edge stretch: 3.793x -> 2.728x. Rest topology no longer has the deep pointed saddle; inspect `16_underbody_front`, `17_underbody_back`, `22_underbody_pelvis`, `31_pelvis_side` and per-pose pelvis images. The clothed silhouette still exhibits thigh/panel interaction in movement. Underbody numeric improvement does not resolve the whole hip/clothing assembly or certify all self-intersections.
