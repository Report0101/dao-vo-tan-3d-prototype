from pathlib import Path
import struct,zlib,json,hashlib,collections,math
src=Path('R:/ArtSource/Meshy_AI_Jade_Warden_1017231731_generate.fbx');data=src.read_bytes();version=struct.unpack_from('<I',data,23)[0]
def prop(p):
 t=chr(data[p]);p+=1
 scalars={'Y':'h','C':'?','I':'i','F':'f','D':'d','L':'q'}
 if t in scalars:
  f='<'+scalars[t];return struct.unpack_from(f,data,p)[0],p+struct.calcsize(f)
 if t in 'fdilbc':
  n,enc,size=struct.unpack_from('<III',data,p);p+=12;b=data[p:p+size];b=zlib.decompress(b) if enc else b
  f={'f':'f','d':'d','i':'i','l':'q','b':'b','c':'b'}[t]
  return list(struct.unpack('<'+f*n,b)),p+size
 if t in 'SR':
  n=struct.unpack_from('<I',data,p)[0];p+=4;b=data[p:p+n]
  return (b.decode('utf-8',errors='replace') if t=='S' else {'byteLength':n,'sha256':hashlib.sha256(b).hexdigest()}),p+n
 raise ValueError((t,p))
def node(p):
 end,n,size=struct.unpack_from('<III',data,p);length=data[p+12];p+=13
 if not end:return None,p
 name=data[p:p+length].decode();p+=length;ps=[]
 for _ in range(n):v,p=prop(p);ps.append(v)
 children=[]
 while p<end-13:
  c,p=node(p)
  if c:children.append(c)
 return {'name':name,'properties':ps,'children':children},end
nodes=[];p=27
while p<len(data)-13:
 n,p=node(p)
 if not n:break
 nodes.append(n)
def child(n,name):return next((c for c in n['children'] if c['name']==name),None)
def arr(n,name):
 c=child(n,name);return c['properties'][0] if c and c['properties'] else []
objects=next(n for n in nodes if n['name']=='Objects')['children'];geoms=[]
for o in objects:
 if o['name']!='Geometry':continue
 v=arr(o,'Vertices');verts=[tuple(v[i:i+3]) for i in range(0,len(v),3)];ix=arr(o,'PolygonVertexIndex');faces=[];face=[]
 for i in ix:
  face.append(i if i>=0 else -i-1)
  if i<0:faces.append(face);face=[]
 if not verts:continue
 edges=collections.Counter();parent=list(range(len(verts)))
 def find(i):
  while parent[i]!=i:parent[i]=parent[parent[i]];i=parent[i]
  return i
 def union(a,b):
  a,b=find(a),find(b)
  if a!=b:parent[b]=a
 areas=[];normals=[];degenerate=0
 for f in faces:
  for a,b in zip(f,f[1:]+f[:1]):edges[tuple(sorted((a,b)))]+=1;union(a,b)
  a,b,c=(verts[i] for i in f[:3]);u=[b[i]-a[i] for i in range(3)];w=[c[i]-a[i] for i in range(3)];cr=(u[1]*w[2]-u[2]*w[1],u[2]*w[0]-u[0]*w[2],u[0]*w[1]-u[1]*w[0]);area=math.sqrt(sum(x*x for x in cr))/2;areas.append(area);normals.append(cr)
  if len(set(f))<len(f):degenerate+=1
 groups=collections.defaultdict(list)
 for i in range(len(verts)):groups[find(i)].append(i)
 components=sorted(({'vertices':len(ids),'boundsMin':[min(verts[j][a] for j in ids) for a in range(3)],'boundsMax':[max(verts[j][a] for j in ids) for a in range(3)]} for ids in groups.values()),key=lambda x:-x['vertices'])
 vertex_positions=collections.Counter(verts);layers=[]
 for c in o['children']:
  if not c['name'].startswith('LayerElement'):continue
  info={'type':c['name'],'layer':c['properties'],'fields':{cc['name']:(len(cc['properties'][0]) if cc['properties'] and isinstance(cc['properties'][0],list) else cc['properties']) for cc in c['children']}}
  if c['name']=='LayerElementNormal':
   ns=arr(c,'Normals');lens=[math.sqrt(sum(x*x for x in ns[i:i+3])) for i in range(0,len(ns),3)]
   info['normalLengths']={'count':len(lens),'nonFinite':sum(not math.isfinite(x) for x in lens),'zero':sum(x<1e-10 for x in lens),'min':min(lens) if lens else None,'max':max(lens) if lens else None}
   mapping=arr(c,'MappingInformationType');reference=arr(c,'ReferenceInformationType');indices=arr(c,'NormalsIndex');cursor=0;opposed=0;tested=0
   if mapping=='ByPolygonVertex':
    for f,normal in zip(faces,normals):
     for _ in f:
      k=indices[cursor] if indices else cursor;nv=ns[k*3:k*3+3];cursor+=1
      if len(nv)==3:tested+=1;opposed+=sum(a*b for a,b in zip(normal,nv))<0
    info['cornerNormalsOpposedToWinding']={'tested':tested,'negativeDot':opposed}
  layers.append(info)
 volume=0
 for f in faces:
  a,b,c=(verts[i] for i in f[:3]);volume+=(a[0]*(b[1]*c[2]-b[2]*c[1])+a[1]*(b[2]*c[0]-b[0]*c[2])+a[2]*(b[0]*c[1]-b[1]*c[0]))/6
 geoms.append({'id':o['properties'],'vertices':len(verts),'polygons':len(faces),'polygonSizes':dict(collections.Counter(map(len,faces))),'trianglesAfterFanTriangulation':sum(len(f)-2 for f in faces),'components':components,'componentCount':len(components),'degenerateIndexFaces':degenerate,'zeroAreaFaces':sum(a<1e-16 for a in areas),'boundaryEdges':sum(n==1 for n in edges.values()),'nonManifoldEdgesOverTwoFaces':sum(n>2 for n in edges.values()),'duplicatePositionVertices':sum(n-1 for n in vertex_positions.values()),'edgeCount':len(edges),'EulerCharacteristic':len(verts)-len(edges)+len(faces),'signedVolumeLocalUnits3':volume,'boundsMin':[min(v[a] for v in verts) for a in range(3)],'boundsMax':[max(v[a] for v in verts) for a in range(3)],'layers':layers})
def compact(n):
 return {'name':n['name'],'properties':n['properties'],'children':[compact(c) for c in n['children']]}
result={'source':str(src),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'fbxVersion':version,'objectTypes':dict(collections.Counter(o['name'] for o in objects)),'objectCount':len(objects),'objects':[{'type':o['name'],'identity':o['properties']} for o in objects],'geometry':geoms,'globalSettings':compact(next(n for n in nodes if n['name']=='GlobalSettings')),'connections':compact(next(n for n in nodes if n['name']=='Connections')),'modelDetails':[compact(o) for o in objects if o['name'] in ['Model','Material','Texture','Video','Deformer','AnimationStack','NodeAttribute']],'header':compact(nodes[0])}
out=Path('outputs/PLAYER_V1_MESH_AUDIT');out.mkdir(parents=True,exist_ok=True);(out/'raw-fbx.json').write_text(json.dumps(result,indent=2),encoding='utf-8');print(json.dumps({k:result[k] for k in ['bytes','sha256','fbxVersion','objectTypes','objectCount']}));print(json.dumps([{k:v for k,v in g.items() if k not in ['components','layers']} for g in geoms],indent=2))
