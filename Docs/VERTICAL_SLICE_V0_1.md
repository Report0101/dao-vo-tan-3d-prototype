# Vertical Slice v0.1

Status: **Scope locked**

## Goal

Produce a 5–10 minute third-person playable prototype that proves the future game can feel like a premium open-world wuxia/xianxia title while preserving the living-world simulation architecture behind it.

The slice is not a content demo. It is a proof of:

- movement feel;
- camera quality;
- atmosphere;
- combat readability;
- exploration rhythm;
- cultivation presentation;
- NPC presence;
- clean integration boundaries.

## Reference direction

Use **Where Winds Meet** as a reference for:

- third-person framing;
- movement fluidity;
- scenic composition;
- misty mountainous wuxia atmosphere;
- traversal fantasy;
- grounded melee readability;
- environmental density and verticality.

Do not copy its world, characters, quests, UI, combat data, art assets, level layouts, or proprietary content.

## Slice location

Working name: **Bắc Hoang Valley**

A compact mountain valley around 500 × 500 metres, visually implying a much larger world.

Required spaces:

1. valley trail entrance;
2. small village edge / roadside rest point;
3. bamboo or woodland path;
4. elevated cliff route;
5. Tụ Linh Trận cultivation site;
6. cave entrance and short interior;
7. hidden overlook revealing distant mountains / settlement silhouettes.

The environment should support vertical traversal and layered sightlines rather than a flat arena.

## Player fantasy

The player begins as an unknown mortal cultivator, not a chosen saviour.

Within the first ten minutes the player should be able to:

- walk and run through a convincing landscape;
- jump and perform light qinggong traversal;
- meet an NPC who appears to have their own purpose;
- fight one hostile opponent;
- parry at least one attack;
- discover a cultivation site without a floating quest marker;
- cultivate and receive visible/sensory feedback;
- find one material resource;
- enter a cave and discover an opportunity;
- see the world change subtly with time/fog/light;
- understand that the world extends beyond this valley.

## Feature scope

### Movement

Required:

- walk;
- run;
- sprint;
- jump;
- landing;
- dodge;
- light qinggong burst;
- slope handling;
- basic ledge / traversal assistance if practical.

Stretch:

- wall run;
- air step;
- long-distance qinggong chain.

Do not implement full traversal skill trees in v0.1.

### Camera

Required:

- third-person follow camera;
- mouse/controller orbit;
- collision avoidance;
- configurable shoulder distance;
- modest combat framing adjustment;
- smooth recentering.

Camera quality is a gate, not polish left for later.

### Combat

Required:

- sword idle/locomotion integration;
- light attack chain;
- heavy attack;
- dodge;
- guard/parry;
- one enemy archetype;
- readable hit reaction;
- hit-stop / camera feedback kept restrained;
- death/reset for prototype combat.

Prototype combat is presentation-authority only until the simulation core has an authoritative combat domain.

### Cultivation

Required presentation:

- sit / stance transition;
- qi gathering VFX;
- environment response such as wind, particles or subtle light;
- progress feedback;
- optional material-assisted variant;
- visible warning state for unstable cultivation.

Do not duplicate cultivation formulas from `tu-ti-n`.

### NPCs

Two NPCs minimum:

1. **Wandering swordsman**
   - moves between 2–3 local points;
   - idles, observes, and can converse;
   - not presented as a static quest dispenser.

2. **Local resident / gatherer**
   - follows a simple time-based routine;
   - can move, stop, work, and return.

These routines are visual mocks for the slice until connected to authoritative simulation agents.

### Opportunity

One cave opportunity:

- discover through spatial exploration;
- no mandatory world-map icon;
- cave contains environmental storytelling;
- interaction produces a mock bridge outcome;
- later replace mock outcome with authoritative `tu-ti-n` outcome.

### World presentation

Required:

- dynamic directional light;
- fog;
- basic day/night transition;
- wind response;
- ambient audio zones;
- distant landscape silhouettes;
- foliage movement;
- local particles / insects / leaves where performance allows.

## Explicit exclusions

Not in v0.1:

- full open world;
- large city;
- multiplayer;
- mounts;
- flying swords as long-range transport;
- character creator;
- dozens of weapons;
- production inventory system;
- production quest system;
- full dialogue system;
- procedural planet/world generation in Unreal;
- authoritative faction simulation inside Unreal;
- full save migration pipeline;
- final UI art.

## Acceptance gates

### Gate VS1 — Boot

- project opens cleanly;
- playable map loads;
- no missing critical assets;
- development build can launch.

### Gate VS2 — Movement

- 10 minutes of movement without camera breakage or stuck states;
- walk/run/sprint/jump/dodge/qinggong all reachable;
- no obvious animation-state deadlocks.

### Gate VS3 — Environment

- valley has at least three distinct visual subareas;
- elevated route and cave provide vertical depth;
- fog/light changes remain performant.

### Gate VS4 — Combat

- player can defeat one enemy using light/heavy/parry/dodge;
- parry feedback is readable;
- combat can reset safely.

### Gate VS5 — Cultivation

- player can enter/exit cultivation state;
- cultivation has visible environment/character feedback;
- normal and material-assisted presentation paths exist through mock data.

### Gate VS6 — Living-world illusion

- both NPCs move according to simple routines;
- at least one visible environmental change occurs independent of player input;
- NPC interaction does not freeze the whole world.

### Gate VS7 — Bridge mock

- Unreal consumes a sample world snapshot;
- Unreal sends a sample player intent;
- mock adapter returns an outcome;
- UI/animation reacts to outcome without domain logic leaking into presentation code.

### Gate VS8 — Performance

Initial target for development hardware:

- stable interactive frame rate at 1080p in the slice;
- no runaway memory growth during a 20-minute session;
- no blocking load on every routine interaction.

Exact performance target should be set after hardware profiling, not guessed in advance.

## Definition of done

Vertical Slice v0.1 is done when a new tester can launch the project, spend 5–10 minutes exploring Bắc Hoang Valley, fight, traverse, cultivate, interact with NPCs, discover the cave opportunity, and come away understanding both the visual ambition and the living-world direction of Đạo Vô Tận.
