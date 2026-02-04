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

// the `boid` position is stored independent of the window size as `f32` (0 - 1)
// `screen_scale` is used to properly render the boid to the window. it usually
// is just
// `{window_width, window_height}`
void draw_boid(Boid *boid, Vector2 screen_scale);

typedef struct {
  Boid *boids;
  u32 n;
} Flock;

void draw_flock(Flock *flock, Vector2 screen_scale);
void update_flock(Flock *flock);

#endif // _H_BOIDS_BOID
