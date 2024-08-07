
# Simple pygame program

# Import and initialize the pygame library
import pygame
import math
import sys
import random
import numpy as np
from PIL import Image
from pygame.constants import MOUSEWHEEL
from pygame.display import update

pygame.init()

### CONFIGURATION
### ----------

SCREEN_WIDTH = 1000
SCREEN_HEIGHT = 1000

GRID_X = 50
GRID_Y = 50

CELL_WIDTH = SCREEN_WIDTH/GRID_X
CELL_HEIGHT = SCREEN_HEIGHT/GRID_Y
print("cell width is " + str(CELL_WIDTH))
print("cell height is " + str(CELL_HEIGHT))

# Set up the drawing window
screen = pygame.display.set_mode([SCREEN_WIDTH, SCREEN_HEIGHT])
pygame.display.set_caption("Yuchencraft")

# Run until the user asks to quit
running = True





### STATE
### ----------

#---colors---
down_color = (99,247,143)
up_color = (99,143,247)
selected_color = (247,95,30)
particle_color = ((219,142,61),(219,217,94),(219,83,101) ,(88,186,219),(138,72,219))
line_color = ((105,143,63),(156,219,86),(34,143,111),(64,219,174))

#---
UP = 1
DOWN = 0

#---inventories
FieldGrid = []
Particles = []
pause = 1
DeltaVelocity = {}

#---parameters
currently_selected = [1,1]
start_velocity = (0,0)
start_decay = 0.98
start_acceleration = 1
history_length = 500
start_field_value = (0,0.001)
arrow_scale = 2400
arrow_darkness = 10000
stickiness_range = 40
stickiness_factor = 0.0001
perturb_coefficient = 30

#---toggles
field_toggle = True
stickiness_toggle = True
randomness_toggle = False
image_toggle = False

#image = Image.open(sys.argv[1]).resize((SCREEN_WIDTH, SCREEN_HEIGHT))
image = Image.open("test_image.jpg").resize((SCREEN_WIDTH, SCREEN_HEIGHT))
image = np.array(image)

### LOGIC
### ----------

class Particle():
    def __init__(self, center):
        self.center = center
        self.history = []
        self.velocity = start_velocity
        self.decay = start_decay
        self.particle_color = particle_color
        self.line_color = random.choice(line_color)

    def draw(self, screen):
        for i in range(len(self.history)-1):
            pygame.draw.line(screen, self.line_color, self.history[i], self.history[i+1], 2)
        pygame.draw.circle(screen,random.choice(particle_color),self.center, 4)

    def accelerate(self, acceleration = start_acceleration):
        self.velocity = (self.velocity[0]*acceleration, self.velocity[1]*acceleration)
        #self.perturb()

    def perturb(self, coefficient=1):
        self.center = (self.center[0] + coefficient*self.velocity[0], self.center[1] + coefficient*self.velocity[1])

    def update_history(self):
        self.history.append(self.center)

    def is_moving(self):
        if len(self.history) > history_length or (abs(self.velocity[0])<0.001 and self.velocity[1]>0.07):
            return False
        return True

    def is_alive(self):
        if self.center[0] >= SCREEN_WIDTH or self.center[1] >= SCREEN_HEIGHT:
            return False
        if self.center[0] < 0 or self.center[1] < 0:
            return False
        return True


max_strength = 0
class FieldVizCircles():
    def __init__(self, center, radius):
        self.center = center
        self.radius = radius
        self.state = DOWN
        self.a_value = start_field_value

    def get_color(self, is_selected):
        if is_selected:
            return selected_color
        if self.state == DOWN:
            return down_color
        else:
            return up_color

    def flip(self):
        if self.state == DOWN:
            self.state = UP
            self.a_value = (self.a_value[0], self.a_value[1]-0.005)
        else:
            self.state = DOWN
            self.a_value = (self.a_value[0], self.a_value[1]+0.005)
    
    def draw_arrow(self):
        start_point = self.center
        value_strength = strength(self.a_value)
        end_point = (self.center[0] + self.a_value[0]*arrow_scale, self.center[1] + self.a_value[1]*arrow_scale)

        global max_strength
        if value_strength > max_strength:
            max_strength = value_strength
            print(value_strength)
        pygame.draw.line(screen, (155 - min(155, math.ceil(0.608*value_strength * arrow_darkness)), 0, 255 - min(255, value_strength * arrow_darkness)),
                          start_point, end_point, math.ceil(value_strength*100))

    def draw(self, screen, is_selected):
        if is_selected:
            radius = self.radius + 9
        elif self.state == UP:
            radius = self.radius + 5
        else:
            radius = self.radius
        pygame.draw.rect(screen, self.get_color(is_selected), (self.center[0], self.center[1], radius, radius), 0)

def strength(vector):
   return math.sqrt(vector[0]**2 + vector[1]**2)

#create field FieldGrid
for gy in range(GRID_Y):
    cy = CELL_WIDTH*(1/2 + gy)
    row = []
    for gx in range(GRID_X):
        cx = CELL_HEIGHT*(1/2 + gx)
        center = (cx, cy)
        radius = 3
        row.append(FieldVizCircles(center, radius))
    FieldGrid.append(row)


def handle_event(event):
    if event.type == pygame.QUIT:
        running = False
#        events = pygame.event.get()

#keys up down left right space esc
    if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_LEFT:
            currently_selected[0] = max(0, currently_selected[0]-1)
        if event.key == pygame.K_RIGHT:
            currently_selected[0] = min(currently_selected[0]+1, GRID_X)
        if event.key == pygame.K_DOWN:
            currently_selected[1] = min(GRID_Y, currently_selected[1]+1)
        if event.key == pygame.K_UP:
            currently_selected[1] = max(0, currently_selected[1]-1)
        if event.key == pygame.K_RETURN:
            FieldGrid[currently_selected[1]][currently_selected[0]].flip()
        if event.key == pygame.K_SPACE:
            global pause
            pause = 1 - pause
        # Done! Time to quit.
        if event.key == pygame.K_ESCAPE:
            pygame.quit()

#mouse click right button
 #   if event.type == pygame.MOUSEBUTTONUP and event.button == 3:
 #       print(event.button)
 #       pos = pygame.mouse.get_pos()
 #       for i in range(5):
 #           r = random.random()-0.5
 #           particle_pos = (pos[0]+r,pos[1]+r)
 #           Particles.append(Particle(particle_pos))

 #mouse drag left button
    if event.type == pygame.MOUSEMOTION and event.buttons[0] == 1:
        pos = event.pos
        cx = math.floor(pos[0]/CELL_WIDTH)
        cy = math.floor(pos[1]/CELL_HEIGHT)
        if cx >= 0 and cy >= 0 and cx < GRID_X and cy < GRID_Y:
            FieldGrid[cy][cx].a_value = (FieldGrid[cy][cx].a_value[0] + event.rel[0]/10000, FieldGrid[cy][cx].a_value[1] + event.rel[1]/10000)

 #mouse drag middle button
    if event.type == pygame.MOUSEMOTION and event.buttons[1] == 1:
        pos = event.pos
        for p in Particles:
            if (pos[0] - p.center[0])**2 < 400 and (pos[1] - p.center[1])**2 < 400:
                Particles.remove(p)

#mouse drag right button
    if event.type == pygame.MOUSEMOTION and event.buttons[2] == 1:
        pos = pygame.mouse.get_pos()
        for i in range(3):
            r = random.random()-0.5
            s = random.random()-0.5
            t = 0.05*(random.random()-0.5)
            particle_pos = (pos[0]+r,pos[1]+s)
            Particles.append(Particle(particle_pos))
            Particles[-1].velocity = (t,0)

#mouse scroll up
    if event.type == pygame.MOUSEBUTTONDOWN and event.button == 4:
        pos = event.pos
        for p in Particles:
            if (pos[0] - p.center[0])**2 < CELL_WIDTH**2 and (pos[1] - p.center[1])**2 < CELL_HEIGHT**2:
                p.velocity = (p.velocity[0]*1.5, p.velocity[1]*1.5)

#mouse scroll down             
    if event.type == pygame.MOUSEBUTTONDOWN and event.button == 5:
        pos = event.pos
        for p in Particles:
            if (pos[0] - p.center[0])**2 < CELL_WIDTH**2 and (pos[1] - p.center[1])**2 < CELL_HEIGHT**2:
                p.velocity = (p.velocity[0]*0.5, p.velocity[1]*0.5)
        
#def evaluate_gravity_vector_field(pos):
#    return (0,0.01)

#-----velocity vector fields------
def evaluate_grid_vector_field(Particles, toggle = True):
    if toggle == False:
        return
    for i in range(len(Particles)):
        cx = math.floor(Particles[i].center[0]/CELL_WIDTH)
        cy = math.floor(Particles[i].center[1]/CELL_HEIGHT)
        if cx >= GRID_X or cy >= GRID_Y:
            delta_v = (0,1)
        elif cx < 0 or cy < 0:
            delta_v = (0,-1)
        else:
            delta_v = FieldGrid[cy][cx].a_value
        if i in DeltaVelocity.keys():
            DeltaVelocity[i] = (DeltaVelocity[i][0] + delta_v[0], DeltaVelocity[i][1] + delta_v[1])
        else:
            DeltaVelocity[i] = delta_v

def evaluate_image_vector_field(Particles, toggle = True):
    if toggle == False:
        return
    energy = 3 - ((image[min(int(pos[1]),image.shape[1]-1)][min(int(pos[0]),image.shape[0]-1)])/255.0).sum()
    return (0, (-energy) / 50) # Divided by 50 to make it slower

def evaluate_random_vector_field(Particles, toggle = True):
    if toggle == False:
        return
    for i in range(len(Particles)):
        delta_v = ((random.random()-.5)/500,0)
        if i in DeltaVelocity.keys():
            DeltaVelocity[i] = (DeltaVelocity[i][0] + delta_v[0], DeltaVelocity[i][1] + delta_v[1])
        else:
            DeltaVelocity[i] = delta_v

def evaluate_stickiness_vector_field(Particles, toggle = True):
    if toggle == False:
        return
    for i in range(len(Particles)):
        delta_v = (0,0)
        for j in range(len(Particles)):
            if j == i:
                continue
            delta_p = (Particles[i].center[0] - Particles[j].center[0], Particles[i].center[1] - Particles[j].center[1])
            p_distance = strength(delta_p)
            v_distance = strength((Particles[j].velocity[0] - Particles[i].velocity[0], Particles[j].velocity[1] - Particles[i].velocity[1]))
            if v_distance > 0.1 or p_distance > stickiness_range:
                continue
            intensity = stickiness_factor*(stickiness_range-p_distance**1.5)
            delta_v = (delta_v[0]+delta_p[0]/p_distance*intensity, delta_v[1]+delta_p[1]/p_distance*intensity)
        if i in DeltaVelocity.keys():
            DeltaVelocity[i] = (DeltaVelocity[i][0] + delta_v[0], DeltaVelocity[i][1] + delta_v[1])
        else:
            DeltaVelocity[i] = delta_v

acceleration_vector_fields = []
velocity_vector_fields = [ evaluate_grid_vector_field]
#------velocity vector fields------

while running:
    for event in pygame.event.get():
        handle_event(event)

    if pause == 0:
        continue

    # Fill the background with white
    screen.fill((255, 255, 255))
    
    evaluate_grid_vector_field(Particles, field_toggle)
    evaluate_image_vector_field(Particles, image_toggle)
    evaluate_random_vector_field(Particles, randomness_toggle)
    #evaluate_stickiness_vector_field(Particles, stickiness_toggle)

    # Update Particles
    DeadParticles = []

    for i in range(len(Particles)):
        p = Particles[i]
        if not p.is_moving(): continue
        if not p.is_alive():
            DeadParticles.append(p)

        a = 1
        for vector_field in acceleration_vector_fields:
            a *= vector_field(p.center)
        p.accelerate(acceleration = a)

        #velocity vector fields
        p.velocity = (p.velocity[0]*p.decay+DeltaVelocity[i][0], p.velocity[1]*p.decay+DeltaVelocity[i][1])
        p.perturb(perturb_coefficient)

        p.update_history()

        p.decay = p.decay**1.001

    for p in DeadParticles:
        Particles.remove(p)

    DeltaVelocity = {}

    # Draw field visualization
    for gy in range(GRID_Y):
        for gx in range(GRID_X):
            fv = FieldGrid[gy][gx]
            is_selected = False
            if (gx,gy) == tuple(currently_selected):
                is_selected = True
            fv.draw(screen, is_selected)
            fv.draw_arrow()

    # Draw Particles
    for p in Particles:
        p.draw(screen)

    # Flip the display
    pygame.display.flip()
    pygame.time.wait(0)
     
    


