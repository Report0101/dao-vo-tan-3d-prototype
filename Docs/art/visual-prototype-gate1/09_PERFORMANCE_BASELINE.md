# Performance baseline — not measured

Date: 2026-10-15 (Europe/Budapest). Repository: `Report0101/dao-vo-tan-3d-prototype`; local branch `main`, starting commit `9fa034e82fe9fd1538174f11f9e1e16314e23c40`. **Prepared specification/contracts only; Unreal runtime unbuilt and unvalidated.**

| Visible humanoids | FPS | Game thread ms | Render thread ms | GPU ms | Skeletal work | Materials/sections/draw calls |
|---|---|---|---|---|---|---|
| 1 | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED |
| 10 | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED |
| 50 | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED |
| 100 | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED | NOT MEASURED |

Reason: no Unreal project/engine/runtime, rig/assets or reproducible renderer adapter available. Do not substitute schema tests, Blender, screenshots or a software preview for Unreal/GPU evidence. Persistent 10,000 core actors does not mean 10,000 visible/animated meshes.

Future reproducible scenarios: fixed valley camera route; deterministic recipes; quiet and aura/trail-heavy variants; same actor motion schedules; 1080p starting output; warm-up then repeated 60-second captures and 20-minute memory stability test. Record CPU/GPU/RAM/OS, Unreal/build/plugin versions, scalability/resolution, asset hashes, capture method and frame distribution.

Metrics: frame p50/p95, game/render/RHI/GPU ms, animation tick/deformation costs, visible skeletal components/bones, material sections/draw calls, texture memory, load spikes and particle/overdraw cost where tools support them. Separate cold load and steady state. No optimization before a baseline; no performance acceptance threshold invented as achieved.

Presentation budgets may throttle distant animation; simulation outcomes remain unaffected by skipped poses/effects. Compare shared pose/component count and merge alternatives only after measurements. Use sealed game-dev adapter run receipts when tooling is ready; hardware/GPU measurement authorization remains a separate concrete step per installed workflow instructions.
