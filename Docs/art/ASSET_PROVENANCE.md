# Asset provenance — Gate 1 and Visual Gate 1.1

External content is limited to installed Epic Examples and referenced engine primitives. No QCBH content, unknown-license content, marketplace download or paid generation was used.

| Asset | Source | License | Purpose | Status |
|---|---|---|---|---|
| SKM_Manny_Simple, SKM_Quinn_Simple, SK_Mannequin; associated materials, textures and rig resources | Local UE 5.8.3 Templates/TemplateResources/High/Characters/Content/Mannequins | [Unreal Engine EULA](https://www.unrealengine.com/eula/unreal), Examples under section 1; development use section 2; Examples distribution section 5(b) | Shared temporary humanoid family for all three characters | Temporary Epic Examples |
| BS_Idle_Walk_Run, MM_Idle, unarmed walk/jog/jump and attack clips; accompanying ABP_Unarmed | Same installed Templates location, Anims/Unarmed | Same EULA Examples terms | Locomotion and temporary arm-swing sword action; ABP is included but runtime uses single-node animation | Temporary Epic Examples |
| Engine basic cube, sphere, cylinder and cone; engine font, sky and atmosphere | Referenced from installed engine; not copied into repository | Unreal Engine EULA Licensed Technology | Original blockout sword, garment modules, rocks, conifers, gate, altar, UI and lighting | Temporary primitive-based art |
| AS_MeditationEnter/Loop/Exit | Generated locally by Gate1AuthorCommandlet from Manny rig and Epic idle pose, with original kneeling/breathing key adjustments | Project-authored modifications to Epic Examples, subject to EULA | Meditation pipeline proof on the same skeleton | Temporary derived animation |
| StillwaterValley landscape/height data, layout, M_Color/M_Aura/M_Ground/M_Rock/M_Stone/M_Wood/M_Leaf/M_Gold/M_Steel, DA_Player/DA_NPC_A/DA_NPC_B | Original local C++ authoring in this repository | Project-authored content; engine use remains subject to Unreal EULA | Valley, palettes, wardrobe/accessory configuration and original reusable aura | Original prototype content |

The per-file manifest `template-assets.manifest.json` records each copied template file, source, SHA-256, size, purpose and temporary status. No engine binary or engine source is vendored. Preserve Epic notices and applicable license terms; the project is not granting a new unrestricted license to Epic assets. EULA checked 2026-10-16. No license acceptance was automated.

The rig resources and imported ABP are supporting template files, not separately authored project pipelines. Manny/Quinn are temporary neutral silhouettes; bespoke cultivation clothing/head/hair/skinned wardrobe is deferred. Current garment and accessory modules are rigid bone attachments, not a claimed production skinned modular character system.

## Visual Gate 1.1 additions

No Fab/Marketplace downloads, QCBH assets, unknown-license content or paid asset generation were used. Asset generation is reproducible in `Visual11AuthorCommandlet.cpp`; all new meshes are original project-authored geometry. The existing Epic Manny skeleton/idle pose remain the shared compatibility foundation.

| Name | Source | License / commercial use | Modification | Status |
|---|---|---|---|---|
| SM_Head0..2, SM_Hair0..3, SM_Accessory0..1, SM_Jian | Original local parametric mesh authoring in Visual11AuthorCommandlet | Project-authored; no third-party art input; usable in this project commercially | Original geometry | Prototype art, not final art |
| SK_Robe0..2 | Original local garment/underlayer geometry, weighted to the existing Epic Manny reference skeleton | Original geometry; shared Epic rig remains subject to Unreal EULA; commercial Unreal product use permitted under applicable EULA terms | Original skinned meshes on existing rig; no new skeleton | Prototype skinned wardrobe; no cloth physics |
| AS_SwordIdle, AS_SwordAttackA | Original guard/cut/return keys authored over existing Epic idle reference pose | Modified Epic Examples under Unreal EULA | Original sword choreography, same rig | Prototype reusable animation |
| NS_CultivationAura | Installed UE 5.8.3 Niagara Fountain emitter template, composed into one project system | Epic-provided engine content under Unreal EULA; commercial Unreal product use permitted subject to EULA terms | Local-space emitter, exposed parameters, project sprite material | Prototype shared VFX system |
| M_QiSprite, M_Robe, M_Embroidery, M_Skin, M_Hair, M_JianSteel, M_PortraitBackground | Original local material expressions/parameters | Project-authored; Unreal runtime use subject to EULA | Original materials | Prototype materials |
| SM_Canopy, SM_ScholarRock | Original local geometry | Project-authored, commercially usable in this project | Original geometry | Prototype valley kit |
| Portrait render targets | Runtime SceneCapture of each actual recipe-rendered actor | Derived render of the same documented character assets | No separate portrait art | Runtime-generated, not an imported asset |
| DA_Player, DA_NPC_A, DA_NPC_B; StillwaterValley | Existing project-authored assets | Existing provenance retained | Wardrobe slots/layout/atmosphere updated; same map | Prototype |

The installed template source is `Engine/Plugins/FX/Niagara/Content/DefaultAssets/Templates/Emitters/Fountain.uasset`. Engine Niagara modules are referenced through the installed plugin; no engine binary/source is copied into the repository. The per-file Visual Gate 1.1 manifest records hashes and source categories for new/modified content.

The [official Unreal Engine EULA](https://www.unrealengine.com/eula/unreal) was checked on 2026-10-16: sections 1–2 cover installed Epic content and development, sections 4–5 cover distribution. Epic content is not relicensed as unrestricted project art. No license agreement was accepted by automation.
