# 04 — Rigging feasibility

No source skeleton/skin exists. Do not create a permanent second humanoid skeleton, new PlayerV1Character class or a second locomotion stack. The target remains the accepted Manny-family skeleton and existing Motion Matching / Pose Search / Chooser / trajectory / foot-IK architecture.

| Region | Required work before binding |
| --- | --- |
| Shoulders / armpits | Rebuild joint loops, volume-preserving armhole and sleeve separation; fit to Manny upperarm/clavicle |
| Elbows | Establish a real elbow axis, inner bend loops and cuff clearance; source sleeve contour is not a joint |
| Wrists / fingers | Remake anatomical hand with five separated fingers, palm, thumb opposition and knuckle loops; fit Manny hand/finger joints and preserve SwordGrip_R |
| Spine / pelvis | Retopo underbody independently from belt and robe; do not weight one continuous skirt as pelvis anatomy |
| Hips / knees | Reconstruct covered legs and joint clearance; source does not expose usable knee topology |
| Ankles / boots | Fit foot, ball and ankle to existing bones; fix pointed/faceted toe and heel/sole support |
| Neck / head | Separate neck seam and retopo face/head with stable loops; no facial rig assumed |

Temporary skinning proof deliberately NOT attempted: fused hands and connected lower garment require remaking first; automatic binding would produce an unrepresentative throwaway result, not feasibility evidence. This is the task's allowed optional-step skip. No new skinning software or provider service was installed or invoked.

Later optional cosmetic chains: 2–4 controlled hair clumps, a few lower robe/sash chains, and pendant/accessory attachments where they improve poses. Their transforms remain cosmetic children of the accepted skeleton; they must not become a new gameplay skeleton or simulation identity. No cloth/Groom/physics added in this gate.
