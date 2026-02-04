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

  Boid boids[10];
  u32 n_boids = array_count(boids);
  for (u32 i = 0; i < n_boids; i++) {
    boids[i].velocity = (Vector2){50.0f * 2.0f * (pcg32_randomf() - 0.5f),
                                  50.0f * 2.0f * (pcg32_randomf() - 0.5f)};
    boids[i].position =
        (Vector2){pcg32_randomf() * window_w, pcg32_randomf() * window_h};
  }

  Flock flock = {
      .boids = boids,
      .n = n_boids,
      .protected_radius = 35.0f,
      .avoid_factor = 0.2f,
      .visual_radius = 100.00f,
      .matching_factor = 0.1f,
  };

  while (!WindowShouldClose()) {
    BeginDrawing();
    {
      ClearBackground(BLACK);
      DrawFPS(10, 10);

      update_flock(&flock, window_w, window_h);
      draw_flock(&flock);
    }
    EndDrawing();
  }

  return 0;
}
