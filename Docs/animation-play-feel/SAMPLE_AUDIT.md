# Game Animation Sample: scoped read-only audit

Source project: `C:\Users\Le Anh Hoang\OneDrive\Documents\Unreal Projects\GameAnimationSample\GameAnimationSample.uproject`.
Its EngineAssociation is `5.8`; this PC runs UE **5.8.3, CL 58210709** from `C:\Program Files\Epic Games\UE_5.8`. No other sample checkout was searched or used.

Accepted destination baseline: `feat/ue58-visual-gate1-1`, `b57f0e39dba329bfa660fa9a391cb89b403c511e`. Implementation branch: `feat/ue58-animation-play-feel`. The canonical presentation checkout and StillwaterValley are retained.

The audit mounts the source content read-only in an inspection process launched against the destination project. It exports graph/property and database dependency information to destination Saved files, without saving any sample package. A full before/after SHA256 inventory covers the sample .uproject, Config and Content. The sample Editor itself is never launched. Installed engine code is consulted for UE 5.8 API signatures; it is not patched.

## Locomotion findings and minimum adaptation

| System | Actual source evidence | Destination approach |
|---|---|---|
| Motion Matching | `Blueprints/SandboxCharacter_CMC_ABP`, MotionMatching node and `Update_MotionMatching` / post-selection functions; sample settings include 0.5s blend, 0.3s pose history and 0.85–1.15 play rate | Native AnimInstance with a real MotionMatching node, bounded databases and blended cosmetic action branch; no sample character Blueprint |
| Pose Search | `Characters/UEFN_Mannequin/Animations/MotionMatchingData/Databases/Sparse`, plus standing idle, turn, jump and landing databases; `Schemas/PSS_Default` has trajectory and pose feature groups at 30Hz | Retarget selected clips to the existing Manny skeleton; author one local schema using trajectory and foot/pelvis position/velocity, and six state/gait databases |
| Choosers | `CHT_PoseSearchDatabases`, nested Sparse chooser filters on movement mode, stance, state and gait | Author one six-row Chooser using local airborne/landing/moving/turning/gait inputs; separate Walk/Run databases prevent cross-gait selection; no crouch, traversal, interaction or Mover rows |
| Trajectory | `Update_Trajectory`, history collector, CharacterMovement-based prediction; sample also has a separate Mover path | CharacterMovement intent/history prediction, gravity/floor correction for the animation query only; never moves the pawn or advances simulation time |
| Start/stop | Sparse standing walk/run Start and Stop databases | Keep directional right-foot transitions, excluding redundant opposite-foot duplicates and box/hourglass/diamond exercises |
| Pivots | Sparse standing walk/run Pivot databases; `IsPivoting` function | Retain directional pivot coverage in grounded search; trajectory changes and current pose choose transitions |
| Turn in place | Standing TurnInPlace database, `ShouldTurnInPlace`, offset-root and steering nodes | Four 90/180 degree turns, selected by the turning state while stationary; camera-facing mode is a local toggle |
| Strafe/omnidirectional | Eight-direction loops, rotation-mode state, relative direction and orientation-warping graphs | Keep directional walk/run coverage and CharacterMovement camera-facing option; do not copy the full orientation/steering/per-sample graph framework |
| Jump/fall/landing | Jump and idle-land databases, `JustLanded_Light/Heavy`, `Get_LandVelocity` | Five jump/fall clips and four light landings with actual falling/landing detection; no traversal or heavy gameplay reaction framework |
| Foot placement / IK | FootPlacement node uses pelvis, ik_foot_root, foot/ball bones and `contact_l` / `contact_r` curves. A RemapCurves node converts each contact to `(1-contact)*100` before FootPlacement, followed by LegIK. Source also contains `CR_Biped_FootPlacement` | Retain both curves on every clip; a small native conversion node, native foot placement and two leg IK solves on the existing skeleton. No CurveExpression plugin or full Control Rig asset migration |
| Acceleration / lean | `CalculateRelativeAccelerationAmount`, `Get_LeanAmount`, lean blendspaces and additive application | Bounded local acceleration lean on spine_02; no imported lean asset family or gameplay acceleration authority |

The full sample also contains offset-root accumulation, steering, orientation warping, gait/stance frameworks and an experimental state machine. These were inspected as locomotion dependencies, but are not wholesale copied. The first bounded adaptation keeps animation root motion available to Pose Search and discards it at the AnimInstance boundary. CharacterMovement remains responsible for local capsule movement. This is not a claim that every advanced sample feature has been reproduced.

## Asset boundary

`selection.json` explicitly names **86** source sequences: 72 grounded walk/run loops/transitions/pivots/stops, 1 idle, 4 standing turns, 5 jump/fall clips and 4 light landings. Export uses the sample's `RTG_UEFN_to_UE5_Mannequin` only as a read-only offline retarget reference. Destination clips use the existing Manny skeleton; the source mesh, retarget rigs, sample map, sample character/controller, widgets, Mover, ragdoll, traversal, interactions, audio, Foley/gameplay notify Blueprint classes and MetaHumans are excluded.

The scope is a bounded initial set, not a mathematical claim that no further clip could be removed. Each retained directional category supplies coverage missing from the old speed-only blendspace. Asset counts, hashes, destination dependencies and actual selected poses are recorded after migration and testing.

The raw inspection process uses the destination's configuration, not the sample's GameplayTags configuration, so loading referenced source Foley notifies reports missing source-only tags. Those notify classes are stripped from destination clips. An early audit probe used a nonexistent skeleton alias; the actual source skeleton is `Characters/UEFN_Mannequin/Meshes/SK_UEFN_Mannequin`, confirmed by the source AnimBP TargetSkeleton and retarget mesh. This probe error is not a missing sample dependency.

## Authority and preserved presentation

AppearanceRecipe, original modular robe/head/hair/accessories, jian assets, Niagara cultivation aura, portrait pipeline and CultivationCommandAdapter remain in the destination. Animation reads local movement and cosmetic action phases. It does not accept commands, create EventIds/ActionIds, grant cultivation, alter life/death or advance strategic time. The real TypeScript simulation round-trip remains a mandatory regression check.

Human play-feel, camera comfort and garment clipping review remain separate from automated input/pose-selection checks.
