# Player V1 Retopology Gate 1B — Identity & Deformation Recovery

Starting revision: e17d912ea653dc48f0e52141f8b792f1cfd10ed6. Branch: `feat/player-v1-retopo-gate1b`.
Experimental candidate remains `/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/`; production player is unchanged.

Evidence: `outputs/PLAYER_V1_RETOPO_GATE1B` in the task workspace; deliverable archive `PLAYER_V1_RETOPO_GATE1B_EVIDENCE.zip`.
Images of deformed meshes are inspection renders of UE-imported LOD0 vertices skinned from the accepted clips. They are **not live PIE captures**, runtime Animation Blueprint playback, final shaders, cloth or collision certification. Seven frames per clip are measured; three are exported/rendered. CPU linear-blend skinning is used. The reference pose is also inspected separately in Blender.

15 modules; **46,248 triangles** (accepted 41,894). Within 33k–52k total guidance.

|Module|Triangles|Slots|
|---|---:|---:|
|Body_Underbody|4030|1|
|UpperRobe|6340|2|
|Hands|3592|1|
|Head|10914|3|
|Boots|3264|1|
|Hair|5692|1|
|LowerRobe_FrontL|1736|1|
|LowerRobe_FrontR|1736|1|
|LowerRobe_BackL|1736|1|
|LowerRobe_BackR|1736|1|
|LowerRobe_SideL|1736|1|
|LowerRobe_SideR|1736|1|
|Belt|1536|1|
|Sash|224|1|
|Accessory_Jade|240|1|

Major groups: body/head/hands/boots 21,800; robe including six panels 16,756; hair 5,692; accessories 2,000. Robe is 756 over the approximate 16k sub-guideline due to inner lapels; total remains in range. Total imported sections 18; one slot on other modules, Head three, UpperRobe two.

UV0 retained on unchanged modules and repaired/repacked where topology changed. All 15 modules: zero positive-area UV overlap pairs and zero zero-area UV triangles in current inspection. No final textures or texel-density polish. New head openings and lapels increase island count. Finite normals/tangents and no authoring non-manifold/boundary edges/zero-area faces; small sliver triangles remain (see topology-quality.json), so this is not production topology approval or geometric self-intersection certification. Multiple disconnected closed shells are intentional on face/hair and layered robe.
