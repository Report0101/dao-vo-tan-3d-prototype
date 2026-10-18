import pathlib
p=pathlib.Path(__file__).with_name('gate1b_retopo_render_validate.py')
exec(p.read_text().split("for name,loc in [('01_front'")[0])
for o in parts:
 if o.name.startswith(('UpperRobe','LowerRobe','Hair','Belt','Sash','Accessory')):o.hide_render=True
shot('20_underbody_armpit',(2,-3,1.40),(.18,0,1.40),.45,True)
shot('21_underbody_elbow',(2,-3,1.21),(.35,.02,1.21),.30,True)
shot('22_underbody_pelvis',(0,-3,.91),(0,0,.91),.5,True)
shot('23_underbody_knee',(2,-3,.50),(.12,0,.5),.35,True)
for o in parts:
 if o.name=='Boots':o.hide_render=True
shot('24_underbody_ankle',(2,-3,.12),(.14,.01,.12),.30,True)
for o in parts:o.hide_render=False
shot('25_head_front',(0,-3,1.69),(0,-.01,1.69),.35)
shot('26_head_side',(3,0,1.69),(0,-.01,1.69),.35)
shot('27_head_three_quarter',(2,-3,1.69),(0,-.01,1.69),.35)
shot('28_hair_side',(3,0,1.48),(0,.06,1.48),.86)
shot('29_wrist_neutral',(2,-3,1.06),(.45,-.18,1.06),.30,True)
shot('30_palm_thumb',(2,3,1.00),(.50,-.23,1.00),.27)
for o in parts:
 if o.name.startswith(('UpperRobe','LowerRobe','Hair','Belt','Sash','Accessory')):o.hide_render=True
shot('31_pelvis_side',(3,0,.91),(0,0,.91),.52,True)
print('RETOPO_GATE1B_DETAIL_CAPTURE_SUCCESS')
