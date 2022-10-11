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

test = Vec3(1,2,3)
print("Vec3 is:" + str(test))
testnp = np.array(test)
print("np array is: " + str(testnp))
testback = Vec3(tuple(testnp[i] for i in range(3)))
print("Vec3 back is: " + str(testback))

print("\n\n\n")
l = Entity(model = "line")
l.model.vertices = ((1,2,3),(2,3,4),(3,4,5))
l.model.generate()

start_point = Draggable(model='circle', color=color.orange, scale=.025, position=(-0,-0))

# set up pause function
pause_handler = Entity(ignore_paused=True)

def pause_handler_input(key):
    if key == "escape":
        application.paused = not application.paused

pause_handler.input = pause_handler_input


### STATE
### ----------
#---inventories
acc_field_lines = []
vel_field_lines = []
tail_colors = [color_hsv(174,0.8,0.9), color_hsv(157,0.8,0.9), color_hsv(140,0.7,0.9),color_hsv(123,0.8,0.9), color_hsv(103,0.8,0.9)]
particle_colors = [color_hsv(31,0.9,0.9), color_hsv(51,0.8,0.9),color_hsv(333,0.9,0.9),color_hsv(196,0.9,0.9),color_hsv(246,0.9,0.9)]

#---parameters
#toggles
ortho = 0
drawing_line = False
plane_on = True
pause = False

#initial parameters
draw_tail_frequency = 15
mouse_old_position = Vec3(0,0,0)
TIME_SCALE = 1/1500.0

#vector strength of tails
a_field_factor = 2
v_preserve_factor = 0.6
v_attract_factor = 12
v_field_factor = 22
v_rotation_factor = 600
random_x_factor = 0.2
random_y_factor = 0
random_z_factor = 0.2
decay = 0.99

#particle seeds
radius = 0.2
seed_n = 6
rotation_angle = 0

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

        super().__init__(model = Mesh(vertices = [self.center, self.center + self.direction], mode = "line", thickness = 2),
            color = color_hsv(280,1,0.8)
        )
    
def GravityField(point):
    return Vec3(0,-1,0)

def RandomField(point):
    return Vec3(random_x_factor*random.random(), random_y_factor*random.random(), random_z_factor*random.random())

vector_fields = [GravityField]
#-----acceleration vector fields-----


#-----velocity vector fields-----
class DrawVelField(Entity):
    def __init__(self,center,direction):
        self.direction = Vec3(direction)
        self.center = Vec3(center)

        super().__init__(model = Mesh(vertices = [self.center, self.center + self.direction], mode = "line", thickness = 3),
            color = color_hsv(320,1,0.8)
        )

def RotationField(p_position, l_position, direction):
    p_to_l = l_position - p_position
    projection = np.dot(p_to_l, direction) * direction / np.dot(direction, direction)
    rotation = np.cross(p_to_l - projection, direction)
    return rotation
#-----velocity vector fields-----


#-----particles-----
class ParticleUpdater(Entity):
    def __init__(self, particle_list):
        super().__init__(
            parent = scene,
        )
        self.particle_list = particle_list
        self.tails = []
        self.skip_count = 0
    def update(self):
        if len(self.particle_list) == 0:
            return
        vertices = []
        tris = []
        i = 0
        dead_particles = []
        self.skip_count += 1
        colors = []
        for particle in self.particle_list:
            #new line segment should be a tuple of two points ((x1,y1,z1),(x2,y2,z2))
            #if (random.random() <0.8): continue
            new_line_segment = particle.do_update()

            if new_line_segment == None:
                dead_particles.append(particle)
                continue
            if len(new_line_segment) == 1:
                continue
            if (self.skip_count % 5 != 0):
                continue
            vertices.append(new_line_segment[0])
            vertices.append(new_line_segment[1])
            tris.append((i,i+1))
            colors.append(particle.tail_color)
            colors.append(particle.tail_color)
            i += 2
        for p in dead_particles:
            self.particle_list.remove(p)
        if len(vertices) == 0:
            return
        #tail = Entity(model = Mesh(vertices=vertices, triangles=tris, mode = "line", thickness = 2), color = color.blue)
        tail = Mesh(vertices=vertices, triangles=tris, mode="line", thickness=1, colors = colors)
        tail.reparentTo(self)
        #self.attach_new_node(tail)
        self.tails.append(tail)
        #if self.skip_count % 60 == 0:
        #    self.combine()

particle_list = []
particle_updater = ParticleUpdater(particle_list)

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
            scale=0.02
          #  scale = 0.1,
        )
        self.position = world_position
        self.velocity = Vec3(random.random()*3,random.random()*3,0)
        self.tail_color = tail_color
        self.particle_color = particle_color
        self.tail_list = []
        
        self.step = 0
        self.within_range = True

    def do_update(self):
        # stop particles that have low speeds 
        if len(self.tail_list)>20 and magnitude(self.velocity)<0.1:
            return None

        # remove dead particles
        if self.x > 5.5 or self.x < -0.5 or self.z > 5.5 or self.z < -0.5 or self.y < 0:
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
        closest_vdir = Vec3(0,0,0)

        for l in vel_field_lines:
            if magnitude(l.center-self.position)<0.25:
                v_line = True
                closest_vf = l
                closest_vdis = min(closest_vdis, magnitude(l.center-self.position))
                closest_vdir = l.direction
        
        if v_line == False:
            self.within_range = False
            return None

        v_attractor = closest_vf.center - self.position

        ### apply a field
        for l in acc_field_lines:
            n = 0
            distance = magnitude(l.center-self.position)
            if distance < 0.25:
                n += 1
                if n == 2:
                    break
                self.velocity = Vec3(tuple(self.velocity[i] + a_field_factor * l.direction[i]/(magnitude(l.direction))
                                           for i in range(3)))


        ## apply vector fields and decay to velocity
        self.velocity = Vec3(tuple(v_preserve_factor*self.velocity[i] # last velocity
                                   + v_attract_factor*v_attractor[i] # attraction to draw field
                                   + v_rotation_factor*RotationField(self.position, closest_vf.center, closest_vdir)[i] # rotation field
                                   + v_field_factor*math.exp(-closest_vdis)*closest_vdir[i]/magnitude(closest_vdir) # v field intensity
                                      for i in range(3)))
        
        for field in vector_fields:
            self.velocity += field(self.position)

        #print(RotationField(self.position, closest_vf.center, closest_vdir))
        self.position += Vec3(tuple(TIME_SCALE*self.velocity[i] + 0.01*(random.random() - 0.5) + 0*RotationField(self.position, closest_vf.center, closest_vdir)[i]
                                    for i in range(3)))

        self.velocity = Vec3(tuple(self.velocity[i]*decay for i in range(3)))

        # draw tail
        self.tail_list.append(self.position)

        if len(self.tail_list) < 2:
            return (self.tail_list[-1],)
        else:
            return (self.tail_list[-1], self.tail_list[-2])

class ParticleSeeds(Entity):
    def __init__(self, position = (0,0,0), radius = 0.3, seed_n = 6, rotation_angle = 0):
        #self.position = Vec3(position)
        super().__init__(
            parent = scene,
            position = position
        )
        self.position = (2.5,0,2.5)
        self.radius = radius
        self.look_at(camera,axis="up")
        self.seed_n = seed_n
        self.rotation_angle = rotation_angle
        c = models.procedural.circle.Circle(radius = radius, resolution = 32, thickness = 2)
        c.reparentTo(self)
    def pump(self):
        angle = 360/self.seed_n
        spots = []
        for i in range(self.seed_n):
            point = Vec3(self.position[0] + self.radius*sin(math.radians(i*angle + self.rotation_angle)),
                         0.1,
                         self.position[2] + self.radius*cos(math.radians(i*angle + self.rotation_angle)))
            spots.append(point)
        return spots

circle = ParticleSeeds(position =(0,0,0), radius = radius, seed_n = seed_n)
#-----particles-----


class Plane(Entity):
    def __init__(self, plane_on, position=(0,0,0)):
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

p = Plane(plane_on)

def input(key):
    global particle_list

    #switch between ortho and pers
    if key == "tab":
        global ortho
        ortho = 1 - ortho
        camera.orthographic = ortho
    
    #change fov
    if held_keys["a"]:
        camera.fov = camera.fov + 100*time.dt

    if held_keys["d"]:
        camera.fov = camera.fov - 100*time.dt

    #change plane distance
    global p
    if held_keys["w"]:
        p.distance +=0.05

    if held_keys["s"]:
        p.distance = max(0, p.distance-0.05)

    #change particle seeds
    global circle
    if held_keys["up arrow"]:
        circle.position = Vec3(circle.position[0], circle.position[1], min(circle.position[2]+0.05, 5.5))
    if held_keys["down arrow"]:
        circle.position = Vec3(circle.position[0], circle.position[1], max(circle.position[2]-0.05, -0.5))
    if held_keys["left arrow"]:
        circle.position = Vec3(max(circle.position[0]-0.05, -0.5), circle.position[1], circle.position[2])
    if held_keys["right arrow"]:
        circle.position = Vec3(min(circle.position[0]+0.05, 5.5), circle.position[1], circle.position[2])
    if held_keys["-"]:
        circle.rotation_angle -= 1
    if held_keys["="]:
        circle.rotation_angle += 1
    #pump particles
    if held_keys["1"]:
        new_particle = Particle(random.choice(particle_colors),random.choice(tail_colors),
                 Vec3(0.01*random.random() + mouse.world_point[0], mouse.world_point[1], 0.01*random.random() + mouse.world_point[2]))
        particle_list.append(new_particle)

    if held_keys["2"]:
        for i in range(6):
            new_p = Particle(random.choice(particle_colors), random.choice(tail_colors), circle.pump()[i])
            particle_list.append(new_p)

    global mouse_old_position
    global drawing_line

    #draw field lines
    if held_keys["z"]:
        mouse_position = mouse.world_point
        if drawing_line == False:
            mouse_old_position = mouse_position
            drawing_line = True
        else:
            direction = Vec3(tuple(mouse_position[i] - mouse_old_position[i] for i in range(3)))
            l = DrawAccField(mouse_old_position, direction)
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
            mouse_old_position = mouse_position
            if magnitude(direction) > 0.01:
                vel_field_lines.append(l)

    #if key == "c":
    if held_keys["z"] == 0 and held_keys["x"] == 0:
        drawing_line = False

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

        print("hit info: " + str(hit_info.normal) + ", type is " + str(type(hit_info.normal)) + "\n\n--------------------\n\n")




player = EditorCamera()
app.run()
