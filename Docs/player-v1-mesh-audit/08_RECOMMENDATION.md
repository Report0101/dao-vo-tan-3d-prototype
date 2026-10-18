# 08 — Recommendation and modular architecture

**Final verdict B — USABLE AFTER RETOPOLOGY / MODULAR REBUILD.**

This is a usable visual/shape source, not a direct production mesh. Its closed indexed surface and overall silhouette make it suitable for retopo reference. Fused hand anatomy, monolithic garment/hair connectivity, absent UVs/materials and non-deformation-oriented triangle flow rule out “directly usable with cleanup.” Do not start production implementation in this audit.

| Source region | Decision | Future AppearanceRecipe mapping |
| --- | --- | --- |
| Hidden body / underbody | REBUILD | Existing Body skeletal visual on Manny; coverage/seams must be authored |
| Head / face / neck | RETOPO (plus resculpt) | Existing HeadVariant concept; explicit seam and portrait validation |
| Hair | REBUILD | HairVariant; controlled clumps and optional cosmetic chains |
| Upper robe / sleeves | RETOPO | Existing garment visual / GarmentVariant on the same skeleton |
| Lower robe panels | REBUILD | Same appearance owner; deliberate panel modules, no new character class |
| Belt | RETOPO | AccessoryVariant or garment trim attachment |
| Pendant / sash / hanging ornament | OPTIONAL | Existing cosmetic attachment flow; no simulation references |
| Hands | REBUILD | Manny-bound body/hand region preserving sword socket contract |
| Boots | RETOPO | Same visual skeleton; future coverage-aware boot module if needed |
| Overall design silhouette | KEEP as reference | No raw asset references in production recipes |

The current recipe exposes Body plus integer head/hair/garment/accessory variants and aura colors; it does not already provide arbitrary independent skeletal slots for every proposed panel. Production may extend that ONE existing recipe/catalog and appearance owner deliberately after a clean asset exists. Do not claim these slots were implemented here. Shared skeleton/leader pose is the intended route; existing locomotion, foot IK, aura and portraits remain the consumers. No mesh path enters TypeScript.

Concrete next pipeline: verify Meshy generation/license/reference provenance -> preserve raw reference -> retopo/resculpt head and underbody -> rebuild five-finger hands -> rebuild robe lower panels and clean sleeves -> rebuild/simplify hair clumps -> UV/material authoring and texture bake -> fit to existing Manny rest pose -> bind/weight polish -> isolated ten-pose acceptance -> only then consider AppearanceRecipe integration in a separately authorized Player V1 production gate.

**NOT READY FOR PLAYER V1 PRODUCTION GATE.** License evidence and a suitable fitted/skinned modular candidate remain missing. The audit is complete when its measurements, static fixture and preservation proof are delivered; this statement is not a request to begin that next pipeline automatically. Visual Gate 2 remains out of scope.
