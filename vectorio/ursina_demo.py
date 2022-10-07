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
acc_field_lines = []
vel_field_lines = []
line_colors = [color_hsv(174,0.8,0.9), color_hsv(157,0.8,0.9), color_hsv(140,0.7,0.9),color_hsv(123,0.8,0.9), color_hsv(103,0.8,0.9)]
particle_colors = [color_hsv(31,0.9,0.9), color_hsv(51,0.8,0.9),color_hsv(333,0.9,0.9),color_hsv(196,0.9,0.9),color_hsv(246,0.9,0.9)]

#---parameters
TIME_SCALE = 1/1000.0
ortho = 0
mouse_old_position = Vec3(0,0,0)

#change drawing state
drawing_line = False
a_line_frequency = 5

#vector strength of tails
a_field_factor = 2
v_preserve_factor = 0.7
v_attract_factor = 0.3
v_field_factor = 12
sin_x_factor = 5
sin_y_factor = 0.1
sin_z_factor = 5
random_x_factor = 0.5
random_y_factor = 0
random_z_factor = 0.5
frequency_x = 15
frequency_y = 20
frequency_z = 25
decay = 0.99


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

#-----acceleration vector fields-----
class DrawAccField(Entity):
    def __init__(self, center, direction):
        self.direction = Vec3(direction)
        self.center = Vec3(center)
        self.y_vec = Vec3(0,direction[1],0)

        super().__init__(model = Mesh(vertices = [self.center, self.center + self.direction], mode = "line", thickness = 3),
            color = color_hsv(280,1,0.8)
        )
    
def GravityField(point, phase):
    return Vec3(0,-1,0)

def CosineField(point, phase):
    return Vec3(sin_x_factor*math.sin(point[0]*frequency_x*phase[2]+phase[0]),
    sin_y_factor*math.sin(point[1]*frequency_y),sin_z_factor*math.sin(point[2]*frequency_z*phase[3]+phase[1]))

def RandomField(point, phase):
    return Vec3(random_x_factor*random.random(), random_y_factor*random.random(), random_z_factor*random.random())


vector_fields = [GravityField, CosineField, RandomField]
#-----acceleration vector fields-----


#-----velocity vector fields-----
class DrawVelField(Entity):
    def __init__(self,center,direction):
        self.direction = Vec3(direction)
        self.center = Vec3(center)

        super().__init__(model = Mesh(vertices = [self.center, self.center + self.direction], mode = "line", thickness = magnitude(self.direction)),
            color = color_hsv(320,1,0.8)
        )
#-----velocity vector fields-----


#-----particles-----
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
            scale=0.04
          #  scale = 0.1,
        )
        self.position = world_position
        self.velocity = Vec3(random.random()*3,random.random()*3,0)
        0
        self.tail_color = tail_color
        self.particle_color = particle_color
        self.tail = Entity(model = Mesh(vertices=[Vec3(0), Vec3(0)], mode = "line", thickness = 1), color = self.tail_color)
        self.tail_list = []
        
        self.initial = True
        self.step = 0
        #self.phase = [0,0]
        self.phase = [math.pi*random.random(), math.pi*random.random(), random.uniform(0.4,0.9), random.uniform(0.4,0.9)]


    def update(self):
        # stop particles that have low speeds 
        if len(self.tail_list)>20 and magnitude(self.velocity)<0.1:
            return

        # remove dead particles
        if self.x > 5.5 or self.x < -0.5 or self.z > 5.5 or self.z < -0.5 or self.y < 0:
            scene.entities.remove(self)

        # apply vector fields and decay
        v_line = False
        closest_v_distance = 20
        closest_v_direction = Vec3(0,0,0)

        for l in vel_field_lines:
            if magnitude(l.center-self.position)<0.25:
                v_line = True
                closest_v_field = l
                closest_v_distance = min(closest_v_distance, magnitude(l.center-self.position))
                closest_v_direction = l.direction
        
        if v_line == True:
            v_attractor = closest_v_field.center - self.position
            self.velocity = Vec3(tuple(v_preserve_factor*self.velocity[i] + v_attract_factor*v_attractor[i] +
                                       v_field_factor*math.exp(-magnitude(closest_v_direction))*closest_v_direction[i]/magnitude(closest_v_direction)
                                      for i in range(3)))
        else:
            for l in acc_field_lines:
                n = 0
                distance = magnitude(l.center-self.position)
                if distance < 0.25:
                    n += 1
                    if n == 2:
                        break
                    self.velocity = Vec3(tuple(self.velocity[i] + a_field_factor * l.direction[i]/(magnitude(l.direction))
                                          for i in range(3)))
          
        for field in vector_fields:
            self.velocity += field(self.position, self.phase)

        self.velocity = Vec3(tuple(self.velocity[i]*decay for i in range(3)))

        # draw tail
        old_position = self.world_position
        self.position += Vec3((TIME_SCALE*self.velocity[0], TIME_SCALE*self.velocity[1], TIME_SCALE*self.velocity[2]))
        self.tail_list.append(self.position)
        self.step += 1
        if self.step % a_line_frequency != 0:
            if self.initial == True:
                self.initial = False
            else:
                self.tail.model.vertices = self.tail_list
                self.tail.model.generate()
        
#-----particles-----

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

    if held_keys["a"]:
        camera.fov = camera.fov + 100*time.dt

    if held_keys["d"]:
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
        else:
            direction = Vec3(tuple(mouse_position[i] - mouse_old_position[i] for i in range(3)))
            print("held!!!")
            l = DrawAccField(mouse_old_position, direction)
            print("magnitude is: " + str(magnitude(direction)))
            mouse_old_position = mouse_position
            if magnitude(direction) > 0.01:
                acc_field_lines.append(l)

    if held_keys["x"]:
        mouse_position = mouse.world_point
        if drawing_line == False:
            mouse_old_position = mouse_position
            drawing_line = True
        else:
            direction = Vec3(tuple(mouse_position[i] - mouse_old_position[i] for i in range(3)))
            l = DrawVelField(mouse_old_position, direction)
            print("magnitude is: " + str(magnitude(direction)))
            mouse_old_position = mouse_position
            if magnitude(direction) > 0.01:
                vel_field_lines.append(l)

    if key == "c":
        drawing_line = False


 #   if held_keys["x"]:
 #       l = DrawAccField(mouse.world_point, (1,2,1))

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
