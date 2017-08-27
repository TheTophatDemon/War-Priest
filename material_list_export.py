# War Priest Urho3d Material List Export Script

import bpy
import os
import operator

scene = bpy.context.scene
selMesh = bpy.context.object.data
selMesh.materials

print ("================MAT NAMES")

matNames = []

for poly in selMesh.polygons:
    matname = selMesh.materials[poly.material_index].name
    documented = 0
    for i in range(len(matNames)):
        if matNames[i] == matname:
            documented = 1
            break
    if documented == 0:
        matNames.append(matname)
fn = bpy.data.scenes["Scene"].urho_exportsettings.outputPath + bpy.context.object.name + ".txt"
f = open(fn, 'w', encoding='utf-8')        
for name in matNames:
    f.write("Materials/" + name + ".xml\n")
f.close()