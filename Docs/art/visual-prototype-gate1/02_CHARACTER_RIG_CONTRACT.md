# Shared rig contract — proposed, not instantiated

Date: 2026-10-15 (Europe/Budapest). Repository: `Report0101/dao-vo-tan-3d-prototype`; local branch `main`, starting commit `9fa034e82fe9fd1538174f11f9e1e16314e23c40`. **Prepared specification/contracts only; Unreal runtime unbuilt and unvalidated.**

Rig family `humanoid_gate1_v1` is a contract identifier, not an existing skeleton asset. Pin actual skeleton asset/package hash when a rights-cleared foundation is admitted. No engine version or vendor skeleton is assumed.

- Units: centimeters; root at ground origin; +X forward, +Y right, +Z up in Unreal space. Export settings and unit conversion recorded in each asset receipt.
- Bind pose: one documented A-pose for the chosen family; all modules share bone names, hierarchy, bind transforms and scale. Do not declare a generated list a validated skeleton.
- Skeleton ownership: one reference skeleton; clothing/head meshes use its compatible subset. No independent skeleton per robe. Proportion variants limited to tested compatible morphs initially.
- Socket roles: weapon-hand, weapon-sheath, headwear, back-accessory, aura-root, feet and portrait-focus. Bind actual bone/socket names through a validated rig mapping; do not assume all vendor rigs use our names.
- Head/hair: shared neck seam, rig/coverage compatibility; hair masks for headwear; skin-tone/morph interface consistent. Rigid accessories attach by socket. Deforming hair uses the same family or clearly optional close-up secondary chain.
- Garments: approved body coverage masks hide occluded skin; tested collar/shoulder/hip/knee limits and robe hem clearance. Hidden regions are a visual optimization, not injury state.
- Root motion: in-place locomotion first. Presentation movement cannot rewrite core region/travel authority. Future root-motion integration requires explicit reconciliation.
- Materials: few shared slots; palette/insignia through shared data/custom parameters where feasible, not one unique base material per actor. Measure module section cost before merge/crowd optimization.

Compatibility acceptance: each module imported with provenance; skeleton/bind/scale inspection; idle/run/jump/dodge/sword/meditation extreme poses; neck/cloth/hair seam/clipping review; same recipe at reduced detail; sword socket matches hand/sheath. No compatibility pass is claimed until engine import, animation and visual review occur.
