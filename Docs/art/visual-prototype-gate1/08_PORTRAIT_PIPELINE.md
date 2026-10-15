# Cached portraits from the same character

Date: 2026-10-15 (Europe/Budapest). Repository: `Report0101/dao-vo-tan-3d-prototype`; local branch `main`, starting commit `9fa034e82fe9fd1538174f11f9e1e16314e23c40`. **Prepared specification/contracts only; Unreal runtime unbuilt and unvalidated.**

Use an offscreen/scene-capture portrait stage over the same modular assembler/recipe: controlled neutral lighting, fixed portrait camera/focus socket, plain original background, transparent output if supported. Never create a separate illustration library in Gate 1.

Cache key = recipeId + revision + cosmeticVersion + asset-manifest digest + portrait preset + expression + resolution. Persist/rebuild recipe deterministically, invalidate only when appearance/preset/assets change. Do not capture continuously every frame. Start a modest original 256/512 output size after UI scale tests; not a QCBH resolution claim.

Pool portrait stage and character instance; isolate its visibility/effect state from gameplay; disable unnecessary physics/cloth/AI there. A portrait capture cannot advance core time, consume materials or mutate actor state. Near/far LOD selection does not change canonical modules.

Validation: player/NPC portrait matches head/hair/garment/palette, reload preserves result within declared render tolerance, palette change invalidates, UI lists reuse cache, capture work does not hitch interactions. Current state: specified only; no image/RenderTarget/SceneCapture implementation or rendered portrait supplied.
