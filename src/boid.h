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

typedef struct {
  bool enabled;
  bool protected;
  bool visual;
  bool velocity;
  bool env_edge;
} DebugFlags;

typedef struct {
  Boid *boids;
  f32 protected_radius;
  f32 avoid_factor;
  f32 visual_radius;
  f32 matching_factor;
  f32 centering_factor;
  f32 turn_factor;
  f32 min_speed;
  f32 max_speed;
  Color color;
  u32 n;
  u64 id;

  // stuff i dont want a config struct for
  Vector2 env_bounds_min; // usually 0, 0
  Vector2 env_bounds_max; // usually window_width, window_height
  DebugFlags debug_draw;
} Flock;

Flock init_flock(u64 id, u32 n, Color color, Vector2 env_bounds_min,
                 Vector2 env_bounds_max);
void deinit_flock(Flock flock);
void draw_flock(const Flock *flock);
void update_flock(Flock *flock);

#endif // _H_BOIDS_BOID
