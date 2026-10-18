# AGENTS.md

## Project mode: Playable-First V2

This repository is the presentation and embodied-interaction layer for Đạo Vô Tận.

### Non-negotiable architecture boundary
- `Report0101/tu-ti-n` owns authoritative simulation truth.
- Unreal must not reimplement cultivation progression, body-condition consequences, faction truth, resource/material ownership, opportunity resolution, death resolution, or historical causality.
- Unreal may own render-frame state, animation state, camera, locomotion feel, VFX, UI, local interaction affordances and visual proxies.
- All simulation-facing work goes through one explicit bridge/adapter boundary.

### Playable-first rule
Every feature gate must produce a visible or playable improvement unless it is fixing a demonstrated blocker. Avoid speculative frameworks and architecture-only gates.

### Character rule
Gameplay code must not depend on the internal hierarchy of one imported character model. The visual assembly must be replaceable without rewriting gameplay logic.

### Anti-spaghetti rules
1. No giant `GameManager`, all-knowing Blueprint, or Level Blueprint containing durable game rules.
2. UI consumes presentation view-model/state, not arbitrary raw domain payloads.
3. Character movement, interaction, combat presentation, cultivation presentation and bridge integration remain separate responsibilities.
4. Do not duplicate bridge parsing in multiple gameplay classes.
5. New subsystems require a concrete blocker/use-case, owner, inputs/outputs and acceptance criterion.
6. Prefer small composable components over inheritance-heavy frameworks.
7. Refactor only when required by the current playable slice or by a proven correctness/performance issue.

### Gate rule
A vertical-slice gate passes only when all three are satisfied:
- Engineering: build/tests/guards pass and architecture boundaries remain clean.
- Visual: the relevant feature meets its declared quality bar in gameplay lighting/camera.
- Player: the feature is observable and usable in PIE/dev build.

### Current priority
Vertical Slice V1 / Player Q1:
- attractive production-direction Player V1;
- stable third-person movement/camera;
- cultivation interaction;
- at least 10 NPC visual actors bound to stable simulation identities;
- one NPC interaction;
- one material/resource path;
- one opportunity path;
- autonomous time/environment change;
- save/load identity preservation.
