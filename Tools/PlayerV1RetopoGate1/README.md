# Player V1 Retopo Gate 1 tooling

Internal experimental, local-only authoring and read-only validation. No provider/install/upload.

Read Docs/player-v1-retopo-gate1/FINAL_REPORT.md first. Gate result B; grip/robe/head rework remains. Do not replace the player.

Recorded workspace: C:/Users/Le Anh Hoang/Documents/Codex/2026-10-15/files-pasted-by-the-user-you. R: is a verified subst alias of its existing work/dao-vo-tan-3d-prototype checkout. Scripts intentionally target this supplied source/task workspace; adjust task root deliberately if relocating. Source FBX hash is asserted; it is never an export destination. The preserved T-pose PNG is not used.

Order: baseline_all -> baseline tests; export references with UE Python commandlet; build_candidate with existing Blender; uv_quality and topology_quality; import_candidate with UE Python; RetopoGate1Validate C++ commandlet with fixture JSON; render_engine_poses and refresh_wires; final_all. The .blend under the candidate Content/SourceData boundary is editable; generated FBX/accepted clip FBX/OBJ exports stay in the output bundle. Export geometry nodes are Geo_ prefixed to avoid a verified Head/head bone-name collision. Re-running build overwrites only task-derived outputs; import overwrites only this explicitly owned experimental prefix.

PythonScriptPlugin/EditorScriptingUtilities are enabled for the commandlet invocation only. Legacy FBX is selected by supported per-process Interchange.FeatureFlags.Import.FBX=0, not an engine patch or persistent config change. Early Interchange reimport waited indefinitely for a skinned asset; only that owned commandlet was stopped. Legacy FBX emits recovered bind-pose warnings; the C++ fixture directly checks reference bones/weights and CPU skinning before structural acceptance.

Run read-only engine fixture after Editor build:

    UnrealEditor-Cmd.exe R:/Unreal/DaoVoTan3D.uproject -run=RetopoGate1Validate -Fixture=R:/Unreal/Saved/RetopoGate1Fixture.json -Evidence=R:/Unreal/Saved/RetopoGate1EngineFixed -unattended -nop4

Fixture schema: referenceMesh asset path; modules array strictly constrained to /Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/; clips object of existing accepted animation paths. It never saves source assets or gameplay state. It samples seven times, reports skeleton/section/weight/strain metrics and writes midpoint OBJ and exact existing JianGripTransform. It is not a new runtime animation system. Rendering uses the exact exported positions with smooth inspection normals, not fabricated pose images or a UE final-shader claim.

The baseline/final worker fixture runners create fresh ignored Saved sessions and stop only their own workers. They do not modify simulation source. Evidence/fixture.json and regression JSON identify exact coverage. No production defaults, existing skeleton, recipe, core MM, sword logic or authority protocol should change.

Run PowerShell runners from the recorded task workspace (the parent containing work/ and outputs/). Geometry capture writes geometry images only; authoritative pose images come exclusively from retopo_render_engine_poses.py after the UE validator.
