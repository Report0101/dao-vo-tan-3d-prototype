# Player V1 Mesh Feasibility Audit — final report

## Verdict

**B — USABLE AFTER RETOPOLOGY / MODULAR REBUILD.**

**NOT READY FOR PLAYER V1 PRODUCTION GATE.** The shape is useful; the raw mesh is not a directly skinnable modular player. Fused fingers, robe/hair connectivity, absent UV/materials and face/joint topology require retopo plus partial remaking. License and generation/reference provenance remain unverified. No Player V1 production implementation or Visual Gate 2 was started.

## Revision and baseline

Start: 88f6f6f086325cd18596c645be2d9628cecb80b1. Branch: feat/player-v1-mesh-feasibility. Final SHA is recorded in the delivered revisions.json and final chat response; the report is itself part of that commit. No main change, merge or push.

Baseline status: no modified tracked files. Exactly two untracked ArtSource files were investigated: supplied immutable FBX and T-pose PNG. The user confirmed the PNG must be preserved but not used. Both remain unchanged and untracked. No simulation repository file or revision changed.

UE 5.8.3 CL 58210709, C:\Program Files\Epic Games\UE_5.8. Compiler directory 14.44.35207, actual cl ProductVersion 14.44.35229.0; UBT selected 14.44.35229 / SDK 10.0.26100.0. Banned 14.50.35722 was not used. Baseline and final Development Editor/Game builds PASS. No project-generation change was required: no C++/module/plugin configuration was changed. Python plugins were enabled only on the audit commandlet invocation.

Full Unreal baseline PASS (13 reported successes, including three fixture-specific informative returns); ten existing tests actually execute there. Separate offline/live input fixtures PASS. Coverage includes accepted animation/play-feel, grip/path/landing/steering/camera correction, four sword replays and E/M with the real worker. Seven-day Cultivate, duplicate once, WRONG_ACTOR rejection and snapshot semantics pass. Existing production gameplay was not changed by this static audit, so these results are preserved rather than represented as Meshy deformation tests. Logs retain non-fatal initialization warnings.

## Source preservation

Exact source: C:\Users\Le Anh Hoang\Documents\Codex\2026-10-15\files-pasted-by-the-user-you\work\dao-vo-tan-3d-prototype\ArtSource\Meshy_AI_Jade_Warden_1017231731_generate.fbx

Size: 3,264,956 bytes. Binary FBX 7400 (7.4). Created 2026-10-18 01:17:37.4687506 +02:00; modified 01:17:38.3901759 +02:00.

SHA256 before: `faa4aada36218026646fbaccc8e28226629797f8d89b5eb543ca58d13b5261b3`

SHA256 after: `faa4aada36218026646fbaccc8e28226629797f8d89b5eb543ca58d13b5261b3`

Byte-identical. Original remains untracked; only matching working/import copies were read by tools. The filename and embedded Meshy/mblend writer metadata support origin attribution, not commercial rights. Generation job, account tier, prompt and license proof were unavailable; no stronger rights claim is made.

## Raw structure

Two FBX object records: one Mesh model and one mesh Geometry. 51,339 vertices, 102,722 triangle polygons, one connected surface. Zero open boundary edges, >2-face edges, repeated-index triangles, zero-area faces, exact duplicate vertex positions, inconsistent directed edges or non-cycle vertex links in the tested indexed topology. Supplied normals are finite/unit and none oppose their face winding. Geometric self-intersection certification was not performed.

No skeleton, bones, skin clusters/weights, animation takes, morph targets, UVs, source materials, textures, embedded media or vertex colors. No tangents/binormals. Unreal import warns about tangent bases with absent UVs; that issue was not silently repaired. The default import material slot is fallback, not authored content.

Header axes Y-up/+Z front, unit centimetres; model rotation X=-90 degrees, local scale 100. Applied hierarchy resolves to Z-up, 190.0123 cm high, 154.3691 cm wide, 56.0545 cm deep. Origin is around mid-height. Raw local dimensions alone would misstate unit scale. Unreal import preserves authored normals and uses static/reference type, no Nanite, no collision and no lightmap-UV generation. Assets are confined to /Game/DaoVoTan/Experimental/PlayerV1/MeshyAudit/.

## Manny fit

Existing SKM_Manny_Simple reference only, not altered. Bounds height approximately 180.5439 cm. Non-destructive Unreal global fit: scale .950169519, rotation (0,0,0), translation (0,0,90.422100) cm plus fixture-only side offsets. Fit category **B: moderate fitting required** for the retained silhouette. The rigid overlay shows why one transform cannot solve near-T versus A-pose arms or separate joint locations. The source has no homologous joints; hidden pelvis/knees and hair-covered head anatomy are not assigned invented numerical ratios. Exact Manny joints/chains, raw bounding ratios, export/bounds discrepancy and measurement limitations are in 03_MANNY_PROPORTION_COMPARISON.md / proportion-metrics.json.

## Problem areas and production decisions

- Face/head: RETOPO / RESCULPT; recognizable silhouette but soft/faceted eye/mouth/nose and no verified expression/cavity structure. Portrait quality requires more than smoothing.
- Shoulders/elbows: irregular generated triangle flow, sleeve volume and cuff pinch; rebuild bend loops and match Manny rest pose before weight polish.
- Hands: REBUILD; thumb plus fused finger-like strips cannot articulate five fingers around the jian.
- Hair: rebuild larger controlled clumps, retain topknot/long-back design; current broad sheet/ragged tips approach the robe. Cosmetic bones later, no Groom/physics now.
- Robe: recommendation B, skinned upper robe plus separate lower panels. Rebuild panel cuts, underbody, knee/hip clearance and intentional interiors. No cloth simulation.
- Legs/boots: covered knees cannot be evaluated as joints; retopo boots and fit ankle/ball/sole to Manny. No fabricated motion clearance.

Topology classification: **REQUIRES PARTIAL REMAKE**. Optional temporary skinning was NOT forced. Consequently the Meshy Idle/Walk/Run/Strafe/pivot/Jump/Landing/Sword/meditation deformations are all **not tested**, not passed. The existing Manny tests are not substituted for these missing deformation proofs.

## Architecture and cost

Retain Manny -> existing Motion Matching/Pose Search/Chooser/IK -> one AppearanceRecipe/appearance owner. Map body/head/hair/garment/accessory design to that existing system after clean assets exist. Existing recipe variants are not already arbitrary skeletal panel slots; any later extension belongs in that one catalog, not a second Character or locomotion stack. No Meshy paths in gameplay C++ or TypeScript, no production recipe/default/map mutations.

Raw cost: 102,722 triangles, 51,339 control points and 308,166 polygon corners; GPU split-vertex count was not fabricated where the commandlet subsystem was unavailable. Initial hero LOD0 target: body/head/hands/boots 18–26k, hair 4–7k, robe 10–16k, accessories 1–3k; total 33–52k triangles. These are proposed targets, not generated optimized assets or performance claims. Reduce broad low-information surfaces while retaining portrait/finger/silhouette detail.

Next pipeline, requiring a separate production authorization: provenance/license evidence -> retopo head/underbody -> remake hands and lower robe -> rebuild hair clumps -> UV/material/bake -> Manny rest-pose fit -> bind/weights -> isolated ten-pose acceptance -> AppearanceRecipe integration -> Player V1 production gate.

## Evidence

See 00_SOURCE.md through 08_RECOMMENDATION.md and evidence/*.json. Screenshots are neutral inspection materials, not supplied source textures. Front, side, back, 3/4, Manny side-by-side and overlay, shoulder/hand/wire/lower-robe/hair/head views are retained. No image of a deformation is claimed because no skinning proof exists. Working .blend/reference copies remain in the output bundle, not production content. Original ArtSource files, normal player/defaults, Manny, jian, recipes and Stillwater content remain intact.

## Final regression verification

PlayerV1FinalFullRetry completed: 9 successes + 4 warning-successes, 0 failures (13 reported entries; fixture-specific informative returns are not additional executed tests). Warnings are external HTTP connectivity / Epic telemetry, not assertion failures. The earlier final run was interrupted and produced no report; its log is retained. The R: alias was restored to the verified canonical checkout and a fresh worker fixture was used, without reusing or resetting any prior session.

Accepted / baseline / final actions.live JSON SHA256 all match: c29ee1837b4e89a662fc7172f37b05014df57d5403341eba1df089992d664fa4. See evidence/authority-agreement.json. Source FBX and preserved-unused PNG checksums were rechecked unchanged. Simulation HEAD remains 57d84b3dec06d22fe3f250902db7d7cbe79c54f7 with no working-tree changes.
