// clang-format off
#include "base.h"
#include "boid.h"
#include "rand.h"

#include <raylib.h>
#include <raymath.h>
// clang-format on

#define ZERO_VECTOR2 ((Vector2){0.0, 0.0})

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

Flock init_flock(u64 id, u32 n, Color color, Vector2 env_bounds_min,
                 Vector2 env_bounds_max) {
  Boid *boids = MemAlloc(sizeof(Boid) * n);
  for (u32 i = 0; i < n; i++) {
    boids[i].velocity = (Vector2){100.0f * 2.0f * (pcg32_randomf() - 0.5f),
                                  100.0f * 2.0f * (pcg32_randomf() - 0.5f)};
    boids[i].position = (Vector2){pcg32_randomf() * env_bounds_max.x,
                                  pcg32_randomf() * env_bounds_max.y};
  }

  Flock flock = {
      .id = id,
      .boids = boids,
      .n = n,
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
      .debug_draw = true,
      .debug_protected = false,
      .debug_visual = false,
      .debug_env_edge = false,
      .debug_velocity = false,
  };

  return flock;
}

void deinit_flock(Flock flock) { MemFree(flock.boids); }

void draw_flock(const Flock *flock) {
  Boid *boid;
  for (u32 i = 0; i < flock->n; i++) {
    boid = &flock->boids[i];
    draw_boid(boid, &flock->color);

    if (!flock->debug_draw) {
      continue;
    }

    if (flock->debug_velocity) {
      DrawText(TextFormat("(%.2f %.2f)", boid->velocity.x, boid->velocity.y),
               (i32)(boid->position.x + 20.0f), (i32)(boid->position.y), 10,
               GREEN);
    }

    if (flock->debug_protected) {
      DrawCircleLinesV(boid->position, flock->protected_radius, RED);
    }

    if (flock->debug_visual) {
      DrawCircleLinesV(boid->position, flock->visual_radius, BLUE);
    }
  }
  if (flock->debug_draw && flock->debug_env_edge) {
    Rectangle rec = {0};
    rec.x = flock->env_bounds_min.x;
    rec.y = flock->env_bounds_min.y;
    rec.width = flock->env_bounds_max.x - rec.x;
    rec.height = flock->env_bounds_max.y - rec.y;

    DrawRectangleLinesEx(rec, 3, flock->color);
  }
}

void update_flock(Flock *flock) {
  f32 delta_time = GetFrameTime();
  f32 speed;
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
      if (Vector2Distance(me->position, other->position) <
          flock->protected_radius) {
        close =
            Vector2Add(close, Vector2Subtract(me->position, other->position));
      }

      if (Vector2Distance(me->position, other->position) <
          flock->visual_radius) {
        avg_velocity = Vector2Add(avg_velocity, other->velocity);
        avg_position = Vector2Add(avg_position, other->position);
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
