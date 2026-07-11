#ifndef _H_BOIDS_BOID
#define _H_BOIDS_BOID

// clang-format off
#include "base.h"

#include <raylib.h>
#include <raymath.h>
#include <threads.h>
// clang-format on

#define ZERO_VECTOR2 ((Vector2){0.0, 0.0})

typedef struct {
  Vector2 position;
  Vector2 velocity;
} Boid;

typedef struct {
  Boid **boids;
  u32 cap;
  u32 len;
} BoidList;

typedef struct {
  bool enabled;
  bool protected;
  bool visual;
  bool velocity;
  bool env_edge;
  bool partitions;
} DebugFlags;

typedef struct {
  Boid *boids;
  BoidList *partitions;
  Vector2 *new_velocities;
  u32 partition_y;
  u32 partition_x;
  f32 protected_radius;
  f32 avoid_factor;
  f32 visual_radius;
  f32 matching_factor;
  f32 centering_factor;
  f32 turn_factor;
  f32 min_speed;
  f32 max_speed;
  Color color;
  u32 n; // active boids
  u32 cap;
  u64 id;

  // stuff i dont want a config struct for
  Vector2 env_bounds_min; // usually 0, 0
  Vector2 env_bounds_max; // usually window_width, window_height
  DebugFlags debug_draw;
  bool is_influenced_by_mouse;
  bool init_partitions;
} Flock;

typedef struct {
  Flock *flock;
  u32 start_part;
  u32 end_part;
  f32 delta_time;
  bool mouse_active;
  Boid mouse_boid;
} FlockUpdateArgs;

typedef struct {
  u32 n_procs;
  u32 n_workers;
  thrd_t *threads;
  bool *valid_threads;
  FlockUpdateArgs *args;
} ThreadWorkerData;

Flock flock_init(u64 id, u32 n, Color color, Vector2 env_bounds_min,
                 Vector2 env_bounds_max);
void flock_deinit(Flock flock);
void flock_draw(const Flock *flock);
void flock_update(Flock *flock, ThreadWorkerData *worker_data);
bool flock_add_boid(Flock *flock, Boid boid);
bool flock_add_rand_boid(Flock *flock);
bool flock_remove_last_boid(Flock *flock);

ThreadWorkerData worker_data_init(void);
void worker_data_deinit(ThreadWorkerData);

#endif // _H_BOIDS_BOID
