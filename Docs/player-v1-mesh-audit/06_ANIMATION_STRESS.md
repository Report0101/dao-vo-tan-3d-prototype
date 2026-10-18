# 06 — Animation / pose stress status

No temporary skin exists; therefore NONE of the raw Meshy's Idle/Walk/Run/Strafe/180-pivot/Jump/Landing/Sword guard/Sword attack/Meditation deformations has been executed or accepted. Static T-ish reference views and a rigid global fit are not animation stress tests. Existing Manny gameplay tests passing proves only that the accepted player has not regressed.

| Pose | Specific predicted risk to test after rebuild | Current status |
| --- | --- | --- |
| Idle / strafe | Hair-sheet contact, cuff/hand seam, head/neck silhouette | Not skinned/tested |
| Walk / run / pivot | Long joined skirt crossing thighs, back hair through robe, sash lag | Not skinned/tested |
| Jump / landing | Knee lift trapped in long robe; ankle and hem compression | Not skinned/tested |
| Sword guard / slash | Fused finger grip, armpit collapse, sleeve/body contact, hair/forearm overlap | Structural hand blocker verified; motion not tested |
| Meditation | Large hip/knee folds; joined lower robe cannot form separate panels; back hair pools into waist | Not skinned/tested |

Required future proof: fit retopo to existing Manny rest pose, bind once, polish shoulder/elbow/finger/hip/knee/ankle weights, preserve SwordGrip_R, then run those ten poses in an isolated fixture with closeups and severity metrics. Until then no deformation severity or catastrophic-motion pass/fail is fabricated. No normal playable character was replaced.
