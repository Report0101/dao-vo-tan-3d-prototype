"""Repair six leg rotations and pelvis height in owned meditation clips.

Run using UE PythonScript commandlet. Timing and other tracks stay intact.
The target pose is solved in component space, avoiding mirrored local-axis twists.
"""
import json, math, pathlib, unreal

ROOT = pathlib.Path(__file__).resolve().parent
spec = json.loads((ROOT / 'meditation-pose.json').read_text())
idle = unreal.load_asset('/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle')

def data(asset, bone, time):
    p = unreal.AnimationLibrary.get_bone_pose_for_time(asset, bone, time, False)
    return [p.translation.x, p.translation.y, p.translation.z,
            p.rotation.x, p.rotation.y, p.rotation.z, p.rotation.w,
            p.scale3d.x, p.scale3d.y, p.scale3d.z]

def slerp(a, b, t):
    d = sum(x*y for x,y in zip(a,b))
    if d < 0: b = [-v for v in b]; d = -d
    if d > .9995:
        q = [x+(y-x)*t for x,y in zip(a,b)]
    else:
        angle = math.acos(min(1,d))
        q = [(x*math.sin((1-t)*angle)+y*math.sin(t*angle))/math.sin(angle) for x,y in zip(a,b)]
    length = math.sqrt(sum(v*v for v in q))
    return unreal.Quat(*[v/length for v in q])

def multiply(a,b):
    x,y,z,w=a; X,Y,Z,W=b
    return [w*X+x*W+y*Z-z*Y,w*Y-x*Z+y*W+z*X,w*Z+x*Y-y*X+z*W,w*W-x*X-y*Y-z*Z]

def clearance(asset, times):
    bones = ['calf_l','calf_r','foot_l','foot_r','ball_l','ball_r']
    minimum = {n:float('inf') for n in bones}
    for t in times:
        cache = {}
        def component(n):
            if n in cache: return cache[n]
            v = data(asset,n,t)
            path = unreal.AnimationLibrary.find_bone_path_to_root(asset,n)
            p,q = component(str(path[1])) if len(path)>1 else ([0,0,0],[0,0,0,1])
            rotated = multiply(multiply(q,v[:3]+[0]),[-q[0],-q[1],-q[2],q[3]])[:3]
            cache[n] = ([x+y for x,y in zip(p,rotated)],multiply(q,v[3:7]))
            return cache[n]
        for n in bones: minimum[n] = min(minimum[n],component(n)[0][2])
    assert min(minimum.values())>=-1, minimum
    return minimum

report = []
for mode in ['Enter', 'Loop', 'Exit']:
    a = unreal.load_asset('/Game/DaoVoTan/Animation/AS_Meditation'+mode)
    assert a and idle
    model = a.data_model_interface
    count, frames, duration = model.get_number_of_keys(), model.get_number_of_frames(), a.get_play_length()
    times = [duration*i/frames for i in range(count)]
    names = unreal.AnimationLibrary.get_animation_track_names(a)
    before = {str(n):[data(a,n,t) for t in times] for n in names}
    controller = a.controller
    controller.open_bracket('Repair meditation leg pose above the floor', False)
    for n, q in spec['rotationKeys'].items():
        base = data(idle,n,0)[3:7]
        rotations = []
        for i in range(count):
            t = i/frames; alpha = t*t*(3-2*t)
            alpha = 1 if mode == 'Loop' else alpha if mode == 'Enter' else 1-alpha
            rotations.append(slerp(base,q,alpha))
        original = before[n]
        assert controller.set_bone_track_keys(n, [unreal.Vector(*v[:3]) for v in original], rotations,
                                             [unreal.Vector(*v[7:]) for v in original], False)
    # Lift the pelvis along the transition arc: straight height interpolation
    # combined with quaternion interpolation otherwise pushes toes below ground.
    positions = []
    base_z = data(idle,'pelvis',0)[2]
    for i, original in enumerate(before['pelvis']):
        t = i/frames; alpha = t*t*(3-2*t)
        alpha = 1 if mode == 'Loop' else alpha if mode == 'Enter' else 1-alpha
        positions.append(unreal.Vector(original[0],original[1],base_z-48*alpha+36*alpha*(1-alpha)))
    assert controller.set_bone_track_keys('pelvis', positions,
                                         [unreal.Quat(*v[3:7]) for v in before['pelvis']],
                                         [unreal.Vector(*v[7:]) for v in before['pelvis']],False)
    controller.close_bracket(False)
    assert model.get_number_of_keys() == count and a.get_play_length() == duration
    for n in before:
        after = [data(a,n,t) for t in times]
        if n == 'pelvis': assert all(x[:2]==y[:2] and x[3:]==y[3:] for x,y in zip(after,before[n])), n
        elif n not in spec['rotationKeys']: assert after == before[n], n
        else: assert all(x[:3] == y[:3] and x[7:] == y[7:] for x,y in zip(after,before[n])), n
    assert unreal.EditorAssetLibrary.save_loaded_asset(a,False)
    minima = clearance(a, [duration*i/120 for i in range(121)])
    report.append({'asset':a.get_path_name(),'keys':count,'duration':duration,
                   'changedRotationTracks':list(spec['rotationKeys']), 'changedTranslationTracks':['pelvis'],
                   'otherTracksPreserved':True,'sampleCount':121,'minimumBoneHeightCm':minima,
                   'limitation':'Authored skeleton sample only; live mesh/robe and floor checks remain required.'})
out = pathlib.Path(unreal.Paths.project_saved_dir())/'PlayerQ1Recovery'
out.mkdir(exist_ok=True)
(out/'meditation-authoring.json').write_text(json.dumps(report,indent=2))
unreal.log('PLAYER_Q1_MEDITATION_AUTHORED')
