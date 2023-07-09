#pragma once

// configuration
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// density grid
#define GRID_NUM 288
#define STAMP_SPREAD 6
#define MAX_STAMP 15.0f

// particle generation
#define INIT_SPREAD 10.0f
#define MAX_PARTICLES 10000
#define ERASE_TOLERANCE 0.1f

// particle trails
#define PARTICLE_HISTORY_LENGTH 350

// particle movement
#define MAINTAIN_SCALE 0.45f // default is 0.5
#define FIELD_SCALE 7.0f
#define GRAVITY_SCALE 0.01f
//#define MIN_SPEED 0.4f
// XR
#define MIN_SPEED 0.04

// fields
#define NUM_LINES 10000
#define MAX_DISTANCE 3.0f
#define ATTRACTOR_SCALE 0.7f
#define ROTATION_SCALE 4.5f
#define DRAG_SCALE 1.4f