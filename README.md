# Dao Vo Tan 3D Prototype

A standalone Unreal Engine 5 vertical-slice prototype for **Đạo Vô Tận**.

This repository is intentionally separate from `Report0101/tu-ti-n`.

- `tu-ti-n` is the simulation authority: world state, agents, cultivation, body condition, factions, resources, materials, death, history, and deterministic outcomes.
- `dao-vo-tan-3d-prototype` is the 3D presentation and interaction layer: world rendering, character control, animation, camera, combat presentation, VFX, UI, and spatial interaction.

The 3D prototype must not duplicate simulation rules that already belong to the core simulation.

## Direction

The target game feel is a high-quality third-person open-world wuxia/xianxia action RPG, with **Where Winds Meet** used as a visual and movement reference rather than a system design copy.

The defining difference of Đạo Vô Tận is a living simulated world where NPCs, factions, cultivation methods, resources, opportunities, injuries, deaths, and historical change continue independently of the player.

## Vertical Slice v0.1

The first playable slice is deliberately small:

- one compact mountain-valley environment;
- third-person movement and camera;
- walk, run, jump, dodge / light qinggong traversal;
- basic sword combat and parry presentation;
- one cultivation interaction;
- one cave / opportunity encounter;
- two NPCs with simple routines;
- day/night and fog/weather presentation;
- material-assisted cultivation presentation;
- body-condition feedback;
- a thin bridge contract prepared for later integration with `tu-ti-n`.

See:

- `Docs/ARCHITECTURE.md`
- `Docs/VERTICAL_SLICE_V0_1.md`
- `Docs/VISUAL_DIRECTION.md`

## Non-goals for v0.1

No giant open world, MMO networking, production character creator, full faction simulation in Unreal, duplicated cultivation physics, or large content pipeline. The prototype exists to prove movement, atmosphere, combat feel, spatial exploration, and the simulation-to-3D boundary.

## Repository layout

```text
dao-vo-tan-3d-prototype/
├─ Unreal/                # Unreal Engine project, added after architecture lock
├─ Bridge/
│  ├─ contracts/          # Stable transport-neutral contracts
│  └─ sample-data/        # Example snapshots/outcomes for offline development
├─ Docs/
│  ├─ ARCHITECTURE.md
│  ├─ VERTICAL_SLICE_V0_1.md
│  └─ VISUAL_DIRECTION.md
├─ .gitignore
└─ README.md
```

## Core rule

**Unreal presents outcomes. The simulation core owns truth.**
