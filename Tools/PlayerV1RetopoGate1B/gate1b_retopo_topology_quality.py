import bpy,bmesh,json,pathlib,math
P=pathlib.Path(r'C:/Users/Le Anh Hoang/Documents/Codex/2026-10-15/files-pasted-by-the-user-you/outputs/PLAYER_V1_RETOPO_GATE1B')
bpy.ops.wm.open_mainfile(filepath=str(P/'source/PlayerV1_RetopoGate1B.blend'))
d={}
for o in bpy.context.scene.objects:
 if o.type!='MESH' or o.hide_render:continue
 me=o.data;me.calc_loop_triangles();bm=bmesh.new();bm.from_mesh(me);unseen=set(bm.verts);components=[]
 while unseen:
  stack=[unseen.pop()];count=0
  while stack:
   v=stack.pop();count+=1
   for e in v.link_edges:
    n=e.other_vert(v)
    if n in unseen:unseen.remove(n);stack.append(n)
  components.append(count)
 aspects=[]
 for t in me.loop_triangles:
  a,b,c=[me.vertices[i].co for i in t.vertices];long=max((a-b).length,(b-c).length,(c-a).length);area=(b-a).cross(c-a).length/2;aspects.append(long*long/max(1e-15,2*area))
 faces=[tuple(sorted(p.vertices)) for p in me.polygons]
 d[o.name]={'connectedComponents':sorted(components,reverse=True),'looseVertices':sum(not v.link_faces for v in bm.verts),'duplicateIndexFaces':len(faces)-len(set(faces)),'maxTriangleAspectLongestEdgeOverAltitude':max(aspects),'trianglesAspectOver20':sum(a>20 for a in aspects),'trianglesAspectOver100':sum(a>100 for a in aspects),'uvFoldRepairOriginalFaces':o.get('uvFoldRepairOriginalFaces',0),'geometricSelfIntersectionCertification':False}
 bm.free()
(P/'metrics/topology-quality.json').write_text(json.dumps(d,indent=2));print('RETOPO_TOPOLOGY_CHECK_COMPLETE')
