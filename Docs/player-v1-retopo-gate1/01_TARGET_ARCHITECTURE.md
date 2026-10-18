# Target and boundary

All 15 candidate skeletal meshes and the reference-pose inspection map are under /Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/. SourceData/PlayerV1_RetopoGate1.blend is the editable authoring source in that same content boundary. FBX exports and read-only animation/reference derivatives are in the local evidence bundle, not automatically imported as additional game content.

Manny -> existing Motion Matching / Pose Search / Chooser / foot IK -> shared-skeleton modular visual followers -> one AppearanceRecipe owner. No PlayerV1Character, duplicate runtime skeleton, animation graph, action owner, movement stack or simulation identity was introduced. The single new C++ commandlet is editor-only inspection code (non-editor Main returns failure); it reads a fixture JSON and does not save assets or execute gameplay.

Normal playable character, production Manny/skeleton, AppearanceRecipe, robes/hair/jian, GameMode/DefaultPawn and Stillwater defaults are unchanged. Candidate meshes are not selected by any production recipe. Simulation repository and CultivationCommandAdapter/protocol/time/world authority remain unchanged.

The candidate is authored directly to Manny A-pose joint positions at scale 1; it is not a rigidly scaled T-pose source. The audit's .950169519 scale is only the visual source reference fit. Reference pose matches all named Manny bones to about .00021 cm / .00029 degrees; exact measured values are in evidence/engine-deformation.json. No Manny rest pose was altered.
