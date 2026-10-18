import runpy,collections,json
from pathlib import Path
d=runpy.run_path(str(Path(__file__).with_name('player_v1_fbx_raw.py')));faces=d['faces'];verts=d['verts'];directed=collections.Counter();links=collections.defaultdict(list)
for f in faces:
 for a,b in zip(f,f[1:]+f[:1]):directed[tuple(sorted((a,b)))]+=1 if a<b else -1
 for i,a in enumerate(f):links[a].append((f[(i+1)%3],f[(i+2)%3]))
badlinks=[]
for v,edges in links.items():
 graph=collections.defaultdict(set)
 for a,b in edges:graph[a].add(b);graph[b].add(a)
 pending=[next(iter(graph))];seen=set()
 while pending:
  a=pending.pop()
  if a in seen:continue
  seen.add(a);pending.extend(graph[a]-seen)
 if len(seen)!=len(graph) or any(len(n)!=2 for n in graph.values()):badlinks.append(v)
result={'inconsistentDirectedEdgePairs':sum(x!=0 for x in directed.values()),'nonCycleVertexLinks':len(badlinks),'testedVertices':len(links),'limitations':'Does not certify geometric self-intersection or anatomical separation.'}
Path('outputs/PLAYER_V1_MESH_AUDIT/extra-topology.json').write_text(json.dumps(result,indent=2));print(result)
