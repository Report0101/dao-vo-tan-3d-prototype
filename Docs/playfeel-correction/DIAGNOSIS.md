# Play-feel correction: measured findings

Starting revision: `97cdbe38f0d82bae848de15335439b6a5cc1de14`, clean before work; new branch `feat/ue58-playfeel-correction`. Human acceptance of that revision failed. Visual Gate 2 remains out of scope.

## Baseline verification

UE 5.8.3; generation PASS (20.41 seconds). A fresh unity Editor build exposed ambiguous `SaveAsset(UObject*)` helpers in the original Gate1 author and the locomotion author. Rename only the latter to `SaveLocomotionAsset`; no authoring behavior changed. Repaired Editor build PASS (74.67 seconds); Game PASS (44.48 seconds). All nine existing tests PASS with no warning-bearing tests in this run. Real Cultivate: tick 1527 to 1534, one logical completion and one cosmetic sequence despite duplicate sends. Original failed build log is retained.

Read-only source inventory: 3,906 descriptor/config/content files, 5,842,295,647 bytes; identical to the prior accepted technical gate inventory before correction work.

## Sword geometry, before changing animation

The authored jian bounds are (-10,-2,-10) to (10,2,101) cm. Blade length follows local +Z, cutting-edge width follows X, blade thickness follows Y. Hilt extends from Z=-5 to Z=14; its centre is Z=4.5. The previous attachment snapped to hand_r and applied roll 90 degrees, mapping the blade onto hand +Y. At Manny idle, that axis points across the body, rather than out of the palm. This is an attachment-axis error, not a sword IK-retargeting error: the attack is already project-authored on the same Manny skeleton.

The original attack is sampled at 121 points, including 0/10/20/30/50/75/100 percent. At startup, hand_r is approximately (-33.42,-30.91,98.73) in mesh component space. The old blade tip is (59.87,-58.21,71.30). During 8–11% of the clip, the blade crosses approximate hip/thigh volumes. Dense bone-volume checks find a minimum signed clearance of -9.36 cm for the attack, and -12.44 cm for ordinary unarmed idle. These are diagnostic capsule/sphere proxies, not exact triangle intersection tests.

Removing the erroneous roll, centring the hilt in the glove and retaining one hand frame gives positive proxy clearance for all 121 attack samples (minimum 10.77 cm) and idle samples (10.73 cm). The canonical transform uses identity rotation and translation (-3,0,-4.5) in hand_r space: hilt centre 3 cm along hand -X, inside the existing glove. This is one project-owned `SwordGrip_R` scene attachment, not separate transforms per animation. Mesh pivot and scale are valid; no mesh/pivot repair is required. Runtime blend/locomotion clearance still needs separate PIE validation.

## Before-correction timing trace

The measurement-only PIE test preserves original motion behavior. It records every frame's movement, input, grounded state, requested database, selected clip/time, action blend/time, contacts, camera/actor/controller yaw and sword/body points.

| Input maintained through landing | Contact to requested locomotion DB | Contact to actual locomotion clip |
|---|---:|---:|
| Walk at 240 cm/s | 0.459014 s | 0.492347 s |
| Run at 600 cm/s | 0.463547 s | 0.513547 s |

The code holds bLanding for 0.45 seconds regardless of movement intent. Both measured cases select a standing light landing clip; its time continues into the tail while capsule velocity stays 240/600 cm/s. Search/update latency adds time after the Chooser switches database. These numbers describe this trace, not an asserted universal freeze duration or a target latency. Landing implementation has not yet changed at this stage.

The action branch is a full-body blend into a clip derived from idle; at full attack weight it replaces moving legs while CharacterMovement continues. It also holds the action through the full sequence, then fades back to Motion Matching. This requires separate transition diagnosis after grip/path correctness is validated.

## Correction sequence and implementation

The grip-only PIE experiment proved a static fix insufficient: ordinary unarmed locomotion still swings the wrist through unsafe orientations. It is retained as evidence. The shared carry pose now layers only the right clavicle/arm subtree over the existing Motion Matching stack whenever the jian is visible. The attachment remains unchanged across all states.

One project-authored attack is baked onto the existing Manny skeleton. A small set of component-space hand/blade key poses describes guard, anticipation, hip lead, torso follow, outward slash, follow-through and recovery. Offline two-bone arm solving preserves bone lengths and derives wrist rotation from the blade direction. This is animation authoring, not runtime frame-specific weapon transforms. Both original regeneration paths call the same shared editor-only authoring functions, avoiding a second attack/landing truth. The sample is not involved in sword authoring.

Rebuilt clip sampling gives minimum body-proxy clearance 22.36 cm; the carry pose gives 36.02 cm. A separate input-driven PIE run also shows no sampled proxy intersections through locomotion, jumping, pivots, stationary and moving attacks. These checks use simplified bone volumes and are paired with actual PIE images; they are not a universal mesh-intersection proof.

Landing now retains 0.10 seconds of impact under movement and 0.18 seconds at rest. Input arriving during recovery shortens the remaining hold. Those are authored impact beats, not fabricated measured targets. The four landing database entries are bounded around their measured minimum pelvis height (compression), excluding long falling/standing tails. Walk and run recover into their respective databases. Example intermediate corrected actual-clip latencies were 0.132241/0.145810 seconds; final measurements are reported separately.

The single action branch now uses per-bone weights for sword actions: spine descendants receive the action, pelvis receives a limited contribution and legs retain Motion Matching. Meditation remains full-body. The same branch eases action contribution down over the final 18 percent of the baked clip. Motion Matching continues updating throughout; search was never paused. Before-correction recovery to below five-percent action weight measured 0.362861/0.366668 seconds for stationary/moving attacks; intermediate correction measured 0.270853/0.216668 seconds. These are blend-release measurements from the defined 82-percent recovery boundary, not claims of search resumption latency. Existing CharacterMovement momentum remains; no animation moves the capsule or owns combat.

Steering diagnosis: input/controller yaw and camera yaw are continuous in the controlled trace, but body rotation enters/exits a fixed 500-deg/s budget, while velocity direction changes rapidly under digital direction reversal. The minimal correction eases CharacterMovement's angular-speed budget based on facing error. It does not introduce another rotation owner, teleport the actor or import the sample's offset-root/steering framework. In the measured diagonal+yaw segment actor peak angular speed changed from 500.06 to 268.67 deg/s, with controller/camera yaw unchanged near 52.50 deg/s. This is a bounded trace observation, not proof of human comfort. The sample's orientation-warping and offset-root concepts remain documented in the prior scoped audit; no complete framework was copied.

Camera tuning follows body steering: positional lag speed 11 to 16, maximum lag distance 35 to 22 cm; rotation lag explicitly disabled, FOV 66 retained. Direct yaw is preserved. Foot placement and two-leg IK remain active for upper-body sword actions; airborne and full-body meditation continue to suppress IK. The added test checks finite bone transforms/reach and returns to locomotion; images remain necessary for sliding and robe review.
