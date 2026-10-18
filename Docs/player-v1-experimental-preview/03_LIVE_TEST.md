# Live PIE validation

PlayerV1PreviewTest.cpp adds DaoVoTan.Preview.AssetContract and the dedicated real-worker DaoVoTan.Preview.LivePIE test. The latter requires -PlayerV1PreviewTest and -CultivationBridgeDir. Normal full suites report an informative guard for this test; the dedicated fixture executes it separately.

The three-session exercise starts each PIE on Current Player, routes simulated F7 press/release through the actual controller input binding, and checks the same pawn/controller/movement/animation instance/current recipe/simulation actor/session/sword transform. It counts 15 followers / 18 sections / 46,248 triangles and verifies every component material binding and Leader Pose link. NPC selection remains unchanged.

Session 1 records current, experimental idle/back, front, side, walk, run, strafe, pivot, jump, landing, sword attack, sword carry/guard and authoritative meditation. Movement uses existing input actions; camera views use the same camera, with fixture-only 550 cm framing. No test clip is substituted into the runtime graph. Screenshots read the actual PIE backbuffer including HUD.

Appearance is switched during sword and Cultivate to verify the existing action/request is not restarted or replaced. E and M submit real worker commands. Four completed commands cover 1/7/17/30 days across three distinct PIE sessions, preserving the worker world tick across Stop/Play. Toggle back restores the accepted appearance without accumulating instance components. Each session ends with preview selected so the next session's default reset is explicitly tested.

An initial test driver sent M in the same frame that animation returned to locomotion, before the adapter's recovery tick. That run was canceled and retained as a trial; the driver now waits for recovery before sending the next command. No Cultivate implementation was changed. The next functional run passed but exposed unbound materials; those captures are retained separately. Final evidence uses the profile's component material overrides.

Performance uses short warm idle samples at the same scene/location/view, measuring wall time between automation updates. World delta alone was deliberately replaced because it can reflect a fixed simulation interval. This is a sanity observation including editor/worker overhead, not GPU timing or a benchmark. Exact samples, viewport size, mean/median/p95 and FPS are in evidence performance.json.

Run the committed Run-PreviewValidation.ps1 with -SimulationRepository to create a fresh owned worker fixture and execute the preview tests. It assumes the Editor target is already compiled. Existing full, animation, sword, Offline/Live Cultivate and reconnect regressions are also run separately for delivery; see FINAL_REPORT.md and evidence automation reports for actual results and warnings.

An additional -PreviewInspectionCamera pass keeps fixture framing at 550 cm during meditation, lowers the observer focus during meditation and writes PlayerV1PreviewWideEvidence separately. Normal-camera evidence is retained alongside it. This is observer framing only; the production meditation camera and clips are unchanged. The validation helper exposes this as -InspectionCamera.

The wide inspection option runs one capture session. Run the default validation option for the full three-session reconnect exercise. Neither option changes production camera settings.
