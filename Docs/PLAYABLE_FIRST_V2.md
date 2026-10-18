# Playable-First V2

Status: ACTIVE EXECUTION POLICY

## Objective
Convert the existing simulation + Unreal foundation into a coherent playable xianxia vertical slice without allowing Unreal to become a second simulation engine.

## Development weighting
Use this as a planning bias, not a hard quota:
- ~70% playable + visual work
- ~20% simulation integration
- ~10% architecture hardening

Architecture-only work requires a demonstrated blocker, regression, determinism/correctness problem, or measurable performance issue.

## Ownership
### Simulation (`tu-ti-n`)
Owns authoritative world state, persistent Agent identity, cultivation truth, body condition, faction truth, resource/material provenance, opportunity state, death/legacy, time, save/load and causal history.

### Unreal
Owns player embodiment, camera, animation, local movement feel, visual characters, environment representation, interaction affordances, VFX, audio, UI and presentation of authoritative outcomes.

## V1 playable path
A tester must be able to:
1. spawn as Player V1;
2. walk/run/jump and navigate with a convincing camera;
3. observe at least ten NPC visual actors associated with stable simulation identities;
4. interact with at least one NPC;
5. enter and exit cultivation presentation;
6. observe one resource/material path;
7. encounter one opportunity path;
8. observe world/time/environment change independent of direct player input;
9. save/load and preserve the identities/state required by the slice.

## Three-part gate
No gate passes on code health alone.

### Engineering PASS
- Development Editor build succeeds.
- Relevant automation/guards pass.
- Simulation formulas are not duplicated in Unreal.
- No giant gameplay authority object or Level Blueprint appears.
- Bridge translation has one clear owner.

### Visual PASS
- Required content is judged in target gameplay lighting and camera.
- Player Q1 is mandatory for V1.
- Animation, attachment, cloth/hair response and environment integration are credible enough to represent production direction.

### Player PASS
- PIE/dev build exposes the new behavior end to end.
- A tester can reach it without developer-only intervention.
- Failure/retry paths do not leave the game in a broken state.

## Scope control
Do not add a large city, full quest framework, inventory framework, skill tree, procedural open-world generator, multiplayer, mounts, mass content pipelines or generalized framework work before the V1 path passes.

## Replacement rule
Temporary visual assets are allowed only when their replacement does not require rewriting gameplay/simulation code. Player Q1 itself may not be satisfied by a technical mannequin/proxy.
