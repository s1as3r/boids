/*
 * boids - a simple 2d boid simulation in raylib
 * reference:
 * https://people.ece.cornell.edu/land/courses/ece4760/labs/s2021/Boids/Boids.html
 */

// clang-format off
#include "base.h"

#include "boid.c"
#include "rand.c"

#include <raylib.h>
#include <raymath.h>
// clang-format on

void draw_boids(Boid boids[], u32 n, Vector2 scale) {
  for (u32 i = 0; i < n; i++) {
    draw_boid(&boids[i], scale);
  }
}

i32 main(void) {
  const i32 window_w = 1200, window_h = 800;
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(window_w, window_h, "boids");
  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

  Boid boids[5];
  u32 n_boids = array_count(boids);
  for (u32 i = 0; i < n_boids; i++) {
    boids[i].velocity = (Vector2){2.0f * (pcg32_randomf() - 0.5f),
                                  2.0f * (pcg32_randomf() - 0.5f)};
    boids[i].position = (Vector2){pcg32_randomf(), pcg32_randomf()};
  }

  const Vector2 scale = {(f32)window_w, (f32)window_h};
  while (!WindowShouldClose()) {
    BeginDrawing();
    {
      ClearBackground(BLACK);
      DrawFPS(10, 10);

      draw_boids(boids, n_boids, scale);
    }
    EndDrawing();
  }

  return 0;
}
