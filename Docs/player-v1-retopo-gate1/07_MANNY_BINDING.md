# Binding proof

All 15 final meshes use existing /Game/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin, 89 reference bones. No new skeleton or physics asset was created. Read-only reference-pose checks compare named component-space position/rotation/scale to the existing Manny mesh. Maximum errors: 0.000206735 cm, 0.000289629 degrees, 0.000005424 scale. No unknown bones, zero unweighted vertices.

Weight method: barycentric projection of nearest triangles from read-only Manny LOD0 for body/hands/boots/upper robe; controlled analytic head/neck, head/spine hair, pelvis/thigh/calf panels and belt/accessory weights. Normalize to max four influences. This is initial binding, not cinematic weight polish. Clothing differs from body weighting.

Read-only C++ validator evaluates the original accepted clip tracks in Unreal, computes imported LOD0 CPU linear-blend skinning at seven times/clip and exports the midpoint surface for inspection. This proves imported skin data against Manny and the existing SwordGrip transform; it does NOT instantiate a new runtime animation graph or claim candidate Motion Matching/foot-IK play acceptance. The current normal player's graph is exercised by the separate regressions.

Important limitations: wrist/finger grip is a blocker; shoulder/elbow/hip strain and garment collisions remain. Recovered importer bind-pose warnings were not treated as a pass on their own. The head1 collision failed the first validator and was fixed by Geo_ export node names, preserving the accepted skeleton.
