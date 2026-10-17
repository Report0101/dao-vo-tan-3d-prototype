# Technical validation and human retest

Final technical status: READY FOR HUMAN RETEST. Generation, Editor/Game builds, dedicated reconnect/duration/timing fixture, offline and live input fixtures, full Unreal regression and stress all PASS. The full Unreal report has 13 success entries: ten existing executed tests plus three fixture-specific informative returns; those three fixtures each execute and PASS separately. Stress has two successful tests. Simulation build/typecheck PASS; all 223 test cases are covered successfully across the serial run (222/223, one timeout) and the isolated rerun (all six MVP tests PASS, including the timed-out case at 11.417 seconds under its unchanged 20-second limit). This is not a claim that the final single full simulation invocation had zero failures. The earlier full simulation invocation also passed 223/223 before the final CLI scheduling adjustment. Timeout logs are retained.

Presentation start: d6932ffce2b499b8f2b3be66419679f5cb85a2db, branch feat/ue58-cultivation-bridge-stabilization.
Simulation start: b7774fe23473f695869d76a38dfe04d34bb75791, branch feat/cultivation-presentation-resync.

## Reconnect and exact durations

One CLI worker, PID 23336, remained alive throughout three StartPIE/EndPIE cycles in the same Editor process and was verified alive after the Editor exited. It was then stopped by its owning test runner. No worker restart or authoritative world reset occurred between sessions. These are controlled real PIE worlds using raw E/M input, not a claim of new human acceptance.

| PIE | Fresh stream identity | Days | Authoritative ticks | Minimum / observed start of Exit |
| --- | --- | --- | --- | --- |
| 1 | 1B446A4945960AD661F5F9BAF0F5303E | 1 | 1527 -> 1528 | 3.000 / 3.013 s |
| 2 | DBE1A25D428AE773FC7BB9A60CFDF943 | 7 | 1528 -> 1535 | 3.340 / 3.352 s |
| 3 | 3EB5863D40C9A4B9898FBAB8F92B9DD1 | 17 | 1535 -> 1552 | 4.540 / 4.549 s |
| 3 | same active session | 30 | 1552 -> 1582 | 6.000 / 6.000 s |

Every new session began at sequence 1, zero historical cues, zero old command previews and a controlled actor bound to the possessed pawn. World tick matched the preceding session's committed outcome. All 30 integer selections, clamping 0 -> 1 and 31 -> 30, and raw -/+ decrement/increment passed. JSON command.days and receipt dueTick-startTick matched 1/7/17/30 unchanged. The total authoritative advance was exactly 55 days and four actions. Repeated receipt consumption did not restart a preview or increment the result twice.

Fast completions arrived before the visual minimum in all four cases; the authoritative outcome was already committed while Unreal continued meditating. The component clock test held 30-day meditation beyond six seconds without a result, then resolved it and verified Exit -> Locomotion. Its extra cosmetic transitions are explicitly logged; they are not extra simulation commands.

## Regression evidence

The offline raw-key fixture proves E/M configuration feedback, active mappings, four separate animation replays, active-click suppression and recovery. Sequence starts were below .029 s and progressed above 1.37 s on all four strikes; the final phase was locomotion with four starts. The live fixture injects the actual Cultivate InputAction through Enhanced Input, then raw LMB/M/LMB and proves two authoritative completions and both sword replays. The older seven-day canonical fixture still proves duplicate-send completion once and WRONG_ACTOR rejection without a preview/time advance.

The canonical and live-input first-command world hashes match the accepted reference:
`c29ee1837b4e89a662fc7172f37b05014df57d5403341eba1df089992d664fa4`.
This hashes JSON.stringify(session.actions.live), excluding transport request identifiers. The 55-day reconnect fixture is intentionally a different world outcome; it is not compared against a seven-day reference.

Play-feel correction trace contains 2004 frames. Walk ground-contact to actual locomotion clip: .125641 s. Zero sampled blade/body-proxy intersections. Steering/camera, grip/path, landing recovery, Motion Matching, Chooser, foot IK, robe/hair, Niagara and portrait regression checks retain the existing assets and behavior. Sample source inventory matched all 3906 files (5,842,295,647 bytes). Proxy clearance does not constitute triangle-level or human art acceptance.

## Build and test accounting

UE 5.8.3 CL 58210709 at C:\Program Files\Epic Games\UE_5.8. Builds use supported MSVC 14.44.35229 from the 14.44.35207 directory and SDK 10.0.26100.0. Project generation, Development Editor and Development Game builds succeeded. Exact logs and test report indexes accompany the evidence bundle.

The full Unreal run includes fixture-specific informative returns for the two manual-input tests and StabilizationPIE; their actual assertions are executed in three dedicated runs. Do not count those returns as additional full test executions. Non-fatal startup Pose Search/index and external-service warnings remain visible in raw reports.

The first lifecycle test run mistakenly inspected a retiring PIE world during asynchronous End/Start; waiting for a new adapter identity corrected that test. The first live test assumed the sword counter started at zero despite a startup focus click; the final test asserts exact increments from its baseline and still checks real sequence progression. Both failed reports are retained. A concurrent simulation test run timed out under Unreal resource contention; the final simulation verification runs serially without increasing timeout limits. These test corrections do not change sword/runtime behavior.

Human retest: use the updated TypeScript worker (npm run build:p1), start the existing launcher once, run PIE/Cultivate/Stop three times without restarting it, select arbitrary days with -/+, and inspect readable meditation plus repeated sword use. Detailed steps are in Docs/animation-play-feel/PLAYTEST.md. Visual Gate 2 remains out of scope.
