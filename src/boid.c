// clang-format off
#include "base.h"
#include "boid.h"
#include "rand.h"

#include <raylib.h>
#include <raymath.h>
#include <threads.h>
#include <sys/sysinfo.h>
#include <string.h>
// clang-format on

#define MOUSE_DELTA_TO_VELOCITY 350.0f
#define FLOCK_CAP_GROWTH_FACTOR 1.5

void draw_boid(const Boid *boid, const Color *color) {
  const f32 half_width = 7.0f;
  const f32 half_height = 7.0f;
  Vector2 center = boid->position;

  Vector2 p1 = (Vector2){-half_width, half_height};
  Vector2 p2 = (Vector2){2 * half_height, 0};
  Vector2 p3 = (Vector2){-half_width, -half_height};

  // have to multiply y with -1 because positive y is down
  f32 angle =
      Vector2Angle(Vector2Multiply(boid->velocity, (Vector2){1.0f, -1.0f}), p2);

  Vector2 points[] = {center, Vector2Add(center, Vector2Rotate(p1, angle)),
                      Vector2Add(center, Vector2Rotate(p2, angle)),
                      Vector2Add(center, Vector2Rotate(p3, angle))};

  DrawTriangleFan(points, 4, *color);
}

void _flock_init_partitions(Flock *flock) {
  flock->partition_x = (u32)((f32)SCREEN_W / (2.0f * flock->visual_radius));
  flock->partition_y = (u32)((f32)SCREEN_H / (2.0f * flock->visual_radius));
  u32 n_partitions = flock->partition_x * flock->partition_y;
  flock->partitions = (BoidList *)MemAlloc(sizeof(BoidList) * n_partitions);
  flock->init_partitions = false;

  u32 cap = 2 * flock->n / n_partitions;
  cap = cap > 10 ? cap : 10;
  for (u32 i = 0; i < n_partitions; i++) {
    flock->partitions[i] = (BoidList){
        .boids = MemAlloc(sizeof(Boid *) * cap),
        .cap = cap,
        .len = 0,
    };
  }
}

Flock flock_init(u64 id, u32 n, Color color, Vector2 env_bounds_min,
                 Vector2 env_bounds_max) {
  u32 cap = 2 * n;
  Boid *boids = MemAlloc(sizeof(Boid) * cap);
  for (u32 i = 0; i < n; i++) {
    boids[i].velocity = (Vector2){100.0f * 2.0f * (pcg32_randomf() - 0.5f),
                                  100.0f * 2.0f * (pcg32_randomf() - 0.5f)};
    boids[i].position = (Vector2){pcg32_randomf() * env_bounds_max.x,
                                  pcg32_randomf() * env_bounds_max.y};
  }

  // clang-format off
  Flock flock = {
    .id = id,
    .boids = boids,
    .n = n,
    .cap = cap,
    .color = color,
    .protected_radius = 30.0f,
    .avoid_factor = 0.5f,
    .visual_radius = 70.00f,
    .matching_factor = 0.5f,
    .centering_factor = 0.05f,
    .turn_factor = 20.0f,
    .min_speed = 150.0f,
    .max_speed = 300.0f,
    .env_bounds_min = env_bounds_min,
    .env_bounds_max = env_bounds_max,
    .is_influenced_by_mouse = true,
    .debug_draw = {
      .enabled = true,
      .protected = false,
      .visual = false,
      .env_edge = false,
      .velocity = false,
    },
  };
  // clang-format on
  _flock_init_partitions(&flock);
  flock.new_velocities = MemAlloc(sizeof(Vector2) * cap);

  return flock;
}

void flock_deinit(Flock flock) {
  MemFree(flock.boids);
  MemFree(flock.new_velocities);
  u32 n_partitions = flock.partition_x * flock.partition_y;
  for (u32 i = 0; i < n_partitions; i++) {
    MemFree(flock.partitions[i].boids);
  }
  MemFree(flock.partitions);
}

void _debug_draw_boid(const Boid *boid, f32 protected_radius, f32 visual_radius,
                      DebugFlags debug_draw) {
  if (!debug_draw.enabled) {
    return;
  }

  if (debug_draw.velocity) {
    DrawText(TextFormat("(%.2f %.2f)", boid->velocity.x, boid->velocity.y),
             (i32)(boid->position.x + 20.0f), (i32)(boid->position.y), 10,
             GREEN);
  }
  if (debug_draw.protected) {
    DrawCircleLinesV(boid->position, protected_radius, RED);
  }

  if (debug_draw.visual) {
    DrawCircleLinesV(boid->position, visual_radius, BLUE);
  }
}

Boid _get_mouse_boid(void) {
  return (Boid){.position = GetMousePosition(),
                .velocity =
                    Vector2Scale(GetMouseDelta(), MOUSE_DELTA_TO_VELOCITY)};
}

void flock_draw(const Flock *flock) {
  Boid *boid;
  for (u32 i = 0; i < flock->n; i++) {
    boid = &flock->boids[i];
    draw_boid(boid, &flock->color);

    _debug_draw_boid(boid, flock->protected_radius, flock->visual_radius,
                     flock->debug_draw);
  }
  if (flock->debug_draw.enabled && flock->debug_draw.env_edge) {
    Rectangle rec = {0};
    rec.x = flock->env_bounds_min.x;
    rec.y = flock->env_bounds_min.y;
    rec.width = flock->env_bounds_max.x - rec.x;
    rec.height = flock->env_bounds_max.y - rec.y;

    DrawRectangleLinesEx(rec, 3, flock->color);
  }
  if (flock->debug_draw.enabled && flock->debug_draw.partitions) {
    u32 n_x = (u32)(flock->env_bounds_max.x / (2.0f * flock->visual_radius));
    u32 n_y = (u32)(flock->env_bounds_max.y / (2.0f * flock->visual_radius));
    for (u32 i = 1; i < n_x; i++) {
      f32 line_x =
          flock->env_bounds_min.x + ((f32)i * 2.0f * flock->visual_radius);
      DrawLine((i32)line_x, (i32)flock->env_bounds_min.y, (i32)line_x,
               (i32)flock->env_bounds_max.y, WHITE);
    }

    for (u32 i = 1; i < n_y; i++) {
      f32 line_y =
          flock->env_bounds_min.y + ((f32)i * 2.0f * flock->visual_radius);
      DrawLine((i32)flock->env_bounds_min.x, (i32)line_y,
               (i32)flock->env_bounds_max.x, (i32)line_y, WHITE);
    }
  }

  if (flock->is_influenced_by_mouse && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    Boid mb = _get_mouse_boid();
    _debug_draw_boid(&mb, flock->protected_radius, flock->visual_radius,
                     flock->debug_draw);
  }
}

bool boid_list_add(BoidList *list, Boid *boid) {
  bool did_resize = false;
  if (list->len + 1 > list->cap) {
    u32 new_cap = (u32)(FLOCK_CAP_GROWTH_FACTOR * (f32)list->cap);
    list->cap = new_cap;
    list->boids = MemRealloc(list->boids, sizeof(Boid *) * list->cap);
    did_resize = true;
  }
  list->boids[list->len++] = boid;
  return did_resize;
}

i32 _flock_update_phase1(void *arg) {
  FlockUpdateArgs *a = (FlockUpdateArgs *)arg;
  Flock *flock = a->flock;

  for (u32 part = a->start_part; part < a->end_part; part++) {
    u32 px = part % flock->partition_x;
    u32 py = part / flock->partition_x;
    BoidList *self_lst = &flock->partitions[part];

    i32 nx_start = (i32)px - 1;
    i32 nx_end = (i32)px + 1;
    i32 ny_start = (i32)py - 1;
    i32 ny_end = (i32)py + 1;

    if (nx_start < 0) {
      nx_start = 0;
    };
    if (ny_start < 0) {
      ny_start = 0;
    };
    if (nx_end > (i32)flock->partition_x - 1) {
      nx_end = (i32)flock->partition_x - 1;
    }
    if (ny_end > (i32)flock->partition_y - 1) {
      ny_end = (i32)flock->partition_y - 1;
    }

    for (u32 i = 0; i < self_lst->len; i++) {
      Boid *me = self_lst->boids[i];
      u32 idx = (u32)(me - flock->boids);

      Vector2 close = ZERO_VECTOR2;
      Vector2 avg_velocity = ZERO_VECTOR2;
      Vector2 avg_position = ZERO_VECTOR2;

      u32 n_neighbors = 0;
      for (i32 ny = ny_start; ny <= ny_end; ny++) {
        for (i32 nx = nx_start; nx <= nx_end; nx++) {
          BoidList *neighbors =
              &flock->partitions[(u32)ny * flock->partition_x + (u32)nx];
          for (u32 j = 0; j < neighbors->len; j++) {
            Boid *other = neighbors->boids[j];
            if (other == me) {
              continue;
            }
            f32 dist = Vector2Distance(me->position, other->position);
            if (dist < flock->visual_radius) {
              if (dist < flock->protected_radius) {
                close = Vector2Add(
                    close, Vector2Subtract(me->position, other->position));
              }
              avg_velocity = Vector2Add(avg_velocity, other->velocity);
              avg_position = Vector2Add(avg_position, other->position);
              n_neighbors++;
            }
          }
        }
      }

      if (a->mouse_active) {
        f32 dist = Vector2Distance(me->position, a->mouse_boid.position);
        if (dist < flock->visual_radius) {
          if (dist < flock->protected_radius) {
            close = Vector2Add(
                close, Vector2Subtract(me->position, a->mouse_boid.position));
          }
          avg_velocity = Vector2Add(avg_velocity, a->mouse_boid.velocity);
          avg_position = Vector2Add(avg_position, a->mouse_boid.position);
          n_neighbors++;
        }
      }

      // start from current velocity, but never write it back until phase 2
      Vector2 new_vel =
          Vector2Add(me->velocity, Vector2Scale(close, flock->avoid_factor));
      if (n_neighbors > 0) {
        avg_velocity = Vector2Scale(avg_velocity, 1.0f / (f32)n_neighbors);
        new_vel = Vector2Add(
            new_vel, Vector2Scale(Vector2Subtract(avg_velocity, new_vel),
                                  flock->matching_factor));
        avg_position = Vector2Scale(avg_position, 1.0f / (f32)n_neighbors);
        new_vel = Vector2Add(
            new_vel, Vector2Scale(Vector2Subtract(avg_position, me->position),
                                  flock->centering_factor));
      }

      flock->new_velocities[idx] = new_vel;
    }
  }
  return 0;
}

i32 _flock_update_phase2(void *arg) {
  FlockUpdateArgs *a = (FlockUpdateArgs *)arg;
  Flock *flock = a->flock;

  for (u32 part = a->start_part; part < a->end_part; part++) {
    BoidList *lst = &flock->partitions[part];
    for (u32 i = 0; i < lst->len; i++) {
      Boid *me = lst->boids[i];
      u32 idx = (u32)(me - flock->boids);
      me->velocity = flock->new_velocities[idx];

      f32 speed = Vector2Length(me->velocity);
      if (speed > flock->max_speed) {
        me->velocity = Vector2Scale(me->velocity, flock->max_speed / speed);
      }
      if (speed < flock->min_speed) {
        me->velocity = Vector2Scale(me->velocity, flock->min_speed / speed);
      }
      me->position =
          Vector2Add(me->position, Vector2Scale(me->velocity, a->delta_time));
      if (me->position.x < flock->env_bounds_min.x) {
        me->velocity.x += flock->turn_factor;
      }
      if (me->position.x > flock->env_bounds_max.x) {
        me->velocity.x += -flock->turn_factor;
      }
      if (me->position.y < flock->env_bounds_min.y) {
        me->velocity.y += flock->turn_factor;
      }
      if (me->position.y > flock->env_bounds_max.y) {
        me->velocity.y += -flock->turn_factor;
      }
    }
  }
  return 0;
}

void _run_phase(Flock *flock, ThreadWorkerData *worker_data, thrd_start_t fn,
                f32 delta_time, bool mouse_active, Boid mouse_boid) {
  u32 n_partitions = flock->partition_x * flock->partition_y;
  u32 chunk =
      (n_partitions + worker_data->n_workers - 1) / worker_data->n_workers;

  memset(worker_data->valid_threads, 0, sizeof(bool) * worker_data->n_workers);
  for (u32 w = 0; w < worker_data->n_workers; w++) {
    u32 start = w * chunk;
    if (start >= n_partitions) {
      break;
    }
    u32 end = start + chunk;
    if (end > n_partitions) {
      end = n_partitions;
    }

    worker_data->args[w] = (FlockUpdateArgs){
        .flock = flock,
        .start_part = start,
        .end_part = end,
        .delta_time = delta_time,
        .mouse_active = mouse_active,
        .mouse_boid = mouse_boid,
    };
    if (thrd_create(&worker_data->threads[w], fn, &worker_data->args[w]) !=
        thrd_success) {
      fn(&worker_data->args[w]);
      worker_data->valid_threads[w] = false;
    } else {
      worker_data->valid_threads[w] = true;
    }
  }

  for (u32 w = 0; w < worker_data->n_workers; w++) {
    if (worker_data->valid_threads[w]) {
      thrd_join(worker_data->threads[w], NULL);
    }
  }
}

ThreadWorkerData worker_data_init(void) {
  i32 nprocs = get_nprocs();
  u32 n_workers = nprocs > 1 ? (u32)nprocs : 1;

  return (ThreadWorkerData){
      .n_procs = n_workers,
      .n_workers = n_workers,
      .threads = MemAlloc(sizeof(thrd_t) * n_workers),
      .valid_threads = MemAlloc(sizeof(bool) * n_workers),
      .args = MemAlloc(sizeof(FlockUpdateArgs) * n_workers),
  };
}

void worker_data_deinit(ThreadWorkerData worker_data) {
  MemFree(worker_data.args);
  MemFree(worker_data.threads);
  MemFree(worker_data.valid_threads);
}

void flock_update(Flock *flock, ThreadWorkerData *worker_data) {
  f32 delta_time = GetFrameTime();

  if (flock->init_partitions) {
    u32 n_partitions = flock->partition_x * flock->partition_y;
    for (u32 i = 0; i < n_partitions; i++) {
      MemFree(flock->partitions[i].boids);
    }
    MemFree(flock->partitions);
    _flock_init_partitions(flock);
    worker_data->n_workers = worker_data->n_procs;
  }

  u32 n_partitions = flock->partition_x * flock->partition_y;
  if (worker_data->n_workers > n_partitions) {
    worker_data->n_workers = n_partitions;
  }
  for (u32 i = 0; i < n_partitions; i++) {
    flock->partitions[i].len = 0;
  }
  for (u32 i = 0; i < flock->n; i++) {
    Boid *b = &flock->boids[i];
    u32 x = (u32)Clamp(b->position.x / (2.0f * flock->visual_radius), 0.0f,
                       (f32)flock->partition_x - 1.0f);
    u32 y = (u32)Clamp(b->position.y / (2.0f * flock->visual_radius), 0.0f,
                       (f32)flock->partition_y - 1.0f);
    boid_list_add(&flock->partitions[(y * flock->partition_x) + x], b);
  }

  bool mouse_active =
      flock->is_influenced_by_mouse && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
  Boid mouse_boid = mouse_active ? _get_mouse_boid() : (Boid){0};

  _run_phase(flock, worker_data, _flock_update_phase1, delta_time, mouse_active,
             mouse_boid);
  _run_phase(flock, worker_data, _flock_update_phase2, delta_time, mouse_active,
             mouse_boid);
}

// returns `true` if flock had to be resized
bool flock_add_boid(Flock *flock, Boid boid) {
  bool did_resize = false;
  if (flock->n + 1 > flock->cap) {
    flock->cap = (u32)(FLOCK_CAP_GROWTH_FACTOR * (f32)flock->cap);
    flock->boids = MemRealloc(flock->boids, sizeof(Boid) * flock->cap);
    flock->new_velocities =
        MemRealloc(flock->new_velocities, sizeof(Vector2) * flock->cap);
    did_resize = true;
  }
  flock->boids[flock->n++] = boid;
  return did_resize;
}

// returns `true` if flock had to be resized
bool flock_add_rand_boid(Flock *flock) {
  return flock_add_boid(
      flock,
      (Boid){.velocity = {.x = 100.0f * 2.0f * (pcg32_randomf() - 0.5f),
                          .y = 100.0f * 2.0f * (pcg32_randomf() - 0.5f)},
             .position = {.x = pcg32_randomf() * flock->env_bounds_max.x,
                          .y = pcg32_randomf() * flock->env_bounds_max.y}});
}

// returns `true` if flock->n > 0
bool flock_remove_last_boid(Flock *flock) {
  if (flock->n == 0) {
    return false;
  }
  flock->n--;
  return true;
}
