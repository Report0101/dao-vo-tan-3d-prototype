# Vertical Slice V1 / Player Q1 acceptance

Status: **incomplete; do not merge main**. This supersedes the primitive-bootstrap
checklist. The recovered project uses AGateCharacter, AGateGameMode,
UAppearanceComponent and the existing snapshot/command adapters. Required checks
live in V1_GATE_MANIFEST.json, CHARACTER_QUALITY_GATE.md and V1_EXECUTION_GATE.md.

## Three mandatory gates

1. Engineering: Development Editor build, native smoke/automation, source and V1
   guards; simulation golden contracts and boundary guard remain green.
2. Visual: Q1 anatomy, face/eyes/skin/hair, garment deformation, materials, weapon
   grip, enter/loop/exit and locomotion in actual gameplay lighting and camera.
3. Player: spawn, movement/camera, authoritative cultivation, ten NPCs bound to
   distinct stable simulation identities, one NPC interaction, one resource path,
   one opportunity path, autonomous time/environment change and save/load.

Source checks alone cannot accept native playback; native tests alone cannot
accept the visual gate. A default proxy or opt-in experimental preview is not a
shipping Q1 player. Observer-camera captures must be labelled as such.

## Recovered candidate, 2026-10-19

- Native project and live worker-to-PIE cultivation are operational. F7 swaps
  appearance on the same pawn and animation instance. Fresh PIE sessions restore
  the default appearance while the worker preserves authoritative identity/time.
- The experimental character still has visible head/material and robe-weight
  deficiencies. Its original FBX/PNG are preserved; provenance is not certified.
- Current snapshot selection supplies two NPCs; there is no accepted ten-NPC
  identity-binding result. Interaction/resource/opportunity paths are incomplete.
- World ticks advance through authoritative cultivation commands; an autonomous
  world/environment presentation is not accepted.
- A successful build or fixture guard does not confer Q1 acceptance.

## Local validation

Run Scripts/Verify-V1Gate.ps1 and Scripts/Build-Unreal.ps1. The build script performs
native build, smoke and base automation with a unique report directory.
Distinguish fixture-guard successes from fully exercised tests.

Run Tools/PlayerV1ExperimentalPreview/Run-PreviewValidation.ps1 with the sibling
simulation repository built. It starts an owned worker, runs the asset contract
and three-session live PIE exercise, and validates both required test results.
Use -InspectionCamera for a separate labelled full-body observer capture.

Retain exact commit IDs, commands, logs, reports and unedited screenshots.
Record failures and warnings, including Pose Search async-index warnings.
