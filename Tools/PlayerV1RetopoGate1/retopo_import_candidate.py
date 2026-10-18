import unreal,pathlib,json
O=pathlib.Path(r'C:/Users/Le Anh Hoang/Documents/Codex/2026-10-15/files-pasted-by-the-user-you/outputs/PLAYER_V1_RETOPO_GATE1')
prefix='/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1'
# Supported per-process console variable selects the legacy FBX reimport path.
# Does not persist project/engine config or alter compiler/importer code.
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
unreal.SystemLibrary.execute_console_command(world,'Interchange.FeatureFlags.Import.FBX 0')
manny=unreal.load_asset('/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple');sk=manny.skeleton
result={};library=unreal.EditorAssetLibrary
for path in sorted((O/'fbx').glob('SK_*.fbx')):
 options=unreal.FbxImportUI()
 for k,v in {'import_mesh':True,'import_as_skeletal':True,'mesh_type_to_import':unreal.FBXImportType.FBXIT_SKELETAL_MESH,'automated_import_should_detect_type':False,'import_materials':False,'import_textures':False,'import_animations':False,'create_physics_asset':False,'skeleton':sk}.items():options.set_editor_property(k,v)
 for k,v in {'update_skeleton_reference_pose':False,'use_t0_as_ref_pose':False,'import_meshes_in_bone_hierarchy':False,'import_morph_targets':False,'convert_scene':True,'convert_scene_unit':True,'force_front_x_axis':False,'import_uniform_scale':1.,'normal_import_method':unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS}.items():options.skeletal_mesh_import_data.set_editor_property(k,v)
 task=unreal.AssetImportTask()
 for k,v in {'filename':str(path),'destination_path':prefix,'destination_name':path.stem,'automated':True,'replace_existing':True,'replace_existing_settings':True,'save':True,'options':options,'factory':unreal.FbxFactory()}.items():task.set_editor_property(k,v)
 target=prefix+'/'+path.stem
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 assert all(p.startswith(prefix+'/') for p in task.imported_object_paths),task.imported_object_paths
 mesh=unreal.load_asset(target);assert isinstance(mesh,unreal.SkeletalMesh),target
 assert mesh.skeleton==sk,(target,mesh.skeleton)
 result[path.stem]={'asset':mesh.get_path_name(),'skeleton':mesh.skeleton.get_path_name(),'materialSlots':len(mesh.materials),'boundsHeightCm':mesh.get_bounds().box_extent.z*2,'importedObjects':list(task.imported_object_paths)}
 # Reuse existing simple material parents; no final shader or imported texture.
 slots=mesh.get_editor_property('materials')
 for i,slot in enumerate(slots):
  label=str(slot.material_slot_name);kind='M_Skin' if 'Skin' in label else ('M_Hair' if 'Hair' in label else 'M_Robe')
  mat=unreal.load_asset('/Game/DaoVoTan/Visual11/'+kind)
  if mat:slot.set_editor_property('material_interface',mat);slots[i]=slot
 mesh.set_editor_property('materials',slots)
 assert library.save_loaded_asset(mesh)
# A static reference-pose inspection world; no GameMode/default replacement.
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
map_path=prefix+'/L_RetopoGate1'
assert level.new_level(map_path) if not library.does_asset_exist(map_path) else level.load_level(map_path)
for a in actors.get_all_level_actors():actors.destroy_actor(a)
for name,r in result.items():
 a=actors.spawn_actor_from_class(unreal.SkeletalMeshActor,unreal.Vector(0,0,0));a.set_actor_label('INTERNAL_'+name);c=a.skeletal_mesh_component;c.set_skeletal_mesh_asset(unreal.load_asset(r['asset']));a.set_actor_enable_collision(False)
 r['boneCount']=c.get_num_bones()
 r['jointPositionsCm']={str(n):list(c.get_socket_transform(n,unreal.RelativeTransformSpace.RTS_COMPONENT).translation.to_tuple()) for n in ['pelvis','head','hand_r','calf_l','foot_l']}
ref=actors.spawn_actor_from_class(unreal.SkeletalMeshActor,unreal.Vector(0,-200,0));ref.set_actor_label('Manny_ReadOnlyReference');ref.skeletal_mesh_component.set_skeletal_mesh_asset(manny)
for label,rot,intensity in [('Key',unreal.Rotator(-35,-30,0),8.),('Fill',unreal.Rotator(-25,-150,0),4.)]:
 a=actors.spawn_actor_from_class(unreal.DirectionalLight,unreal.Vector(0,0,300));a.set_actor_label(label);a.set_actor_rotation(rot,False);a.light_component.set_intensity(intensity)
actors.spawn_actor_from_class(unreal.SkyLight,unreal.Vector(0,0,300))
assert level.save_current_level()
(O/'metrics/unreal-import.json').write_text(json.dumps({'modules':result,'map':map_path,'expectedSkeleton':sk.get_path_name(),'newSkeletonCreated':False,'productionDefaultsChanged':False},indent=2))
unreal.log('RETOPO_IMPORT_SUCCESS '+str(len(result)))
