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

  f32 angle = Vector2Angle(boid->velocity, p2);

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
