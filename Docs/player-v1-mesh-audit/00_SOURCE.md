# 00 — Source and baseline

Purpose: original Player V1 prototype feasibility audit, not production implementation or Visual Gate 2.

- Repository: Report0101/dao-vo-tan-3d-prototype.
- Starting branch: feat/ue58-cultivation-bridge-stabilization; SHA 88f6f6f086325cd18596c645be2d9628cecb80b1.
- Audit branch: feat/player-v1-mesh-feasibility.
- Original: C:\Users\Le Anh Hoang\Documents\Codex\2026-10-15\files-pasted-by-the-user-you\work\dao-vo-tan-3d-prototype\ArtSource\Meshy_AI_Jade_Warden_1017231731_generate.fbx.
- Size: 3,264,956 bytes. FBX binary 7400 (7.4).
- SHA256 before: faa4aada36218026646fbaccc8e28226629797f8d89b5eb543ca58d13b5261b3.
- Filesystem created 2026-10-18 01:17:37.4687506 +02:00; modified 01:17:38.3901759 +02:00. These are filesystem times, not proof of generation time. Embedded writer date is a 1970 placeholder.
- Source is intentionally untracked and stays untracked; it is never overwritten, moved, renamed, reset or stashed. The matching working copy is in outputs/PLAYER_V1_MESH_AUDIT/working/MeshyAudit_SourceCopy.fbx.
- Additional untracked ArtSource/T-pose character 2D.png was investigated. The user explicitly requested preservation without use in this audit; it was not opened or used as a visual reference. Its checksum is retained in baseline.json.

Unreal 5.8.3, CL 58210709, C:\Program Files\Epic Games\UE_5.8; project association 5.8. VS 2022 Enterprise 17.14.42, installation 17.14.37806.7. MSVC directory 14.44.35207, cl ProductVersion 14.44.35229.0; UBT selected 14.44.35229 and Windows SDK 10.0.26100.0. VS 2026/14.50.35722 is installed but was not used. Blender 5.2.0 LTS was already installed. No software was installed.

Baseline Editor/Game builds PASS. Full Unreal suite has 12 success entries plus one warning-success entry; three fixture-specific tests return informative skips here. Dedicated offline/live manual-input fixtures each PASS. Executed coverage includes animation/play-feel, correction sequence, repeat attacks, E/M, real seven-day Cultivate and snapshot/receipt regression. Logs retain warnings; no unexpected broken baseline was bypassed.

Provenance: supplied by the user as Meshy AI; FBX writer metadata reports Meshy / mblend native FBX writer 5.12.7. The Meshy generation prompt, job ID, account tier, original reference provenance and license/ownership evidence were not supplied. Filename and writer metadata do not establish a commercial grant or prove “Meshy 6 Lite.” Commercial rights remain UNVERIFIED. No QCBH or other game-character assets were imported. Manny is the existing project reference and was only exported/read, never edited.

## Final regression verification

PlayerV1FinalFullRetry completed: 9 successes + 4 warning-successes, 0 failures (13 reported entries; fixture-specific informative returns are not additional executed tests). Warnings are external HTTP connectivity / Epic telemetry, not assertion failures. The earlier final run was interrupted and produced no report; its log is retained. The R: alias was restored to the verified canonical checkout and a fresh worker fixture was used, without reusing or resetting any prior session.

Accepted / baseline / final actions.live JSON SHA256 all match: c29ee1837b4e89a662fc7172f37b05014df57d5403341eba1df089992d664fa4. See evidence/authority-agreement.json. Source FBX and preserved-unused PNG checksums were rechecked unchanged. Simulation HEAD remains 57d84b3dec06d22fe3f250902db7d7cbe79c54f7 with no working-tree changes.
