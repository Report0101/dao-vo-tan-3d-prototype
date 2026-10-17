# Manual input + action state fix

Status: READY FOR HUMAN MANUAL RE-TEST. Visual Gate 2 has not started.
Branch: fix/ue58-manual-input-actions. Starting commit: ee7a15e.

## Cultivate diagnosis and fix

Physical E/M reached the callback before the fix. Ordinary PIE lacked the external bridge configuration and controlled-actor snapshot, so the first missing step was request creation. Its small feedback was unclear. E/M mappings were unchanged between 97cdbe3 and ee7a15e; reverting the correction would not repair this flow. The older authority test used a configured fixture and injected an InputAction, bypassing physical mappings and ordinary PIE configuration.

E and M remain separate Boolean runtime InputActions, IA_Command_E and IA_Command_M, in the command context at priority 10. Each has one Started command binding; Completed only logs. The pawn context is priority 0. Raw controller logging delegates unchanged to Super. Missing configuration/worker/snapshot now produces prominent CONFIG UNAVAILABLE / WORKER UNAVAILABLE / SIMULATION NOT READY and an input counter, without inventing success. A worker lock detects absence, not stale-lock liveness.

Physical post-fix PIE used normal transport, without test/developer-preview flags. E -> raw controller -> EnhancedInput Started -> adapter -> inbox -> TypeScript worker -> accepted/completed receipt -> meditation -> locomotion. E request 25C0CC064D20482FF30762A6C92A226A completed at tick 1534, one session. After four sword attacks, physical M request 98EB0B0245AEECF78604289B16AC54FA completed at tick 1541, two sessions. Screenshots and logs accompany this report. These were actual keyboard/mouse events delivered through computer-use, not direct adapter calls.

## Sword diagnosis and fix

Before the first attack the phase was Ready/Locomotion. After recovery it correctly returned to Locomotion; subsequent clicks reached Sword again. The animation proxy retained the same ActionClip for blend-out and only reset sequence time when its asset pointer changed. The second attack therefore remained at the end of the same clip. It was an animation restart defect, not a permanently locked enum.

Every accepted action phase now increments ActionRevision; the proxy resets sequence time when that revision changes, including repeated use of the same asset. The existing Ready -> Attack -> Recovery -> Ready gate is retained. Active clicks are ignored; there is no queue or combo.

Four physical attacks restarted AS_SwordAttackA with revisions 4/5/6/7, each returned to phase 0. A fifth attack after M restarted revision 11 and recovered. Automated raw-key tests additionally prove actual sequence progression on all four attacks, with initial times below .024 seconds and later times above 1.39 seconds, and prove that rapid active clicks create no extra attack. This checks replay, not just enum transitions.

## Validation

UE 5.8.3 CL 58210709, C:\Program Files\Epic Games\UE_5.8. MSVC directory 14.44.35207, cl.exe ProductVersion 14.44.35229.0, UBT compiler 14.44.35229, Windows SDK 10.0.26100.0.

- Project generation PASS (17.11 s).
- Final Development Editor build PASS (8.03 s); an earlier test compile failed and was corrected before this successful build.
- Development Game build PASS (124.40 s).
- New offline raw-key/mapping/repeated-animation test PASS in its dedicated fixture.
- New live actual Cultivate InputAction / raw sword / M coexistence test PASS in its dedicated worker fixture.
- Full suite: ten existing tests PASS; two new fixture-specific entries return informative skips here and were executed separately above. The full report lists 12 successful entries; this is not twelve executions of their full assertions.
- Stress test PASS. Dedicated/stress reports contain non-fatal warnings, including transient Pose Search index initialization and external service connectivity; exact warnings are retained. No test errors.
- Previous landing/steering/grip/path/camera tests PASS. Walk/run landing-to-clip latency .161632/.148933 s; zero sampled blade/body-proxy intersections. Proxy clearance is not triangle-level or human art acceptance.
- Duplicate Cultivate still completes once; wrong actor is rejected. The one-command canonical, live-input and physical E outcomes are compared with the accepted authority reference in authority-agreement.json. A legitimate subsequent M advances another seven days, so its final world is not compared against the one-command hash.

No simulation source, presentation assets, grip/path, landing timing, steering, camera, Motion Matching databases or foot IK were changed. The sample remains read-only. Human play-feel acceptance remains pending.

## Human retest

Use Tools/RunCultivationCommandDemo.ps1 with the existing TypeScript checkout for real Cultivate. In StillwaterValley PIE focus the viewport, press E, wait for completion/presentation, press M, then LMB four times with recovery between clicks. Test rapid clicks during a strike, sword -> Cultivate and Cultivate -> sword. Normal PIE without the launcher must show configuration feedback when E/M is pressed. Retain the landing, steering, grip and blade-path checks in Docs/animation-play-feel/PLAYTEST.md.
