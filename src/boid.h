#ifndef _H_BOIDS_BOID
#define _H_BOIDS_BOID

// clang-format off

#include <raylib.h>
#include <raymath.h>
// clang-format on

typedef struct {
  Vector2 position;
  Vector2 velocity;
} Boid;

// the `boid` position is stored independent of the window size as `f32` (0 - 1)
// `scale` is used to properly render the boid to the window. it usually is just
// `{window_width, window_height}`
void draw_boid(Boid *boid, Vector2 scale);

#endif // _H_BOIDS_BOID
