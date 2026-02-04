// clang-format off
#include "base.h"
#include "boid.h"

#include <raylib.h>
#include <raymath.h>
// clang-format on

void draw_boid(Boid *boid, Vector2 scale) {
  const f32 half_width = 7.0f;
  const f32 half_height = 7.0f;
  Vector2 center = Vector2Multiply(boid->position, scale);

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

#if defined(BOIDS_DEBUG_DRAW)
  const Vector2 debug_draw_offset = {20.0f, 20.0f};
  DrawText(TextFormat("heading: %.2f %.2f", boid->velocity.x, boid->velocity.y),
           (i32)(center.x + debug_draw_offset.x),
           (i32)(center.y + debug_draw_offset.y), 10, GREEN);
#endif
}

void draw_flock(Flock *flock, Vector2 screen_scale) {
  for (u32 i = 0; i < flock->n; i++) {
    draw_boid(&flock->boids[i], screen_scale);
  }
}

void update_flock(Flock *flock) {
  f32 delta_time = GetFrameTime();
  Boid *boid;
  for (u32 i = 0; i < flock->n; i++) {
    boid = &flock->boids[i];
    boid->position =
        Vector2Add(boid->position, Vector2Scale(boid->velocity, delta_time));
    if (boid->position.x > 1.0 || boid->position.x < 0.0) {
      boid->velocity.x *= -1.0f;
    }

    if (boid->position.y > 1.0 || boid->position.y < 0.0) {
      boid->velocity.y *= -1.0f;
    }
  }
}
