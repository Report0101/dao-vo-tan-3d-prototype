# Visual Gate 1.1 validation — 2026-10-17

Starting visual commit: `578e49dedd4733e42c46d67d6b9703f7c9b6501c`.
Branch: `feat/ue58-visual-gate1-1`. Simulation remains unchanged at `b7774fe23473f695869d76a38dfe04d34bb75791`.

UE 5.8.3 CL 58210709, installed at `C:\Program Files\Epic Games\UE_5.8`. Project remains in the existing canonical checkout at `C:\Users\Le Anh Hoang\Documents\Codex\2026-10-15\files-pasted-by-the-user-you\work\dao-vo-tan-3d-prototype\Unreal\DaoVoTan3D.uproject`. R: is a path alias for that checkout.

Compiler directory: `C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.44.35207`; compiler `bin\Hostx64\x64\cl.exe` ProductVersion `14.44.35229.0` (FileVersion `19.44.35229.0`). UBT selected 14.44.35229 with Windows SDK 10.0.26100.0. No compiler ban bypass or UBT patch.

Project generation, Development Editor build and Development Game build PASS. Full final Editor automation: **7 success, 0 warning, 0 failed**. Fresh-process Visual11 stress: **2 success, 0 warning, 0 failed**. The actual Editor opened StillwaterValley and ran PIE for these checks. No missing SDK/toolchain/plugin dependency. Native asset authoring also completed with 0 errors and 0 warnings.

The prior baseline was verified before modifications: project generation, both targets, all five existing automation tests, and 3-/11-character timings. Final full coverage retains PlayableSlice, CommandReceiptProtocol, CommandRoundTripPIE, ReadOnlyPIE, SnapshotProtocol, plus AssetAndSeedContract and PresentationPIE. One intermediate world-X movement test failure was diagnosed and its camera input isolated; the assertion remains, and the final full run passes. The failed run is retained in the evidence audit.

Real integration regression: request `00281ED243C3528FC127FD9537947558`, action `command1:action:00000001`, cultivation event `command1:event:00000044`, tick 1527 → 1534 for 7 strategic days; one cosmetic sequence and one completed result. Duplicate receipt did not repeat completion; wrong actor returns WRONG_ACTOR. Final authoritative world save SHA256 `c29ee1837b4e89a662fc7172f37b05014df57d5403341eba1df089992d664fa4` matches the accepted Integration Gate 2 reference. Authority adapter/receipt/parser/contracts were not changed. Fresh-process seed proofs match byte-for-byte, and snapshot resync checks retain selected modular slots/palettes.

## Timings

Windows Editor PIE, Intel Iris Xe, i7-11370H, 16GB RAM. Final viewport 1218×646, roughly 8 seconds of sampling after warmup; frame rate is capped near 60. All final character modules are present and all final aura instances active, with one shared Niagara system asset and at most three portrait captures.

| Scene | FPS | Frame ms | Game ms | Render ms | GPU ms | Visible skeletal garments | Active Niagara |
|---|---:|---:|---:|---:|---:|---:|---:|
| Accepted baseline, player + 2 | 59.99 | 16.669 | 8.082 | not recorded | 16.668 | 3 legacy bodies | 0 |
| Final, player + 2 | 59.995 | 16.668 | 8.190 | 6.533 | 16.672 | 3 | 3 |
| Accepted baseline, player + 10 | 60.00 | 16.668 | 8.899 | not recorded | 16.666 | 11 legacy bodies | 0 |
| Final, player + 10 | 59.999 | 16.667 | 8.555 | 6.632 | 16.675 | 11 | 11 |

These are observations, not a demonstrated optimization or regression: the visual workload, aura backend, camera and potentially baseline viewport differ, and the cap hides headroom. Hidden Manny pose leaders still animate alongside the visible garment meshes. Final RenderThread stats were explicitly enabled during sampling; an earlier stale-stat experiment is excluded.

## Visual and warning review

Actual PIE screenshots cover hero, player with two NPCs, sword idle/attack, meditation entry, three aura presets, matching portrait UI, valley and real Cultivate/receipt result. Separate portrait PNGs are exported from the actual actor render targets. No retouching or generated replacement screenshots.

Long neck and portrait overexposure were corrected. Remaining prototype limitations: hard sleeve/elbow joins, rounded chest silhouette, skirt/leg overlap during kneeling, simple hair/vegetation silhouettes, and lower-body HUD overlap at close meditation framing. No final cloth or final art quality claim. Human walk/run/jump/attack/meditation and camera-comfort review is still required.

Final automation reports have zero test warnings. Raw startup log retains an EditorLayouts v1.5/v1.6 migration warning, outside test execution. Known hand socket, movement iteration and material usage warnings do not recur in final test results. The engine also logs unavailable optional platform icons at startup; these do not block this Windows project. Warnings were not suppressed.

Assets: 25 new prototype assets, plus 3 updated recipe assets and the same valley map. Per-file hashes/source/license/commercial-use/modification/temporary status are recorded in asset-manifest.json; Epic rig/idle and Niagara template dependencies retain Unreal EULA terms. No QCBH, unknown-license, marketplace or paid-generation imports.

Technical verification PASS. **NOT READY FOR VISUAL GATE 2** pending human visual/play-feel acceptance and review of the recorded prototype limitations. Visual Gate 2 has not begun.
