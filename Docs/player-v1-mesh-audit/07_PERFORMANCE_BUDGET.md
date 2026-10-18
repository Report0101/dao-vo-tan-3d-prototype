# 07 — Performance budget

Raw source: 51,339 control points, 102,722 triangles, 308,166 polygon corners, one mesh and no source material/UV/texture. FBX is 3,264,956 bytes. The many authored normal indices may split render vertices; raw control points must not be equated to exact GPU vertices. StaticMeshEditorSubsystem metrics are unavailable in this commandlet context and are explicitly null, not fabricated. No Nanite/collision was enabled. This audit does not benchmark a skinned Meshy frame or claim a performance gain.

Proposed initial hero-player LOD0 budget (targets, not produced assets):

| Group | Triangle target | Reason |
| --- | --- | --- |
| Body + head + hands + boots | 18k–26k | Retain portrait silhouette, eyelids/lips and grip fingers; remove hidden under-robe surface only with modular coverage policy |
| Hair clumps | 4k–7k | Spend on outer silhouette/topknot; eliminate dense flat sheet interiors and ragged microtips |
| Upper/lower robe | 10k–16k | Bend loops, controlled thickness and visible hems; avoid uniform triangles over broad flat back/skirt |
| Belt / pendant / sash / accessories | 1k–3k | Independent simple attachments; detail that reads at camera distance |
| Total hero prototype | 33k–52k | A hero/portrait asset, not a crowd NPC; validate with existing three-actor and 11-actor scenarios |

Then derive lower LODs, consolidate a small material set and measure skinning, shadows, overdraw and portrait captures. Do not use a generic AAA count as acceptance. The best reduction is reauthoring broad generated surfaces; do not sacrifice fingers or silhouette to a blind decimator. Texture budgeting is pending a real UV/material plan; no absent source maps are implied.
