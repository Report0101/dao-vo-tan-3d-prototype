# Player V1 Retopology Gate 1B — Identity & Deformation Recovery

Starting revision: e17d912ea653dc48f0e52141f8b792f1cfd10ed6. Branch: `feat/player-v1-retopo-gate1b`.
Experimental candidate remains `/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/`; production player is unchanged.

Evidence: `outputs/PLAYER_V1_RETOPO_GATE1B` in the task workspace; deliverable archive `PLAYER_V1_RETOPO_GATE1B_EVIDENCE.zip`.
Images of deformed meshes are inspection renders of UE-imported LOD0 vertices skinned from the accepted clips. They are **not live PIE captures**, runtime Animation Blueprint playback, final shaders, cloth or collision certification. Seven frames per clip are measured; three are exported/rendered. CPU linear-blend skinning is used. The reference pose is also inspected separately in Blender.

## Construction
Reframed shoulder/elbow cross-sections, separate outward garment shell with 3 mm edge thickness, continuous axial/arm weights, and two geometric inner lapels joined into the same UpperRobe module. Two material slots; no new appearance owner.

## Stretch
Worst sampled edge ratio 5.137x -> 3.697x across the same ten clips/seven times. Body/robe hard spatial and nearest-segment switches were diagnosed with exported worst-edge coordinates and removed. The remaining peak occurs during high-motion poses and still requires visual review; a threshold alone is not a pass.

Jump/landing/sword attack remain structural rework cases. The elbow can crease and sleeves/shoulders remain bulky. Inspection renders `torso_Run`, `torso_Jump`, `torso_SwordAttack`, all pose sequences and `08_shoulder_armpit` / `09_elbow` document this. No smoothing-only acceptance and no claim of final weight polish.
