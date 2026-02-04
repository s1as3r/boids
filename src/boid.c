// clang-format off
#include "base.h"
#include "boid.h"

#include <raylib.h>
#include <raymath.h>
// clang-format on

void draw_boid(Boid *boid) {
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

  DrawTriangleFan(points, 4, WHITE);
}

void draw_flock(Flock *flock) {
  Boid *boid;
  for (u32 i = 0; i < flock->n; i++) {
    boid = &flock->boids[i];
    draw_boid(boid);

#if defined(BOIDS_DEBUG_DRAW)
    DrawText(
        TextFormat("velocity: %.2f %.2f", boid->velocity.x, boid->velocity.y),
        (i32)(boid->position.x + 20.0f), (i32)(boid->position.y), 10, GREEN);

    DrawCircleLinesV(boid->position, flock->protected_radius, RED);
    DrawCircleLinesV(boid->position, flock->visual_radius, BLUE);
#endif
  }
}

void _flock_separate(Flock *flock) {
  Boid *me, *other;
  Vector2 close;
  for (u32 i = 0; i < flock->n; i++) {
    me = &flock->boids[i];
    close = (Vector2){0.0, 0.0};
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
    }
    me->velocity =
        Vector2Add(me->velocity, Vector2Scale(close, flock->avoid_factor));
  }
}

void _flock_align(Flock *flock) {
  Boid *me, *other;
  Vector2 velocity_avg;
  u32 n_neighbors;
  for (u32 i = 0; i < flock->n; i++) {
    me = &flock->boids[i];
    velocity_avg = (Vector2){0.0, 0.0};
    n_neighbors = 0;
    for (u32 j = 0; j < flock->n; j++) {
      if (i == j) {
        continue;
      }

      other = &flock->boids[j];
      if (Vector2Distance(me->position, other->position) <
          flock->visual_radius) {
        velocity_avg = Vector2Add(velocity_avg, other->velocity);
        n_neighbors += 1;
      }
    }
    if (n_neighbors > 0) {
      velocity_avg = Vector2Scale(velocity_avg, 1.0f / (f32)n_neighbors);
      me->velocity =
          Vector2Add(me->velocity,
                     Vector2Scale(Vector2Subtract(velocity_avg, me->velocity),
                                  flock->matching_factor));
    }
  }
}

void update_flock(Flock *flock, f32 screen_w, f32 screen_h) {
  _flock_separate(flock);
  _flock_align(flock);
  f32 delta_time = GetFrameTime();
  Boid *boid;
  for (u32 i = 0; i < flock->n; i++) {
    boid = &flock->boids[i];
    boid->position =
        Vector2Add(boid->position, Vector2Scale(boid->velocity, delta_time));

    if (boid->position.x > screen_w || boid->position.x < 0.0) {
      boid->velocity.x *= -1.0f;
    }

    if (boid->position.y > screen_h || boid->position.y < 0.0) {
      boid->velocity.y *= -1.0f;
    }
  }
}
