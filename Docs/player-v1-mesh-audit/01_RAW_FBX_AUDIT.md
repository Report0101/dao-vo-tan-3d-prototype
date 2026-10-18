# 01 — Raw FBX audit

Direct binary parsing, before Unreal import, independently verified:

| Item | Result |
| --- | --- |
| FBX Objects entries | 2: one Geometry and one Mesh Model |
| Meshes | 1 |
| Control-point vertices | 51,339 |
| Polygons / triangles | 102,722 / 102,722; all triangles |
| Connected surface components | 1 |
| Skeleton / bones / skin deformers | none / 0 / 0 |
| Skin weights / animation stacks / blendshapes | none / 0 / 0 |
| UV channels / source materials / textures / embedded Video | 0 / 0 / 0 / 0 |
| Vertex colors / tangents / binormals | none / none / none |
| Authored normals | 102,721 direct unit vectors indexed at 308,166 polygon corners |
| Bad / zero normals | 0 / 0; lengths approximately 0.99999995..1.00000005 |
| Corner normals opposed to face winding | 0 of 308,166 tested |

Header: Y-up (+), FrontAxis Z (+), CoordAxis X (+), unit scale 1 cm. One root Mesh model connects to root 0; Geometry connects to that model. Model has local rotation X=-90 degrees and scale (100,100,100), no authored translation/pivot offset. Local bounds: [-0.7736988067626953, -0.2846931219100952, -0.9516417980194092] to [0.7699925899505615, 0.27585139870643616, 0.9484813213348389]. Interpreting local dimensions without hierarchy/unit conversion would be wrong. Blender resolves a Z-upright mesh with dimensions 1.5436914 x .5605446 x 1.9001231 m; Unreal bounds agree in centimetres, with the front/depth sign conversion recorded in unreal-import.json.

The pivot is around the mesh centre, not the feet. Unreal raw bounds Z=-95.16418..94.84813 cm. Foot alignment is an actor translation, not a source edit. Both Blender and the FBX node parser independently confirm no armature, vertex groups, shape keys, UV, materials or animation data. A header TimeSpan is not an animation take.
