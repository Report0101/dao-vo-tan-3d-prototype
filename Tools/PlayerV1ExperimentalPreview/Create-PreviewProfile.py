import unreal,json,pathlib
O=pathlib.Path(unreal.Paths.project_saved_dir())/'PlayerV1PreviewSetup';O.mkdir(parents=True,exist_ok=True)
prefix='/Game/DaoVoTan/Experimental/PlayerV1/RetopoGate1/'
order=['Body_Underbody','Head','Hands','Boots','Hair','UpperRobe','LowerRobe_FrontL','LowerRobe_FrontR','LowerRobe_BackL','LowerRobe_BackR','LowerRobe_SideL','LowerRobe_SideR','Belt','Sash','Accessory_Jade']
meshes=[unreal.load_asset(prefix+'SK_'+n) for n in order];assert all(isinstance(m,unreal.SkeletalMesh) for m in meshes)
manny=unreal.load_asset('/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple');assert all(m.skeleton==manny.skeleton for m in meshes)
base=unreal.load_asset('/Game/DaoVoTan/Data/DA_Player');assert not base.get_editor_property('skeletal_modules')
target='/Game/DaoVoTan/Experimental/PlayerV1/DA_PlayerV1Experimental'
r=unreal.load_asset(target)
if r is None:r=unreal.EditorAssetLibrary.duplicate_asset(base.get_path_name(),target)
assert r
r.set_editor_property('display_name','Player V1 Experimental');r.set_editor_property('skeletal_modules',meshes)
unreal.EditorAssetLibrary.save_loaded_asset(r,False)
assert r.get_editor_property('skeletal_modules') and len(r.get_editor_property('skeletal_modules'))==15
(O/'asset-mapping.json').write_text(json.dumps({'previewRecipe':r.get_path_name(),'acceptedDefaultRecipe':base.get_path_name(),'skeleton':manny.skeleton.get_path_name(),'leader':manny.get_path_name(),'modules':[m.get_path_name() for m in meshes],'noCandidateReimport':True,'sourceCommit':'dfac6897983adc58406a820928eae70801902833'},indent=2))
print('PLAYER_V1_PREVIEW_RECIPE_CREATED',r.get_path_name(),len(meshes))

import unreal,json,pathlib
O=pathlib.Path(unreal.Paths.project_saved_dir())/'PlayerV1PreviewSetup';O.mkdir(parents=True,exist_ok=True)
r=unreal.load_asset('/Game/DaoVoTan/Experimental/PlayerV1/DA_PlayerV1Experimental');assert r
colors={'Skin':(.57,.38,.26,1),'Hair':(.018,.022,.028,1),'EyeFoundation':(.72,.69,.62,1),'InnerCloth':(.13,.22,.25,1),'OuterCloth':(.23,.43,.40,1),'Leather':(.12,.065,.045,1),'MetalJade':(.12,.48,.36,1)}
bindings={};paths={};folder='/Game/DaoVoTan/Experimental/PlayerV1/Preview'
for name,color in colors.items():
 target=folder+'/MI_Preview_'+name;mat=unreal.load_asset(target)
 if mat is None:mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('MI_Preview_'+name,folder,unreal.MaterialInstanceConstant,unreal.MaterialInstanceConstantFactoryNew())
 parent=unreal.load_asset('/Game/DaoVoTan/Visual11/'+('M_Skin' if name in ['Skin','EyeFoundation'] else 'M_Hair' if name=='Hair' else 'M_Robe'))
 assert mat and parent
 unreal.MaterialEditingLibrary.set_material_instance_parent(mat,parent)
 unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(mat,'Tint',unreal.LinearColor(*color))
 assert unreal.EditorAssetLibrary.save_loaded_asset(mat,False)
 bindings[unreal.Name('INTERNAL_'+name)]=mat;paths['INTERNAL_'+name]=mat.get_path_name()
r.set_editor_property('skeletal_slot_materials',bindings);assert unreal.EditorAssetLibrary.save_loaded_asset(r,False)
d=json.loads((O/'asset-mapping.json').read_text());d['componentOnlySlotMaterials']=paths;d['materialReason']='Current Gate1B mesh slots have null material interfaces. Preview recipe supplies simple existing-parent material instances without changing source meshes.';(O/'asset-mapping.json').write_text(json.dumps(d,indent=2))
print('PREVIEW_SLOT_BINDINGS_SAVED',len(bindings))
