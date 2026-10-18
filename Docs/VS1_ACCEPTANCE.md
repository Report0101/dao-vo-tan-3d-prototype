# VS1 Acceptance Gate — Unreal Bootstrap

Status: implementation candidate

## Goal

Prove that the 3D prototype can boot as a native Unreal Engine C++ project without introducing coupling to the `tu-ti-n` simulation repository.

## Required behavior

When the project is built and launched locally in Unreal Engine:

1. The game boots into the engine Entry map using `ADaoVoTanGameMode`.
2. A collision floor and simple landmark primitives are created at runtime.
3. A visible placeholder player pawn is spawned and possessed.
4. Third-person camera follows the player.
5. WASD moves relative to camera yaw.
6. Mouse controls camera yaw/pitch.
7. Space jumps.
8. Left Shift sprints while held.
9. No `tu-ti-n` code, package, submodule, or direct source dependency exists in the Unreal project.
10. Runtime prototype code remains presentation-only. Simulation rules are not duplicated here.

## Source gate

GitHub CI may verify repository structure and architectural boundaries without Unreal installed.

## Native build gate

VS1 is NOT accepted until a machine with Unreal Engine and the required C++ toolchain successfully:

- generates project files,
- builds `DaoVoTan3DEditor`,
- opens `DaoVoTan3D.uproject`,
- enters Play In Editor,
- validates controls 3–8 above.

Do not mark this gate green based only on source inspection.

## Next gate

VS2 should replace primitive presentation with the first real environment/character asset layer while preserving the same gameplay boundary.
