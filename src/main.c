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

i32 main(void) {
  const i32 window_w = 1200, window_h = 800;
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(window_w, window_h, "boids");
  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

  Boid boids[30];
  u32 n_boids = array_count(boids);
  for (u32 i = 0; i < n_boids; i++) {
    boids[i].velocity = (Vector2){0.2f * (pcg32_randomf() - 0.5f),
                                  0.2f * (pcg32_randomf() - 0.5f)};
    boids[i].position = (Vector2){pcg32_randomf(), pcg32_randomf()};
  }

  Flock flock = {.boids = boids,
                 .n = n_boids,
                 .protected_radius = 0.05f,
                 .avoid_factor = 0.2f};

  const Vector2 scale = {(f32)window_w, (f32)window_h};
  while (!WindowShouldClose()) {
    BeginDrawing();
    {
      ClearBackground(BLACK);
      DrawFPS(10, 10);

      update_flock(&flock);
      draw_flock(&flock, scale);
    }
    EndDrawing();
  }

  return 0;
}
