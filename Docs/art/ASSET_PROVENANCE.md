# Gate 1 asset provenance

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
