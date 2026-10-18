# Head — classification B

**B: still requires a dedicated sculpt pass.** A separate neck/head quad surface, jaw/cheek envelope, nose, mouth/eye-region relief and separate intentional ear shells exist. These are a low-detail geometric foundation, not portrait-ready facial anatomy. Facial identity is not faithfully recovered yet; eyes/lids, mouth cavity/lip loops, ears, forehead/crown silhouette and hairline need artist sculpt/retopology. No facial rig, morphs or expressions are claimed.

The first UE import exposed a Head mesh node / head bone name collision: FBX produced head1 and the skinning fixture failed. Export geometry nodes now use Geo_ prefixes; the accepted final mesh has no unknown bones and matches Manny. Initial failed evidence is retained separately. No production skeleton was changed to accommodate the collision.
