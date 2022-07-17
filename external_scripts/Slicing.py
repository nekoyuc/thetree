import bpy, bmesh, mathutils, itertools

class ModeManager:
    def set_mode(self, m):
        self.old_mode = bpy.context.active_object.mode
        bpy.ops.object.mode_set(mode=m)
    def restore_mode(self):
        bpy.ops.object.mode_set(mode=self.old_mode)
        
mm = ModeManager()

def RequireMode(required_mode):
    def decorator(fun):
        def inner(*args, **kwargs):
            old_mode = bpy.context.active_object.mode
            bpy.ops.object.mode_set(mode=required_mode)
            fun(*args, **kwargs)
            bpy.ops.object.mode_set(mode=old_mode)
        return inner
    return decorator

###----------
### basics
###----------
def editMode():
    bpy.ops.object.mode_set(mode='EDIT')
    
def objectMode():
    bpy.ops.object.mode_set(mode='OBJECT')
    
def vertMode():
    bpy.ops.mesh.select_mode(type="VERT")
    
def edgeMode():
    bpy.ops.mesh.select_mode(type="EDGE")
    
def faceMode():
    bpy.ops.mesh.select_mode(type="FACE")
    
#set object to be active
def setActObj(obj):
    bpy.context.view_layer.objects.active = obj
    
#return active object
def actObj():
    return bpy.context.object

#deselect all objects
def dAllObject():
    bpy.ops.object.select_all(action = "DESELECT")

#select all objects
def sAllObject():
    bpy.ops.object.select_all(action = "SELECT")

#deselect all mesh
def dAllMesh():
    bpy.ops.mesh.select_all(action = "DESELECT")

#select all mesh
def sAllMesh():
    bpy.ops.mesh.select_all(action = "SELECT")

#reset cursor to origin
def resetCur():
    bpy.context.scene.cursor.location=(0,0,0)

#reset origin to cursor. Object mode only
def oriToCur():
    bpy.ops.object.origin_set(type="ORIGIN_CURSOR")

#apply and clear all transformations
def transformApp():
    bpy.ops.object.transform_apply()

#get bmesh from an object
def getBmesh(object):
    setActObj(object)
    editMode()
    bm = bmesh.from_edit_mesh(object.data)
    return bm

#get active polygons in the active object
def getSelectedPoly():
    for poly in actObj().data.polygons:
        if poly.select == True:
            return poly
    return None

#proportional object mode
def proObj(Toggle):
    bpy.context.scene.tool_settings.use_proportional_edit_objects = Toggle

###----------
### individual actions
###----------
# @RequireMode("OBJECT")
# rotate a face object to align with a parent object
def rotateToFace(faceObj, parentNor):
    objectMode()
    qrot = parentNor.rotation_difference(mathutils.Vector((0,0,1)))
    faceObj.rotation_mode = "QUATERNION"
    faceObj.rotation_quaternion = qrot.inverted()
    return faceObj

# add a circle of radius r at the center of faceObj
def addCircleToFace(faceObj, r, center):
    # if the line below is deteleted, "ReferenceError: StructRNA of type Object has been removed" error appears
    objectMode()
    
    parentNor = faceObj.data.polygons[0].normal
    
    bpy.ops.mesh.primitive_circle_add(location=center, fill_type = "NGON", radius = r)
    editMode()
    sAllMesh()
    bpy.ops.mesh.dissolve_limited()
    return rotateToFace(actObj(), parentNor)

# return the world coordinates of an object's one vertice
def woCo(obj, obj_vert):
    return obj.matrix_world @ obj_vert

# determine if 2D ObjS is contained in 2D ObjL. ObjL has to be triangulated.
def isContained2D(ObjS, ObjL):
    intersect = []
    k = 0
    for s in ObjS.data.vertices:
        for l in itertools.combinations(ObjL.data.vertices, 3):
            point = mathutils.geometry.intersect_point_tri(woCo(ObjS, s.co),
            woCo(ObjL, l[0].co), woCo(ObjL, l[1].co), woCo(ObjL, l[2].co))
            if point!=None:
                k+=1
                intersect.append(s)
                break
    # print("k is " + str(k))
    # print("number of vertices is " + str(len(ObjS.data.vertices)))
    if k == len(ObjS.data.vertices):
        # print(ObjS.name + " is inside " + ObjL.name + "\n")
        return True
    else:
        # print(ObjS.name + " is outside " + ObjL.name + "\n")
        return False

# determine if a point (in the form of vector) are contained in a bmesh. return contained points. do not use in 2D
def isPointInside(point, obj):
    objectMode()
    setActObj(obj)
    dAllObject()
    editMode()
    sAllMesh()
    
    # below function is edit mode only
    bm = bmesh.from_edit_mesh(obj.data)

    bvh = mathutils.bvhtree.BVHTree.FromBMesh(bm, epsilon = 0.00001)
    
    fco, normal, _, _ = bvh.find_nearest(point)
    p2 = fco - mathutils.Vector(point)
    v = p2.dot(normal)
    if v >= 0:
        return True
    else:
        return False

# fill holes of selected objects
def fillOpen(obj):
    objectMode()
    dAllObject()
    obj.select_set(True)
    editMode()
    bpy.ops.mesh.select_mode(type="VERT")
    dAllMesh()
    bpy.ops.mesh.select_non_manifold()
    bpy.ops.mesh.fill()

# cut an object by plane (origin = p_co, normal = p_no) into 2 meshes, generates 2 new objects
def cutM(object, p_co, p_no, names):
    bm = getBmesh(object)
    newMesh = []
    for i in range(2):
        Nbm = bm.copy()
        
        bmesh.ops.bisect_plane(Nbm, geom=Nbm.verts[:]+Nbm.edges[:]+Nbm.faces[:],
        plane_co=p_co, plane_no=p_no, clear_outer=i, clear_inner=1-i)
        
        newm = bpy.data.meshes.new(object.name+str(i+1))
        Nbm.to_mesh(newm)
        newMesh.append(newm)
    
    editMode()
    for i in range(2):
        newo = bpy.data.objects.new(names[i], newMesh[i])
        bpy.context.collection.objects.link(newo)
    
# get a list of circle centers fron surface inset
def getContainedCirCen(srfObj, t):
    setActObj(srfObj)
    srfObj.select_set(True)
    l0 = len(srfObj.data.vertices)
    editMode()
    sAllMesh()
    bpy.ops.mesh.inset(thickness = t)
    
    # update the inset outcome in blender system
    objectMode()
    editMode()
    
    l = len(srfObj.data.vertices)
    C = []
    for i in range(l-l0):
        C.append(srfObj.data.vertices[i+l0].co)
    return C

# Triangulate the cut surface
def triSrf(srfObj):
    setActObj(srf)
    
    # make sure srfObj is the only selected object
    dAllObject()
    srf.select_set(True)
    
    editMode()
    sAllMesh()
    bpy.ops.mesh.dissolve_limited()
    bpy.ops.mesh.quads_convert_to_tris()

###----------
### whole process of slicing
### inputs: object to slice, coordinate of plane, normal of plane, 2 names resulted from slicing, dictionary
### of tuples (objects resulted from slicing, resulted cut surface, circles contained in cut surface
### outputs: updated dictionary
###----------
def sliceProcess(obj, p_co, p_no, name1, name2, radius, Dic):
    # Create 2 halves by slicing
    cutM(obj, p_co, p_no, [name1, name2])
    fillOpen(bpy.data.objects[name1])
    fillOpen(bpy.data.objects[name2])
    
    # Create a cut surface
    objectMode()
    bpy.data.objects[name2].select_set(True)
    bpy.ops.object.duplicate()
    editMode()
    bpy.ops.mesh.separate()
    bpy.context.selected_objects[1].name = obj.name + " cutsurf"
    srf = bpy.data.objects[obj.name + " cutsurf"]
    
    # Remove cutsurface remainder
    objectMode()
    bpy.data.objects.remove(bpy.context.selected_objects[0])
    
    # set srf origin to (0,0,0)
    srf.select_set(True)
    setActObj(srf)
    objectMode()
    resetCur()
    oriToCur()
   
    # make a copy of srf
    bpy.ops.object.duplicate(linked=False)
    srfCopy = bpy.context.object
        
    # get a list of circle centers
    V = getContainedCirCen(srfCopy, 1.5)
    
    # Create circles
    C = []
    n = 0
    Contained = []
    
    for i in V:
        n += 1
        c = addCircleToFace(srfCopy, radius, i)
        c.name = obj.name + " circle " + str(n)
        C.append(c)
    
    for c in C:
        if isContained2D(c, srf) == True:
            Contained.append(c)
        else:
            bpy.data.objects.remove(c)
    
    # remove srf and srfCopy
    bpy.data.objects.remove(srfCopy)
    bpy.data.objects.remove(srf)

    # Add extrusion
    #addCircleToFace(cir, 0.5).name = obj.name + " PinSrf"
    
    
    # Add original, half 1, half 2, contained circles to dictionary
    dAllObject()

    Dic[obj.name] = [obj, bpy.data.objects[name1], bpy.data.objects[name2], Contained]
    return Dic


###----------
### whole process carving
### inputs: Collection from whole process slicing
### outputs:
###----------

def carveProcess(Collection, testLength, offset, extrudeLength):
    CC = []

    #test good circles
    for key in Collection:
        original = Collection[key][0]
        circles = Collection[key][3]

        for c in circles:
            if len(CC) > 0:
                bpy.data.objects.remove(c)
                continue

            vertN = len(c.data.vertices)

            setActObj(c)
            objectMode()
            dAllObject()
            c.select_set(True)
            transformApp()
            bpy.ops.object.duplicate(linked = False)
            c0 = bpy.context.object
            dAllObject()
            setActObj(c)

            editMode()
            dAllMesh()
            
            sAllMesh()
            bpy.ops.mesh.extrude_faces_move(TRANSFORM_OT_shrink_fatten = {"value": testLength/2})

            # to register extrusion
            objectMode()
            editMode()

            k = 0
            l = 0
           
            for i in range(vertN):
                point = mathutils.Vector(c.data.vertices[i+vertN].co)
                if isPointInside(point, original) == False:
                    bpy.data.objects.remove(c)
                    bpy.data.objects.remove(c0)
                    break
                else:
                    k += 1
            
            if k == vertN:
                setActObj(c)
                editMode()
                dAllMesh()
                bpy.ops.mesh.select_non_manifold()
                bpy.ops.mesh.fill()
                bpy.ops.mesh.dissolve_limited()
                bpy.ops.mesh.extrude_faces_move(TRANSFORM_OT_shrink_fatten = {"value": testLength/2})
                
                # to register extrusion
                objectMode()
                editMode()
                
                for i in range(vertN):
                    point = mathutils.Vector(c.data.vertices[i + vertN * 2].co)
                    if isPointInside(point, original) == False:
                        bpy.data.objects.remove(c)
                        bpy.data.objects.remove(c0)
                        break
                    else:
                        l += 1

                if l == vertN:
                    CC.append([c0, c])

        Collection[key][3] = CC
    
    # carve out
    for key in Collection:
        i = Collection[key][3][0][0]
        objectMode()
        setActObj(i)
        dAllObject()
        editMode()
        dAllMesh()
        sAllMesh()

        bpy.ops.mesh.inset(thickness = offset)
        bpy.ops.mesh.select_all(action = "INVERT")
        bpy.ops.mesh.delete(type = "VERT")

        sAllMesh()
        bpy.ops.mesh.extrude_faces_move(TRANSFORM_OT_shrink_fatten = {"value": extrudeLength})
        dAllMesh()
        bpy.ops.mesh.select_non_manifold()
        bpy.ops.mesh.fill()
        bpy.ops.mesh.dissolve_limited()
        bpy.ops.mesh.extrude_faces_move(TRANSFORM_OT_shrink_fatten = {"value": extrudeLength})

        for half in [Collection[key][1], Collection[key][2]]:
            objectMode()
            setActObj(half)
            dAllObject()
            bpy.ops.object.modifier_add(type = "BOOLEAN")
            bpy.context.object.modifiers["Boolean"].operation = "DIFFERENCE"
            bpy.context.object.modifiers["Boolean"].object = i
            bpy.ops.object.modifier_apply(modifier = "Boolean", report = True)


    print("CC is: ")
    print(CC)
    return Collection  




####----------
#### Execution
####----------

Collection = {}
objList = [bpy.data.objects["Icosphere"]]
planeCoList = [(0,0,0.5)]
planeNoList = [(1,1,1)]

for i in range(len(objList)):
    Collection = sliceProcess(objList[i], planeCoList[i], planeNoList[i], objList[i].name + " half 1",
    objList[i].name + " half 2", 1, Collection)

print(Collection)




Collection = carveProcess(Collection, 2, 0.2, 6)

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