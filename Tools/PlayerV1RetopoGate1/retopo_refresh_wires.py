import pathlib
p=pathlib.Path(__file__).with_name('retopo_render_validate.py')
exec(p.read_text().split("for name,loc in [('01_front'")[0])
shot('05_wire_front',(0,-4,.95),(0,0,.95),2.15,True)
shot('06_wire_back',(0,4,.95),(0,0,.95),2.15,True)
clothes=[o for o in parts if o.name.startswith(('UpperRobe','LowerRobe','Hair','Belt','Sash','Accessory'))]
for o in clothes:o.hide_render=True
shot('16_underbody_front',(0,-4,.95),(0,0,.95),2.1,True)
shot('17_underbody_back',(0,4,.95),(0,0,.95),2.1,True)
for o in clothes:o.hide_render=False
shot('19_reference_pose',(3,-4,1.1),(0,0,.95),2.15,True)
print('RETOPO_FINAL_WIRE_CAPTURE_SUCCESS')
