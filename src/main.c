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

  Boid boids[15];
  u32 n_boids = array_count(boids);
  for (u32 i = 0; i < n_boids; i++) {
    boids[i].velocity = (Vector2){100.0f * 2.0f * (pcg32_randomf() - 0.5f),
                                  100.0f * 2.0f * (pcg32_randomf() - 0.5f)};
    boids[i].position =
        (Vector2){pcg32_randomf() * window_w, pcg32_randomf() * window_h};
  }

  Flock flock = {
      .boids = boids,
      .n = n_boids,
      .protected_radius = 30.0f,
      .avoid_factor = 0.5f,
      .visual_radius = 70.00f,
      .matching_factor = 0.5f,
      .centering_factor = 0.05f,
      .turn_factor = 20.0f,
      .min_speed = 50.0f,
      .max_speed = 300.0f,
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
