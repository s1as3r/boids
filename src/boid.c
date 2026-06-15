// clang-format off
#include "base.h"
#include "boid.h"
#include "rand.h"

#include <raylib.h>
#include <raymath.h>
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
    .min_speed = 50.0f,
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

  return flock;
}

void flock_deinit(Flock flock) { MemFree(flock.boids); }

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
  if (flock->is_influenced_by_mouse && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    Boid mb = _get_mouse_boid();
    _debug_draw_boid(&mb, flock->protected_radius, flock->visual_radius,
                     flock->debug_draw);
  }
}

void flock_update(Flock *flock) {
  f32 delta_time = GetFrameTime();
  f32 speed, dist;
  Boid *me, *other;
  Vector2 close, avg_velocity, avg_position;
  u32 n_neighbors = 0;
  for (u32 i = 0; i < flock->n; i++) {
    me = &flock->boids[i];
    n_neighbors = 0;
    close = ZERO_VECTOR2;
    avg_velocity = ZERO_VECTOR2;
    avg_position = ZERO_VECTOR2;
    for (u32 j = 0; j < flock->n; j++) {
      if (i == j) {
        continue;
      }

      other = &flock->boids[j];
      dist = Vector2Distance(me->position, other->position);
      if (dist < flock->visual_radius) {
        if (dist < flock->protected_radius) {
          close =
              Vector2Add(close, Vector2Subtract(me->position, other->position));
        }
        avg_velocity = Vector2Add(avg_velocity, other->velocity);
        avg_position = Vector2Add(avg_position, other->position);
        n_neighbors += 1;
      }
    }

    if (flock->is_influenced_by_mouse && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      Boid mouse = _get_mouse_boid();
      dist = Vector2Distance(me->position, mouse.position);
      if (dist < flock->visual_radius) {
        if (dist < flock->protected_radius) {
          close =
              Vector2Add(close, Vector2Subtract(me->position, mouse.position));
        }
        avg_velocity = Vector2Add(avg_velocity, mouse.velocity);
        avg_position = Vector2Add(avg_position, mouse.position);
        n_neighbors += 1;
      }
    }

    // separation
    me->velocity =
        Vector2Add(me->velocity, Vector2Scale(close, flock->avoid_factor));

    if (n_neighbors > 0) {
      // alignment
      avg_velocity = Vector2Scale(avg_velocity, 1.0f / (f32)n_neighbors);
      me->velocity =
          Vector2Add(me->velocity,
                     Vector2Scale(Vector2Subtract(avg_velocity, me->velocity),
                                  flock->matching_factor));

      // cohesion
      avg_position = Vector2Scale(avg_position, 1.0f / (f32)n_neighbors);
      me->velocity =
          Vector2Add(me->velocity,
                     Vector2Scale(Vector2Subtract(avg_position, me->position),
                                  flock->centering_factor));
    }
    speed = Vector2Length(me->velocity);

    if (speed > flock->max_speed) {
      me->velocity = Vector2Scale(me->velocity, flock->max_speed / speed);
    }

    if (speed < flock->min_speed) {
      me->velocity = Vector2Scale(me->velocity, flock->min_speed / speed);
    }

    me->position =
        Vector2Add(me->position, Vector2Scale(me->velocity, delta_time));

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

// returns `true` if flock had to be resized
bool flock_add_boid(Flock *flock, Boid boid) {
  bool did_resize = false;
  if (flock->n + 1 > flock->cap) {
    flock->cap = (u32)(FLOCK_CAP_GROWTH_FACTOR * flock->cap);
    flock->boids = MemRealloc(flock->boids, sizeof(Boid) * flock->cap);
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
