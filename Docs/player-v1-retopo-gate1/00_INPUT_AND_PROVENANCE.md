# Input, provenance and baseline

INTERNAL / EXPERIMENTAL CHARACTER DEVELOPMENT. No production, shipping or commercial clearance is implied.

Accepted starting commit: 6965030d1fe79d1706a2b0a377af360eaf1ffa4d. Starting branch feat/player-v1-mesh-feasibility; implementation branch feat/player-v1-retopo-modular-gate1. Initial working tree contained exactly two intentional untracked inputs, no tracked modifications.

Immutable source: C:\Users\Le Anh Hoang\Documents\Codex\2026-10-15\files-pasted-by-the-user-you\work\dao-vo-tan-3d-prototype\ArtSource\Meshy_AI_Jade_Warden_1017231731_generate.fbx

FBX SHA256 before AND after: faa4aada36218026646fbaccc8e28226629797f8d89b5eb543ca58d13b5261b3. Size 3,264,956 bytes, FBX 7.4. Original remains untracked and unchanged. The preserved T-pose PNG remains untracked and was NOT opened or used. No raw triangles were reused: reconstruction uses new controlled quad surfaces informed by the accepted silhouette audit, plus the existing Manny FBX as joint/weight reference. This is an engineering foundation, not a claim of finished artist sculpt/identity matching.

Audit inputs: Docs/player-v1-mesh-audit/01 through 08 and FINAL_REPORT.md. Source Meshy attribution is unverified commercial provenance; no generation-job/license proof was added. Existing Epic Manny/jian/accepted project animations were read-only references. No QCBH content, cloud service, external upload, paid generation or new installation was used.

UE 5.8.3 CL 58210709 at C:/Program Files/Epic Games/UE_5.8. MSVC directory 14.44.35207; cl ProductVersion 14.44.35229.0; UBT selects 14.44.35229 / SDK 10.0.26100.0. Banned 14.50.35722 unused. Existing Blender 5.2.0 LTS used. Baseline Editor/Game builds passed before content work.

- RetopoGate1BaselineFull: 12 successes + 1 warning-successes, 0 failures.
- RetopoGate1BaselineOffline: 0 successes + 1 warning-successes, 0 failures.
- RetopoGate1BaselineLive: 0 successes + 1 warning-successes, 0 failures.
- RetopoGate1FinalFull: 13 successes + 0 warning-successes, 0 failures.

Reported successes include fixture-specific informative returns; these are not falsely counted as additional executed tests. Full baseline executes ten existing assertions suites and reports 13 entries. Offline/live fixtures separately exercise their real input paths. Warnings/logs remain in the ZIP. Final revision is in delivered revisions.json; no merge/push.
