import bpy,sys,json,math,pathlib,hashlib
from mathutils import Vector
args=sys.argv[sys.argv.index('--')+1:];out=pathlib.Path(args[0]);mode=args[1] if len(args)>1 else 'raw';out.mkdir(parents=True,exist_ok=True)
src=out/'working/MeshyAudit_SourceCopy.fbx'
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.fbx(filepath=str(src),use_custom_normals=True)
meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];assert len(meshes)==1
obj=meshes[0];obj.name='Meshy_ImmutableCopy_Reference'
worldverts=[obj.matrix_world@v.co for v in obj.data.vertices]
mn=Vector(tuple(min(v[i] for v in worldverts) for i in range(3)));mx=Vector(tuple(max(v[i] for v in worldverts) for i in range(3)))
result={'Blender':bpy.app.version_string,'sourceCopySHA256':hashlib.sha256(src.read_bytes()).hexdigest(),'meshObjects':len(meshes),'vertices':len(obj.data.vertices),'polygons':len(obj.data.polygons),'uvLayers':len(obj.data.uv_layers),'colorAttributes':len(obj.data.color_attributes),'materials':len(obj.data.materials),'vertexGroups':len(obj.vertex_groups),'armatures':sum(o.type=='ARMATURE' for o in bpy.context.scene.objects),'shapeKeys':len(obj.data.shape_keys.key_blocks) if obj.data.shape_keys else 0,'worldMinM':list(mn),'worldMaxM':list(mx),'dimensionsM':list(mx-mn),'matrixWorld':[list(r) for r in obj.matrix_world],'modifiers':[(m.name,m.type) for m in obj.modifiers]}
(out/'blender-raw.json').write_text(json.dumps(result,indent=2))
# Neutral clay inspection, not a claim of supplied colors/materials.
mat=bpy.data.materials.new('AUDIT_ONLY_neutral_clay');mat.diffuse_color=(.48,.58,.57,1);mat.use_nodes=True
bsdf=mat.node_tree.nodes.get('Principled BSDF');bsdf.inputs['Base Color'].default_value=(.48,.58,.57,1);bsdf.inputs['Roughness'].default_value=.7
wire=mat.node_tree.nodes.new('ShaderNodeWireframe');wire.use_pixel_size=True;wire.inputs['Size'].default_value=.7
mix=mat.node_tree.nodes.new('ShaderNodeMixRGB');mix.inputs[1].default_value=(.48,.58,.57,1);mix.inputs[2].default_value=(.025,.04,.04,1)
mat.node_tree.links.new(wire.outputs[0],mix.inputs[0])
obj.data.materials.clear();obj.data.materials.append(mat)
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=12;scene.cycles.device='CPU';scene.render.threads_mode='FIXED';scene.render.threads=2
scene.render.resolution_x=1200;scene.render.resolution_y=1200;scene.render.resolution_percentage=100;scene.render.image_settings.file_format='PNG'
scene.world.color=(.12,.12,.12);scene.view_settings.view_transform='AgX'
def area(name,loc,power,size):
 bpy.ops.object.light_add(type='AREA',location=loc);a=bpy.context.object;a.name=name;a.data.energy=power;a.data.shape='DISK';a.data.size=size;a.rotation_euler=(Vector((0,0,1))-a.location).to_track_quat('-Z','Y').to_euler()
area('Audit_Key',(-3,-4,5),500,4);area('Audit_Fill',(4,-1,3),350,3);area('Audit_Rim',(0,4,4),550,3)
bpy.ops.object.camera_add();camera=bpy.context.object;camera.data.type='ORTHO';scene.camera=camera
images=out/'screenshots';images.mkdir(exist_ok=True)
def shot(name,location,target,scale,wireframe=False):
 if wireframe:mat.node_tree.links.new(mix.outputs[0],bsdf.inputs['Base Color'])
 else:
  for l in list(bsdf.inputs['Base Color'].links):mat.node_tree.links.remove(l)
 camera.location=Vector(location);camera.rotation_euler=(Vector(target)-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.ortho_scale=scale;scene.render.filepath=str(images/(name+'.png'));bpy.ops.render.render(write_still=True)
center=(mn+mx)/2;h=mx.z-mn.z
if mode=='raw':
 shot('01_front',(center.x,-4,center.z),center,h*1.15)
 shot('02_side',(4,center.y,center.z),center,h*1.15)
 shot('03_back',(center.x,4,center.z),center,h*1.15)
 shot('04_three_quarter',(3,-4,center.z+.3),center,h*1.15)
 bpy.ops.wm.save_as_mainfile(filepath=str(out/'working/mesh-audit-reference.blend'))
else:
 # Comparison export is a read-only derivative of the existing Manny asset.
 before=set(bpy.context.scene.objects);bpy.ops.import_scene.fbx(filepath=str(out/'working/MannyReference.fbx'),use_custom_normals=True)
 added=[o for o in bpy.context.scene.objects if o not in before];manny=[o for o in added if o.type=='MESH']
 for o in added:
  if o.type=='ARMATURE':o.hide_render=True
 mm=bpy.data.materials.new('AUDIT_ONLY_Manny_blue');mm.diffuse_color=(.11,.28,.65,1);mm.use_nodes=True;mm.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=(.11,.28,.65,1)
 for o in manny:o.data.materials.clear();o.data.materials.append(mm)
 # Transform inspection exports Unreal X-forward coordinates to Blender's -Y-forward.
 allmv=[o.matrix_world@v.co for o in manny for v in o.data.vertices];mmin=Vector(tuple(min(v[i] for v in allmv) for i in range(3)));mmax=Vector(tuple(max(v[i] for v in allmv) for i in range(3)))
 result['mannyWorldMinM']=list(mmin);result['mannyWorldMaxM']=list(mmax);result['heightRatioMeshyToManny']=(mx.z-mn.z)/(mmax.z-mmin.z)
 obj.location.z += mmin.z-mn.z
 # Side-by-side retains source proportions. Position changes are preview-only.
 for o in added:
  if o.parent is None:o.location.x-=1.1
 obj.location.x+=1.1
 shot('05_manny_side_by_side',(0,-5,1),(0,0,1),3.3)
 obj.location.x-=1.1
 for o in added:
  if o.parent is None:o.location.x+=1.1
 ratio=1/result['heightRatioMeshyToManny'];obj.scale*=ratio;obj.location.z+=mn.z*(1-ratio)
 for o in manny:o.hide_render=True
 # Closeup positions will be refined from the observed raw model, not assumed anatomy.
 for name,loc,target,scale in [('06_head_profile',(3,-.8,1.7),(0,0,1.7),.5),('07_shoulder',(1,-3,1.45),(.35,0,1.45),.55),('08_hand',(1.6,-3,1.25),(.72,0,1.25),.35),('09_lower_robe',(2,-3,.55),(0,0,.55),1.0),('10_hair_back',(0,3,1.45),(0,0,1.45),.85)]:shot(name,loc,target,scale)
 shot('11_shoulder_wire',(1,-3,1.45),(.35,0,1.45),.55,True)
 shot('12_hand_wire',(1.6,-3,1.25),(.72,0,1.25),.35,True)
 for o in manny:o.hide_render=False
 # Geometric overlay uses silhouette contrast; it is not a skinning proof.
 obj.display_type='WIRE';obj.hide_render=True
 shot('13_manny_reference',(0,-4,1),(0,0,1),2.3)
 obj.hide_render=False
 (out/'blender-comparison.json').write_text(json.dumps(result,indent=2))
 bpy.ops.wm.save_as_mainfile(filepath=str(out/'working/mesh-audit-comparison.blend'))
print('PLAYER_V1_BLENDER_AUDIT_DONE '+mode)
