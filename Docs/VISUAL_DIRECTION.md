# Visual Direction

Status: **Reference lock for Vertical Slice v0.1**

## 1. Target feeling

Đạo Vô Tận should feel like entering a believable xianxia world rather than entering a theme park built for the player.

Primary visual/game-feel reference: **Where Winds Meet**.

Use that reference for the quality bar of:

- third-person cinematic framing;
- natural landscape composition;
- mist, mountain depth and atmospheric perspective;
- grounded wuxia motion before supernatural escalation;
- elegant rather than noisy combat readability;
- layered vertical exploration;
- restrained diegetic presentation.

Do not imitate its exact UI, locations, characters, costumes, animations, story beats or branded visual identity.

## 2. Đạo Vô Tận identity

The world should communicate:

- age;
- causality;
- impermanence;
- spiritual ecology;
- human activity continuing without the player;
- cultivation as a physical and environmental process.

The player is initially small relative to the world.

## 3. Environment language

### Terrain

Prefer layered natural spaces:

```text
sky / distant peaks
        ↓
cliff monasteries / paths
        ↓
forest / village / river
        ↓
caves / tombs / earth veins
```

Avoid large flat combat fields unless narratively justified.

### Composition

Each major view should ideally contain foreground, middle distance and far distance.

Examples:

- bamboo leaves in foreground;
- player and trail in middle distance;
- temple silhouette through mountain fog in far distance.

This creates world scale without requiring a huge playable map.

### Spiritual ecology

Qi should not be represented only by glowing particles everywhere.

Use environmental cues:

- grass leaning toward a spirit source;
- unusual mist movement;
- leaves orbiting a formation;
- water surface disturbance;
- subtle colour-temperature shift;
- insects or animals avoiding unstable areas;
- sound becoming unnaturally quiet;
- vegetation density changing around depleted sites.

VFX should communicate rules, not simply decorate scenes.

## 4. Colour and lighting

Baseline world palette should remain natural and relatively restrained.

Use stronger supernatural colour only when the world state earns it.

Examples:

- ordinary valley: earth, stone, bamboo, muted sky;
- spirit gathering: faint cyan/green atmospheric response;
- unstable cultivation: warmer pulses, desaturation, local distortion;
- dangerous cave: cold directional light, low saturation, sparse emissive traces.

Avoid constant neon fantasy lighting.

## 5. Character direction

For the prototype:

- realistic/semi-realistic human proportions;
- readable layered robes;
- physically believable cloth motion;
- weapon placement that survives locomotion without clipping badly;
- grounded idle stance;
- cultivation stance clearly different from combat stance.

The character should not visually begin as a legendary immortal.

Progression should eventually be readable through posture, movement, equipment, aura restraint and environmental reaction, not just bigger glowing effects.

## 6. Movement feel

Desired sequence of power fantasy:

```text
human locomotion
→ trained wuxia movement
→ qinggong
→ superhuman traversal
→ high cultivation mobility
→ true immortal movement
```

Do not begin the game with unlimited flight.

Early qinggong should feel physical:

- strong push-off;
- momentum;
- robe/cloth response;
- controlled landing;
- short air correction rather than hovering.

## 7. Combat feel

Combat should read as martial technique augmented by cultivation.

Early game:

- weapon weight;
- footwork;
- timing;
- parry;
- body positioning;
- restrained qi reinforcement.

Later systems may escalate into techniques, spells, artifacts, domains and supernatural effects.

VFX must preserve attack readability.

## 8. Camera

Default camera should prioritize exploration and landscape.

Combat camera may tighten modestly but should avoid excessive shaking and constant cuts.

Recommended principles:

- smooth acceleration/deceleration;
- collision handling near walls;
- preserve horizon where possible;
- allow the player to appreciate vertical scenery;
- minimal camera shake for ordinary attacks;
- stronger camera language reserved for meaningful impacts.

## 9. UI philosophy

The finished game should avoid looking like an admin dashboard.

Prefer:

- small contextual prompts;
- restrained HUD;
- world-space cues where appropriate;
- menus inspired by scrolls, records, maps and cultivation journals without sacrificing readability.

Avoid covering the exploration view with permanent panels.

Important simulation information should be translated into player-understandable states.

Example:

```text
physicalPressure = 0.31
```

should not normally appear directly.

The player might instead perceive:

```text
Qi circulation is becoming unstable.
Meridians feel strained.
```

Developer/debug views may still expose raw values.

## 10. Living-world visual cues

The world simulation must be visible indirectly.

Examples:

- a faction takes control of a road, guards and banners change;
- resource depletion removes workers and reduces trade traffic;
- a dead master stops appearing and disciples change behaviour;
- abandoned structures weather over time;
- damaged ecology changes vegetation;
- a new settlement produces roads, lights and activity;
- war leaves repairs, graves and displaced NPCs.

The goal is for history to leave scars in the environment.

## 11. Vertical Slice mood board in words

**Opening:** pale morning mountain light, fog trapped below cliffs, distant temple bells.

**Trail:** wet stone, bamboo movement, cloth and footsteps, sparse travellers.

**Cultivation site:** subtle wind convergence, leaves spiral slowly, sound narrows as concentration deepens.

**Combat:** grounded sword exchange, short qi flashes only on committed techniques, readable parry sparks.

**Cave:** transition from warm exterior to cold rock, moisture, old traces of cultivation, uncertain depth.

**Overlook:** player reaches a cliff and sees settlements, distant sect architecture and mountains beyond the playable valley, implying a living world much larger than the slice.

## 12. Quality rule

If forced to choose between:

- a larger map with generic content; or
- a smaller map with convincing depth, animation, atmosphere and world behaviour;

choose the smaller, deeper map for the prototype.
