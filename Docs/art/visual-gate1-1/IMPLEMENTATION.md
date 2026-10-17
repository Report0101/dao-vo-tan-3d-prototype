# Visual Gate 1.1 implementation

Starting revision: `578e49dedd4733e42c46d67d6b9703f7c9b6501c`.
Branch: `feat/ue58-visual-gate1-1`.

The presentation uses one Manny rig family. The hidden body is the pose leader; one skinned garment/underlayer follows that pose. Head, hair and accessory meshes are small recipe-selected bone modules. Three head variants, four hairstyles, three garments, two accessories and several palettes share this stack across player and NPCs. FactionColor controls cosmetic trim and conveys no faction authority.

AppearanceRecipe remains the asset/data boundary. A pure appearanceSeed mapping selects head/hair/robe/accessory/skin/hair palette. Snapshot rebinding/resync only reapplies appearance when identity/seed changes. Integration regression checks every slot after resync; asset tests write deterministic signatures for comparison across Editor processes.

The robe is an original skinned wrap coat with sleeves, underlayer, boots, collar/chest closure, sash and trim. It uses the existing rig, no cloth simulation and no robe-specific character classes. It is prototype art; hem/leg intersections and sleeve folds require human review in the existing walk/run/jump/sword/meditation poses. No final-cloth claim is made.

The jian is original pointed/ridged blade, guard, grip and pommel geometry in one mesh. It attaches to hand_r only after the body mesh exists. Original guard-idle and diagonal-cut/return keys replace the unarmed attack reference. Sword remains cosmetic and emits no damage/hit command.

AuraComponent now owns a NiagaraComponent, not instanced sphere geometry. One NS_CultivationAura contains one local-space emitter, derived from the installed Epic Fountain template, with graph bindings for density, radius, vertical speed, lifetime, size and color. Gravity is zero for slow rising qi. Per-instance sprite material color/intensity and cosmetic pulse belong to the same focused component. Three calm/dense/high-energy styles reuse this system. Pulse/animation seconds never advance the simulation or grant a realm.

CultivationCommandAdapter and receipt validation remain the existing authority boundary. HUD reads requested/accepted/reserved/completed/rejected receipts; meditation is explicitly visual playback. A completed result may arrive while cosmetic meditation is finishing. No simulation delay or outcome is tied to animation.

PortraitComponent captures the actual actor's currently selected meshes and pose into a 256px render target. It uses a controlled key light and capture-only background, with only that actor in the capture list. The HUD requests portraits only for the player and two displayed NPCs. Capture refresh is one second, rather than every frame; no separate portrait asset selection exists.

The same StillwaterValley map and route remain. Authored scholar rocks/canopies, gate roof accents and altar approach improve silhouettes. A restrained warm sun, jade distance fog, low bloom and vignette establish the shared palette. The editor authoring command rebuilds the existing stage components and saves the same map, not a duplicate map.

Camera uses a bounded spring arm, collision probe, 66-degree FOV and restrained lag/substepping, with a closer meditation distance. Human camera comfort review is required. HUD retains authoritative identity/cultivation/tick facts, distinguishes offline fixtures and places command feedback away from the character's face.

Authoring is reproducible with the built Editor commandlet `-run=Visual11Author` on this project. It creates original geometry/materials/animation, composes the engine-template Niagara graph and saves the existing map. It runs in client/editor context so render-only Niagara content is loaded. It does not change the simulation repository, engine source, compiler policy or authoritative protocol.

Known warning fixes are local: delayed hand attachment, supported material usage setter, explicit UE5.8 include order, and 16 movement substeps while retaining the existing 50ms maximum step for presentation movement. Genuine logs are not suppressed.

See PLAYTEST.md for the short manual review. Raw build/test reports, real command receipts, screenshots and timing observations are delivered in the Visual Gate 1.1 evidence output.

Final screenshot review (2026-10-17): the exaggerated neck gap and overexposed portrait were corrected. Faces, hair silhouettes, three robe palettes, sword and all aura presets are visible in actual PIE captures. The deliberately simple garment still has hard sleeve/elbow joins, a rounded chest silhouette, and skirt/leg overlap in the kneeling transition. There is no cloth simulation. These are remaining prototype-art limitations, not a claim of final clothing quality. The HUD can overlap the lower body in a close meditation camera; the right-hand portraits remain readable. Human movement/clipping and camera-comfort review remains open.

Automation reliability: one intermediate full run failed the existing backward-movement assertion while physical mouse look rotated its world-X reference. The test harness now ignores physical look and fixes camera yaw before the injected movement check; the assertion and production controls remain unchanged. Visual screenshots use the same input isolation. The failing report is retained in the delivered audit alongside final results.
