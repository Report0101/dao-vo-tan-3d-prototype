# Asset mapping

Profile: /Game/DaoVoTan/Experimental/PlayerV1/DA_PlayerV1Experimental.

Leader: /Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.
Skeleton: /Game/Characters/Mannequins/Meshes/SK_Mannequin, 89 reference bones.
Production startup recipe remains /Game/DaoVoTan/Data/DA_Player; connected snapshots may create seeded transient variants from accepted Player/NPC recipes. Preview does not overwrite them.

All mesh references are in /Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/:

|Part|Asset|
|---|---|
|Underbody|SK_Body_Underbody|
|Head|SK_Head|
|Hands|SK_Hands|
|Boots|SK_Boots|
|Hair|SK_Hair|
|Upper robe|SK_UpperRobe|
|Front panels|SK_LowerRobe_FrontL, SK_LowerRobe_FrontR|
|Back panels|SK_LowerRobe_BackL, SK_LowerRobe_BackR|
|Side panels|SK_LowerRobe_SideL, SK_LowerRobe_SideR|
|Belt/sash/jade|SK_Belt, SK_Sash, SK_Accessory_Jade|

The read-only material audit found null interfaces in the existing Gate 1B mesh slots, explaining the checker material in the first live run. The preview profile now maps seven INTERNAL_* slot names to simple MI_Preview_* instances under /Game/DaoVoTan/Experimental/PlayerV1/Preview/. Skin/EyeFoundation reuse M_Skin, Hair reuses M_Hair, and InnerCloth/OuterCloth/Leather/MetalJade reuse M_Robe. These existing parents already support skeletal meshes. Only Tint values are supplied; no new shader graph, final textures or material polish.

Bindings are applied to follower components, not saved into the source meshes. Every mesh/weight/UV/animation asset from Gate 1B remains byte-for-byte unchanged. All mesh paths live in the recipe data; they are not scattered through gameplay code.

Tools/PlayerV1ExperimentalPreview/Create-PreviewProfile.py recreates/updates only the isolated profile and preview material instances when run through Unreal Python after compiling the current schema. It never reimports Meshy or modifies ArtSource. The delivered profile/assets are already created; no generation step is required for normal human preview.

Candidate cost: 15 visible follower modules, 18 skeletal sections, 46,248 triangles at LOD0. This excludes NPCs and the hidden Manny animation leader; it is not a whole-scene draw-call count.
