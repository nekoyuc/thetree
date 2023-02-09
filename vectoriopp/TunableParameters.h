#pragma once

// configuration
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// density grid
#define GRID_NUM 288
#define STAMP_SPREAD 3
#define MAX_STAMP 15.0f

// particle generation
#define INIT_SPREAD 5.0f
#define MAX_PARTICLES 10000
#define ERASE_TOLERANCE 0.1f

// particle trails
#define PARTICLE_HISTORY_LENGTH 200

// particle update
#define MAINTAIN_SCALE 0.5f
#define FIELD_SCALE 7.0f
#define ACCELERATION_SCALE 0.02f
#define MIN_SPEED 0.4f

// fields
#define NUM_LINES 10000
#define MAX_DISTANCE 3.0f
#define ATTRACTOR_SCALE 0.4f
#define ROTATION_SCALE 5.5f
#define DRAG_SCALE 1.2f