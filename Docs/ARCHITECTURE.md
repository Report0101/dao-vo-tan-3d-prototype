# Architecture Lock

Status: **Locked for Vertical Slice v0.1**

## 1. Responsibility split

### `Report0101/tu-ti-n`
Owns authoritative simulation state and deterministic rules:

- world substrate and ecology;
- population and persistent agents;
- birth traits;
- cultivation state and method physics;
- body condition, injury, recovery, viability;
- knowledge and method lineage;
- factions;
- resource control;
- material holdings and provenance;
- opportunity state;
- death and legacy;
- historical event ledger;
- persistence and deterministic replay.

### `Report0101/dao-vo-tan-3d-prototype`
Owns presentation and embodied player interaction:

- Unreal world geometry and streaming;
- player locomotion;
- camera;
- animation state machines;
- combat animation and hit presentation;
- VFX, sound and feedback;
- UI/HUD;
- spatial interactables;
- local visual proxies for NPCs and world objects;
- translation between player intent and simulation contracts.

## 2. Hard boundary

The Unreal project must not become a second simulation engine.

Forbidden duplication includes:

- cultivation gain formulas;
- physical-consequence thresholds;
- natural-recovery formulas;
- material-consumption truth;
- authoritative NPC history;
- faction ownership rules;
- death resolution;
- opportunity outcome rules.

If a value influences authoritative world history, it belongs in the simulation core.

## 3. Integration model

Vertical Slice v0.1 develops offline first using sample bridge data.

Later integration follows:

```text
Player Input
   ↓
Unreal Interaction Layer
   ↓
PlayerIntent contract
   ↓
Simulation Adapter
   ↓
tu-ti-n authoritative action
   ↓
Outcome + State Delta + Events
   ↓
Unreal Presentation Adapter
   ↓
Animation / VFX / UI / world representation
```

The bridge is transport-neutral. It must not assume HTTP, sockets, embedded Node, or a specific IPC mechanism until a later integration gate chooses one.

## 4. Contract categories

### PlayerIntent
Examples:

- MoveTo / traversal intent where simulation relevance exists;
- Cultivate;
- UseMaterialForCultivation;
- Interact;
- SearchOpportunity;
- ClaimOpportunity;
- ExtractResource;
- JoinFaction;
- Attack / defend only when combat simulation authority is introduced.

Pure local locomotion animation does not require round-tripping to the simulation every frame.

### WorldSnapshot
A coarse authoritative snapshot used to populate or refresh the 3D representation.

Suggested fields:

- world tick;
- controlled agent identity and region;
- body-condition presentation state;
- cultivation summary;
- known opportunities;
- held materials;
- nearby faction/world facts;
- weather/ecology presentation hints where authoritative.

### Outcome
Represents the result of an authoritative action.

Suggested fields:

- accepted/rejected;
- action type;
- resulting state delta;
- causal event IDs;
- presentation hints;
- interruption reason such as death.

## 5. Update-rate rule

Do not synchronize the whole simulation at render-frame frequency.

Three clocks are allowed:

1. **Render clock**: Unreal frames, visual only.
2. **Local gameplay clock**: animation, camera, movement, non-authoritative traversal feel.
3. **Simulation clock**: authoritative world time and domain actions.

Bridging between them must be explicit.

## 6. NPC rule

NPC visual actors are projections of simulation agents, not the source of truth.

For v0.1, simple local routines may be mocked for visual testing, but they must be clearly marked prototype-only and must not silently become authoritative faction/life-history logic.

## 7. Combat rule

Vertical Slice v0.1 may implement combat feel locally to test animation, camera, timing, hit-stop, particles and parry readability.

Until the simulation core owns combat, prototype combat results must be labeled **non-authoritative**. Do not write permanent world history from local combat mock logic.

## 8. Asset and source-control rule

- Generated Unreal folders are ignored.
- Large binary assets should use Git LFS once production assets begin.
- Never commit DerivedDataCache, Intermediate, Saved, or local IDE build output.
- Keep architecture/contracts/docs reviewable as text.

## 9. Anti-spaghetti rules

1. Unreal gameplay code cannot directly mutate arbitrary domain mirrors.
2. One bridge adapter owns translation to/from simulation contracts.
3. UI reads presentation view-models, not raw domain payloads everywhere.
4. Character movement, combat presentation, interaction, UI and bridge integration stay separate modules/components.
5. No giant `GameManager` or all-knowing Blueprint.
6. Blueprint is allowed for presentation composition; durable rules and contracts should remain explicit and testable.
7. A new subsystem requires an owner, input/output boundary and acceptance criterion before implementation.

## 10. Vertical Slice architecture freeze

Do not add another major game system until the slice proves:

- stable third-person movement;
- convincing camera;
- a small navigable environment;
- one combat loop;
- one cultivation interaction;
- one opportunity interaction;
- one NPC interaction;
- clean bridge mock contract;
- acceptable performance on the target development PC.
