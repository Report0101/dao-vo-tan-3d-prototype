# Player V1 experimental preview architecture

Starting SHA: dfac6897983adc58406a820928eae70801902833. Implementation branch: feat/player-v1-experimental-preview.

The existing UAppearanceComponent remains the only visual appearance owner. Existing UPresentationRecipe (the project's AppearanceRecipe data asset type) gains optional SkeletalModules and SkeletalSlotMaterials. Empty arrays/maps retain the accepted variant-based renderer. Production DA_Player is unchanged.

One experimental profile, /Game/DaoVoTan/Experimental/PlayerV1/DA_PlayerV1Experimental, references the same 15 Gate 1B meshes. Followers attach to the existing hidden Manny component using Leader Pose, with collision/overlaps/navigation disabled. No extra animation instance, skeleton, pawn, controller, GameMode, locomotion, sword or Cultivate system is created.

F7 is bound in the existing controller only for editor PIE. UAppearanceComponent also validates local player ownership and PIE world type. Its transient selection defaults false on each new pawn/session. Non-editor Development/Shipping builds have no input binding, no HUD badge and ToggleExperimentalPreview returns false. Packaged builds were not separately cooked in this gate.

The component retains the latest accepted/seeded Recipe when Apply is called, even during preview. The selected preview recipe is used only to rebuild visible modules; AGateCharacter::Recipe remains the accepted current recipe. Toggle back restores its latest variants and colors. Snapshot appearance resync can update this recipe without silently discarding the local preview selection.

Assets and material bindings are resolved before destroying existing visual components. The Manny asset is assigned only if it actually differs, avoiding animation-instance reset on a toggle. Old instance components are removed/destroyed; repeated switching is tested for stable component count. Blade/socket, action phase, movement component, controller, actor identity, commands and receipts are untouched. Portraits use their existing periodic actor-component refresh.

No preview state is written into simulation, save schema, world state or protocol. No default recipe/map/GameMode/DefaultPawn changes. The profile requires explicit local F7 selection and resets on the next PIE session.
