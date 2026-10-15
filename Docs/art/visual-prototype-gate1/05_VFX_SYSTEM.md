# Six reusable VFX templates and cultivation variants

Date: 2026-10-15 (Europe/Budapest). Repository: `Report0101/dao-vo-tan-3d-prototype`; local branch `main`, starting commit `9fa034e82fe9fd1538174f11f9e1e16314e23c40`. **Prepared specification/contracts only; Unreal runtime unbuilt and unvalidated.**

Six **specified**, not implemented Niagara templates: Charge, Trail, Projectile, Impact, Aura, GroundField. A shared parameter contract carries color, intensity, scale, duration, theme, socket and realm presentation level. No duplicated realm graph; no graph contains damage/loot/simulation writes.

Three synthetic cultivation variants use the same Aura family: QuietMist (muted cool), LeafCurrent (green/warm), EmberWarning (restrained amber warning). These are parameter fixtures, not rendered effects and not inferred cultivation results. Warning presentation is only selected by an accepted cue or explicit offline preview.

Sequence: accepted/mock presentation request → MeditationEnter → MeditationLoop → aura buildup → ambient visual detail → optional ground field → MeditationExit. Visual envelope controls blend-in/out; renderer seconds never advance authoritative simulation days. Cancellation/actor despawn stops owned instances and resets pool state.

Socket policy resolves logical socket against rig contract; effect instance identity prevents duplicate spawn on retry; bounds/intensity capped; socket/material/visibility/lifetime reset on reuse. Ground-field geometry communicates a visual radius only; collision/damage remains absent or separately authoritative later.

Gate acceptance needs the three variants rendered from one actual graph, clean transitions, correct cancellation and a measured VFX-heavy test. JSON variants alone do not satisfy that gate.
