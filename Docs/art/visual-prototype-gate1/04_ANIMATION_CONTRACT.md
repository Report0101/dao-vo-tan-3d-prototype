# Presentation action and shared motion contract

Date: 2026-10-15 (Europe/Budapest). Repository: `Report0101/dao-vo-tan-3d-prototype`; local branch `main`, starting commit `9fa034e82fe9fd1538174f11f9e1e16314e23c40`. **Prepared specification/contracts only; Unreal runtime unbuilt and unvalidated.**

`ActionPresentationRequest` is a versioned presentation-side envelope: request/action identity, actor, sequence, snapshot revision, action kind/phase, optional target/direction, visual duration/playback metadata, authorized cosmetic cues and authority provenance. Core IDs are opaque references; no core implementation classes, string command evaluation or physics values are embedded.

Source `offline_mock` is clearly non-authoritative and cannot claim cultivation success/damage/death. A later validated `simulation` adapter carries accepted outcome references; this is still not permission for animation to author an outcome. Receiving external JSON requires full schema validation and authorization at the adapter, not trust in its self-declared source field.

Phases: queued, windup, active, recovery, completed, cancelled. Explicit request sequence/revision and actionInstanceId support duplicate/out-of-order handling. Presentation cues use cueId and offset seconds. Semantic animation family is an enum, not an arbitrary executable string.

Minimum target: Idle, Walk, Run, JumpStart, AirLoop, Land, Dodge, AttackA, AttackB, Parry, Cast, Hit, Death, MeditationEnter, MeditationLoop, MeditationExit. Sixteen keys are specified; **zero clips supplied**. Cultivate uses MeditationLoop; Breakthrough initially composes existing motion/VFX. No unique realm skeleton or library.

Animation notifies may trigger footstep, trail_start/stop, local sound or camera impulse. They cannot change damage/health, death, resources, material quantities, cultivation outcome, relationships, NPC decisions or world time. Culling/skipping/pooling/missing notifies cannot alter committed history.

Locomotion is local presentation feel until a bridge contract makes region-relevant travel explicit. Sword motions can play offline without HP, loot, kill or simulated victory. Death/Hit keys exist for future accepted cues or explicitly labeled test previews; mock previews never update core state. No authoritative combat implementation is invented for Gate 1.

Execution adapter owns transport/validation; animation component owns motion playback; VFX component owns reusable effect instances; HUD owns views; character assembler owns modules; camera owns framing. None becomes a giant all-knowing Blueprint.
