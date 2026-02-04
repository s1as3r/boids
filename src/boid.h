#ifndef _H_BOIDS_BOID
#define _H_BOIDS_BOID

// clang-format off
#include "base.h"

#include <raylib.h>
#include <raymath.h>
// clang-format on

typedef struct {
  Vector2 position;
  Vector2 velocity;
} Boid;

void draw_boid(Boid *boid);

typedef struct {
  Boid *boids;
  f32 protected_radius;
  f32 avoid_factor;
  f32 visual_radius;
  f32 matching_factor;
  u32 n;
} Flock;

void draw_flock(Flock *flock);
void update_flock(Flock *flock, f32 screen_w, f32 screen_h);

#endif // _H_BOIDS_BOID
