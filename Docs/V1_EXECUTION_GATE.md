# Vertical Slice V1 Execution Gate

Status: ACTIVE

## Gate name
`VS-V1 / PLAYER-Q1`

## Required proof

### Engineering
- Development Editor build succeeds on the self-hosted Windows runner.
- Existing source guard passes.
- `Scripts/Verify-V1Gate.ps1` passes.
- no authoritative simulation formula is introduced into Unreal;
- no generated Unreal folders are committed.

### Visual
- Character Quality Gate Q1 passes with captured evidence.
- target gameplay camera/lighting is used for review.

### Player
- Player spawns and can complete a coherent path in PIE/dev build.
- movement/camera functional;
- cultivation enter/loop/exit functional;
- at least 10 NPC visual proxies have stable non-empty simulation IDs;
- one NPC interaction is reachable;
- one resource/material presentation path is reachable;
- one opportunity presentation path is reachable;
- time/environment changes without direct player input;
- save/load preserves the identities/state required for this slice.

## Explicit non-goals for this gate
Do not block V1 on a full inventory, full dialogue tree, quest framework, skill tree, open-world streaming solution, procedural world generator, multiplayer, mounts or cinematic facial polish.

## Failure policy
If a criterion fails, fix the smallest responsible layer. Do not respond to a local failure by creating a generalized framework unless the failure proves that a shared abstraction is necessary.

## Merge policy
Do not merge to `main` solely because CI is green. V1 requires Engineering + Visual + Player evidence.
