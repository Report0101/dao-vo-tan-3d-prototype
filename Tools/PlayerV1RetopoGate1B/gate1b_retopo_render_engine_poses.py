"""Render exact read-only UE CPU-skinned LOD0 fixture exports, not fabricated poses."""
import bpy,bmesh,json,pathlib,math
from mathutils import Vector,Matrix
P=pathlib.Path(r'C:/Users/Le Anh Hoang/Documents/Codex/2026-10-15/files-pasted-by-the-user-you');O=P/'outputs/PLAYER_V1_RETOPO_GATE1B';E=O/'engine'
bpy.ops.wm.open_mainfile(filepath=str(O/'source/PlayerV1_RetopoGate1B.blend'))
for o in bpy.context.scene.objects:o.hide_render=True
materials={m.name:m for m in bpy.data.materials}
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=4;scene.render.threads_mode='FIXED';scene.render.threads=2;scene.render.resolution_x=700;scene.render.resolution_y=700;scene.render.resolution_percentage=100;scene.render.image_settings.file_format='PNG';scene.world.color=(.16,.16,.16);scene.view_settings.view_transform='AgX'
def area(loc,power,size):
 bpy.ops.object.light_add(type='AREA',location=loc);a=bpy.context.object;a.data.energy=power;a.data.shape='DISK';a.data.size=size;a.rotation_euler=(Vector((0,0,1))-a.location).to_track_quat('-Z','Y').to_euler()
area((-3,-4,5),600,4);area((4,-2,3),350,3);area((0,4,4),500,3)
bpy.ops.object.camera_add();cam=bpy.context.object;cam.data.type='ORTHO';scene.camera=cam
def shot(name,target,scale,offset=Vector((2.8,-4,.3))):
 cam.location=Vector(target)+offset;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=scale;scene.render.filepath=str(O/'screenshots'/(name+'.png'));bpy.ops.render.render(write_still=True)
before=set(bpy.context.scene.objects);bpy.ops.import_scene.fbx(filepath=str(O/'references/Jian.fbx'))
jian=[o for o in bpy.context.scene.objects if o not in before and o.type=='MESH'];swordgeom=[]
for o in jian:
 swordgeom.append(([tuple(o.matrix_world@v.co) for v in o.data.vertices],[tuple(p.vertices) for p in o.data.polygons]));o.hide_render=True
report=json.loads((E/'engine-deformation.json').read_text(encoding='utf-8-sig'));C=Matrix.Diagonal(Vector((1,-1,1)))
captured=[]
tasks=[(c,i,suf) for c in report['clips'] for i,suf in [(1,'_f1'),(3,''),(5,'_f5')]]
for clip,frame_index,suffix in tasks:
 name=clip['name'];capture_name=name+suffix;verts=[];groups={};face_mats={};current=None;current_mat=''
 for line in (E/('pose_'+capture_name+'.obj')).read_text(encoding='utf-8-sig').splitlines():
  if line.startswith('o '):current=line[2:];groups[current]=[];face_mats[current]=[]
  elif line.startswith('usemtl '):current_mat=line[7:]
  elif line.startswith('v '):verts.append(tuple(float(v) for v in line[2:].split()))
  elif line.startswith('f '):groups[current].append(tuple(int(v)-1 for v in line[2:].split()));face_mats[current].append(current_mat)
 spawned=[]
 for module,faces in groups.items():
  ids=sorted({i for f in faces for i in f});remap={i:j for j,i in enumerate(ids)};me=bpy.data.meshes.new(module);me.from_pydata([verts[i] for i in ids],[],[tuple(remap[i] for i in f) for f in faces]);me.update()
  for kind in ['Skin','Hair','EyeFoundation','InnerCloth','OuterCloth','Leather','MetalJade']:me.materials.append(materials['INTERNAL_'+kind])
  kinds=['Skin','Hair','EyeFoundation','InnerCloth','OuterCloth','Leather','MetalJade']
  for p,label in zip(me.polygons,face_mats[module]):
   p.material_index=next((i for i,k in enumerate(kinds) if k in label),0)
  # OBJ has no imported tangent basis: merge coincident UV-split vertices for neutral
  # inspection normals only. UE-skinned vertex positions and silhouette are unchanged.
  bm=bmesh.new();bm.from_mesh(me);bmesh.ops.remove_doubles(bm,verts=list(bm.verts),dist=1e-7);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(me);bm.free()
  o=bpy.data.objects.new(module,me);bpy.context.collection.objects.link(o);spawned.append(o)
  for p in me.polygons:p.use_smooth=True
 f=clip['frames'][frame_index]
 if name in ['SwordGuard','SwordAttack']:
  R=Matrix([f['swordX'],f['swordY'],f['swordZ']]).transposed();R=C@R@C;T=C@Vector(f['swordRootCm'])/100
  for vs,fs in swordgeom:
   me=bpy.data.meshes.new('Existing_Jian_ReadOnly');me.from_pydata([R@Vector(v)+T for v in vs],[],fs);me.update();o=bpy.data.objects.new('Existing_Jian_SwordGrip_R',me);bpy.context.collection.objects.link(o);spawned.append(o)
 mn=Vector(tuple(min(v[i] for v in verts) for i in range(3)));mx=Vector(tuple(max(v[i] for v in verts) for i in range(3)));center=(mn+mx)/2
 shot('pose_'+capture_name,center,max(2.1,(mx.z-mn.z)*1.2));captured.append(name)
 if name in ['SwordGuard','SwordAttack']:
  shot('grip_'+capture_name,C@Vector(f['swordRootCm'])/100,.38,Vector((1,-2,.3)))
 if name=='Meditation' and not suffix:shot('meditation_back',center,max(1.7,(mx.z-mn.z)*1.3),Vector((1,4,.4)))
 if not suffix:
  shot('pelvis_'+name,(center.x,center.y,center.z-.20),.80,Vector((0,-4,0)))
  if name in ['Run','Jump','SwordAttack']:shot('torso_'+name,(center.x,center.y,center.z+.25),1.05,Vector((2,-4,.2)))
 for o in spawned:bpy.data.objects.remove(o,do_unlink=True)
(O/'metrics/engine-captures.json').write_text(json.dumps({'clips':captured,'source':'UE imported LOD0 CPU-skinned OBJ, fractions 1/6, 1/2, 5/6; accepted animation assets; existing JianGripTransform exported by validator; Blender neutral lighting only','normals':'OBJ does not export UE tangent basis. Coincident UV-split vertices are merged at 1e-7m for smooth inspection normals; positions/silhouette unchanged. Not a UE shading-quality test.','notRuntimeGraphPlayback':True},indent=2))
print('RETOPO_ENGINE_CAPTURE_SUCCESS',len(captured))
