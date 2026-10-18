"""Deterministic INTERNAL geometry authoring; Manny FBX is a read-only rig/weight reference.
Original Meshy FBX is never an export destination. No gameplay or animation authoring.
"""
import bpy,bmesh,math,json,pathlib,sys,hashlib
from mathutils import Vector
from mathutils.bvhtree import BVHTree
from mathutils.geometry import barycentric_transform
P=pathlib.Path(r'C:/Users/Le Anh Hoang/Documents/Codex/2026-10-15/files-pasted-by-the-user-you')
OUT=P/'outputs/PLAYER_V1_RETOPO_GATE1';OUT.mkdir(exist_ok=True)
for d in ['source','fbx','screenshots','metrics']: (OUT/d).mkdir(exist_ok=True)
SOURCE=P/'work/dao-vo-tan-3d-prototype/ArtSource/Meshy_AI_Jade_Warden_1017231731_generate.fbx'
assert hashlib.sha256(SOURCE.read_bytes()).hexdigest()=='faa4aada36218026646fbaccc8e28226629797f8d89b5eb543ca58d13b5261b3'
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.fbx(filepath=str(P/'outputs/PLAYER_V1_MESH_AUDIT/working/MannyReference.fbx'),use_custom_normals=True)
rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
ref=next(o for o in bpy.context.scene.objects if o.name=='SKM_Manny_Simple_LOD0')
# Bake only reference object transforms, retaining the original armature rest matrices.
def world(o,v):return o.matrix_world@v
J={b.name:world(rig,b.head_local) for b in rig.data.bones}
refv=[world(ref,v.co) for v in ref.data.vertices]
ref.data.calc_loop_triangles();tris=[tuple(t.vertices) for t in ref.data.loop_triangles]
bvh=BVHTree.FromPolygons(refv,tris,all_triangles=True)
refgroups={g.index:g.name for g in ref.vertex_groups}
refw=[{refgroups[g.group]:g.weight for g in v.groups} for v in ref.data.vertices]
parts=[];stats={}
def active(o):
 bpy.ops.object.select_all(action='DESELECT');o.select_set(True);bpy.context.view_layer.objects.active=o
def create(name,vs,fs,mat,sub=0):
 me=bpy.data.meshes.new(name);me.from_pydata(vs,[],fs);me.update();o=bpy.data.objects.new(name,me);bpy.context.collection.objects.link(o);active(o)
 bm=bmesh.new();bm.from_mesh(me);bmesh.ops.delete(bm,geom=[v for v in bm.verts if not v.link_faces],context='VERTS');bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(me);bm.free()
 if sub:
  m=o.modifiers.new('AuthoringQuadSubdivision','SUBSURF');m.levels=sub;bpy.ops.object.modifier_apply(modifier=m.name)
 for p in me.polygons:p.use_smooth=True
 me.materials.append(materials[mat]);parts.append(o);return o
def skin(o,custom=None):
 names={b.name for b in rig.data.bones};groups={n:o.vertex_groups.new(name=n) for n in sorted(names)}
 for v in o.data.vertices:
  if custom:weights=custom(v.co)
  else:
   hit,normal,idx,dist=bvh.find_nearest(v.co);a,b,c=[refv[i] for i in tris[idx]]
   q=barycentric_transform(hit,a,b,c,Vector((1,0,0)),Vector((0,1,0)),Vector((0,0,1)))
   weights={}
   for i,f in zip(tris[idx],q):
    for n,w in refw[i].items():weights[n]=weights.get(n,0)+max(0,f)*w
  weights=sorted(((n,w) for n,w in weights.items() if n in names and w>.00001),key=lambda x:-x[1])[:4]
  total=sum(w for n,w in weights);assert total>0,(o.name,v.index)
  for n,w in weights:groups[n].add([v.index],w/total,'REPLACE')
 mod=o.modifiers.new('MannyBindingProof','ARMATURE');mod.object=rig
 # Match the reference's centimetre local coordinates and .01 object transform.
 for v in o.data.vertices:v.co*=100
 o.parent=rig;o.matrix_world=rig.matrix_world.copy()
def uv(o):
 bm=bmesh.new();bm.from_mesh(o.data);bmesh.ops.triangulate(bm,faces=[f for f in bm.faces if len(f.verts)>4]);bm.to_mesh(o.data);bm.free()
 active(o);bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT')
 bpy.ops.uv.smart_project(angle_limit=math.radians(20 if o.name=='UpperRobe' else 30),island_margin=.008,area_weight=0.,correct_aspect=True,scale_to_bounds=False)
 bpy.ops.object.mode_set(mode='OBJECT');o.data.uv_layers.active.name='UV0_BakeFoundation'
 if o.name=='UpperRobe' and o.parent==rig:repair_uv_folded_faces(o)
def repair_uv_folded_faces(o):
 # Isolate only positive-area overlapping faces after projection, not thousands of clean quads.
 from collections import defaultdict
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
 me=o.data;me.calc_loop_triangles();ud=me.uv_layers.active.data;ts=[[tuple(ud[i].uv) for i in t.loops] for t in me.loop_triangles];grid=defaultdict(list);pairs=set();bad=set()
 for i,t in enumerate(ts):
  mn=[min(p[k] for p in t) for k in [0,1]];mx=[max(p[k] for p in t) for k in [0,1]]
  for cell in [(x,y) for x in range(int(mn[0]*32),int(mx[0]*32)+1) for y in range(int(mn[1]*32),int(mx[1]*32)+1)]:
   for j in grid[cell]:
    if (j,i) in pairs:continue
    pairs.add((j,i))
    if overlap(ts[j],t)>1e-9:bad.update([me.loop_triangles[i].polygon_index,me.loop_triangles[j].polygon_index])
   grid[cell].append(i)
 if not bad:return
 density=math.sqrt(sum(area(t) for t in ts)/sum(p.area for p in me.polygons));bm=bmesh.new();bm.from_mesh(me);bm.faces.ensure_lookup_table();flag=bm.faces.layers.int.new('Gate1_UVFoldRepair')
 for f in bm.faces:f[flag]=int(f.index in bad)
 bmesh.ops.triangulate(bm,faces=[f for f in bm.faces if f[flag]]);bm.to_mesh(me);bm.free();ud=me.uv_layers.active.data;flags=me.attributes['Gate1_UVFoldRepair']
 for p in me.polygons:
  if not flags.data[p.index].value:continue
  points=[me.vertices[i].co for i in p.vertices];u=(points[1]-points[0]).normalized();v=p.normal.cross(u).normalized()
  for li,co in zip(p.loop_indices,points):ud[li].uv=(3+p.index*4+(co-points[0]).dot(u)*density,3+(co-points[0]).dot(v)*density)
 active(o);bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.pack_islands(rotate=True,margin=.008);bpy.ops.object.mode_set(mode='OBJECT');o['uvFoldRepairOriginalFaces']=len(bad)
def finish(o,custom=None):uv(o);skin(o,custom)
def bridge(fs,a,b):
 assert len(a)==len(b)
 for k in range(len(a)):fs.append((a[k],a[(k+1)%len(a)],b[(k+1)%len(b)],b[k]))
def ellipse_ring(vs,c,rx,ry,n=16,u=Vector((1,0,0)),v=Vector((0,1,0))):
 ids=[]
 for i in range(n):ids.append(len(vs));vs.append(tuple(Vector(c)+u*rx*math.cos(2*math.pi*i/n)+v*ry*math.sin(2*math.pi*i/n)))
 return ids
def tube(vs,fs,points,radii,n=12):
 rings=[]
 for i,c in enumerate(points):
  axis=(Vector(points[min(i+1,len(points)-1)])-Vector(points[max(0,i-1)])).normalized()
  u=axis.cross(Vector((0,1,0))).normalized()
  if u.length<.1:u=Vector((1,0,0))
  v=axis.cross(u).normalized();r=radii[i];r=(r,r) if isinstance(r,(int,float)) else r
  rings.append(ellipse_ring(vs,c,*r,n,u,v))
 for a,b in zip(rings,rings[1:]):bridge(fs,a,b)
 fs.append(tuple(reversed(rings[0])));fs.append(tuple(rings[-1]));return rings
materials={}
for name,color in {'Skin':(.57,.38,.26,1),'Hair':(.055,.045,.065,1),'InnerCloth':(.13,.22,.25,1),'OuterCloth':(.23,.43,.40,1),'Leather':(.12,.065,.045,1),'MetalJade':(.12,.48,.36,1)}.items():
 m=bpy.data.materials.new('INTERNAL_'+name);m.diffuse_color=color;m.use_nodes=True;m.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=color;m.node_tree.nodes.get('Principled BSDF').inputs['Roughness'].default_value=.66;materials[name]=m
# Underbody: continuous torso, true branching hip/crotch and shoulder/armpit openings.
vs=[];fs=[];N=40
zs=[.90,.94,.98,1.02,1.06,1.10,1.14,1.18,1.22,1.26,1.30,1.34,1.38,1.42,1.46,1.50,1.52,1.54]
rxs=[.147,.15,.145,.132,.12,.12,.13,.145,.16,.17,.175,.177,.177,.17,.15,.105,.062,.047]
rys=[.10,.105,.107,.104,.095,.09,.095,.101,.108,.111,.11,.102,.094,.087,.077,.062,.049,.044]
rings=[ellipse_ring(vs,(0,.002,z),rx,ry,N) for z,rx,ry in zip(zs,rxs,rys)]
holes=[set((i%N) for i in range(c-2,c+2)) for c in [0,20]]
for j in range(len(rings)-1):
 for i in range(N):
  if 11<=j<15 and any(i in h for h in holes):continue
  fs.append((rings[j][i],rings[j][(i+1)%N],rings[j+1][(i+1)%N],rings[j+1][i]))
fs.append(tuple(rings[-1]))
for side,center in [('l',0),('r',20)]:
 boundary=[rings[11][i%N] for i in range(center-2,center+3)]+[rings[j][(center+2)%N] for j in range(12,16)]+[rings[15][i%N] for i in range(center+1,center-3,-1)]+[rings[j][(center-2)%N] for j in range(14,11,-1)]
 assert len(boundary)==16
 sh,el,wr=[J[n+'_'+side] for n in ['upperarm','lowerarm','hand']]
 points=[sh.lerp(el,t) for t in [0,.10,.25,.45,.65,.80,.90,.96,1]]+[el.lerp(wr,t) for t in [.05,.12,.25,.45,.65,.82,.93,1]]
 rad=[.055,.058,.057,.052,.046,.042,.039,.038,.038,.038,.038,.038,.036,.032,.027,.024,.023]
 arm=[]
 for k,p in enumerate(points):
  axis=(el-sh if k<9 else wr-el).normalized();u=axis.cross(Vector((0,1,0))).normalized();v=axis.cross(u).normalized();arm.append(ellipse_ring(vs,p,rad[k],rad[k]*.88,16,u,v))
 # Minimum-distance cyclic correspondence prevents a twisted armhole.
 choices=[a[s:]+a[:s] for a in [arm[0],list(reversed(arm[0]))] for s in range(16)]
 best=min(choices,key=lambda a:sum((Vector(vs[x])-Vector(vs[y])).length_squared for x,y in zip(boundary,a)))
 direction=1 if (best[1]-best[0])%16==1 else -1;offset=(best[0]-arm[0][0])%16
 arm=[a[offset:]+a[:offset] if direction==1 else [a[(offset-i)%16] for i in range(16)] for a in arm]
 bridge(fs,boundary,arm[0])
 for a,b in zip(arm,arm[1:]):bridge(fs,a,b)
 fs.append(tuple(arm[-1]))
# Crotch bridge shared by both separated legs, not overlapping cylinders.
cross=[]
for t in [.25,.5,.75]:cross.append(len(vs));vs.append((0,.10*(1-2*t),.90-.045*math.sin(math.pi*t)))
for side,ids in [('l',[rings[0][i%N] for i in range(30,51)]+cross),('r',[rings[0][i] for i in range(10,31)]+list(reversed(cross)))]:
 # Left perimeter runs back->front then crotch front->back.
 if side=='l':ids=[rings[0][i%N] for i in range(30,51)]+cross
 th,kn,an=[J[n+'_'+side] for n in ['thigh','calf','foot']]
 points=[th.lerp(kn,t) for t in [.10,.22,.40,.60,.78,.90,.96,1]]+[kn.lerp(an,t) for t in [.04,.10,.23,.40,.60,.78,.90,1]]
 radii=[.080,.078,.075,.067,.059,.053,.051,.050,.050,.052,.057,.055,.048,.039,.032,.029]
 prev=ids
 for p,r in zip(points,radii):
  a=ellipse_ring(vs,p,r,r*.88,24)
  opts=[a[s:]+a[:s] for a0 in [0] for s in range(24)]+[list(reversed(a[s:]+a[:s])) for s in range(24)]
  a=min(opts,key=lambda b:sum((Vector(vs[x])-Vector(vs[y])).length_squared for x,y in zip(prev,b)));bridge(fs,prev,a);prev=a
 fs.append(tuple(prev))
body=create('Body_Underbody',vs,fs,'InnerCloth',0)
# Upper robe is a separate outward shell built from the new torso/arm topology.
body.data.update();eligible={v.index for v in body.data.vertices if v.co.z>=1.005}
rfs=[tuple(p.vertices) for p in body.data.polygons if all(i in eligible for i in p.vertices)]
used=sorted({i for f in rfs for i in f});mapping={k:i for i,k in enumerate(used)}
rvs=[tuple(body.data.vertices[i].co+body.data.vertices[i].normal*(.019 if abs(body.data.vertices[i].co.x)<.20 else .025)) for i in used]
robe=create('UpperRobe',rvs,[tuple(mapping[i] for i in f) for f in rfs],'OuterCloth')
active(robe);solid=robe.modifiers.new('GarmentEdgeThickness','SOLIDIFY');solid.thickness=.003;solid.offset=0;bpy.ops.object.modifier_apply(modifier=solid.name)
finish(body);finish(robe)
# Hands: connected quad palm with four finger outlets and a separate side thumb outlet.
for side in ['l','r']:
 vs=[];fs=[];wrist=J['hand_'+side];knuckles=[J[n+'_01_'+side] for n in ['index','middle','ring','pinky']]
 axis=(sum(knuckles,Vector())/4-wrist).normalized();across=(knuckles[-1]-knuckles[0]).normalized();normal=axis.cross(across).normalized()
 boundaries=[knuckles[0]-(knuckles[1]-knuckles[0])*.50]+[(a+b)/2 for a,b in zip(knuckles,knuckles[1:])]+[knuckles[-1]+(knuckles[-1]-knuckles[-2])*.5]
 top=[];bottom=[]
 for j,t in enumerate([0,.25,.58,1]):
  tr=[];br=[]
  for i,b in enumerate(boundaries):
   base=wrist+across*((i-2)*.012);p=base.lerp(b,t);thick=.014*(1-t)+.009*t
   tr.append(len(vs));vs.append(tuple(p+normal*thick));br.append(len(vs));vs.append(tuple(p-normal*thick))
  top.append(tr);bottom.append(br)
 for j in range(3):
  for i in range(4):fs.extend([(top[j][i],top[j][i+1],top[j+1][i+1],top[j+1][i]),(bottom[j][i+1],bottom[j][i],bottom[j+1][i],bottom[j+1][i+1])])
  fs.append((top[j][4],bottom[j][4],bottom[j+1][4],top[j+1][4]))
  if j!=1:fs.append((bottom[j][0],top[j][0],top[j+1][0],bottom[j+1][0]))
 fs.append(tuple(top[0]+list(reversed(bottom[0]))))
 def finger(root,chain,radius):
  prev=root
  for k,p in enumerate(chain):
   # Square-section controls subdivide into rounded finger joint loops.
   a=[];r=radius*(1-.65*k/max(1,len(chain)-1))
   for x,y in [(-1,1),(1,1),(1,-1),(-1,-1)]:a.append(len(vs));vs.append(tuple(p+across*r*x+normal*r*y))
   opts=[a[s:]+a[:s] for s in range(4)]+[list(reversed(a[s:]+a[:s])) for s in range(4)]
   a=min(opts,key=lambda q:sum((Vector(vs[x])-Vector(vs[y])).length_squared for x,y in zip(prev,q)));bridge(fs,prev,a);prev=a
  fs.append(tuple(prev))
 for i,n in enumerate(['index','middle','ring','pinky']):
  a,b,c=[J[n+'_'+str(k).zfill(2)+'_'+side] for k in [1,2,3]];tip=c+(c-b).normalized()*.018
  chain=[a.lerp(b,t) for t in [.12,.40,.85,1]]+[b.lerp(c,t) for t in [.15,.75,1]]+[c.lerp(tip,t) for t in [.5,.88,1]]
  finger([top[-1][i],top[-1][i+1],bottom[-1][i+1],bottom[-1][i]],chain,.009 if i<3 else .0075)
 a,b,c=[J['thumb_'+str(k).zfill(2)+'_'+side] for k in [1,2,3]];tip=c+(c-b).normalized()*.018
 finger([bottom[1][0],top[1][0],top[2][0],bottom[2][0]],[a.lerp(b,t) for t in [.15,.60,1]]+[b.lerp(c,t) for t in [.25,.8,1]]+[tip],.010)
 hand=create('Hand_'+side.upper(),vs,fs,'Skin',1);finish(hand)
# Head: controlled quad surface with jaw, cheek, nose, eye/mouth region; sculpt pass remains explicit.
vs=[];fs=[];nh=56;levels=40
for j in range(levels):
 t=j/(levels-1);z=1.505+t*.292
 width=.043+.043*math.sin(math.pi*max(0,min(1,(t-.08)/.92)))**.8
 width*=.72+.28*min(1,max(0,(z-1.575)/.095))
 depth=.046+.032*math.sin(math.pi*t)**.7
 if j==levels-1:width=.006;depth=.006
 for i in range(nh):
  a=2*math.pi*i/nh;x=width*math.cos(a);y=depth*math.sin(a)
  front=max(0,-math.sin(a))**14
  nose=.029*math.exp(-(x/.016)**2-((z-1.675)/.017)**2)+.008*math.exp(-(x/.010)**2-((z-1.697)/.028)**2)
  chin=.008*math.exp(-((z-1.606)/.018)**2)
  lips=.006*math.exp(-(x/.029)**4)*math.exp(-((z-1.643)/.009)**2)
  mouth=.010*math.exp(-(x/.028)**4)*math.exp(-((z-1.643)/.0028)**2)
  eyes=.013*(math.exp(-((x-.030)/.014)**2)+math.exp(-((x+.030)/.014)**2))*math.exp(-((z-1.705)/.010)**2)
  brow=.005*(math.exp(-((x-.030)/.018)**2)+math.exp(-((x+.030)/.018)**2))*math.exp(-((z-1.719)/.007)**2)
  y-=front*(nose+chin+lips-mouth-eyes+brow);vs.append((x,y,z))
for j in range(levels-1):bridge(fs,list(range(j*nh,(j+1)*nh)),list(range((j+1)*nh,(j+2)*nh)))
fs.extend([tuple(reversed(range(nh))),tuple(range((levels-1)*nh,levels*nh))])
head=create('Head',vs,fs,'Skin',0)
finish(head,lambda p:{'head':min(1,max(0,(p.z-1.545)/.065)),'neck_02':1-min(1,max(0,(p.z-1.545)/.065))})
# Ears are intentional separate anatomical shells within the Head module.
for side in [-1,1]:
 vs=[];fs=[];tube(vs,fs,[(side*.075,.005,1.658),(side*.083,.003,1.675),(side*.082,.006,1.699)],[(.011,.016),(.016,.020),(.008,.012)],12)
 ear=create('Ear_'+str(side),vs,fs,'Skin',1);finish(ear,lambda p:{'head':1})
# Boots: separate feet around Manny ankle/ball, closed soles and supporting ankle rings.
for side in ['l','r']:
 vs=[];fs=[];foot=J['foot_'+side];rs=[]
 for z,rx,ry,cy in [(.018,.055,.127,-.058),(.025,.057,.129,-.058),(.045,.056,.125,-.057),(.075,.054,.113,-.045),(.10,.052,.085,-.025),(.13,.049,.060,-.002),(.17,.050,.049,.008),(.21,.052,.048,.010),(.235,.051,.046,.010)]:rs.append(ellipse_ring(vs,(foot.x,cy,z),rx,ry,24))
 for a,b in zip(rs,rs[1:]):bridge(fs,a,b)
 fs.extend([tuple(reversed(rs[0])),tuple(rs[-1])]);boot=create('Boot_'+side.upper(),vs,fs,'Leather',1);finish(boot)
# Hair cap follows an intentional hairline rather than leaving a bald scalp beneath the knot.
vs=[];fs=[];nc=32
for j in range(9):
 t=j/8;theta=.05+t*1.45
 for i in range(nc):
  a=2*math.pi*i/nc;end=1.65+.25*max(0,math.sin(a))-.50*max(0,-math.sin(a));th=.05+t*(end-.05)
  vs.append((.082*math.sin(th)*math.cos(a),.083*math.sin(th)*math.sin(a)+.005,1.69+.112*math.cos(th)))
for j in range(8):bridge(fs,list(range(j*nc,(j+1)*nc)),list(range((j+1)*nc,(j+2)*nc)))
fs.extend([tuple(reversed(range(nc))),tuple(range(8*nc,9*nc))]);h=create('Hair_Cap',vs,fs,'Hair',1);finish(h,lambda p:{'head':1})
# Larger controlled hair clumps; no cosmetic bones/physics/Groom.
for k,x in enumerate([-.075,-.045,-.015,.015,.045,.075]):
 vs=[];fs=[];pts=[];rads=[]
 for j in range(13):
  t=j/12;pts.append((x*(1+.3*t),.054+.067*math.sin(math.pi*t*.75),1.755-.52*t));rads.append((.025*(1-.82*t),.026*(1-.80*t)))
 tube(vs,fs,pts,rads,10);h=create('Hair_BackClump_'+str(k+1),vs,fs,'Hair',1)
 finish(h,lambda p:{'head':min(1,max(.25,(p.z-1.32)/.35)),'spine_05':1-min(1,max(.25,(p.z-1.32)/.35))})
for side in [-1,1]:
 vs=[];fs=[];tube(vs,fs,[(side*.065,-.045,1.765),(side*.083,-.060,1.705),(side*.087,-.062,1.63),(side*.085,-.062,1.55)],[(.020,.018),(.016,.014),(.012,.010),(.004,.004)],10)
 h=create('Hair_Front_'+str(side),vs,fs,'Hair',1);finish(h,lambda p:{'head':1})
vs=[];fs=[];tube(vs,fs,[(0,.01,1.77),(0,.01,1.805),(0,.01,1.835)],[(.041,.039),(.044,.04),(.020,.021)],20)
h=create('Hair_Topknot',vs,fs,'Hair',1);finish(h,lambda p:{'head':1})
# Six deliberate lower panels, with real gaps at centre front/back and side seams.
for name,lo,hi,hem in [('FrontL',-1.51,-.38,.40),('FrontR',-2.76,-1.63,.40),('BackL',.38,1.51,.34),('BackR',1.63,2.76,.34),('SideL',-.31,.31,.28),('SideR',2.83,3.45,.28)]:
 vs=[];fs=[];nx=8;ny=15
 for j in range(ny):
  t=j/(ny-1);z=.995*(1-t)+hem*t;rx=.162+.12*t;ry=.118+.082*t
  for i in range(nx):a=lo+(hi-lo)*i/(nx-1);vs.append((rx*math.cos(a),ry*math.sin(a),z+.012*math.sin(math.pi*i/(nx-1))*t))
 for j in range(ny-1):
  for i in range(nx-1):a=j*nx+i;fs.append((a,a+1,a+1+nx,a+nx))
 panel=create('LowerRobe_'+name,vs,fs,'OuterCloth',1);active(panel);sol=panel.modifiers.new('PanelThickness','SOLIDIFY');sol.thickness=.003;sol.offset=0;bpy.ops.object.modifier_apply(modifier=sol.name)
 def panelw(p):
  side='l' if p.x>=0 else 'r';leg=min(.92,max(0,(.98-p.z)/.25));calf=max(0,min(.25,(.55-p.z)/.3))
  return {'pelvis':1-leg,'thigh_'+side:leg*(1-calf),'calf_'+side:leg*calf}
 finish(panel,panelw)
# Belt and sash are lightweight reusable shapes, with no gameplay identity.
vs=[];fs=[];rs=[ellipse_ring(vs,(0,0,z),rx,ry,48) for z,rx,ry in [(.985,.17,.126),(.99,.176,.132),(1.045,.176,.132),(1.05,.17,.126)]]
for a,b in zip(rs,rs[1:]):bridge(fs,a,b)
bridge(fs,rs[-1],rs[0]);belt=create('Belt',vs,fs,'Leather',1);finish(belt,lambda p:{'pelvis':.7,'spine_01':.3})
vs=[];fs=[];tube(vs,fs,[(.085,-.14,1.025),(.10,-.16,.90),(.10,-.18,.78),(.095,-.18,.70)],[(.020,.006),(.021,.006),(.019,.006),(.015,.006)],8)
sash=create('Sash',vs,fs,'OuterCloth',1);finish(sash,lambda p:{'pelvis':1})
vs=[];fs=[];tube(vs,fs,[(.10,-.18,.72),(.10,-.185,.69),(.10,-.18,.665)],[(.019,.008),(.023,.010),(.013,.007)],12)
jade=create('Accessory_Jade',vs,fs,'MetalJade',1);finish(jade,lambda p:{'pelvis':1})
# Consolidate reusable modules; disconnected hair clumps/ears are intentional shells.
def combine(name,objects):
 if len(objects)<2:return
 bpy.ops.object.select_all(action='DESELECT')
 for o in objects:o.select_set(True)
 for o in objects[1:]:parts.remove(o)
 bpy.context.view_layer.objects.active=objects[0];bpy.ops.object.join();joined=objects[0];joined.name=name
combine('Hands',[o for o in parts if o.name.startswith('Hand_')])
combine('Boots',[o for o in parts if o.name.startswith('Boot_')])
combine('Hair',[o for o in parts if o.name.startswith('Hair_')])
combine('Head',[o for o in parts if o.name=='Head' or o.name.startswith('Ear_')])
# Keep reference hidden and excluded from exports; authoring file retains its read-only comparison.
for o in list(bpy.context.scene.objects):
 if o not in parts and o!=rig:o.hide_render=True;o.hide_set(True)
for o in parts:
 uv(o) # Repack consolidated hands/head/hair together, not overlapping their pre-join atlases.
 active(o);o.data.calc_loop_triangles();o.data.calc_tangents(uvmap=o.data.uv_layers.active.name)
 bm=bmesh.new();bm.from_mesh(o.data)
 unweighted=sum(not v.groups for v in o.data.vertices)
 stats[o.name]={'vertices':len(o.data.vertices),'triangles':len(o.data.loop_triangles),'polygons':len(o.data.polygons),'quads':sum(len(p.vertices)==4 for p in o.data.polygons),'boundaryEdges':sum(e.is_boundary for e in bm.edges),'nonManifoldEdges':sum(not e.is_manifold for e in bm.edges),'zeroAreaFaces':sum(f.calc_area()<1e-10 for f in bm.faces),'uvChannels':len(o.data.uv_layers),'materialSlots':len(o.data.materials),'unweightedVertices':unweighted,'maxInfluences':max(len(v.groups) for v in o.data.vertices),'normalNonFinite':sum(not all(math.isfinite(x) for x in v.normal) for v in o.data.vertices),'tangentNonFinite':sum(not all(math.isfinite(x) for x in l.tangent) for l in o.data.loops)};bm.free()
 assert unweighted==0 and stats[o.name]['zeroAreaFaces']==0 and stats[o.name]['nonManifoldEdges']==0,(o.name,stats[o.name])
 # One FBX per module, exact same reference armature hierarchy every time.
 original_name=o.name;o.name='Geo_'+original_name # Avoid Head/head collision in FBX/Unreal's case-insensitive bone names.
 bpy.ops.object.select_all(action='DESELECT');o.select_set(True);rig.select_set(True);bpy.context.view_layer.objects.active=rig
 bpy.ops.export_scene.fbx(filepath=str(OUT/'fbx'/('SK_'+original_name+'.fbx')),use_selection=True,object_types={'ARMATURE','MESH'},add_leaf_bones=False,bake_anim=False,axis_forward='-Z',axis_up='Y',use_armature_deform_only=False,apply_unit_scale=True,mesh_smooth_type='FACE',use_mesh_modifiers=True,use_tspace=True)
 o.name=original_name
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'source/PlayerV1_RetopoGate1.blend'))
(OUT/'metrics/geometry.json').write_text(json.dumps({'modules':stats,'totalTriangles':sum(s['triangles'] for s in stats.values()),'totalVertices':sum(s['vertices'] for s in stats.values()),'mannyBones':len(rig.data.bones),'sourceSHA256':hashlib.sha256(SOURCE.read_bytes()).hexdigest(),'method':'New parametric quad control surfaces; barycentric nearest Manny surface weights (max four); analytic panel/hair/rigid weights; no source triangles reused','provenance':'INTERNAL EXPERIMENTAL; Meshy rights unverified; existing Epic Manny read-only reference'},indent=2))
print('RETOPO_GEOMETRY_SUCCESS',len(parts),sum(s['triangles'] for s in stats.values()))
