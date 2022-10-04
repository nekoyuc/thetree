from ursina import *
from ursina.prefabs.first_person_controller import EditorCamera
import panda3d
import numpy as np
import math

from panda3d.core import CollisionRay
from panda3d.core import Quat,Vec3, Vec4
from panda3d.core import MeshDrawer
from panda3d.core import LineSegs, NodePath


app = Ursina()


### CONFIGURATION
### ----------

# define room width, depth and height
ROOM_W = int(6)
ROOM_D = int(6)
ROOM_H = int(6)

def magnitude(vector):
    return math.sqrt(sum(pow(element, 2) for element in vector))

def color_rgb(r,g,b,a=255):
    return color.rgba(r,g,b,a=255)

def color_hsv(h,s,v,a=1):
    return color.color(h,s,v,a=1)


#ls = LineSegs()
#ls.setColor(1,0,0,1)
#ls.setThickness(3)
#ls.moveTo(Vec3(1,2,3))
#ls.drawTo(Vec3(2,3,4))
#print("move to is " + str(ls.moveTo(Vec3(1,2,3))))
#print("vertice number is: " + str(ls.getNumVertices()))
#NodePath(ls.create())


l = Entity(model = "line")
print(l.getNumNodes())
print(l.getX())
print(l.getY())
print(l.getZ())
print(l.isEmpty())
print(l.isHidden)
print(l.getName())
#print(dir(l.model))
print(l.model.vertices)
l.model.vertices = ((1,2,3),(2,3,4),(3,4,5))
l.model.generate()

start_point = Draggable(model='circle', color=color.orange, scale=.025, position=(-0,-0))
    

#line.segment(Vec3(1,2,3), Vec3(2,3,4), Vec4(1,1,1,1), 3, color_hsv(0, 0, random.uniform(.9, 1.0)))


### STATE
### ----------

#---inventories
field_lines = []
line_colors = [color_hsv(174,0.8,0.9), color_hsv(157,0.8,0.9), color_hsv(140,0.7,0.9),color_hsv(123,0.8,0.9), color_hsv(103,0.8,0.9)]
particle_colors = [color_hsv(31,0.9,0.9), color_hsv(51,0.8,0.9),color_hsv(333,0.9,0.9),color_hsv(196,0.9,0.9),color_hsv(246,0.9,0.9)]

#---parameters
TIME_SCALE = 1/1000.0
ortho = 0
draw = 0
mouse_old_position = Vec3(0,0,0)
drawing_line = False

print("held keys are: "+str(held_keys.items()))

class FloorBox(Entity):
    def __init__(self, position=(0,0,0)):
        super().__init__(
            parent = scene,
            position = position,
            model = 'cube',
            origin_y = .5,
            texture = 'white_cube',
            color = color_hsv(0, 0, random.uniform(.9, 1.0)),
            highlight_color = color.lime,
        )

for z in range(ROOM_D):
    for x in range(ROOM_W):
        voxel = FloorBox(position=(
            x,0,z))
        
"""
class FieldViz(Entity):
    def __init__(self, center):
        self.direction = Vec3(0,-0.1,0)
        self.center = Vec3(center)

        super().__init__(model=Mesh(vertices = [self.center, self.center+self.direction], mode = "line", thickness=5),
               color = color_hsv(50,150,0.5))
"""


### LOGIC
### ----------

#-----vector fields-----
class DrawField(Entity):
    def __init__(self, center, direction):
        self.direction = Vec3(direction)
        self.center = Vec3(center)

        super().__init__(model = Mesh(vertices = [self.center, self.center + self.direction], mode = "line", thickness = magnitude(self.direction)),
            color = color_hsv(50,150,0.5)
        )

def GravityField(point):
    return Vec3(0,-1,0)
def CosineField(point):
    scale = 2.0
    innerScale = 10
    return Vec3(scale*math.sin(point[0]*innerScale),scale*math.sin(point[1]*innerScale),scale*math.sin(point[2]*innerScale))

#def RandomField(point):

vector_fields = [GravityField, CosineField]
#-----vector fields-----


class Particle(Entity):
    def __init__(self, particle_color, tail_color, world_position=Vec3(0,0,0)):
        super().__init__(
            parent = scene,
            world_position = world_position,
            model = 'sphere',
            origin_y = .5,
            texture = 'white_cube',
            color = color_hsv(random.random()*360, 1, random.uniform(0.9,1)),
            highlight_color = color.lime,
            scale=0.05
          #  scale = 0.1,
        )
        self.position = world_position
        self.velocity = Vec3(0,0,0)
        
        self.tail_color = tail_color
        self.particle_color = particle_color
        self.tail = Entity(model = Mesh(vertices=[Vec3(0), Vec3(0)], mode = "line", thickness = 1), color = self.tail_color)
        self.tail_list = []
        
        self.initial = True
        self.decay = 1
        self.step = 0


    def update(self):
        # remove dead particles
        if self.x > 5.5 or self.x < -0.5 or self.z > 5.5 or self.z < -0.5 or self.y < 0:
            scene.entities.remove(self)

        # apply vector fields
        for l in field_lines:
            n = 0
            distance = magnitude(l.center-self.position)
            if distance < 0.25:
                n += 1
                if n == 2:
                    break
                self.velocity = Vec3(tuple(self.velocity[i] + 0.5 * l.direction[i]/(magnitude(l.direction))
                                          for i in range(3)))
                
                
        old_position = self.world_position
        self.position += Vec3((TIME_SCALE*self.velocity[0], TIME_SCALE*self.velocity[1], TIME_SCALE*self.velocity[2]))
        self.velocity = Vec3(tuple(self.velocity[i]*self.decay for i in range(3)))
        for field in vector_fields:
            self.velocity += field(self.position)
        #diff = [d * TIME_SCALE for d in diff]
        #self.world_position += tuple(diff)
        self.tail_list.append(self.position)
        self.step += 1
        if self.step % 10 != 0:
            if self.initial == True:
                self.initial = False
            else:
                self.tail.model.vertices = self.tail_list
                self.tail.model.generate()
        
 
       # self.t += time.dt
        # Called every frame


class Plane(Entity):
    def __init__(self, position=(0,0,0)):
        super().__init__(
            parent = scene,
            position = position,
            model = 'plane',

            origin_y = .5,
            texture = 'white_cube',
            color = color_hsv(0, 0, random.uniform(.9, 1.0)),
            highlight_color = color.lime,
            scale=25,
            alpha=0.5
           # rotation=(50,50,50)
        )
        self.collider="box"
        self.distance = 2
    def update(self):
        self.world_position = camera.world_position+camera.forward*self.distance
        self.look_at(camera,axis="up")
p = Plane()


#fields = [[[FieldViz((GRID_W * x, GRID_H * y, GRID_D * z)) for z in range(GRID_D_N)] for y in range(GRID_H_N)] for x in range(GRID_W_N)]
#print(fields)

#fields[1][2][3].color = color.blue

def input(key):
    if key == "tab":
        global ortho
        ortho = 1 - ortho
        camera.orthographic = ortho

    if held_keys["up arrow"]:
        camera.fov = camera.fov + 100*time.dt

    if held_keys["down arrow"]:
        camera.fov = camera.fov - 100*time.dt

    global p
    if held_keys["w"]:
        p.distance +=0.05

    if held_keys["s"]:
        p.distance = max(0, p.distance-0.05)

    if held_keys["space"]:
        Particle(random.choice(particle_colors),random.choice(line_colors), mouse.world_point)

    global mouse_old_position
    global drawing_line

    # draw field lines

    if held_keys["z"]:
        mouse_position = mouse.world_point
        if drawing_line == False:
            mouse_old_position = mouse_position
            drawing_line = True
            print("turned!")
        else:
            direction = Vec3(tuple(mouse_position[i] - mouse_old_position[i] for i in range(3)))
            print(direction)
            l = DrawField(mouse_old_position, direction)
            mouse_old_position = mouse_position
            if magnitude(direction) > 0.01:
                field_lines.append(l)
    else:
        drawing_line = False
        z_points = []


 #   if held_keys["x"]:
 #       l = DrawField(mouse.world_point, (1,2,1))

#
#    if held_keys["left mouse"]:
#        idxs = [int(mouse.world_point[i]) for i in range(3)]
#        idxs = [max(min(j,11),0) for j in idxs]
#        fields[idxs[0]][idxs[1]][idxs[2]].color = color.blue
#        p = Particle(mouse.world_point)


    if key == 'left mouse down':
        
        return
        print("mouse position: " + str(mouse.position) + ", type is " + str(type(mouse.position)) + "\n")
        nearPoint = panda3d.core.LPoint3f(0,0,0)
        farPoint = panda3d.core.LPoint3f(0,0,0)
        matrix = camera.lens.getProjectionMat()
        

        camera.perspective_lens.extrude(mouse.position, nearPoint, farPoint)

        hit_info = raycast(camera.world_position, camera.forward, distance=5)

        #if hit_info.hit:
        #    Voxel(position=hitnfo.entity.position + hit_info.normal)

        #--------
        #c = CollisionRay()
        #c.setFromLens(camera.perspective_lens_node, mouse.x*2, mouse.y*3.556)
        #position = c.origin + c.direction/300
        #quat1 = Quat()
        #quat2 = Quat()
        #x_axis = Vec3(1,0,0)
        #y_axis = Vec3(0,1,0)
        #quat1.setFromAxisAngle(camera.world_rotation[0], x_axis)
        #quat2.setFromAxisAngle(camera.world_rotation[1], y_axis)
        #position = quat1.xform(position)
        #position = quat2.xform(position)
        #Particle((position+camera.world_position)/2)
        
        #--------

        #c = Canvas((0,0,0))
        #c.rotation = -camera.world_rotation

        print("hit info: " + str(hit_info.normal) + ", type is " + str(type(hit_info.normal)) + "\n\n--------------------\n\n")




player = EditorCamera()
app.run()
