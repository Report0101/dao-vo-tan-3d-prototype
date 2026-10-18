import unreal,pathlib,json
out=pathlib.Path(r'C:/Users/Le Anh Hoang/Documents/Codex/2026-10-15/files-pasted-by-the-user-you/outputs/PLAYER_V1_RETOPO_GATE1')
(out/'references').mkdir(parents=True,exist_ok=True)
clips={
 'Idle':'/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle',
 'Walk':'/Game/DaoVoTan/Locomotion/Clips/M_Neutral_Walk_Loop_F',
 'Run':'/Game/DaoVoTan/Locomotion/Clips/M_Neutral_Run_Loop_F',
 'Strafe':'/Game/DaoVoTan/Locomotion/Clips/M_Neutral_Run_Loop_LL',
 'Pivot':'/Game/DaoVoTan/Locomotion/Clips/M_Neutral_Run_Pivot_F_B_Rfoot',
 'Jump':'/Game/DaoVoTan/Locomotion/Clips/M_Neutral_Jump_F_Start_Stand_Rfoot',
 'Landing':'/Game/DaoVoTan/Locomotion/Clips/M_Neutral_Jump_LL_Land_Stand_Light_Lfoot',
 'SwordGuard':'/Game/DaoVoTan/Visual11/AS_SwordIdle',
 'SwordAttack':'/Game/DaoVoTan/Visual11/AS_SwordAttackA',
 'Meditation':'/Game/DaoVoTan/Animation/AS_MeditationLoop'}
result={}
for name,path in clips.items():
 a=unreal.load_asset(path);assert a,path
 t=unreal.AssetExportTask();t.object=a;t.filename=str(out/'references'/(name+'.fbx'));t.automated=True;t.prompt=False;t.replace_identical=True;t.exporter=unreal.AnimSequenceExporterFBX();t.options=unreal.FbxExportOption()
 assert unreal.Exporter.run_asset_export_task(t),name
 result[name]={'asset':path,'length':a.get_play_length(),'export':t.filename}
j=unreal.load_asset('/Game/DaoVoTan/Visual11/SM_Jian')
t=unreal.AssetExportTask();t.object=j;t.filename=str(out/'references/Jian.fbx');t.automated=True;t.prompt=False;t.replace_identical=True;t.exporter=unreal.StaticMeshExporterFBX();assert unreal.Exporter.run_asset_export_task(t)
(out/'references/manifest.json').write_text(json.dumps(result,indent=2))
unreal.log('RETOPO_REFERENCE_EXPORT_SUCCESS')
