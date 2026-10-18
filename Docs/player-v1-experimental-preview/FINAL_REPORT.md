# Player V1 experimental preview — final report

**READY FOR HUMAN PLAYER V1 PREVIEW.**

This approves preview integration only. Player V1 remains NOT READY for production default or Weight/Polish. No deformation correction or Visual Gate 2 was begun.

## Revision
Starting SHA dfac6897983adc58406a820928eae70801902833. Branch feat/player-v1-experimental-preview. Final delivery SHA is recorded in evidence REVISION.json and the delivery response; no self-referencing commit hash is embedded here. No push or merge.

## Architecture and controls
F7 in a focused PIE viewport toggles Current Player <-> Player V1 Experimental. F8 remains Eject. The existing AppearanceComponent owns both the latest accepted recipe reference and the local preview selection. One new profile: /Game/DaoVoTan/Experimental/PlayerV1/DA_PlayerV1Experimental. Same GateCharacter pawn, Manny leader/skeleton (89 bones), controller, movement, Motion Matching/Pose Search/Chooser/foot IK, sword, aura/portrait and Cultivate pipeline.

Fifteen skeletal followers use the original Manny Leader Pose; no extra animation instance or gameplay class. Seven simple material-instance overrides resolve pre-existing null mesh material interfaces without saving changes into the candidate meshes. The actor's accepted Recipe, simulation identity and command state are not replaced. F7 is editor-PIE-only; non-editor builds expose neither key nor badge. Each PIE starts on Current Player. No save/protocol schema changes.

WASD move, Shift walk/run, Q strafe, Space jump, mouse camera, LMB sword, E/M Cultivate, -/+ duration. E/M requires the existing real worker. Start-LivePreview.ps1 opens the same project with its bridge argument and keeps the worker alive across PIE. It can infer a sibling tu-tien checkout or accept -SimulationRepository. A plain project open does not silently start a worker or fake cultivation.

## Live PIE
Observed via actual runtime PIE backbuffer: Current Player, experimental Idle/back/front/side, Walk, Run, Strafe, Pivot, Jump, Landing, sword carry/guard, attack, meditation and restored Current Player. Input uses the actual controller binding and existing movement/actions. Screenshots include the preview HUD badge and existing portraits.

The three-session normal-camera exercise passed. Each new PIE uses a fresh transport session, starts on Current Player, and retains the same persistent worker world. F7 preserves pawn/controller/movement/animation instance/recipe identity, current actor ID, socket transform and in-flight request payload/ID. Four real 1/7/17/30-day commands advance world tick 1527 -> 1528 -> 1535 -> 1552 -> 1582. Repeated toggle-back restores the accepted appearance without accumulating instance components.

Additional wide observer evidence lowers only the automation camera focus during meditation so the garment can be inspected above the HUD. The normal meditation camera evidence is retained separately. Production camera and all animation clips are unchanged. The wide capture pass is one PIE session; the normal preview and existing reconnect tests separately prove three sessions.

## Known visual issues — unchanged
Sword hand remains BLOCKER. Robe deformation remains MAJOR. Meditation deformation remains MAJOR. Face/sculpt/hair are not final, with pose-dependent intersections. This integration intentionally exposes those defects and does not relabel them as passes. Source Gate 1B meshes, weights, UVs, clips, sword/socket/transform and production default recipe are untouched. No QCBH assets or original Meshy reimport.

## Performance sanity
Candidate LOD0: 15 registered visible/recently-rendered modules, 18 skeletal sections, 46,248 triangles. Hidden Manny leader/NPCs are excluded from these candidate counts. FPS sampling includes the scene/editor/worker; it is not a whole-scene draw-call count or GPU benchmark.

|Same 1218x646 PIE view|Mean ms|Mean FPS|p95 ms|
|---|---:|---:|---:|
|current|16.650|60.06|18.379|
|experimental|16.671|59.98|18.410|

Short warm idle samples use wall intervals between automation updates at fixture-only 550 cm framing. Both observations are near the editor's 60 FPS cap. No optimization or production frame-budget claim.

## Regression and preservation
Development Editor and Development Game builds PASS on UE 5.8.3 CL58210709 / MSVC 14.44.35229, directory 14.44.35207 / SDK 10.0.26100.0. The final camera-only fixture edit is excluded from non-editor code. Existing BlendStack declaration warning remains unchanged.

Full suite: 15 Success reports, including four fixture guards. Dedicated Offline/Live input, original stabilization reconnect, actual Player V1 three-session preview and wide inspection fixtures all pass with zero failures. Pose Search async-index warm-up warnings are preserved in their reports; they are not suppressed or called warning-free.

Accepted/baseline/final Full-fixture canonical actions.live SHA256 is identical: c29ee1837b4e89a662fc7172f37b05014df57d5403341eba1df089992d664fa4. This compares the normalized authoritative action payload, not whole files containing different transport session IDs. Simulation repository remains clean at 57d84b3dec06d22fe3f250902db7d7cbe79c54f7.

FBX SHA256 remains faa4aada36218026646fbaccc8e28226629797f8d89b5eb543ca58d13b5261b3. The separate T-pose PNG is preserved and unused. All 15 original candidate meshes and DA_Player pass byte-for-byte preservation checks; no tracked existing content asset changes. Initial driver timing/material/framing trials are retained separately from final evidence.

See LIVE_PREVIEW.html, live-pie/, live-inspection/, asset-mapping.json, material-audit.json, preservation.json, authority-agreement.json and regression-results.json in the evidence archive. Human physical-key retest remains the purpose of this delivery; automated PIE success is not acceptance of the known visual quality.
