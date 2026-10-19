# G1A dynamic Agent identity bridge

## Restriction found before implementation

The simulation projector selected at most two NPCs even though snapshot v1 allowed
ten actors. Unreal `SetSlots` retained the player and only two supplied NPCs;
`ApplyJson` required exactly three prepared slots and used slot indexes for binding
and aliases. GameMode pre-spawned the fixture actors and passed them as slots.
The three portrait panels are a separate display budget, not an identity bound.

## Smallest change

Problem: a ten-ID snapshot cannot be represented by fixed three-Actor storage.
Existing structure: the snapshot component already owns the ID lookup, parsing,
sequence and cue handling; it can solve the problem without a new abstraction.
Smallest change: retain the existing component, use its AgentID-to-Actor map as
the lifecycle owner, retain the existing player pawn, and spawn/retire only NPC
presentation Actors when IDs enter/leave. Immediate gate unlocked: player plus
nine persistent identities with stable refresh/reload and a one-ID replacement.
No subsystem, manager, interest framework, new schema or transport is introduced.

## Behavior

Simulation Agents -> deterministic nearby selection <=10 -> snapshot v1 ->
existing JSON worker -> snapshot component -> AgentID-to-Unreal-Actor map.

The simulation default view selects active Agents in the controlled Agent's region,
ordered by the existing ordinal comparator, capped at nine NPCs. Explicit bounded
selections keep existing dead/inactive inspection semantics. The final array stays
ID-sorted as required by v1; the player is identified by `controlledActorId`, never
array position. Empty and smaller interest sets are supported.

Unreal binds the controlled ID to the existing pawn. Retained IDs keep their
Actors, transforms and appearance; removed NPCs are destroyed, and new IDs get
fresh Actors. Local grid anchors are display positions only, not identities or
simulation region/location authority. Offline visual fixtures are retired upon the
first accepted snapshot. A removed ID is not inferred dead. Current status and
appearance remain simulation-derived detached presentation values.

Preset proxy recipes/visual assets are warmed once at session initialization.
Repeated snapshots do not perform a world scan or respawn retained IDs. Both
bridge validators reject behavioral versions other than `rules-v2` / `0.0.2`.
Snapshot schema remains 1; save schema remains 4. No character assets are edited.

## Reproducible validation

Build the paired simulation with `npm run build:p1`. Handoffs in
`Bridge/sample-data/g1a` come from `export-dynamic-agent-scenario.js`: actual birth
and lifecycle rules materialize persistent IDs, then one explicit selection
replaces an ID without deleting its authoritative Agent. The retained integration
cue fixture explicitly selects two NPCs solely for its original replay regression.

Run `Scripts/Verify-G1ABridge.ps1 -SimulationRepository <paired-checkout>` after the
native build. It creates fresh fixtures and a durable session, launches the
existing real worker, and requires both named protocol/PIE tests. Its PIE test
checks ten unique bindings, 100 refreshes without respawn, one-out/one-in, current
player reuse, reload, 0/1/3/5/10 membership and stale-version atomic rejection.
Screenshots and JSON come from the actual PIE world. The optional evidence-only
HUD lists stable IDs; normal gameplay UI keeps its existing portrait budget.

Do not call this character quality, gameplay autonomy, combat, or a larger slice
acceptance. It proves bounded identity projection and Actor lifecycle only.
