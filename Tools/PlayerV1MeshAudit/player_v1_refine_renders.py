import bpy,pathlib,json,sys
from mathutils import Vector
out=pathlib.Path(sys.argv[sys.argv.index('--')+1]);bpy.ops.wm.open_mainfile(filepath=str(out/'working/mesh-audit-comparison.blend'))
scene=bpy.context.scene;scene.cycles.samples=6;scene.render.resolution_x=1100;scene.render.resolution_y=1100;camera=scene.camera
obj=bpy.data.objects['Meshy_ImmutableCopy_Reference'];manny=[o for o in scene.objects if o.type=='MESH' and o!=obj]
roots=[o for o in scene.objects if o!=obj and o.type in ('ARMATURE','EMPTY','MESH') and o.parent is None]
meta=json.loads((out/'blender-raw.json').read_text());ue=json.loads((out/'unreal-import.json').read_text());mn=meta['worldMinM'];s=ue['nonDestructiveFit']['uniformScale']
def shot(name,target,scale):
 camera.location=Vector((target[0],-5,target[2]));camera.rotation_euler=(Vector(target)-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.ortho_scale=scale;scene.render.filepath=str(out/'screenshots'/name);bpy.ops.render.render(write_still=True)
for o in manny:o.hide_render=False
for r in roots:r.location.x=-.95
obj.location=Vector((.95,0,-mn[2]));obj.scale=Vector((1,1,1))
shot('05_manny_side_by_side.png',(0,0,.96),3.7)
if '--only-side' in sys.argv:
 bpy.ops.wm.quit_blender()
for r in roots:r.location.x=0
obj.location=Vector((0,0,ue['nonDestructiveFit']['translationCm'][2]/100));obj.scale=Vector((s,s,s))
overlay=bpy.data.materials.new('AUDIT_ONLY_translucent_overlay');overlay.use_nodes=True;n=overlay.node_tree.nodes;n.clear();output=n.new('ShaderNodeOutputMaterial');mix=n.new('ShaderNodeMixShader');transparent=n.new('ShaderNodeBsdfTransparent');emit=n.new('ShaderNodeEmission');emit.inputs['Color'].default_value=(.8,.3,.06,1);emit.inputs['Strength'].default_value=.7;mix.inputs[0].default_value=.25
overlay.node_tree.links.new(transparent.outputs[0],mix.inputs[1]);overlay.node_tree.links.new(emit.outputs[0],mix.inputs[2]);overlay.node_tree.links.new(mix.outputs[0],output.inputs['Surface'])
previous=obj.data.materials[0];obj.data.materials[0]=overlay
shot('15_manny_fitted_overlay.png',(0,0,.91),2.15)
obj.data.materials[0]=previous
for o in manny:o.hide_render=True
shot('16_face_front_closeup.png',(0,0,1.62),.40)
print('REFINED_AUDIT_VIEWS_DONE')
