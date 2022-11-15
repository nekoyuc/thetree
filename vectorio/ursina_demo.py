from re import A, I
from ursina import *
from ursina.prefabs.first_person_controller import EditorCamera
import panda3d
import numpy as np
import math
from datetime import datetime
import time

from panda3d.core import CollisionRay
from panda3d.core import Quat,Vec3, Vec4
from panda3d.core import MeshDrawer
from panda3d.core import LineSegs, NodePath
from panda3d.core import PStatClient

debug = False
def dprint(string):
    if debug == True:
        print(string)

app = Ursina()
#PStatClient.connect()
Sky(texture="bg8.jpg")

### CONFIGURATION
### ----------
# define room width, depth and height
ROOM_W = int(6)
ROOM_D = int(6)
ROOM_H = int(6)

CELL_WN = 576
CELL_DN = 576
CELL_HN = 576

CELL_W = ROOM_W/CELL_WN
CELL_D = ROOM_D/CELL_DN
CELL_H = ROOM_H/CELL_HN


def magnitude(vector):
    m = 0
    for i in (0,1,2):
        m += vector[i]**2
    return math.sqrt(m)
def color_rgb(r,g,b,a=255):
    return color.rgba(r,g,b,a=255)

def color_hsv(h,s,v,a=1):
    return color.color(h,s,v,a=1)

def now():
    return datetime.now().time()


#ls = LineSegs()
#ls.setColor(1,0,0,1)
#ls.setThickness(3)
#ls.moveTo(Vec3(1,2,3))
#ls.drawTo(Vec3(2,3,4))
#dprint("move to is " + str(ls.moveTo(Vec3(1,2,3))))
#dprint("vertice number is: " + str(ls.getNumVertices()))
#NodePath(ls.create())

#l = Entity(model = "line")
#l.model.vertices = ((0,1,0),(3,5,3),(5,1,0))
#l.model.generate()

start_point = Draggable(model='circle', color=color.orange, scale=.025, position=(-0,-0))

# set up pause_ function
pause__handler = Entity(ignore_pause_d=True)

def pause__handler_input(key):
    if key == "escape":
        application.pause_d = not application.pause_d

pause__handler.input = pause__handler_input


### STATE
### ----------
#---inventories
ACC_FIELD_LINES = []
VEL_FIELD_LINES = []
ATT_FIELD_LINES = []
PARTICLE_LIST = []
DENSITY_MAP = []
TEMP_LINE = {}
TAIL_COLORS = [color_hsv(174,0.8,0.9), color_hsv(157,0.8,0.9), color_hsv(140,0.7,0.9),color_hsv(123,0.8,0.9), color_hsv(103,0.8,0.9)]
PARTICLE_COLORS = [color_hsv(31,0.9,0.9), color_hsv(51,0.8,0.9),color_hsv(333,0.9,0.9),color_hsv(196,0.9,0.9),color_hsv(246,0.9,0.9)]

#---field density
#FIELD_DENSITY = [[[0 for z in range(CELL_HN)] for y in range(CELL_DN)] for x in range(CELL_WN)]
FIELD_DENSITY = {}
#FIELD_DENSITY_GRID = [[[0 for z in range(CELL_DN)] for y in range(CELL_HN) ]for x in range(CELL_WN)]

#---parameters
#toggles
ortho_ = 0
drawling_line_ = False
plane_on_ = True
pause_ = False
render = True

#initial parameters
draw_tail_frequency_ = 15
mouse_old_position_ = Vec3(0,0,0)
time_scale_ = 1/1500.0
max_distance = 0.25

#vector strength of tails
acc_field_factor_ = 1

v_preserve_factor_ = 0.6
v_attract_factor_ = 5
v_field_factor_ = 300
v_rotation_factor_ = 800

att_field_factor_ = 10

p_random_factor_ = 0.003

density_expansion = 3
density_max = 14
density_threshold = 250

profile_factor = 0.06

#random_x_factor = 0.1
#random_y_factor = 0
#random_z_factor = 0.1
decay_ = 0.98

class FloorBox(Entity):
    def __init__(self, position=(0,0,0)):
        super().__init__(
            parent = scene,
            position = position,
            model = 'cube',
            origin_y = .5,
            texture = 'tx5.jpg',
            color = color_hsv(0, 0, random.uniform(.9, 1.0)),
            highlight_color = color.lime,
        )

for z in range(ROOM_D):
    for x in range(ROOM_W):
        voxel = FloorBox(position=(
            x+0.5,0,z+0.5))
        
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
#-----field density-----
#def density_stamp_grid(position):
#    x = int(position[0]/CELL_W)
#    y = int(position[1]/CELL_H)
#    z = int(position[2]/CELL_D)
#    for xi in (x-2, x-1, x, x+1, x+2):
#        if xi < CELL_WN and xi >= 0:
#            for yi in (y-2, y-1, y, y+1, y+2):
#                if yi < CELL_HN and yi >= 0:
#                    for zi in (z-2, z-1, z, z+1, z+2):
#                        if zi < CELL_DN and zi >= 0:
#                            stamp = max(0, 3 - abs(x - xi) - abs(y - yi) - abs(z - zi))
#                            if stamp != 0:
#                                FIELD_DENSITY_GRID[xi][yi][zi] += stamp

# every particle makes a stamp at each move
def density_decoder(string):
    xs = False
    ys = False
    zs = False
    x = ""
    y = ""
    z = ""
    for letter in string:
        if zs == True:
            z += letter
        if letter == "z":
            zs = True
        if ys == True and zs == False:
            y += letter
        if letter == "y": 
            ys = True
        if xs == True and ys == False:
            x += letter
        if letter == "x":
            xs = True
    cell_index = (int(x), int(y), int(z))
    return cell_index

def density_encoder(vector):
    key = "x"+str(vector[0])+"y"+str(vector[1])+"z"+str(vector[2])
    return key

def density_stamp(position):
    x = int(position[0]/CELL_W)
    y = int(position[1]/CELL_H)
    z = int(position[2]/CELL_D)
    for xi in range(x-density_expansion, x+density_expansion):
        if xi < CELL_WN and xi >= 0:
            for yi in range(y - density_expansion, y + density_expansion):
                if yi < CELL_HN and yi >= 0:
                    for zi in range(z - density_expansion, z + density_expansion):
                        if zi < CELL_DN and zi >= 0:
                            key = density_encoder((xi, yi, zi))
                            stamp_raw = density_max - int((abs(x-xi)*1.2)**1.5+(abs(y-yi)*1.2)**1.5+(abs(z-zi)*1.2)**1.5)
                            stamp = max(0, stamp_raw)
                            #stamp = max(0, 3 - abs(x - xi) - abs(y - yi) - abs(z - zi))
                            if stamp != 0:
                                if key in FIELD_DENSITY.keys():
                                    FIELD_DENSITY[key] += stamp
                                else:
                                    FIELD_DENSITY[key] = stamp
    for key in FIELD_DENSITY.keys():
        if FIELD_DENSITY[key] >= density_threshold:
            DENSITY_MAP.append(density_decoder(key))

# one-time button produces density visualization
def density_vis(string, intensity):
    position = (CELL_W*density_decoder(string)[0], CELL_H*density_decoder(string)[1], CELL_D*density_decoder(string)[2])
    if intensity < density_threshold:
        Entity(model = "sphere", position = position, scale = (intensity/10000000000)**0.3,
           color = color.color(280, 0.6, 3**(-intensity/700)))
    else:
        Entity(model = "sphere", position = position, scale = (intensity/10000000000)**0.3,
           color = color.color(200, 0.9, 3**(-intensity/700)))
    #Mesh(vertices=[position], mode="point", colors = color.color(200, 0.9, 1 - intensity/40))

# one-time button produces density profiler
def density_profiler():
    xmax = 0
    xmin = 576
    ymax = 0
    ymin = 576
    zmax = 0
    zmin = 576
    DENSITY_PROFILE = []
    print("yes")
    for i in DENSITY_MAP:
        xmax = max(xmax, i[0])
        xmin = min(xmin, i[0])
        ymax = max(ymax, i[1])
        ymin = min(ymin, i[1])
        zmax = max(zmax, i[2])
        zmin = min(zmin, i[2])

    # screen along x direction
    for yi in range(ymin, ymax+1, 5):
        for zi in range(zmin, zmax+1):
            start = False
            value = 0
            for xi in range(xmin, xmax+1):
                if start == False and (xi, yi, zi) in DENSITY_MAP:
                    start = True
                    #value += FIELD_DENSITY["x"+str(xi)+"y"+str(yi)+"z"+str(zi)]
                    DENSITY_PROFILE.append((xi, yi, zi))
                    continue

                if start == True and (xi, yi, zi) in DENSITY_MAP:
                    #value += FIELD_DENSITY["x"+str(xi)+"y"+str(yi)+"z"+str(zi)]
                    continue

                if start == True and (xi, yi, zi) not in DENSITY_MAP:
                    start = False
                    #mid_value = value/2
                    #pass_mid = False
                    #x = xi
                    DENSITY_PROFILE.append((xi-1, yi, zi))
                    #while pass_mid == False:
                    #    x -= 1
                    #    mid_value -= FIELD_DENSITY["x" + str(x) + "y" + str(yi) + "z" + str(zi)]
                    #    if mid_value < 0:
                    #        pass_mid = True
                    #        DENSITY_PROFILE.append(((CELL_W*(x-math.atan(value)), CELL_H*yi, CELL_D*zi),
                    #                                 (CELL_W*(x+math.atan(value)), CELL_H*yi, CELL_D*zi)))
                    #value = 0
                    continue

    # screen along z direction
    for yi in range(ymin, ymax+1, 5):
        for xi in range(xmin, xmax+1):
            start = False
            value = 0
            for zi in range(zmin, zmax+1):
                if start == False and (xi, yi, zi) in DENSITY_MAP:
                    start = True
                    #value += FIELD_DENSITY["x"+str(xi)+"y"+str(yi)+"z"+str(zi)]
                    DENSITY_PROFILE.append((xi, yi, zi))
                    continue

                if start == True and (xi, yi, zi) in DENSITY_MAP:
                    #value += FIELD_DENSITY["x"+str(xi)+"y"+str(yi)+"z"+str(zi)]
                    continue

                if start == True and (xi, yi, zi) not in DENSITY_MAP:
                    start = False
                    #mid_value = value/2
                    #pass_mid = False
                    #z = zi
                    DENSITY_PROFILE.append((xi, yi, zi-1))
                    #while pass_mid == False:
                    #    z -= 1
                    #    mid_value -= FIELD_DENSITY["x" + str(xi) + "y" + str(yi) + "z" + str(z)]
                    #    if mid_value < 0:
                    #        pass_mid = True
                    #        DENSITY_PROFILE.append(((CELL_W*xi, CELL_H*yi, CELL_D*(z-math.atan(value))),
                    #                                 (CELL_W*xi, CELL_H*yi, CELL_D*(z+math.atan(value)))))
                    #
                    #value = 0
                    continue

    for i in DENSITY_PROFILE:
        center = (CELL_W*i[0], CELL_H*i[1], CELL_D*i[2])
        Entity(model = "cube", position = center, scale = 0.001)
        #Entity(model = "cube", position = i[0], scale = 0.001)
        #Entity(model = "cube", position = i[1], scale = 0.001)

#-----field density-----


#-----acceleration vector fields-----
class DrawAccField(Entity):
    def __init__(self, center, direction):
        self.direction = Vec3(direction)
        self.center = Vec3(center)
        #self.y_vec = Vec3(0,direction[1],0)

        super().__init__(model = Mesh(vertices = [self.center, self.center + self.direction], mode = "line", thickness = 2),
            color = color_hsv(280,1,0.8)
        )
    
def gravity_field(point):
    return Vec3(0,-1,0)

#def RandomField(point):
#    return Vec3(random_x_factor*random.random(), random_y_factor*random.random(), random_z_factor*random.random())

vector_fields = [gravity_field]
#-----acceleration vector fields-----


#-----velocity vector fields-----
class DrawVelField(Entity):
    def __init__(self,center,direction):
        self.direction = direction
        self.center = Vec3(center)

        super().__init__(model = Mesh(vertices = [self.center, self.center + self.direction], mode = "line", thickness = 1),
            color = color_hsv(320,1,0.8) 
        )

def rotation_field(p_position, l_position, direction):
    p_to_l = l_position - p_position
    dot_pl_d = p_to_l[0]*direction[0] + p_to_l[1]*direction[1] + p_to_l[2]*direction[2]
    dot_d_d = direction[0]*direction[0] + direction[1]*direction[1] + direction[2]*direction[2]

    a = dot_pl_d/dot_d_d
    projection0 = direction[0]*a
    projection1 = direction[1]*a
    projection2 = direction[2]*a

    rotation = ((p_to_l[1]-projection1)*direction[2]-(p_to_l[2]-projection2)*direction[1],
                     (p_to_l[2]-projection2)*direction[0]-(p_to_l[0]-projection0)*direction[2],
                     (p_to_l[0]-projection0)*direction[1]-(p_to_l[1]-projection1)*direction[0])

    return rotation # rotation is a tuple not Vec3

def _rotation_field(p_position, l_position, direction):
    p_to_l = l_position - p_position
    projection = np.dot(p_to_l, direction) * direction / np.dot(direction, direction)
    rotation = np.cross(p_to_l - projection, direction)
    return rotation
#-----velocity vector fields-----


#-----attractor fields-----
class DrawAttField(Entity):
    def __init__(self, center, direction):
        self.direction = Vec3(direction)
        self.center = Vec3(center)
        #self.y_vec = Vec3(0,direction[1],0)

        super().__init__(model = Mesh(vertices = [self.center, self.center + self.direction], mode = "line", thickness = 2),
            color = color_hsv(220,1,0.8)
        )
#-----attractor fields-----

#-----particles-----
class BaseParticle(Entity):
    def __init__(self):
        super().__init__(
            parent = None,
            model = 'sphere',
            scale = 0.02
        )

god_particle = BaseParticle()

#god_sphere = 
#
#class DensitySphere(Entity):
    #def __init__(self):
      #  super().__init__(parent = scene)
      #  global god_sphere
      #  god_sphere.instanceTo(self)

class Particle(Entity):
    def __init__(self, particle_color, tail_color, world_position=(0,0,0)):
        super().__init__(
            parent = scene,
            world_position = world_position,
            #model = 'sphere',
            origin_y = .5,
           # texture = 'white_cube',
            color = color_hsv(random.random()*360, 1, random.uniform(0.9,1)),
           # highlight_color = color.lime,
            #scale=0.02
          #  scale = 0.1,
        )
        self.position = world_position
        self.velocity = (random.random()*3,random.random()*3,0) # a tuple, not Vec3
        self.tail_color = tail_color
        self.particle_color = particle_color
        self.tail_list = []
        global god_particle
        god_particle.instanceTo(self)
        
        self.step = 0
        self.within_range = True

    def do_update(self):
        # stop particles that have low speeds 
        if len(self.tail_list)>20 and (abs(self.velocity[0])+abs(self.velocity[1])+abs(self.velocity[2]))<0.1:
            return None


        # remove dead particles
        if self.position[0] > 5.5 or self.position[0] < -0.5 or self.position[2] > 5.5 or self.position[2] < -0.5 or self.position[1] < 0:
            destroy(self)
            # TODO remove from particle list
            return None


        # set up particle stop
        if self.within_range == False:
            return None


        ### apply v field
        # find closest v_field to particle
        v_line = False
        closest_vdis = 20

        for l in VEL_FIELD_LINES:
            distance = magnitude(l.center - self.position)
            if distance < max_distance:
               if distance < closest_vdis:
                   closest_vdis = distance
                   v_line = True
                   closest_vf = l

        if v_line == False:
            self.within_range = False
            return None

        v_attractor = closest_vf.center - self.position


        ### apply acc field
        for l in ACC_FIELD_LINES:
            n = 0
            distance = magnitude(l.center-self.position)
            if distance < max_distance:
                n += 1
                if n == 2:
                    break
                self.velocity = tuple(self.velocity[i] + acc_field_factor_*l.direction[i]/(magnitude(l.direction)) for i in (0,1,2))
               # self.velocity = Vec3(tuple(self.velocity[i] + acc_field_factor_ * l.direction[i]/(magnitude(l.direction))
                #                           for i in (0,1,2)))

        rot = rotation_field(self.position, closest_vf.center, closest_vf.direction) #rot is a tuple not Vec3


        ### apply att field
        for l in ATT_FIELD_LINES:
            distance = magnitude(l.center - self.position)
            direction = l.center - self.position
            if distance < max_distance:
                self.velocity = tuple(self.velocity[i] + direction[i] * att_field_factor_ for i in (0,1,2))


        ### apply vector fields and decay_ to velocity
        self.velocity = tuple(v_preserve_factor_*self.velocity[i] # last velocity
                                   + v_attract_factor_*v_attractor[i] # attraction to draw field
                                   + v_rotation_factor_*rot[i] # rotation field v_rotation_factor_
                                   + v_field_factor_*closest_vf.direction[i] # v field intensity
                                   #v_field_factor_*math.exp(-closest_vdis)*closest_vf.direction[i]/magnitude(closest_vf.direction) # v field intensity
                                   + gravity_field(self.position)[i] # gravity
                                      for i in (0,1,2))


        #dprint(rotation_field(self.position, closest_vf.center, closest_vf.direction))
        self.position += Vec3(tuple(time_scale_*self.velocity[i] # time scale
                                    + p_random_factor_ * (random.random() - 0.5) # positional randomness
                                    + 0*rot[i]
                                    for i in (0,1,2)))

        density_stamp(self.position)

        self.velocity = tuple(self.velocity[i]*decay_ for i in (0,1,2))
        self.tail_list.append(self.position)

        if len(self.tail_list) < 2:
            return (self.tail_list[-1],)
        else:
            return (self.tail_list[-1], self.tail_list[-2])

class ParticleUpdater(Entity):
    def __init__(self, PARTICLE_LIST):
        super().__init__(
            parent = scene,
        )
        self.PARTICLE_LIST = PARTICLE_LIST
        self.tails = []
        self.skip_count = 0
        self.running_average_time_ms = 0
    def update(self):
        if len(self.PARTICLE_LIST) == 0:
            return
        vertices = []
        tris = []
        i = 0
        dead_particles = []
        self.skip_count += 1
        colors = []
        start_time_ms = time.time_ns() / 1e6
        dprint("num particle: " + str(len(self.PARTICLE_LIST)))
        for particle in self.PARTICLE_LIST:
            #new line segment should be a tuple of two points ((x1,y1,z1),(x2,y2,z2))
            #if (random.random() <0.8): continue
            #particle_start_time = time.time_ns() / 1e6
            new_line_segment = particle.do_update()
            #particle_end_time = time.time_ns() / 1e6
            #dprint("particle update time: " + str(particle_end_time-particle_start_time))

            if new_line_segment == None:
                dead_particles.append(particle)
                continue
            elif len(new_line_segment) == 1:
                continue
            elif (self.skip_count % 5 != 0):
                continue
            vertices.append(new_line_segment[0]+Vec3(random.random()*0.02,random.random()*0.02,random.random()*0.02))
            vertices.append(new_line_segment[1]+Vec3(random.random()*0.02,random.random()*0.02,random.random()*0.02))
            tris.append((i,i+1))
            colors.append(particle.tail_color)
            colors.append(particle.tail_color)
            i += 2
        for p in dead_particles:
            self.PARTICLE_LIST.remove(p)
        #end_time_ms = time.time_ns() / 1e6
        #dprint(end_time_ms-start_time_ms)
        #if self.running_average_time_ms == 0: self.running_average_time_ms = (end_time_ms-start_time_ms)
        #self.running_average_time_ms = 0.99*self.running_average_time_ms+(0.01*(end_time_ms-start_time_ms))
       # dprint(self.running_average_time_ms)
        if len(vertices) == 0:
            return
       
        #tail = Entity(model = Mesh(vertices=vertices, triangles=tris, mode = "line", thickness = 2), color = color.blue)
        tail = Mesh(vertices=vertices, triangles=tris, mode="line", thickness=1, colors = colors)
        tail.reparentTo(self)
        #self.attach_new_node(tail)
        self.tails.append(tail)
        #if self.skip_count % 60 == 0:
         #   self.combine()

particle_updater = ParticleUpdater(PARTICLE_LIST)

class ParticleSeeds(Entity):
    def __init__(self, position = (0,0,0), radius = 0.3, seed_n = 6, rotation_angle = 0):
        super().__init__(
            parent = scene,
            position = position
        )
        self.position = (2.5,0,2.5)
        self.radius = radius
        self.look_at(camera,axis="up")
        self.seed_n = seed_n
        self.rotation_angle = rotation_angle
        self.circle = models.procedural.circle.Circle(radius = self.radius, resolution = self.seed_n, thickness = 2, mode = 'ngon')
        self.circle.reparentTo(self)
        
    def pump(self):
        angle = 360/self.seed_n
        spots = []
        for i in range(self.seed_n):
            point = Vec3(self.position[0] + self.radius*sin(math.radians(i*angle + self.rotation_angle)),
                         0.01,
                         self.position[2] + self.radius*cos(math.radians(i*angle + self.rotation_angle)))
            spots.append(point)
        return spots
    def update(self):
        self.circle.removeNode()
        self.circle = models.procedural.circle.Circle(radius = self.radius, resolution = self.seed_n, thickness = 2, mode = 'ngon')
        self.circle.reparentTo(self)

circle = ParticleSeeds((0,0,0), 0.2, 6)
dprint(dir(circle.circle))
#-----particles-----


class Plane(Entity):
    def __init__(self, plane_on_, position=(0,0,0)):
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

p = Plane(plane_on_)


class Controller(Entity):
  def __init__(self):
      super().__init__(parent=scene)
  def input(self,key):
    dprint("Got input yay")
    global PARTICLE_LIST

    #switch between ortho and pers
    if key == "tab":
        global ortho_
        ortho_ = 1 - ortho_
        camera.orthographic = ortho_

    if key == "6":
        for key in FIELD_DENSITY.keys():
            density_vis(key, FIELD_DENSITY[key])

    if key == "7":
        density_profiler()
    
    #change fov
    if held_keys["a"]:
        camera.fov = camera.fov + 100*time.dt

    if held_keys["d"]:
        camera.fov = camera.fov - 100*time.dt

    #change plane distance
    global p
    if held_keys["w"]:
        p.distance +=0.03

    if held_keys["s"]:
        p.distance = max(0, p.distance-0.03)

    #change particle seeds
    global circle
    if held_keys["up arrow"]:
        circle.position = (circle.position[0], circle.position[1], min(circle.position[2]+0.02, 5.5))
    if held_keys["down arrow"]:
        circle.position = (circle.position[0], circle.position[1], max(circle.position[2]-0.02, -0.5))
    if held_keys["left arrow"]:
        circle.position = (max(circle.position[0]-0.02, -0.5), circle.position[1], circle.position[2])
    if held_keys["right arrow"]:
        circle.position = (min(circle.position[0]+0.02, 5.5), circle.position[1], circle.position[2])
    if held_keys["-"]:
        circle.rotation_angle -= 1
    if held_keys["="]:
        circle.rotation_angle += 1
    if held_keys["["]:
        circle.radius = max(0.05, circle.radius - 0.01)
    if held_keys["]"]:
        circle.radius += 0.01
    if held_keys["9"]:
        circle.seed_n = max(3, circle.seed_n -1)
    if held_keys["0"]:
        circle.seed_n += 1
    #pump particles
    if held_keys["1"]:
        new_particle = Particle(random.choice(PARTICLE_COLORS),random.choice(TAIL_COLORS),
                 Vec3(0.01*random.random() + mouse.world_point[0], mouse.world_point[1], 0.01*random.random() + mouse.world_point[2]))
        PARTICLE_LIST.append(new_particle)

    if held_keys["2"]:
        for i in range(circle.seed_n):
            new_p = Particle(random.choice(PARTICLE_COLORS), random.choice(TAIL_COLORS), circle.pump()[i])
            PARTICLE_LIST.append(new_p)

    global mouse_old_position_
    global drawling_line_

    #draw field lines
    if held_keys["z"]:
        mouse_position = mouse.world_point
        if drawling_line_ == False:
            mouse_old_position_ = mouse_position
            drawling_line_ = True
        else:
            direction = Vec3(tuple(mouse_position[i] - mouse_old_position_[i] + (random.random()-0.5)*0.2 for i in (0,1,2)))
            l = DrawAccField(mouse_old_position_, direction)
            mouse_old_position_ = mouse_position
            if magnitude(direction) > 0.01:
                ACC_FIELD_LINES.append(l)

    if held_keys["x"]:
        mouse_position = mouse.world_point
        if drawling_line_ == False:
            mouse_old_position_ = mouse_position
            drawling_line_ = True
        else:
            direction = Vec3(tuple(mouse_position[i] - mouse_old_position_[i] for i in (0,1,2)))
            l = DrawVelField(mouse_old_position_, direction)
            mouse_old_position_ = mouse_position
            if magnitude(direction) > 0.01:
                VEL_FIELD_LINES.append(l)

    if held_keys["c"]:
        mouse_position = mouse.world_point
        if drawling_line_ == False:
            mouse_old_position_ = mouse_position
            drawling_line_ = True
        else:
            direction = Vec3(tuple(mouse_position[i] - mouse_old_position_[i] for i in (0,1,2)))
            l = DrawAttField(mouse_old_position_, direction)
            mouse_old_position_ = mouse_position
            if magnitude(direction) > 0.01:
                ATT_FIELD_LINES.append(l)

    #if key == "c":
    if held_keys["z"] == 0 and held_keys["x"] == 0 and held_keys["c"] == 0:
        drawling_line_ = False

    if key == 'left mouse down':
        
        return
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

        dprint("hit info: " + str(hit_info.normal) + ", type is " + str(type(hit_info.normal)) + "\n\n--------------------\n\n")

player = EditorCamera()
c = Controller()
app.run()
