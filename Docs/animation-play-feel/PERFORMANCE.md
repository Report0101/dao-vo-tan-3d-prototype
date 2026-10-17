# Timing observations

Same personal PC: i7-11370H, Intel Iris Xe, 16GB RAM. Windows Editor PIE, 1218×646, frame rate capped near 60. Final samples cover approximately 8 seconds after warmup. Modules and all cultivation auras are active, with one shared Niagara asset and a maximum of three portrait captures. Hidden Manny pose leaders still animate in addition to the visible skeletal garments.

| Scene | FPS | Frame ms | Game ms | Render ms | GPU ms | Visible garments | Niagara instances |
|---|---:|---:|---:|---:|---:|---:|---:|
| Accepted Visual 1.1, player + 2 | 59.995 | 16.668 | 8.190 | 6.533 | 16.672 | 3 | 3 |
| Animation gate, player + 2 | 59.996 | 16.668 | 8.580 | 6.482 | 16.676 | 3 | 3 |
| Accepted Visual 1.1, player + 10 | 59.999 | 16.667 | 8.555 | 6.632 | 16.675 | 11 | 11 |
| Animation gate, player + 10 | 59.999 | 16.667 | 9.651 | 6.841 | 16.676 | 11 | 11 |

These are capped Editor observations, not demonstrated optimization, regression bounds or standalone shipping performance. The final stress run exercises native locomotion instances on 11 stationary characters with active visual modules/auras. It does not represent eleven independently moving AI actors, network play, or a large world. Separate input-driven PIE tests exercise the player's starts, run, reversal, strafe, turn, jump, landing and actions.

Full final automation: 9 passed, including two tests with external Editor HTTP/EOS warnings. Fresh-process Visual11 stress: 2 passed, including one with an external HTTP warning. There are no failed tests. Fresh-process recipe seed proofs match. Raw timing files and reports are included in the evidence output.
