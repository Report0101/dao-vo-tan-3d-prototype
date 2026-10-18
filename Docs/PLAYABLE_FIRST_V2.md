# Playable-First V2 — Unreal Delivery Policy

Status: **FINAL DIRECTION — apply on top of the newest local Unreal/Player V1 branch**

This policy refines the existing architecture lock. It does not relax the simulation boundary. It changes what counts as progress.

## 1. Primary objective

The next major milestone is a premium-feeling playable cultivation vertical slice, not another foundation-only gate.

Every gate must answer:

> What can the player now see, feel or do that they could not before?

Foundation work is allowed when it directly enables that answer or prevents a demonstrated defect.

## 2. Three-part PASS rule

A gate is PASS only when all three applicable dimensions pass.

### A. Engineering Gate

- project builds and opens;
- automation/source guards pass;
- no authoritative simulation formula is duplicated in Unreal;
- no giant GameManager/Level Blueprint becomes the hidden owner of unrelated systems;
- bridge, character, interaction, camera, combat-presentation and UI responsibilities remain separable;
- no unexplained framework/subsystem is introduced.

### B. Visual Gate

- the player character meets `CHARACTER_QUALITY_GATE.md` for the gate's declared quality level;
- camera, lighting and animation do not visibly undermine the intended premium presentation;
- obvious clipping, broken materials, placeholder mannequin appearance or debug-only presentation are not accepted as final gate evidence;
- performance remains interactive on the target development PC.

### C. Player Gate

The result must be demonstrated in PIE or a development build through a concrete playable path.

A build/test-only result is insufficient for a feature gate.

## 3. Vertical Slice V1 target

The first slice should prove one coherent loop rather than many disconnected mechanics:

1. spawn as the production-direction Player V1;
2. explore a compact valley/village area with premium third-person camera and locomotion;
3. see at least ten NPC visual actors bound to stable simulation identities;
4. interact with at least one NPC;
5. perform one cultivation interaction with visible body/environment feedback;
6. use or discover one material/resource relevant to the slice;
7. discover one opportunity through space rather than a mandatory quest marker;
8. observe world time/environment change independently of direct player input;
9. save, load and preserve the relevant authoritative identities/state;
10. return to play without presentation and simulation diverging.

Combat may remain presentation-authority until an authoritative combat contract exists, but it must be clearly isolated from persistent world-history writes.

## 4. Player V1 is not a disposable placeholder

Player V1 establishes the production visual direction.

It may still improve later, but it must not be accepted merely because it is rigged and technically functional.

Required direction:

- believable anatomy and silhouette;
- strong face/head presentation at gameplay camera distance;
- production-direction hair, skin and eye materials;
- layered xianxia/wuxia clothing;
- credible cloth/hair response;
- clean weapon attachment;
- locomotion, idle, combat-ready and cultivation poses that feel authored rather than mannequin-like;
- modularity sufficient to replace garments/hair without rewriting gameplay systems.

See `CHARACTER_QUALITY_GATE.md` for acceptance.

## 5. Character architecture rule

Gameplay code must depend on a character interface/component contract, not on the internal hierarchy of one imported mesh.

The Player visual stack may change without rewriting movement, interaction or bridge code.

Recommended separation:

```text
Player Controller / Input
        ↓
Player Character gameplay shell
        ├── Movement / traversal
        ├── Interaction
        ├── Combat presentation
        ├── Cultivation presentation
        ├── Bridge identity binding
        └── Visual Character Assembly
              ├── body
              ├── face/head
              ├── hair
              ├── garments
              ├── weapon
              └── animation/cloth presentation
```

Do not bury gameplay rules inside imported character assets or animation Blueprints.

## 6. NPC visual tiers

Use two visual tiers rather than giving every NPC the same asset budget.

### Hero NPC

Important recurring NPCs. Quality approaches Player V1 where camera distance demands it.

### Crowd NPC

Modular, performant, visually coherent NPCs for population presence. They share the same simulation identity model but can use cheaper meshes/materials/animation variation.

Both tiers are projections of simulation Agents. Neither owns authoritative history.

## 7. Anti-spaghetti operating rules

1. Do not create a new framework when a small component/adapter solves the current slice.
2. Do not copy simulation formulas into Blueprint/C++ for convenience.
3. Do not use Level Blueprint for durable gameplay architecture.
4. Do not make animation Blueprint the owner of gameplay truth.
5. Do not make UI write domain state directly.
6. Do not make visual NPC routines silently authoritative.
7. Every new subsystem needs a named owner, input, output and deletion/replacement story.
8. Refactor only after a concrete duplication, defect, performance issue or blocker is demonstrated.

## 8. Work allocation until V1 slice

Directional target:

- ~70% player-visible/playable work;
- ~20% bridge/simulation integration required for that work;
- ~10% architecture hardening.

A gate consisting mostly of invisible refactoring requires a written blocker justification.

## 9. Evidence required for acceptance

For each gate, capture:

- exact Git SHA;
- build/automation result;
- PIE or development-build evidence of the playable path;
- screenshots/video or comparable visual evidence for visual gates;
- performance observation when the change affects rendering/animation/NPC count;
- explicit note on whether any behavior is mock/presentation-only or authoritative.

## 10. Synchronization note

The GitHub remote was behind the newest Visual, Integration and Player V1 local branches when this policy was added. Do not reset the local project to this remote branch. Cherry-pick or re-apply this policy on top of the newest local HEAD after those branches are synchronized.
