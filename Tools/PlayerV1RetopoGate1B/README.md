# Gate 1B validation/authoring tools

Run from the existing task workspace, with R: mapped to the canonical checkout. Scripts use the existing Blender 5.2 and UE 5.8.3 installations; no installers or downloads. Evidence root is outputs/PLAYER_V1_RETOPO_GATE1B. Read 00_SCOPE.md and FINAL_REPORT.md first. This is a B candidate, not approved for production.

Order: preserve inputs / baseline -> builder -> experimental-only import -> UE RetopoGate1Validate commandlet with -ComponentPose and fixture -> render UE OBJ -> topology/UV inspection -> Editor/Game build and real Full/Offline/Live/Reconnect fixtures. Copy the builder output .blend into the existing SourceData filename only after validation. Do not run all authoring scripts blindly against production assets.

The accepted Manny reference/Jian FBXs and clip manifest are task evidence references, not a second asset pipeline. Mounting BeforeRoot is read-only validation of the accepted commit; candidate assets still use only the canonical experimental boundary. Failed trial metrics are preserved separately. Renderer recomputes inspection normals but cannot certify runtime shading, physics or self-intersection.

Regression scripts use unique fixture names and intentionally refuse stale fixture directories. Rename labels for a new run. Current all-build script invokes Full/Reconnect; invoke the inputs script afterward for Offline/Live. Worker cleanup is limited to its owned process/lock. No worker repository edits.
