# Performance observations

Final eight-second Editor PIE samples, D3D11/SM5, RTX 3050 Ti Laptop GPU, viewport captures 1014 x 550. Near 60 FPS cap; not an uncapped optimization benchmark.

| Scene | Recently rendered skeletal actors | FPS | Frame ms | Game thread ms | GPU ms | Niagara systems |
|---|---:|---:|---:|---:|---:|---:|
| Player + two NPCs | 3 | 60.00 | 16.667 | 8.114 | 16.666 | 0 |
| Player + ten NPCs | 11 | 59.97 | 16.675 | 9.329 | 16.674 | 0 |

The aura is one original instanced-mesh implementation, with five fixture NPC auras active during the stress measurement. Both PIE tests passed with startup socket warnings. See FINAL_GATE1_REPORT.md for methodology, limitations and debt. Earlier no-runtime baseline is superseded by these measured runs. No production-scale render claim.
