
Collection = {}
objList = [bpy.data.objects["mesh37-2.001"]]
planeCoList = [(0, 0, 0.12)]
planeNoList = [(0, 0, 1)]

for i in range(len(objList)):
    # sliceProcess(obj, p_co, p_no, name1, name2, circle radius, srfInset, Collection)
    # circle radius = radius of tested circles on cut surfaces
    # srfInset = offset profile from surface edge along which centers of circles are generated
    Collection = sliceProcess(objList[i], planeCoList[i], planeNoList[i], 
                              objList[i].name + " half 1", objList[i].name + " half 2", 
                              0.008, 0.004, Collection)

print("collection is ")
print(Collection)

# carveProcess(Collection, testLength, offset, extrudeLength)
# testLength = min contained length required to legitimize a test circle
# offset = offset from test circle to make final extrusion
# extrudeLength = final extrusion length
# Collection = carveProcess(Collection, 0.02, 0.004, 0.015)

"""
bpy.ops.mesh.extrude_vertices_move(MESH_OT_extrude_verts_indiv={"mirror":False}, TRANSFORM_OT_translate={"value":(0, 0, 0),
"orient_axis_ortho":'X', "orient_type":'GLOBAL', "orient_matrix":((0, 0, 0), (0, 0, 0), (0, 0, 0)), "orient_matrix_type":'GLOBAL',
"constraint_axis":(False, False, False), "mirror":False, "use_proportional_edit":False, "proportional_edit_falloff":'SMOOTH', "proportional_size":1,
"use_proportional_connected":False, "use_proportional_projected":False, "snap":False, "snap_target":'CLOSEST', "snap_point":(0, 0, 0),
"snap_align":False, "snap_normal":(0, 0, 0), "gpencil_strokes":False, "cursor_transform":False, "texture_space":False, "remove_on_cancel":False,
"view2d_edge_pan":False, "release_confirm":False, "use_accurate":False, "use_automerge_and_split":False})

bpy.ops.mesh.extrude_faces_move(MESH_OT_extrude_faces_indiv={"mirror":False}, TRANSFORM_OT_shrink_fatten={"value":0, "use_even_offset":False,
"mirror":False, "use_proportional_edit":False, "proportional_edit_falloff":'SMOOTH', "proportional_size":1, "use_proportional_connected":False,
"use_proportional_projected":False, "snap":False, "snap_target":'CLOSEST', "snap_point":(0, 0, 0), "snap_align":False, "snap_normal":(0, 0, 0),
"release_confirm":False, "use_accurate":False})

bpy.ops.mesh.extrude_faces_move(TRANSFORM_OT_shrink_fatten = override)
"""