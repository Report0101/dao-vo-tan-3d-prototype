# Third-person camera contract and validation

Date: 2026-10-15 (Europe/Budapest). Repository: `Report0101/dao-vo-tan-3d-prototype`; local branch `main`, starting commit `9fa034e82fe9fd1538174f11f9e1e16314e23c40`. **Prepared specification/contracts only; Unreal runtime unbuilt and unvalidated.**

Use one character follow-camera component with spring-arm-style collision probing, separate input/look controller and configurable settings. Initial tuning proposal: 300–400 cm follow distance, ~70–90° FOV, shoulder offset modest, pitch clamps and recenter optional. These are test starting values, not verified feel/performance.

Keep yaw/pitch/local camera timing outside simulation authority. Camera collision shortens the boom; never teleports actor/core location. Smooth recovery after occlusion; avoid swinging through walls, sudden FOV changes and strong meditation shake. Meditation may gently reframe without disabling user look indefinitely. Sword cue camera impulse bounded and cosmetic.

Validation matrix: walk/run/slope; jump/land; dodge near wall; AttackA/B/Parry near tree/cliff; enter/exit meditation; approach both NPCs; tight passage and low ceiling; rotate 360°; change resolution/aspect/input sensitivity. Record stuck camera, near-plane clipping, occlusion recovery, motion sickness and input responsiveness.

Test at least 10 minutes of actual traversal and transitions; record engine version, map, settings, input device and video/build identity. Current result: NOT RUN; no camera code/Blueprint/build exists. No cinematic camera system is planned.
