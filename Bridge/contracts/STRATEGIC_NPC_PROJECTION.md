# Strategic NPC projection contract (v0.1 proposal)

Status: **transport-neutral bridge specification; not wired to a live Unreal actor**.
Related core draft PR: https://github.com/Report0101/tu-ti-n/pull/3
Authoritative NPC domain: `Report0101/tu-ti-n/src/domains/agent`.

## What Unreal receives

Read-only and player-observable information about an NPC whose behavior has been validated by simulation:

```json
{
  "schemaVersion": "strategic-npc-projection.v1",
  "worldTick": 120,
  "agentId": "mock:agent:survey-scholar",
  "regionId": "mock:region:northern-valley",
  "mock": true,
  "presentationState": "studying",
  "visibleActivity": "Comparing cultivation notes at a roadside desk",
  "publiclyKnown": true,
  "causalEventIds": []
}
```

Fixture is explicitly **mock**. It may drive an idle/read notebook animation, UI label or an ambient line. No displayed content is an authoritative record unless the core adapter passes validation and `mock=false`.

## Presentation mapping

| Core strategy hint | Safe visual manifestation |
| --- | --- |
| observe | survey surroundings, inspect an object, pause to listen |
| study | read a scroll, perform an experiment, take notes |
| negotiate | walk toward a public meeting, converse with known actors |
| organize | gather local allies, distribute tasks visibly |
| withdraw | leave the scene via authored locomotion/pathfinding |
| no selected action | idle/normal routine, not magically teleport |

The **selected planner hint alone** does not prove action execution. The bridge must emit an activity only after the relevant existing authoritative simulation command is accepted, or explicitly mark it as mock.

## Privacy and anti-omniscience

- NPC private beliefs, private goals, hidden target references, ranking scores and unobserved faction motives must not be exposed to Player UI.
- Actor may receive only visible presentation hints from the core's player-scoped projection.
- UI dialogue must not assert unseen outcomes or facts.
- One entity ID represents one Agent; spawning/despawning visuals must not create/destroy the underlying Agent state.
- Unreal never recomputes strategic utility, truth, resource costs, faction membership or event history.
- Update from simulation ticks/events, not per-render-frame strategy evaluation.

## Candidate for the valley

**Tạ Viễn Châu**, a fictional original surveyor/reformer defined in core `docs/STRATEGIC_NPC_REFORMER_DESIGN.md`. For the locked v0.1 vertical slice, do **not** silently expand scope to a production AI character: existing wandering NPC can consume a test projection for `observe` and `study` visual actions. Actual autonomous character implementation requires a separately accepted integration gate.

## Acceptance

1. Mock contract drives at least two visual states while leaving simulation state unchanged.
2. `mock=false` payloads are rejected unless emitted by authoritative adapter after validated outcome.
3. Replay/snapshot rehydration does not duplicate an actor or change decision history.
4. Hidden agent plans and private beliefs are never sent to player-facing UI.
5. Not merged until newer local Unreal changes are reconciled against this older GitHub baseline.
