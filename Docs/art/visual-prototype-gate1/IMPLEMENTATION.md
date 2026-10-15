# Gate 1 implementation / operator notes

Start SHA: 9fa034e82fe9fd1538174f11f9e1e16314e23c40.
Branch: feat/ue58-visual-gate1. The accepted baseline was captured in 2f31b45 before implementation.

Open Unreal/DaoVoTan3D.uproject, map /Game/DaoVoTan/Maps/StillwaterValley, and Play.
WASD move; mouse orbit; Shift toggles walk/run; Space jump; LMB sword visual action; E or M toggles meditation; 1 cycles jade/ember/violet aura.

All labels and NPC poses are explicit offline presentation fixtures. Unreal awards no resources, realm progress, damage, death, inventory or relationships. Meditation locks local locomotion only; there is no cultivation computation. The sword has no hit/damage authority.

## Ownership

- GateCharacter: movement, collision-tested spring arm, camera, Enhanced Input routing, and weapon attachment.
- GatePlayerController: camera pitch limits/input focus.
- PresentationRecipe: replaceable body asset, palette and rigid module flags, label and aura parameters.
- AppearanceComponent: the same recipe-to-visible-modules pipeline for every humanoid. Manny and Quinn share SK_Mannequin; no NPC subclasses.
- PresentationActions: typed visual state transitions, shared locomotion blendspace/attack clip, three authored meditation sequences, lock/restore local movement.
- AuraComponent: one bounded original instanced-mesh mote field with color, intensity, radius, density and pulse-speed parameters. Three presets reuse it. This is not Niagara; Niagara migration is deferred and active Niagara count is zero.
- GateGameMode: bounded fixture spawning (2 NPCs by default, max 10 via -GateNPCs=10), no NPC decision logic.
- ValleyStage: deterministic original primitive-based static environment instances.
- GateHUD: Canvas-only name, realm fixture, controls and meditation status, no gameplay menus.
- Gate1AuthorCommandlet: editor-only material/recipe/animation/Landscape map authoring; run with -nullrhi to avoid commandlet-renderer crash observed during Landscape creation.
- Gate1PIETest: editor automation exercise and runtime evidence collection, not packaged gameplay behavior.

Map: roughly 378 x 252 metres. Intended path approximately 280 metres, about two minutes walking at 2.4 m/s. Rocky sides, stylized conifers, mist, sky/lighting, a gate and two altar terraces. No streaming continent or generated simulation world.

Current wardrobe is rigid bone-attached geometry, not a final skinned modular outfit. Head/hair asset slots, faction markers, bespoke garments and accurate sword choreography need further art iteration. Existing bridge JSON contracts remain transport-neutral; actual TypeScript transport is not wired into Gate 1.

## Validation

Use the R: alias to the same checkout for UBT builds. Do not clone or move the repository to address path limits.

Run the editor automation test DaoVoTan.Gate1.PlayableSlice. It loads the valley, waits for shaders, starts PIE, checks shared rigs/camera/HUD, drives Enhanced Input walk/run/backward movement and jump, checks sword attachment, meditation enter/loop/exit, aura enable/disable and movement restoration; it captures screenshots and eight seconds of frame samples.

Normal fixture: player + 2 NPCs. Stress fixture: launch with -GateNPCs=10 -GateStress. Reports go to Saved/Gate1Evidence and Saved/Gate1Evidence-Stress. Frame measurements include this machine/editor overhead; zeros for GPU must be treated as unavailable rather than zero GPU cost. A test pass does not imply art acceptance or complete camera/play-feel review.

Portrait SceneCapture is deferred to Gate 1.1. Packaging is secondary to verified Editor/PIE. No QCBH files were accessed or imported for implementation.
