# Unreal snapshot adapter — Integration Gate 1

Simulation owns identity, time, region, life state, cultivation and committed facts. Unreal owns fixture placement, camera, mesh/material choices, animation, aura and UI. This bridge reads JSON only and has no mutation/command path back to simulation.

## Components and bindings

SnapshotV1::Parse builds a temporary FWorldPresentationSnapshot. USnapshotPresentationAdapter is a GameMode-owned actor component, not a global manager or character implementation. AGateGameMode supplies the existing player and two NPC fixture slots. Player possession may complete after BeginPlay: the component waits for the player pawn before accepting input. No actors are spawned by snapshot application.

controlledActorId binds the player slot. Existing selected NPC bindings retain their slot; newly selected IDs fill free NPC slots. Disappearing IDs unbind and hide their slot; absence never means death. XYZ stays in Stillwater Valley fixture space, independent of region IDs. Full snapshot application cannot change simulation location.

Nameplates explicitly label local Player/NPC aliases. HUD shows the real actor ID, method adopted/no method, completed session count, life status, region ID and committed tick. Fixture realm text is explicitly Presentation Fixture. A missing mortality record remains unknown. Dead state is logged and local movement disabled; no death animation is implemented.

## Validation and stream policy

Mirrored schema: Bridge/contracts/snapshot/world-presentation-snapshot-v1.schema.json. Runtime validation also enforces exact fields, supported schema, world/kind-qualified IDs, safe integers, sorted unique actors, legal enums and action references/timestamps. Contract bound is ten actors; this Gate 1 adapter accepts at most three.

All schema, stream and complete slot planning checks precede cosmetic application. Unsupported/malformed input cannot consume sequence or partially update bindings. World/ruleset/simulation metadata stay fixed for an adapter session. A new world/version needs a new adapter session.

A new stream requires explicit full resync, sequence 1, empty actions and a stream ID not retired in this session. Same-stream updates require increasing sequence and nondecreasing worldTick. Reapplying the same snapshot or an older sequence is rejected. Full resync with the same stream ID is rejected. Connection metadata never enters simulation saves.

Committed EventId is the deduplication key. Repeated IDs with matching actor/tick are ignored even across fresh-stream resync; conflicting fingerprints are rejected. The session keeps at most 4096 unique cue IDs and rejects overflow for explicit session review. This is bounded prototype bookkeeping, not a durable replay service.

## Cosmetic playback and appearance

Each new cultivation_session_completed cue queues the existing local meditation/aura preview once on its actor. The preview leaves the loop at 2.8 local seconds and returns to idle at 4.2 seconds. These seconds do not represent strategic days. Dead actors skip queued cues. A fresh-stream resync cancels pending/playing previews and does not replay prior history.

The first eight hexadecimal appearanceSeed characters form an unsigned 32-bit token. token % 3 chooses an existing DA_Player/DA_NPC_A/DA_NPC_B recipe; token & 4 selects accessory visibility. The local recipe supplies palette, meshes and aura preset. Unchanged actor/seed retains the same cosmetic recipe across save/load resync. TypeScript exports no asset paths. No art imports, Niagara migration or QCBH assets were introduced.

## Running and testing

Build the existing Unreal/DaoVoTan3D.uproject with UE 5.8.3. Supported toolchain used: VS2022 MSVC 14.44.35229 (directory family 14.44.35207), SDK 10.0.26100.0.

Launch with -SnapshotFile=<absolute-json-path> to load an initial full resync and poll changed contents every 0.5 seconds. Write snapshots atomically. New-stream reconnect requires explicit LoadSnapshotFile(path, true), which is presentation-only. Polling same-stream updates never sends ACKs/commands. Producers own local delivery bookkeeping. -SnapshotIntegration disables the offline NPC meditation fixture; ordinary local input remains cosmetic.

Automation filter DaoVoTan runs the retained Gate1.PlayableSlice plus Integration.SnapshotProtocol and Integration.ReadOnlyPIE. Fixtures under Bridge/sample-data/integration-gate1 are actual TypeScript exports. The PIE exercise loads real IDs, triggers two distinct committed completions once each, resends the same events at a new sequence, rejects invalid/out-of-order input, resyncs from a restored simulation save, verifies exact actor pointers/appearance, removes an actor without death inference and benchmarks 100 three-actor updates without spawning.

Screenshots and adapter-proof.txt are generated under Unreal/Saved/IntegrationEvidence; JSON automation report and logs accompany the deliverable. Existing startup hand_r/no skeletal mesh warnings precede recipe application and were also present in the baseline. No Stage 2 or Visual Gate 1.1 work is included.

The final PIE exercise also injects distinct unknown/dead statuses to verify presentation state and disabled local movement without removing either binding. A separate fresh Node process restored the disk save and reproduced the exact canonical resync JSON. Game Development build and project generation also passed; performance and warnings are recorded in the delivered verification report.
