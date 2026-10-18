# AI Engineering Instructions — `dao-vo-tan-3d-prototype`

These instructions apply to AI coding agents working in this Unreal presentation repository.

## Mission

Deliver a premium-feeling playable cultivation vertical slice quickly **without turning Unreal into a second simulation engine**.

Feature progress is judged by three dimensions: Engineering, Visual and Player. A feature gate is not complete if it passes only build/tests.

## Authority boundary

`Report0101/tu-ti-n` owns authoritative world history and domain rules.

Unreal owns:

- input and embodied interaction;
- character visual assembly;
- local locomotion feel;
- camera;
- animation;
- combat presentation where explicitly marked non-authoritative;
- VFX/audio/UI;
- world representation;
- translation through the Bridge contract.

Never duplicate authoritative cultivation, injury/recovery, material, opportunity, faction, death or NPC-history formulas here.
Never write persistent world history from local mock/presentation logic.

## Architecture guardrails

- No giant Level Blueprint.
- No generic all-knowing `GameManager`.
- Animation Blueprint must not become authoritative gameplay state.
- UI must not directly mutate simulation/domain truth.
- Character movement, interaction, combat presentation, cultivation presentation, visual assembly and bridge identity binding must remain separable responsibilities.
- Gameplay code must not depend on the internal hierarchy of one imported character mesh.
- Prefer a small component/adapter over a new framework.
- Add a subsystem only when a concrete current-slice blocker justifies it.
- Keep mock behavior visibly marked and replaceable.

## Player V1 visual rule

Player V1 is not a disposable mannequin.

For the first premium vertical slice it must meet Character Quality Q1 in `Docs/CHARACTER_QUALITY_GATE.md`:

- believable anatomy/silhouette;
- designed face/head;
- credible skin/eyes/hair;
- layered xianxia/wuxia garments;
- acceptable cloth/hair response;
- clean weapon presentation;
- authored-looking locomotion and cultivation states;
- modular visual parts that can be replaced without rewriting gameplay systems.

Do not accept a feature gate merely because the character is technically rigged.

## Playable-first discipline

Until Vertical Slice V1 is accepted, prioritize roughly:

- 70% player-visible/playable work;
- 20% required simulation/bridge integration;
- 10% architecture hardening.

Do not perform invisible refactors unless they fix a demonstrated defect, duplication, performance problem or blocker.

Every feature task should name the player-visible result before implementation.

## Required gate evidence

For each gate report:

- starting and final SHA;
- exact files/assets changed;
- build and automation results;
- PIE/development-build playable path demonstrated;
- visual evidence for character/environment/animation changes;
- performance observation when relevant;
- which behaviors are authoritative vs mock/presentation-only;
- known visual/technical debt and whether it blocks acceptance.

## Source control

- Do not commit generated Unreal folders or local IDE/build output.
- Do not merge to `main` without review.
- Avoid broad unrelated refactors in feature branches.
- Preserve the Bridge contract boundary.

Read `Docs/ARCHITECTURE.md`, `Docs/PLAYABLE_FIRST_V2.md`, `Docs/CHARACTER_QUALITY_GATE.md` and `Docs/VISUAL_DIRECTION.md` before substantial work.
