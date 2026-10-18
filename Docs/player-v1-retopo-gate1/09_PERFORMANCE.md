# Measured LOD0 cost

| Module | Author vertices | Triangles | UE imported vertices | Slots / sections |
| --- | --- | --- | --- | --- |
| Body_Underbody | 2,017 | 4,030 | 3,421 | 1 / 1 |
| UpperRobe | 2,252 | 4,500 | 5,012 | 1 / 1 |
| Hands | 1,800 | 3,592 | 4,007 | 1 / 1 |
| Head | 2,484 | 4,956 | 3,508 | 1 / 1 |
| Boots | 1,636 | 3,264 | 2,486 | 1 / 1 |
| Hair | 4,588 | 9,136 | 7,354 | 1 / 1 |
| LowerRobe_FrontL | 870 | 1,736 | 1,106 | 1 / 1 |
| LowerRobe_FrontR | 870 | 1,736 | 1,106 | 1 / 1 |
| LowerRobe_BackL | 870 | 1,736 | 1,108 | 1 / 1 |
| LowerRobe_BackR | 870 | 1,736 | 1,108 | 1 / 1 |
| LowerRobe_SideL | 870 | 1,736 | 1,111 | 1 / 1 |
| LowerRobe_SideR | 870 | 1,736 | 1,111 | 1 / 1 |
| Belt | 768 | 1,536 | 1,238 | 1 / 1 |
| Sash | 114 | 224 | 199 | 1 / 1 |
| Accessory_Jade | 122 | 240 | 252 | 1 / 1 |


Total: 41,894 triangles, 21,001 DCC vertices; 34,127 UE imported/split vertices; 15 one-material skeletal sections. Imported counts are not a measured GPU-frame benchmark. UV seams increase split vertices. No Nanite or final LOD chain.

Body/head/hands/boots: 15,842 triangles (below 18–26k target); joint strain and face deficits mean the lower group count is NOT evidence that all detail was retained. Hair: 9,136 (above 4–7k, cap/clump waste identified). Robe: 14,916 (within 10–16k). Belt/sash/jade: 2,000 (within 1–3k). Total 41,894 is within 33–52k, but polygon count does not override failed deformation.

Author mesh checks: zero boundary/non-manifold edges, zero tested zero-area faces, no unweighted vertices, finite normal/tangent basis. Separate hands/boots/ears/hair shells are intentional; component counts and triangle aspect statistics are recorded in evidence/topology-quality.json. 162 triangles have longest-edge/altitude aspect >20; none >100; maximum 68.74 in head/cap geometry. These thin cap/edge triangles need review alongside joint/garment failures; no global self-intersection certification is claimed. All inspected modules have no loose vertices or duplicate indexed faces. Intentional garment/body coverage overlaps coexist with visible unwanted clipping.

15 followers/sections are acceptable for an isolated feasibility fixture, not final draw-call approval. Later consolidate suitable garment modules within the same appearance owner and share materials. Keep editable panels in authoring even if runtime sections are merged. No optimization should remove usable fingers or hide bad shoulder topology.
