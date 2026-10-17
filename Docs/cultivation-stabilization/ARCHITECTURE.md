# Cultivation interaction and bridge stabilization

Starting presentation revision: d6932ffce2b499b8f2b3be66419679f5cb85a2db. Simulation starting revision: b7774fe23473f695869d76a38dfe04d34bb75791. No Visual Gate 2 work.

Required worker revision: 57d84b3dec06d22fe3f250902db7d7cbe79c54f7 on feat/cultivation-presentation-resync. Run npm run build:p1 there before launching the existing demo. The old worker does not understand the fresh presentation-session request.

## Reconnect

The worker previously created one PresentationExportStream on startup, publishing sequence 1 and acknowledging its history. Subsequent commands replaced snapshot.json with sequence 2 and later. A new PIE snapshot adapter correctly refuses a fresh stream starting above sequence 1 or containing historical actions. The worker had no signal that its presentation consumer had restarted. Thus the new adapter never bound its controlled actor. Restarting the worker happened to produce the missing full resync.

Each command adapter now creates a non-static GUID presentation identity. Its first connected tick atomically publishes schemaVersion/sessionId to presentation-session.json in the existing local file bridge. The same worker notices a new identity and calls the existing startCommandStream against its existing runtime. This produces a new stream at sequence 1 with actions empty and a watermark at current history. It does not restore, reinitialize, advance or save the world. A retained resync.json holds that initial snapshot even if snapshot.json advances before Unreal polls. Both publications are atomic. Unreal first reads resync.json and accepts only its own session identity, then consumes subsequent snapshot.json updates using existing sequence, identity and cue validation. Old-session snapshots and receipts cannot satisfy the fresh identity/request checks.

This extends connection metadata in the existing file bridge; commands, receipts and snapshot v1 keep their existing contracts. There is one worker writer and one active local presentation consumer, as before. Concurrent editor consumers/multiplayer and recovery from a worker crash with a stale lock are outside this gate. The worker lock still detects absence rather than proving liveness. Switching PIE does not reset simulation state or revive actors based on absent presentation data.

## Duration

UCultivationCommandAdapter owns one private SelectedCultivationDays integer. SelectCultivationDays clamps 1..30; GetSelectedCultivationDays exposes a read view. Existing -/+ Started bindings change exactly one day. The HUD owns no selection state. The outgoing command uses that integer unchanged. Existing lower-level SendCultivateForActor retains authoritative validation for intentionally invalid protocol tests. No presets or animation-derived command duration are introduced.

## Meditation

UPresentationActions owns the command meditation age, minimum and resolved/completed presentation flags. CultivationVisualSeconds is the sole rule: clamp(2.5 + .12 * days, 3, 6). The authoritative receipt remains in the existing adapter; presentation flags neither grant outcomes nor change strategic time.

Accepted/completed receipt starts one preview per request. Enter -> Loop waits until both the minimum and a completed result are available, then emits one completion transition -> Exit -> Locomotion. Receipt completion is recorded immediately even while visuals continue. A late result keeps the loop alive; timer expiry cannot manufacture completion. A rejection before acceptance creates no preview. A later interruption exits without a normal completion response. Explicit authoritative death still cancels cosmetics and disables movement.

The adapter no longer owns hardcoded 2.8/4.2-second stop/reset timers. Snapshot completion-cue previews also use the same presentation lifecycle, with a one-day cosmetic default because that cue contract contains no duration. Sword transitions and ActionRevision replay fix remain unchanged. A new command waits while the prior cosmetic sequence is finishing, preventing one receipt from overwriting another active presentation. The HUD continues to display the already committed authoritative outcome.

## Verification design

The dedicated StabilizationPIE test runs three StartPIE/EndPIE cycles in one Editor process against one continuously running worker. It asserts fresh stream identities/sequence 1/empty cues, correct pawn binding, preserved world tick, no old preview, all integer selections and clamps, raw +/- input, exact 1/7/17/30 request and receipt durations, early completion held until minimum, duplicate receipt idempotency and one result per input. A controlled component clock additionally proves that 30-day presentation remains in Loop beyond its minimum without a result, then exits once resolved. This clock exercise is cosmetic test coverage, not an authoritative command.

Separate existing raw E/M/LMB fixtures, complete Unreal regression, correction suite, canonical seven-day command hash, stress and all simulation tests provide independent coverage. Human visual acceptance remains a separate retest.
