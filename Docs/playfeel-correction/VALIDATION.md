# Correction verification

Starting SHA: `97cdbe38f0d82bae848de15335439b6a5cc1de14`.
Branch: `feat/ue58-playfeel-correction`.
Human acceptance of the starting technical baseline failed. This correction is for human retest, not Visual Gate 2.

## Environment and build

Existing project: `C:\Users\Le Anh Hoang\Documents\Codex\2026-10-15\files-pasted-by-the-user-you\work\dao-vo-tan-3d-prototype\Unreal\DaoVoTan3D.uproject`. R: is the same checkout.

UE 5.8.3 CL 58210709; engine `C:\Program Files\Epic Games\UE_5.8`. MSVC directory 14.44.35207, compiler ProductVersion 14.44.35229.0, UBT selects 14.44.35229; SDK 10.0.26100.0. No compiler ban or engine code modified.

Final generation PASS (16.11 seconds), Editor PASS (135.89 seconds), Game PASS (86.38 seconds). UBT retains the existing advisory that BlendStack is a module dependency without an explicit project plugin entry; the plugin is available transitively and both targets build. No missing toolchain/SDK/plugin blocker.

Full suite: **10/10 PASS**, all Success, zero failures/warning-bearing tests. This includes all nine original tests and the new correction sequence. The additional hardened-name correction test and fresh-process stress results are recorded in the delivered reports.

Fresh-process stress: 2/2 PASS (1 Success, 1 SuccessWithWarnings). The tightened clip-name test: 1/1 PASS with external HTTP/EOS warnings, zero failures. Its Editor rebuild passed in 108.87 seconds. That final source change affects only test classification, not runtime or Game code; the full suite's previous binary has identical runtime behavior. Warning logs are retained.

Final hardened trace: ground contact to actual walk clip **0.161246 s**, actual run clip **0.145525 s**, versus baseline **0.492347 s / 0.513547 s**. Each gait is measured separately with input held. The database switches earlier than the visible clip; the CSV records both. The hardened predicate matches `M_Neutral_Walk_` / `M_Neutral_Run_`, excluding a jump clip whose name contains Run. Recovery to below five-percent action blend measures **0.297071 s stationary / 0.216673 s moving**, versus **0.362861 / 0.366668 s** before. The added impact beat remains visible; no claim of instantaneous recovery.

The final extended trace and hardened rerun each report zero blade intersections against the sampled simplified body volumes. Additional camera-direction and slow/diagonal/short/fall landing images are included. These measurements support readiness for human retest only.

The correction sequence now covers walk/run maintained through landing, slow walk, diagonal jump, short jump, longer fall, camera yaw with forward/four diagonal directions, Q strafe, reversal, stationary and moving attack, return to locomotion and meditation. Finite/reach checks guard leg transforms. Screenshots and per-frame CSV supplement state assertions.

## Simulation and preservation

Real Cultivate request `1D76D2414CF394F76AD393B954DB469E`: action `command1:action:00000001`, cultivation event `command1:event:00000044`, tick 1527 to 1534 (seven strategic days). Repeated sends produce one logical completion and one cosmetic sequence; WRONG_ACTOR rejected.

World-save SHA256 `c29ee1837b4e89a662fc7172f37b05014df57d5403341eba1df089992d664fa4` matches accepted Integration Gate 2. The TypeScript repository, command adapter and receipt implementation are unchanged. Owned test workers are stopped after use; no simulation authority, command, damage or gameplay system enters animation.

The exact GameAnimationSample reference remains read-only: all 3,906 inventoried .uproject/Config/Content files, 5,842,295,647 bytes, are identical before/after. No sample migration in this pass. No QCBH assets. Existing AppearanceRecipe, modular meshes, head/hair, jian mesh, aura, portraits and valley remain; only the two project-authored sword animation assets and landing database are updated.

## Visual checks and limitations

Actual PIE images show the blade outside the torso during inspected guard/anticipation/slash/recovery and moving attack views. Dense offline sampling and the final runtime sequence report zero intersections against simplified body proxies. These are not triangle-level collision proofs and do not guarantee every possible input/pose combination. Source/destination wrist-retarget mismatch is not the cause: both sword animations use the original Manny skeleton and are authored locally.

Rigid robe panels still overlap legs during deep landing compression; sleeve seams and raised-shoulder joins remain visible, and the prototype glove does not have individually authored finger closure. No robe architecture or art library was redesigned. Foot placement/IK remain; automated finite/reach checks and images cannot certify perfect sole locking, zero sliding or subjective comfort. These remain explicit items for human retest.

Performance samples are short, capped Editor PIE observations on Intel Iris Xe/i7-11370H/16GB at 1218x646. They are not shipping benchmarks or proof of optimization. The 11-character scene is stationary. The three-character final sample had two active Niagara instances versus three at the prior baseline, so its timing is not a fully matched workload.

See PLAYTEST.md for the six-item PLAY-FEEL CORRECTION RETEST checklist. Do not infer human acceptance from technical PASS.
