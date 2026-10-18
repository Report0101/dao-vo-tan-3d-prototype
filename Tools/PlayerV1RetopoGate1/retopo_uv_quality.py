import bpy,json,pathlib,math
from collections import defaultdict
P=pathlib.Path(r'C:/Users/Le Anh Hoang/Documents/Codex/2026-10-15/files-pasted-by-the-user-you/outputs/PLAYER_V1_RETOPO_GATE1')
bpy.ops.wm.open_mainfile(filepath=str(P/'source/PlayerV1_RetopoGate1.blend'))
def cross(a,b,c):return (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
def area(p):return abs(sum(a[0]*b[1]-a[1]*b[0] for a,b in zip(p,p[1:]+p[:1])))/2
def overlap(a,b):
 if cross(*b)<0:b=list(reversed(b))
 poly=list(a)
 for u,v in zip(b,b[1:]+b[:1]):
  new=[]
  if not poly:return 0
  for p,q in zip(poly,poly[1:]+poly[:1]):
   cp=cross(u,v,p);cq=cross(u,v,q)
   if cp>=0:new.append(p)
   if (cp>=0)!=(cq>=0):
    t=cp/(cp-cq);new.append((p[0]+(q[0]-p[0])*t,p[1]+(q[1]-p[1])*t))
  poly=new
 return area(poly) if len(poly)>2 else 0
result={}
for o in bpy.context.scene.objects:
 if o.type!='MESH' or o.hide_render:continue
 me=o.data;me.calc_loop_triangles();uv=me.uv_layers.active.data
 ts=[[tuple(uv[i].uv) for i in t.loops] for t in me.loop_triangles]
 grid=defaultdict(list);pairs=set();over=0;overarea=0
 for i,t in enumerate(ts):
  mn=[min(p[k] for p in t) for k in [0,1]];mx=[max(p[k] for p in t) for k in [0,1]]
  cells=[(x,y) for x in range(int(mn[0]*32),int(mx[0]*32)+1) for y in range(int(mn[1]*32),int(mx[1]*32)+1)]
  for cell in cells:
   for j in grid[cell]:
    pair=(j,i)
    if pair in pairs:continue
    pairs.add(pair);a=overlap(ts[j],t)
    if a>1e-9:over+=1;overarea+=a
   grid[cell].append(i)
 result[o.name]={'channels':len(me.uv_layers),'channel0':me.uv_layers.active.name,'packing':'unique per module, smart projected, no intentional mirrors; foundation islands require later seam/texel polish','uvMin':[min(p[k] for t in ts for p in t) for k in [0,1]],'uvMax':[max(p[k] for t in ts for p in t) for k in [0,1]],'zeroUVAreaTriangles':sum(area(t)<1e-12 for t in ts),'positiveAreaOverlapPairs':over,'sumPairOverlapArea':overarea,'occupiedTriangleArea':sum(area(t) for t in ts)}
(P/'metrics/uv-quality.json').write_text(json.dumps(result,indent=2));print('RETOPO_UV_CHECK_COMPLETE',[(n,r['positiveAreaOverlapPairs']) for n,r in result.items()])
