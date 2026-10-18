# Character Quality Gate — Player V1

Status: **Mandatory visual acceptance gate**

Purpose: prevent a technically valid character from being accepted while still looking like a placeholder or prototype mannequin.

## 1. Quality levels

### Q0 — Technical proxy

Allowed only for isolated engineering tests.

- rig works;
- locomotion can run;
- socket/weapon attachment can be tested;
- visual quality is irrelevant.

Q0 may never be used as final evidence for a Player V1 visual gate.

### Q1 — Playable production-direction

Minimum level for the first premium vertical slice.

The player must already communicate the intended art direction at normal gameplay distance.

### Q2 — Hero polish

Later refinement for close dialogue/cinematic use. Q2 is not required to begin the vertical slice, but Q1 must not block future Q2.

## 2. Q1 mandatory checks

### Anatomy and silhouette

- believable human proportions;
- shoulders, hands, neck and head scale read correctly in third-person view;
- silhouette remains intentional with weapon equipped;
- no obvious deformation collapse in walk/run/turn/cultivation poses.

### Face and head

- face reads as a designed character rather than a generic test head;
- skin shading avoids plastic/waxy appearance under target lighting;
- eyes have believable material response and do not look dead at gameplay distance;
- eyelashes/brows/facial hair, when present, do not visibly break the silhouette.

### Hair

- hairstyle supports the xianxia/wuxia direction;
- no severe scalp intersection or floating hairline;
- movement/physics is stable enough for gameplay;
- expensive strand treatment is optional if a cheaper solution meets the visual bar.

### Clothing

- layered outfit reads clearly as cloth/garment construction rather than painted body texture;
- robe/pants/belt/outer layer proportions support movement;
- no persistent severe clipping during core locomotion;
- cloth simulation or authored secondary motion exists where visually important;
- materials remain coherent under valley daylight, fog and cultivation lighting.

### Weapon

- sword placement is anatomically plausible;
- draw/hold/idle presentation does not visibly intersect the body in normal use;
- socket setup is independent enough to support future weapon replacement.

### Animation presentation

Required states at Q1:

- idle;
- walk;
- run/sprint;
- jump/landing;
- turn/reorientation;
- combat-ready idle;
- basic sword action presentation;
- cultivation enter/loop/exit.

Transitions must not expose obvious mannequin snapping or repeated broken poses.

### Character/environment integration

- feet do not constantly float or sink on ordinary terrain;
- lighting and shadowing place the character convincingly in the scene;
- fog/exposure does not destroy facial readability at normal gameplay distance;
- the character does not appear visually detached from the environment because of incompatible shading or scale.

## 3. Modularity requirements

Player V1 must not become a one-off asset that gameplay code depends on structurally.

At minimum, replacement of these visual categories must not require rewriting core gameplay logic:

- body/mesh;
- head/face;
- hair;
- garment set;
- weapon visual.

The animation/visual assembly layer may adapt assets, but movement, interaction, bridge identity and authoritative gameplay contracts remain independent.

## 4. Performance checks

Q1 is not PASS if visual quality is achieved only by making the slice unusably slow.

Record at gate time:

- development PC and target resolution;
- approximate frame rate/frame time in representative exploration view;
- player visible + target NPC count;
- whether expensive hair/cloth features are active;
- any known scalability fallback.

Do not lock arbitrary final-production budgets before profiling, but regressions must be explicit.

## 5. Acceptance evidence

A Q1 review must include all of the following:

1. front/three-quarter gameplay screenshot;
2. rear gameplay screenshot showing outfit/weapon silhouette;
3. movement evidence;
4. cultivation-pose evidence;
5. target valley lighting/fog evidence;
6. one close-enough view to judge face, eyes, hair and skin;
7. exact Git SHA and asset/source revision used;
8. list of known visual defects classified as blocker or accepted debt.

## 6. FAIL conditions

Player V1 fails the visual gate if any of these are true:

- still reads primarily as a mannequin/test dummy;
- anatomy is visibly poor at gameplay distance;
- face/hair/materials are obviously unfinished;
- garments collapse or clip badly during core actions;
- imported mesh hierarchy dictates gameplay architecture;
- the character only looks acceptable in a special beauty-shot lighting setup but breaks in target gameplay lighting;
- the asset cannot be replaced/refined without rewriting unrelated gameplay systems.

## 7. Decision rule

When forced to choose between:

- more features with a placeholder-looking Player; or
- fewer features with a convincing production-direction Player and clean architecture;

choose the second option for Vertical Slice V1.
