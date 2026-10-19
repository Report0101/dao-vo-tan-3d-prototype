# Player Q1 recovery tools

These are offline authoring tools for the existing project-owned procedural
clips. They introduce no runtime animation system or simulation state.

- author_grip.py changes right digit rotations only in AS_SwordIdle and
  AS_SwordAttackA, using grip-pose.json. The hilt is aligned by the existing
  GateCharacter presentation socket. It preserves timing, bone translations,
  scales and all other tracks and writes a preservation receipt under Saved.
- author_meditation.py changes six leg rotations and pelvis Z in the existing
  AS_MeditationEnter/Loop/Exit. The target in meditation-pose.json was solved
  against MM_Idle using component-space two-link leg geometry, +Y forward,
  ankle targets (+/-18,-12,12) cm and forward knee poles. Mirrored local axes
  must not share an assumed bend axis. Feet retain their idle orientation.
- Pelvis height is idleZ - 48*a + 36*a*(1-a), where a is smoothstep progress
  (reversed for exit). The lift term prevents the interpolated toes crossing
  the floor. The original Gate1AuthorCommandlet uses the same target and arc;
  do not rerun its broad map/asset generation just to update these clips.
- Each authoring script runs with UnrealEditor-Cmd, -run=pythonscript and a
  forward-slash -script path. Run with the editor closed, then rebuild and run
  the dedicated live preview fixture. These scripts intentionally save assets.
- The meditation receipt checks 121 samples per clip for skeleton clearance.
  This is not mesh collision, cloth, weight or Q1 certification. The live PIE
  test checks settled knee/ankle/toe clearance and captures enter/loop/exit.

No source FBX/PNG is modified. Existing provenance and visual limitations remain.
Keep the original visual shortcomings visible in the acceptance report.
