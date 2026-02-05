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
  const i32 env_w = 1600, env_h = 1200;
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(env_w, env_h, "boids");
  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

  Boid boids[500];
  u32 n_boids = array_count(boids);
  for (u32 i = 0; i < n_boids; i++) {
    boids[i].velocity = (Vector2){100.0f * 2.0f * (pcg32_randomf() - 0.5f),
                                  100.0f * 2.0f * (pcg32_randomf() - 0.5f)};
    boids[i].position =
        (Vector2){pcg32_randomf() * env_w, pcg32_randomf() * env_h};
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

  RenderTexture2D target = LoadRenderTexture(env_w, env_h);
  while (!WindowShouldClose()) {
    update_flock(&flock, env_w, env_h);
    BeginTextureMode(target);
    {
      ClearBackground(BLACK);
      draw_flock(&flock);
    }
    EndTextureMode();
    BeginDrawing();
    {
      DrawTexturePro(target.texture,
                     (Rectangle){.x = 0,
                                 .y = 0,
                                 .width = (f32)target.texture.width,
                                 .height = (f32)target.texture.height},
                     (Rectangle){.x = 0.0,
                                 .y = 0.0,
                                 .width = (f32)GetScreenWidth(),
                                 .height = (f32)GetScreenHeight()},
                     (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
      DrawFPS(10, 10);
    }
    EndDrawing();
    EndTextureMode();
  }

  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}
