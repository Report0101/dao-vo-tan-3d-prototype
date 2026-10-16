# Unreal cultivation command adapter — Integration Gate 2

## Input and ownership

UCultivationCommandAdapter is a focused PlayerController-owned component. The character and animation code do not validate gameplay or calculate cultivation. Controller Enhanced Input maps E/M to a cultivate request; +/- selects an explicit integer day count in the existing [1,30] range. The initial UI choice is one day, the existing minimum allowed input, not a new simulation duration rule. Tests explicitly select seven days.

Normal E/M no longer starts a local meditation preview. It requires a bound simulation actor and configured bridge. With no bridge, UI reports disconnected and grants nothing. -GateDeveloperPreview is the explicit opt-in for the old local E/M cosmetic preview; HUD labels DEVELOPER PREVIEW ONLY. Sword/movement/aura preset controls remain cosmetic and emit no gameplay commands.

The component allocates a GUID only for requestId, serializes CommandEnvelope v1, writes a UTF-8 temporary file and renames into the local inbox. Simulation allocates persistent action IDs. No client code awards energy, sessions, death or time. Full command logic remains outside AGateCharacter; only its old preview key binding is gated by the developer flag.

## Receipt lifecycle and visuals

CultivationProtocol::ParseReceipt validates exact fields/version, enums, bounded IDs/results, safe integer ticks, lifecycle nullability and result correlation before publishing a temporary DTO. The component then checks expected request/actor/world and stable accepted action metadata. Malformed/mismatched receipts change no logical result or visual state. Accepted/repeated receipts cannot restart the same request preview; terminal duplicates cannot add another logical result.

Accepted starts the existing enter/loop/aura. Completed displays the real session EventId and committed tick; if the simulator finished before the first accepted poll, one cosmetic sequence may start from the committed receipt. Local visual timing exits at 2.8 seconds and returns idle at 4.2 seconds after starting, once terminal. It never advances simulation or delays a server commit. Dead-state presentation remains state/logging; no new death animation is added.

Rejected never starts meditation. Typed codes map to presentation text for busy, invalid/dead actor, control and prerequisites; the remaining supported codes remain visible as codes. Interrupted stops after local cosmetic exit and shows authoritative feedback. Logical request/result and preview counters are test diagnostics, not gameplay state.

Snapshot v1 remains unchanged and displays committed actor/method/session/life/region/tick truth. Receipt fields provide active action truth; no v2 is required. The command component marks its visual ownership of the commanded actor before sending. Snapshot adapter still deduplicates the real completion EventId but does not start another cosmetic cue on that actor while command-owned. Other actors' Gate 1 cues continue normally. Thus acceptance preview plus committed snapshot does not double-play.

## Local launch

The existing canonical project is Unreal/DaoVoTan3D.uproject. No second project/repository or art import.

From the simulation repository, build and initialize a separately named local bridge directory once, then run the external worker:

```powershell
npm run build:p1
node dist-p1/cli/cultivation-bridge.js init <absolute-bridge-directory>
node dist-p1/cli/cultivation-bridge.js watch <absolute-bridge-directory>
```

Launch the existing Unreal Editor project with -CultivationBridgeDir="<absolute-bridge-directory>". Press Play, then E/M; +/- changes requested days. The engine reads actor snapshots and receipts from that directory. Gameplay remains in Node/TypeScript, not inside the engine process.

Optional Tools/RunCultivationCommandDemo.ps1 resolves the sibling simulation checkout, preserves an existing session, starts its worker hidden and opens this existing project interactively. Close other Editor instances for this project first. Press Play in the Editor. Closing that launched Editor stops only the helper it created and removes only that helper's exact PID-owned lock; it never deletes saves. Its PowerShell syntax was checked; the underlying worker/Editor operations are verified by actual PIE automation, not a claimed manual launcher play session.

worker.lock enforces one writer. On abrupt exit, verify its PID has stopped before removing that exact stale lock. request/receipt cache is integration-session persistence, never Unreal save-game truth. Cold Editor reconnect resyncs committed actors; client logical-result/preview counters are session-local. Arbitrary branching/rollback of old world saves and multiplayer reconciliation are not implemented.

## Verification

UE 5.8.3, supported VS2022 MSVC 14.44.35229 (directory family 14.44.35207), SDK 10.0.26100.0. Project generation, Development Editor and Development game targets pass. No UBT patch/compiler-ban bypass or new SDK/plugin install.

Run all five tests with a fresh prepared directory and a running real worker:

```text
-SnapshotIntegrationTest -CultivationGate2Test -CultivationBridgeDir=<prepared-directory>
-ExecCmds="Automation RunTests DaoVoTan" -TestExit="Automation Test Queue Empty"
```

The test fixture flag keeps bridge auto-reading disabled for the older Gate 1/Integration Gate 1 tests; CommandRoundTripPIE explicitly enables it for its actual input round-trip. CommandReceiptProtocol tests strict parsing independently. Outside that fixture flag, a configured bridge connects normally. A test run without the fixture flag does not execute the live round-trip and must not be reported as E2E evidence.

RoundTripPIE injects the actual Enhanced Input action, resends the same request before/after completion, checks one strategic cost and real completedSessions in the HUD, one cosmetic sequence, no separate snapshot replay, then sends an intentionally invalid actor and checks simulation-owned WRONG_ACTOR without another meditation/time cost. It also rejects malformed receipts atomically and captures real viewport screenshots. The existing PlayableSlice, ReadOnlyPIE and SnapshotProtocol remain passing.

Evidence is under Unreal/Saved/Gate2Evidence and the exported test report/logs. Startup hand_r/no skeletal mesh warnings predate this gate and remain in raw reports; successful tests are not claimed warning-free. No packaged distribution, final art, Niagara migration or additional authoritative command is included.
