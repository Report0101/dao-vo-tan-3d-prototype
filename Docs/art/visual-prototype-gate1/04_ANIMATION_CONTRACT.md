# Animation implementation

One SK_Mannequin family is used by Manny/Quinn player/NPCs. Runtime uses the same single-node playback controller for all characters:

- Idle/walk/run: Epic BS_Idle_Walk_Run and its unarmed clips.
- Jump: CharacterMovement ballistic presentation with MM_Fall_Loop while airborne; jump-start/landing refinement deferred.
- Sword: Epic MM_Attack_01 temporary arm swing with an original primitive sword attached under hand_r. This is not final sword choreography and has no collision damage logic.
- MeditationEnter/Loop/Exit: three local original-key sequences authored on Manny, derived from Epic idle base pose. Kneeling and breathing keys are pipeline proof, not final motion capture.

UPresentationActions owns typed visual phases and local movement suppression/restoration. No animation notify decides damage, death, cultivation, inventory, time or NPC actions. Current implementation uses no gameplay animation notifies. The earlier sixteen-motion contract remains a future target; Gate 1 does not claim those sixteen polished clips exist.
