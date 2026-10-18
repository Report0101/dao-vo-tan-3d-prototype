"""Editor-only static import/comparison audit; no gameplay class or production asset edits."""
import unreal, pathlib, os, json, hashlib, shutil
root=pathlib.Path(unreal.Paths.project_dir()).resolve().parent
out=pathlib.Path(os.environ['DAOVOTAN_MESH_AUDIT_OUTPUT']).resolve()
source=root/'ArtSource/Meshy_AI_Jade_Warden_1017231731_generate.fbx'
prefix='/Game/DaoVoTan/Experimental/PlayerV1/MeshyAudit'
out.mkdir(parents=True,exist_ok=True);copy=out/'working/MeshyAudit_SourceCopy.fbx';copy.parent.mkdir(parents=True,exist_ok=True)
digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
before=digest(source)
if copy.exists():assert digest(copy)==before
else:shutil.copy2(source,copy)
asset_path=prefix+'/SM_MeshyRawReference'
mesh=unreal.load_asset(asset_path)
if mesh is None:
 options=unreal.FbxImportUI()
 for name,value in {'import_mesh':True,'import_as_skeletal':False,'mesh_type_to_import':unreal.FBXImportType.FBXIT_STATIC_MESH,'automated_import_should_detect_type':False,'import_materials':False,'import_textures':False,'import_animations':False}.items():options.set_editor_property(name,value)
 data=options.static_mesh_import_data
 for name,value in {'combine_meshes':True,'auto_generate_collision':False,'build_nanite':False,'generate_lightmap_u_vs':False,'remove_degenerates':False,'convert_scene':True,'convert_scene_unit':True,'force_front_x_axis':False,'import_uniform_scale':1.0,'normal_import_method':unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS}.items():data.set_editor_property(name,value)
 task=unreal.AssetImportTask()
 for name,value in {'filename':str(copy),'destination_path':prefix,'destination_name':'SM_MeshyRawReference','automated':True,'replace_existing':False,'save':True,'options':options,'factory':unreal.FbxFactory()}.items():task.set_editor_property(name,value)
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 assert all(p.startswith(prefix+'/') for p in task.imported_object_paths)
 mesh=unreal.load_asset(asset_path)
assert isinstance(mesh,unreal.StaticMesh), 'Source must remain a truthful static import'
manny=unreal.load_asset('/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple')
assert manny
export=out/'working/MannyReference.fbx'
if not export.exists():
 task=unreal.AssetExportTask();task.object=manny;task.filename=str(export);task.automated=True;task.prompt=False;task.replace_identical=False
 task.exporter=unreal.SkeletalMeshExporterFBX();task.options=unreal.FbxExportOption()
 assert unreal.Exporter.run_asset_export_task(task), 'Read-only Manny reference export failed'
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
map_path=prefix+'/L_MeshyAuditComparison'
if unreal.EditorAssetLibrary.does_asset_exist(map_path):assert level.load_level(map_path)
else:assert level.new_level(map_path)
# Everything spawned is owned by the isolated comparison world, not production defaults.
old={a.get_actor_label():a for a in actors.get_all_level_actors()}
def actor(label,cls,location):
 a=old.get(label) or actors.spawn_actor_from_class(cls,unreal.Vector(*location));a.set_actor_label(label);return a
ref=actor('Manny_ReferencePose',unreal.SkeletalMeshActor,(0,0,0));comp=ref.skeletal_mesh_component;comp.set_skeletal_mesh_asset(manny)
bones={str(n):list(comp.get_socket_transform(n,unreal.RelativeTransformSpace.RTS_COMPONENT).translation.to_tuple()) for n in [comp.get_bone_name(i) for i in range(comp.get_num_bones())]}
raw=actor('Meshy_Raw_StaticReference',unreal.StaticMeshActor,(0,180,0));raw.static_mesh_component.set_static_mesh(mesh);raw.set_actor_enable_collision(False)
box=mesh.get_bounding_box();mn=box.min;mx=box.max
raw.set_actor_location(unreal.Vector(180,0,-mn.z),False,False)
manny_bounds=manny.get_bounds();height=manny_bounds.box_extent.z*2
fit_scale=height/(mx.z-mn.z)
fit=actor('Meshy_GlobalFit_NoSkin',unreal.StaticMeshActor,(0,-180,-mn.z*fit_scale));fit.static_mesh_component.set_static_mesh(mesh);fit.set_actor_scale3d(unreal.Vector(fit_scale,fit_scale,fit_scale));fit.set_actor_enable_collision(False)
fit.set_actor_location(unreal.Vector(-180,0,-mn.z*fit_scale),False,False)
material_path=prefix+'/MI_AuditClay_Unlit'
material=unreal.load_asset(material_path)
if material is None:
 material=unreal.AssetToolsHelpers.get_asset_tools().create_asset('MI_AuditClay_Unlit',prefix,unreal.MaterialInstanceConstant,unreal.MaterialInstanceConstantFactoryNew())
 parent=unreal.load_asset('/Engine/EngineDebugMaterials/LevelColorationUnlitMaterial')
 assert parent
 unreal.MaterialEditingLibrary.set_material_instance_parent(material,parent)
 unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(material,'Color',unreal.LinearColor(.35,.55,.5,1))
 unreal.EditorAssetLibrary.save_loaded_asset(material)
for component in [raw.static_mesh_component,fit.static_mesh_component,comp]:
 for index in range(component.get_num_materials()):component.set_material(index,material)
light=actor('Audit_DirectionalLight',unreal.DirectionalLight,(0,0,300));light.set_actor_rotation(unreal.Rotator(-35,-30,0),False)
light.light_component.set_intensity(30.0)
fill=actor('Audit_FillLight',unreal.DirectionalLight,(0,0,300));fill.set_actor_rotation(unreal.Rotator(-30,-150,0),False);fill.light_component.set_intensity(10.0)
sky=actor('Audit_SkyLight',unreal.SkyLight,(0,0,250))
assert level.save_current_level()
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
capture=actors.spawn_actor_from_class(unreal.SceneCapture2D,unreal.Vector(0,650,110))
capture.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(capture.get_actor_location(),unreal.Vector(0,0,100)),False)
target=unreal.RenderingLibrary.create_render_target2d(world,1440,900,unreal.TextureRenderTargetFormat.RTF_RGBA8)
cc=capture.capture_component2d;cc.texture_target=target;cc.capture_source=unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR;cc.fov_angle=65;cc.capture_every_frame=False
cc.set_editor_property('show_flag_settings',[unreal.EngineShowFlagsSetting(show_flag_name='Lighting',enabled=True),unreal.EngineShowFlagsSetting(show_flag_name='PostProcessing',enabled=True),unreal.EngineShowFlagsSetting(show_flag_name='Bloom',enabled=False),unreal.EngineShowFlagsSetting(show_flag_name='EyeAdaptation',enabled=False)])
cc.capture_scene()
(out/'screenshots').mkdir(exist_ok=True)
unreal.RenderingLibrary.export_render_target(world,target,str(out/'screenshots'),'14_unreal_static_comparison.png')
actors.destroy_actor(capture)
static_editor=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
if static_editor is None:
 class CommandletStaticMetrics:
  def get_number_verts(self,*args):return None
  def get_num_uv_channels(self,*args):return None
  def get_lod_count(self,*args):return None
 static_editor=CommandletStaticMetrics()
result={'asset':asset_path,'map':map_path,'sourceCopy':str(copy),'sourceSHA256Before':before,'sourceSHA256After':digest(source),'mannyReferenceAsset':manny.get_path_name(),'mannyExport':str(export),'mannyBonesComponentCm':bones,'staticBoundsCm':{'min':mn.to_tuple(),'max':mx.to_tuple()},'mannyBoundsCm':{'origin':manny_bounds.origin.to_tuple(),'extent':manny_bounds.box_extent.to_tuple()},'nonDestructiveFit':{'uniformScale':fit_scale,'rotationDegrees':[0,0,0],'translationCm':[0,0,-mn.z*fit_scale]},'importSettings':{'skeletal':False,'nanite':False,'collision':False,'materials':False,'textures':False,'lightmapUVGeneration':False,'normals':'import authored normals; compute absent tangents','convertScene':True,'convertSceneUnit':True,'uniformImportScale':1,'geometry':'unchanged; no repair or decimation'},'assetMaterials':len(mesh.static_materials),'renderVerticesLOD0':static_editor.get_number_verts(mesh,0),'UVChannelsLOD0':static_editor.get_num_uv_channels(mesh,0),'LODs':static_editor.get_lod_count(mesh)}
assert before==result['sourceSHA256After']
(out/'unreal-import.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
unreal.log('PLAYER_V1_AUDIT_SUCCESS '+str(out/'unreal-import.json'))
