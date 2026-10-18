# Character Quality Gate

Status: REQUIRED FOR V1 PLAYER ACCEPTANCE

## Quality levels
### Q0 Technical Proxy
Purpose: verify code, rig hookup, retargeting and animation plumbing. Q0 does not satisfy Player V1 acceptance.

### Q1 Playable Production-Direction
Required for Vertical Slice V1. The character must look intentional and attractive in real gameplay conditions, not only in a beauty render.

### Q2 Hero Polish
Later close-up/cinematic quality. Not required before V1.

## Q1 acceptance
All sections must pass.

### Form and silhouette
- believable/semi-realistic anatomy and proportions;
- clear cultivator silhouette at exploration distance;
- no visibly broken deformation in core locomotion poses.

### Face
- face has intentional structure and identity rather than generic mannequin appearance;
- eyes read naturally at gameplay/medium distance;
- skin response is credible under target scene lighting;
- no obvious mouth/eyelid deformation failures in available facial motion.

### Hair
- hair shape supports the character silhouette;
- no severe scalp intersections or detached-looking cards/strands;
- motion/secondary response is believable enough for gameplay.

### Garments
- layered robe/clothing construction reads clearly;
- major cloth layers do not constantly intersect body/weapon during idle, walk, run and cultivation;
- materials do not read as uniform plastic.

### Weapon
- sword/weapon attachment is stable in exploration and combat-ready poses;
- no severe hand, hip or robe intersection in the core loop.

### Animation
Required review set:
- idle;
- walk;
- run/sprint;
- jump + landing;
- dodge or equivalent evasive move if present in slice;
- cultivation enter;
- cultivation loop;
- cultivation exit;
- combat-ready pose if combat remains in V1.

### Gameplay-lighting review
Player must be reviewed in:
- neutral daylight;
- shadow/overcast or foggy environment;
- cultivation/VFX state.

A character that looks good only in a studio render fails Q1.

### Modularity
Gameplay systems must reference a stable Player shell/visual assembly contract, not model-specific bone/object hierarchy outside the dedicated character presentation layer.

## Automatic failure examples
- mannequin/proxy is still the shipping V1 player;
- beauty-shot pass but gameplay-camera appearance is poor;
- broken shoulders/hips/hands in common motion;
- sword attachment visibly floats or clips continuously;
- one imported mesh hierarchy becomes a dependency for movement/interaction/simulation code;
- replacing hair/clothes/body requires rewriting gameplay logic.

## Evidence required
Capture or record evidence for the review set above and record PASS/FAIL with known exceptions. Exceptions must be narrow, visible and scheduled, not hidden behind a generic 'polish later' label.
