# Gate 1B final report

**B — FOUNDATION IMPROVED, FURTHER STRUCTURAL REWORK REQUIRED**

**NOT READY FOR PLAYER V1 WEIGHT/POLISH GATE.** No next gate begun.

## Revision and source
Starting SHA `e17d912ea653dc48f0e52141f8b792f1cfd10ed6`; branch `feat/player-v1-retopo-gate1b`. Final delivery commit is recorded in evidence `REVISION.json` (avoids a self-referencing commit hash). Source before/after SHA256 `faa4aada36218026646fbaccc8e28226629797f8d89b5eb543ca58d13b5261b3`.
Intentional untracked FBX remains unchanged. The separate untracked T-pose PNG is preserved and unused.

## Geometry and identity
15 modules / 46,248 triangles: body/head/hands/boots 21,800; robe 16,756; hair 5,692; accessories 2,000. Imported 18 sections. Existing Manny skeleton (89 UE reference bones), UV0 and existing modular architecture retained.

Rebuilt head grid, eye/mouth loops and closed backing, eyes/brows/ears; lower-density hair cap, knot and broad long clumps; hip/groin/thigh rings; shoulder/elbow frames; layered robe lapels; panel overlap/root weights; digit pad/taper adjustments. Continuous body/robe weights remove discontinuous shoulder and elbow switches discovered in trials. No original Meshy topology reused.

Head identity is improved, but procedural nose/eyelid/ear integration and likeness sculpt remain limited. Hair retains long tied black silhouette, reduced from 9,136 to 5,692 triangles; coarse cap/locks and pose-dependent shoulder/back clearance remain.

## Hands and sword
**Guard BLOCKER / Attack BLOCKER.** Five fingers remain aligned with Manny, neutral/wrist views are provided, but fingers still hang open below the unchanged handle. Digit changes did not solve closure. Guard vs Idle midpoint finger differences are small (max 0.484 cm / 4.292 degrees). No sword transform/socket/geometry/animation changes, pose swaps or static fist shortcuts were made. A compatible closed grip remains unresolved; this gate does not authorize changing the accepted animation architecture.

## Pelvis, upper and lower robe
Groin saddle reduced and upper-thigh entry rebuilt; underbody strain improved. Crotch/hip assembly and clothing clearance still require visual structural work. Upper robe gained geometric layering and continuous shoulder/elbow weights, but Jump/Landing/Attack remain rework cases.

|Peak sampled edge stretch|Gate 1|Gate 1B|
|---|---:|---:|
|Underbody|3.793x|2.728x|
|Upper robe|5.137x|3.697x|
|Hands|1.769x|1.841x|
|Six lower panels|2.151x|3.484x|

Lower panels retain six-part architecture, with wider overlap and pelvis/thigh root blending, no calf weights or cloth. **Panel strain worsens** with the shorter root transition. Walk/run/strafe still show lift/openings and meditation is not accepted. Numeric improvement in the body cannot offset this failure.

## Ten-pose matrix
Idle MINOR ISSUE; Walk MAJOR ISSUE; Run MAJOR ISSUE; Strafe MAJOR ISSUE; Pivot 180 MAJOR ISSUE; Jump MAJOR ISSUE; Landing MAJOR ISSUE; Sword Guard BLOCKER; Sword Attack BLOCKER; Meditation MAJOR ISSUE.

Seven frames measured for every accepted clip, three frames rendered. No original MAJOR/BLOCKER was relabeled as passed. UV inspection: all 15 modules have zero positive-area UV overlaps / zero zero-area UV triangles; finite normals/tangents, zero unweighted vertices. Sliver triangles and intentional multiple shells are documented. No full self-intersection certification or final texture/shader claim.

## Architecture and regression
Production player remains unchanged. Existing Manny, one locomotion stack, one AppearanceRecipe owner. No simulation worker, authority, protocol, camera, production sword, GameMode/DefaultPawn edits. Boots/Belt/Sash/Jade preserved byte-for-byte. Simulation repository remains clean at `57d84b3dec06d22fe3f250902db7d7cbe79c54f7`.

Development Editor and Development Game builds succeeded with UE 5.8.3 CL58210709, MSVC directory 14.44.35207 / cl.exe ProductVersion 14.44.35229.0 and SDK 10.0.26100.0. Existing BlendStack dependency warning remains; plugin declarations are outside this gate and were not changed.

Full regression: 13 Success reports, of which three are fixture guards; dedicated Offline/Live/Reconnect runs separately exercise those paths. Dedicated Offline/Live/Reconnect all passed (each SuccessWithWarnings, zero failures). Detailed final results and warning counts are in `metrics/regression-results.json`. Reconnect verifies three new PIE sessions with persistent world state and 1/7/17/30-day requests. Pose Search async-index warnings are preserved in the automation reports, not suppressed.

Authority `actions.live` canonical SHA256: `c29ee1837b4e89a662fc7172f37b05014df57d5403341eba1df089992d664fa4`, identical accepted/baseline/final Full fixtures. This hashes the same normalized action payload, not whole session files with differing session IDs.

## Evidence limitations and remaining work
Open `BEFORE_AFTER.html` in the evidence folder/archive. Candidate posed views are neutral Blender inspection renders of real UE imported/skinned vertices, not live PIE or final material screenshots. The accepted clips and current sword transform are exported by the commandlet. Runtime production gameplay is tested independently by PIE automation. Raw/component methods agree to <0.000001 scalar stretch on a same-candidate cross-check.

Remaining priorities: plausible closed grip without violating Manny alignment or accepted sword constraints; structural panel/waist/meditation clearance; residual upper-robe and hip deformation; dedicated identity sculpt/hair fit. These are not cosmetic polish tasks. No production integration, push or merge performed.
