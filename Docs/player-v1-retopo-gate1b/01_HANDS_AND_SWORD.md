# Player V1 Retopology Gate 1B — Identity & Deformation Recovery

Starting revision: e17d912ea653dc48f0e52141f8b792f1cfd10ed6. Branch: `feat/player-v1-retopo-gate1b`.
Experimental candidate remains `/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/`; production player is unchanged.

Evidence: `outputs/PLAYER_V1_RETOPO_GATE1B` in the task workspace; deliverable archive `PLAYER_V1_RETOPO_GATE1B_EVIDENCE.zip`.
Images of deformed meshes are inspection renders of UE-imported LOD0 vertices skinned from the accepted clips. They are **not live PIE captures**, runtime Animation Blueprint playback, final shaders, cloth or collision certification. Seven frames per clip are measured; three are exported/rendered. CPU linear-blend skinning is used. The reference pose is also inspected separately in Blender.

## Result: BLOCKER remains
Hands retain five fingers with Manny-aligned segment centers. Digit pads and taper were adjusted, but this does **not** produce a plausible closed jian grip in the accepted Guard/Attack clips. Fingers remain visibly open below the handle. This requirement failed; no pass is claimed.

The existing handle center is approximately (-3,0,0) cm in hand space, while proximal finger joints are about 9 cm from the hand root. The sampled Guard and Idle finger transforms remain close (roughly 0.48 cm / 4.29 degrees maximum difference at the compared midpoint). The supplied animation therefore does not supply a strong closing gesture. Geometry/weights alone have not resolved this while retaining Manny joint placement and the neutral hand.

We did not move the sword, edit `JianGripTransform`, `SwordGrip_R`, sword geometry, attack animation, or introduce a pose-dependent hand swap. A static clenched mesh around the wrist would obscure the problem and break finger alignment; it was not used.

Wrist/neutral and palm/thumb are documented in `10_hand_fingers`, `29_wrist_neutral`, `30_palm_thumb`; sword views `grip_SwordGuard*` / `grip_SwordAttack*` include three clip times. Wrist transitions remain provisional; independent wrist rotation is observed through accepted clips, not an invented extra test animation. Hands stretch peak 1.769x -> 1.841x. No finite-weight/binding failure, but numeric stability does not imply grip success.

Further structural hand fitting and an explicitly scoped compatible closing-pose solution remain necessary. This gate does not authorize changing the accepted sword/animation architecture.
