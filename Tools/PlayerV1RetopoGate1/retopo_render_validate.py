import bpy,bmesh,json,math,pathlib
from mathutils import Vector
P=pathlib.Path(r'C:/Users/Le Anh Hoang/Documents/Codex/2026-10-15/files-pasted-by-the-user-you');O=P/'outputs/PLAYER_V1_RETOPO_GATE1'
bpy.ops.wm.open_mainfile(filepath=str(O/'source/PlayerV1_RetopoGate1.blend'))
rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
parts=[o for o in bpy.context.scene.objects if o.type=='MESH' and any(m.type=='ARMATURE' and m.object==rig for m in o.modifiers) and not o.hide_render]
for o in parts:
 for p in o.data.polygons:p.use_smooth=True
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=8;scene.render.threads_mode='FIXED';scene.render.threads=2
scene.render.resolution_x=1000;scene.render.resolution_y=1000;scene.render.resolution_percentage=100;scene.render.image_settings.file_format='PNG';scene.world.color=(.16,.16,.16);scene.view_settings.view_transform='AgX'
def area(name,loc,power,size):
 bpy.ops.object.light_add(type='AREA',location=loc);a=bpy.context.object;a.name=name;a.data.energy=power;a.data.shape='DISK';a.data.size=size;a.rotation_euler=(Vector((0,0,1))-a.location).to_track_quat('-Z','Y').to_euler()
area('Fixture_Key',(-3,-4,5),600,4);area('Fixture_Fill',(4,-2,3),350,3);area('Fixture_Rim',(0,4,4),500,3)
bpy.ops.object.camera_add();cam=bpy.context.object;cam.data.type='ORTHO';scene.camera=cam
def shot(name,loc,target,scale,wire=False):
 cam.location=loc;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=scale
 wired=[]
 if wire:
  for o in parts:
   dark=bpy.data.materials.get('INTERNAL_Wire') or bpy.data.materials.new('INTERNAL_Wire');dark.diffuse_color=(.012,.018,.020,1);dark.use_nodes=True;dark.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=(.012,.018,.020,1)
   slot=len(o.data.materials);o.data.materials.append(dark)
   w=o.modifiers.new('EvidenceWire','WIREFRAME');w.thickness=.00045/o.matrix_world.to_scale().x;w.use_replace=False;w.offset=1;w.material_offset=slot;wired.append((o,w))
 scene.render.filepath=str(O/'screenshots'/(name+'.png'));bpy.ops.render.render(write_still=True)
 for o,w in wired:o.modifiers.remove(w);o.data.materials.pop(index=len(o.data.materials)-1)
for name,loc in [('01_front',(0,-4,.95)),('02_back',(0,4,.95)),('03_side',(4,0,.95)),('04_three_quarter',(3,-4,1.20))]:shot(name,loc,(0,0,.95),2.15)
shot('05_wire_front',(0,-4,.95),(0,0,.95),2.15,True);shot('06_wire_back',(0,4,.95),(0,0,.95),2.15,True)
details=[('face',(0,-3,1.69),(0,-.01,1.69),.34),('shoulder_armpit',(2,-3,1.40),(.18,0,1.40),.45),('elbow',(2,-3,1.21),(.35,.02,1.21),.30),('hand_fingers',(2,-3,1.01),(.49,-.23,1.01),.25),('pelvis',(0,-3,.91),(0,0,.91),.5),('knee',(2,-3,.50),(.12,0,.5),.35),('ankle',(2,-3,.12),(.14,-.04,.12),.32),('hair_back',(0,3,1.50),(0,.08,1.50),.82),('robe_panels',(1.5,-3,.62),(0,0,.62),1.15)]
for i,(name,loc,t,s) in enumerate(details,7):shot(str(i).zfill(2)+'_'+name,loc,t,s)
# Unclothed authoring topology must also be visible; concealed geometry is not accepted unseen.
clothes=[o for o in parts if o.name.startswith(('UpperRobe','LowerRobe','Hair','Belt','Sash','Accessory'))]
for o in clothes:o.hide_render=True
shot('16_underbody_front',(0,-4,.95),(0,0,.95),2.1,True);shot('17_underbody_back',(0,4,.95),(0,0,.95),2.1,True)
for o in clothes:o.hide_render=False
ref=next(o for o in bpy.context.scene.objects if o.name=='SKM_Manny_Simple_LOD0');ref.hide_render=False;ref.hide_set(False)
old=ref.matrix_world.copy();ref.matrix_world.translation.x=-.8
for o in parts:o.location.x+=80 # objects use centimetre local positions under .01 reference rig
# Object location is metres in parent coordinates (.01 scale inherited); compensate by world matrix.
for o in parts:o.location.x-=80
for o in parts:o.matrix_world.translation.x+=.8
shot('18_manny_side_by_side',(0,-5,.95),(0,0,.95),3.2)
for o in parts:o.matrix_world.translation.x-=.8
ref.matrix_world=old;ref.hide_render=True
shot('19_reference_pose',(3,-4,1.1),(0,0,.95),2.15,True)
print('RETOPO_GEOMETRY_CAPTURE_SUCCESS')
