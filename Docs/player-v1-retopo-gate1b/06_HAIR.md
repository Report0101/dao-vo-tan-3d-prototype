# Player V1 Retopology Gate 1B — Identity & Deformation Recovery

Starting revision: e17d912ea653dc48f0e52141f8b792f1cfd10ed6. Branch: `feat/player-v1-retopo-gate1b`.
Experimental candidate remains `/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/`; production player is unchanged.

Evidence: `outputs/PLAYER_V1_RETOPO_GATE1B` in the task workspace; deliverable archive `PLAYER_V1_RETOPO_GATE1B_EVIDENCE.zip`.
Images of deformed meshes are inspection renders of UE-imported LOD0 vertices skinned from the accepted clips. They are **not live PIE captures**, runtime Animation Blueprint playback, final shaders, cloth or collision certification. Seven frames per clip are measured; three are exported/rendered. CPU linear-blend skinning is used. The reference pose is also inspected separately in Blender.

Hair reduced from 9,136 to **5,692 triangles**, inside the approximate 4k–7k guideline. Rebuilt lower-density cap/hairline, topknot, four flattened back masses and two front locks. One modular Hair asset, same Manny bones, head/spine weighting; no Groom, extra bones or physics.

Long tied black xianxia silhouette retained. Saved budget was reallocated primarily into useful face geometry/lapels. Front locks and cap are still coarse; back/shoulder clearance remains pose dependent. Inspect front/back/side, `14_hair_back`, `28_hair_side`, `meditation_back` and movement poses. No automatic collision or watertight contact certification is claimed.
