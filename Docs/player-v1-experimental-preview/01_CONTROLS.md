# Controls and human retest

Open Unreal/DaoVoTan3D.uproject in UE 5.8.3 and use the existing StillwaterValley startup map. Press Play, then click the PIE viewport to give it keyboard focus.

- F7: Current Player <-> Player V1 Experimental, without respawn or restart.
- F8 remains Unreal's PIE Eject shortcut; it is not the preview toggle.
- WASD: move. Shift: existing walk/run toggle. Q: existing strafe toggle.
- Space: jump. Mouse: rotate camera. LMB: existing sword attack.
- Sword guard/carry is the existing idle sword layer; no new guard input was added.
- E or M: existing authoritative Cultivate. - / +: duration. 1: existing aura palette.
- Shift+F1: release the mouse to the editor. Stop/Play starts again on Current Player.

The HUD badge shows CURRENT PLAYER or PLAYER V1 EXPERIMENTAL. Switching affects only the local player's visuals; NPCs remain unchanged.

## Connected human session
F7 and movement work in ordinary PIE. Authoritative E/M still requires the existing TypeScript worker and -CultivationBridgeDir launch argument. Opening the project alone does not start a worker; the existing unavailable/not-ready feedback remains accurate.

From PowerShell, run Tools/PlayerV1ExperimentalPreview/Start-LivePreview.ps1 -SimulationRepository '<existing tu-tien checkout>'. The parameter is optional when tu-tien is beside the presentation checkout; otherwise supply its actual path. Open this launcher from the physical checkout path. If an Editor was opened without the bridge argument, close it before launching this connected session. Appearance changes themselves never require restart. The helper uses the existing compiled dist-p1/cli/cultivation-bridge.js, creates/reuses a dedicated Saved/PlayerV1HumanPreviewBridge session, and opens this same project with the bridge argument. It keeps its owned worker running across PIE Stop/Play and stops only that owned worker when this Editor exits. An already verified external worker is reused and not stopped. Existing world/session data is preserved. No repository clone, simulation build, engine installation or protocol change.

For this PC the simulation checkout is C:/Users/Le Anh Hoang/Documents/Codex/2026-10-15/files-pasted-by-the-user-you/work/tu-tien. The engine default is C:/Program Files/Epic Games/UE_5.8; -EngineRoot and -BridgeDirectory may be explicitly supplied.

Retest: start on Current Player -> F7 -> walk/run/jump/strafe/pivot -> orbit front/back/side -> LMB -> E/M and wait for recovery -> F7 back. Stop/Play twice more with the worker alive, selecting Player V1 explicitly each time. Do not use -GateDeveloperPreview for the authoritative Cultivate test; that pre-existing flag is an offline cosmetic fixture.
