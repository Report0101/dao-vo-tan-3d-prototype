# 03 — Manny proportion comparison

Reference: existing /Game/Characters/Mannequins/Meshes/SKM_Manny_Simple, read-only export. Its 89 reference bones and component-space transforms are recorded in unreal-import.json; this count describes the supplied project mesh, not every possible full Manny skeleton variant.

- Meshy height 190.01231 cm; Manny bounds height 180.54390 cm; raw height ratio 1.05244.
- Meshy raw arm-span/envelope width 154.36914 cm and depth 56.05445 cm. It is near T-pose while Manny is A-pose; comparing bounding widths directly would falsely imply an anatomical ratio.
- Temporary uniform scale 0.950169519; actor translation (0,0,90.422100) cm; rotation (0,0,0) in the imported mesh frame. Fixture placement additionally offsets X +/-180 cm for comparison. These transforms are not baked into the source or Manny.
- The imported mesh is Z-up and faces the same +Y reference direction as this Manny export. Production actor mesh rotation remains untouched.
- Manny shoulder joint width 38.020 cm; hip joint width 19.938 cm. Left upper-arm/forearm lengths 27.771/27.251 cm; thigh/calf 43.341/42.218 cm.

Manny landmarks (cm): shoulder Z=143.583, elbow Z=120.892, wrist Z=104.467, pelvis Z=95.897, knee Z=50.266, ankle Z=8.238, neck Z=152.804, head joint Z=162.575. The source has no homologous joints. In particular its sleeve hides elbow anatomy, robe hides pelvis/hips/knees, and bun/hair inflate head height/width. Exact source joint ratios are **not measurable honestly from this exterior-only raw mesh**; no invented source knee/hip/head joint numbers are supplied.

**Fit category B — moderate fitting required**, assessed for the retained overall silhouette, not direct skin readiness. The height match is close enough to preserve the concept. Global scale/translation cannot change near-T arms to Manny's A-pose, fit wrist/elbow placement independently, separate fused hands, recover hidden underbody anatomy or create knee/robe clearance. Rebuilt limbs must be fitted to Manny rest joints before skinning. Head/neck need a new seam and face-volume pass; boots need ankle/ball alignment and sole orientation against Manny. Category B is an engineering planning assessment, not deformation acceptance.

Blender reads the exported Manny surface at 180.6448 cm versus Unreal cached bounds 180.5439 cm (approximately .101 cm difference). The saved Unreal fixture transform is the authoritative recorded global fit; Blender closeups use its separately measured exported-surface alignment. This small bounds/export difference is not a claimed anatomical discrepancy.
