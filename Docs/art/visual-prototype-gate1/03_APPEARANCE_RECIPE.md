# Stable modular appearance contract

Date: 2026-10-15 (Europe/Budapest). Repository: `Report0101/dao-vo-tan-3d-prototype`; local branch `main`, starting commit `9fa034e82fe9fd1538174f11f9e1e16314e23c40`. **Prepared specification/contracts only; Unreal runtime unbuilt and unvalidated.**

`Bridge/contracts/gate1/appearance-recipe.schema.json` defines persistent recipe identity, actor reference, rig family, cosmetic seed/version, module keys, palette, insignia, age/scar presentation and aura selection. Three fixtures share the same rig and body but use distinct head/hair/garment selections. Module keys refer to **uncreated original target assets**, not imported bytes or a completed asset catalog.

Cosmetic entropy is separate from core RNG. Gate 1 fixtures store resolved module choices and a cosmetic seed/version. Restore reuses the same recipe; LOD/visibility/fidelity changes choose representation, never reroll parts. Future generated choice uses a dedicated cosmetic generator with pinned version and manifests; no physics RNG draw is allowed for hairstyle.

Projector consumes authorized bridge state, not direct TypeScript stores. Age, wounds, affiliation and aura fields are presentation descriptors derived under declared access policies. They do not own birth age, injury, membership or cultivation. Scars can persist cosmetically only when supported by accepted state/history.

Assembler validates keys/rig compatibility and coverage rules, caches reusable assemblies, applies bounded palette parameters and sockets. One assembler serves player/NPC instances. Fallbacks must be explicit and visible in diagnostics; missing module cannot silently become a different canonical face. Runtime composition and engine rendering remain missing.

Portrait cache keys include recipe ID/revision, cosmetic version, asset manifest version, expression/pose, lighting preset and resolution. Actor A remains Actor A after save/load, cull/uncull, LOD or simulation tier changes. Schemas/fixture tests demonstrate stable data serialization only, not rendered visual equivalence.
