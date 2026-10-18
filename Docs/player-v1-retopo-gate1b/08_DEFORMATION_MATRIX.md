# Player V1 Retopology Gate 1B — Identity & Deformation Recovery

Starting revision: e17d912ea653dc48f0e52141f8b792f1cfd10ed6. Branch: `feat/player-v1-retopo-gate1b`.
Experimental candidate remains `/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/`; production player is unchanged.

Evidence: `outputs/PLAYER_V1_RETOPO_GATE1B` in the task workspace; deliverable archive `PLAYER_V1_RETOPO_GATE1B_EVIDENCE.zip`.
Images of deformed meshes are inspection renders of UE-imported LOD0 vertices skinned from the accepted clips. They are **not live PIE captures**, runtime Animation Blueprint playback, final shaders, cloth or collision certification. Seven frames per clip are measured; three are exported/rendered. CPU linear-blend skinning is used. The reference pose is also inspected separately in Blender.

Seven measured frames per accepted clip; three visual samples per clip. Classification is visual/structural, not based on the peak alone.

|Pose|Verdict|Peak edge ratio|Reason|
|---|---|---:|---|
|Idle|MINOR ISSUE|1.660x|Residual hand/garment/hair fit; no grip demand.|
|Walk|MAJOR ISSUE|2.491x|Thigh/panel opening and root fit.|
|Run|MAJOR ISSUE|2.603x|Lifted panels, hip and sleeve fit.|
|Strafe|MAJOR ISSUE|2.709x|Waist/side panel overlap under hip motion.|
|Pivot|MAJOR ISSUE|2.701x|Panel/thigh clearance and twist.|
|Jump|MAJOR ISSUE|3.697x|Upper robe strain and panel lift.|
|Landing|MAJOR ISSUE|3.430x|Root/panel and shoulder compression.|
|SwordGuard|BLOCKER|2.314x|Open fingers do not wrap accepted handle.|
|SwordAttack|BLOCKER|3.179x|Grip unresolved; sleeve/panel issues also remain.|
|Meditation|MAJOR ISSUE|2.728x|Lower garment/leg seating and back clearance.|

No original MAJOR/BLOCKER has been relabeled as passed without evidence. Numeric improvements are separately reported; Guard/Attack remain BLOCKER.
