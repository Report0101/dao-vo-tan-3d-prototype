"""Re-author only right digit rotations on the existing project-owned jian clips.
Run with UE PythonScript commandlet. No skeleton, mesh, timing or other track edits.
"""
import unreal,json,pathlib,hashlib
ROOT=pathlib.Path(__file__).resolve().parent
spec=json.loads((ROOT/'grip-pose.json').read_text())
report=[]
def pose_data(a,n,t):
 p=unreal.AnimationLibrary.get_bone_pose_for_time(a,n,t,False)
 return [p.translation.x,p.translation.y,p.translation.z,p.rotation.x,p.rotation.y,p.rotation.z,p.rotation.w,p.scale3d.x,p.scale3d.y,p.scale3d.z]
def preserved(a,names,times):
 return {str(n):[pose_data(a,n,t) for t in times] for n in names if str(n) not in spec['rotationKeys']}
for name in ['AS_SwordIdle','AS_SwordAttackA']:
 a=unreal.load_asset('/Game/DaoVoTan/Visual11/'+name);assert a
 model=a.data_model_interface
 count=model.get_number_of_keys();frames=model.get_number_of_frames();length=a.get_play_length()
 times=[length*i/frames for i in range(count)]
 names=unreal.AnimationLibrary.get_animation_track_names(a)
 before=preserved(a,names,times)
 original={n:[pose_data(a,n,t) for t in times] for n in spec['rotationKeys']}
 controller=a.controller
 if controller is None:
  controller=unreal.AnimDataController();controller.set_model(model)
 controller.open_bracket('Articulate jian grip around palm-aligned hilt',False)
 for n,q in spec['rotationKeys'].items():
  data=original[n]
  assert controller.set_bone_track_keys(n,[unreal.Vector(*v[:3]) for v in data],[unreal.Quat(*q) for v in data],[unreal.Vector(*v[7:]) for v in data],False)
 controller.close_bracket(False)
 assert a.get_play_length()==length and model.get_number_of_keys()==count
 assert preserved(a,names,times)==before,'Unexpected non-finger track change'
 for n in spec['rotationKeys']:
  for t,v in zip(times,original[n]):
   after=pose_data(a,n,t);assert after[:3]==v[:3] and after[7:]==v[7:],'Bone length/scale changed'
 assert unreal.EditorAssetLibrary.save_loaded_asset(a,False)
 report.append({'asset':a.get_path_name(),'keys':count,'duration':length,'changedRotationTracks':list(spec['rotationKeys']),'otherTracksPreserved':True,'translationsAndScalesPreserved':True})
out=pathlib.Path(unreal.Paths.project_saved_dir())/'PlayerQ1Recovery';out.mkdir(exist_ok=True)
(out/'grip-authoring.json').write_text(json.dumps(report,indent=2))
unreal.log('PLAYER_Q1_GRIP_AUTHORED')
