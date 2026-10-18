# UV and material foundation

One UV0_BakeFoundation channel per module; unique 0–1 packing, no intentional mirrors/overlaps. No lightmap UV, final bake or authored textures. Smart projection uses area-relative island packing; consolidated hands/head/hair are repacked together. Overlapping upper-robe projection faces are isolated/triangulated/repacked locally. Exact clipping checks find zero positive-area triangle overlap pairs at 1e-9 UV-area tolerance and zero zero-area UV triangles in all modules. Early overlapping atlases and repairs are logged, not concealed.

This is usable bake UV foundation with fragmented islands; manual seam placement/texel-direction polish remains necessary for cloth/face painting. It is not final texturing acceptance. UV checker records bounds, area and overlap counts per module.

Six DCC slot concepts: Skin, Hair, InnerCloth, OuterCloth, Leather, MetalJade. Each candidate mesh uses one slot/section; separate body, head/hands, hair, robe panels, boots/belt and ornament preserve future mapping. Unreal preview reuses existing M_Skin / M_Hair / M_Robe parents; no global material is edited or duplicated. Blender evidence uses flat internal colors, not final shader/texture output.

Blender normals/tangents are finite and exported with tangent space and smoothing. No missing-UV/degenerate-tangent warnings in final import; legacy FBX still reports recovered bind-pose warnings (three/module). Direct engine reference-pose and skinning checks verify the recovered result. Neutral Blender renders inspect shape; OBJ pose renders recompute smooth inspection normals after merging coincident UV splits at 1e-7 m, without changing the skinned silhouette. They are not Unreal shading screenshots.
