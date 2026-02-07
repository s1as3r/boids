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

#define NO_FONT_AWESOME
#include <rlImGui.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
// clang-format on

void draw_ui(Flock *flock) {
  if (igCollapsingHeader_BoolPtr("Debug Draw##dbg", NULL, 0)) {
    igCheckbox("Enable##dbg", &flock->debug_draw);
    igCheckbox("Protected Radius##dbg", &flock->debug_protected);
    igCheckbox("Visual Radius##dbg", &flock->debug_visual);
    igCheckbox("Env Edges##dbg", &flock->debug_env_edge);
    igCheckbox("Velocity##dbg", &flock->debug_velocity);
    igSeparator();
  }
  igSliderFloat("Protected Radius", &flock->protected_radius, 0.0f,
                flock->visual_radius, "%.2f", ImGuiSliderFlags_None);
  igSliderFloat("Visual Radius", &flock->visual_radius, 0.0f, 400.0f, "%.2f",
                ImGuiSliderFlags_None);
  igSliderFloat("Avoid Factor", &flock->avoid_factor, 0.0f, 2.0f, "%.2f",
                ImGuiSliderFlags_None);
  igSliderFloat("Matching Factor", &flock->matching_factor, 0.0f,
                flock->avoid_factor, "%.2f", ImGuiSliderFlags_None);
  igSliderFloat("Centering Factor", &flock->centering_factor, 0.0f, 2.0f,
                "%.2f", ImGuiSliderFlags_None);
  igSliderFloat("Turn Factor", &flock->turn_factor, 5.0f, 100.0f, "%.2f",
                ImGuiSliderFlags_None);
  igSliderFloat("Min Speed", &flock->min_speed, 10.0f, flock->max_speed, "%.2f",
                ImGuiSliderFlags_None);
  igSliderFloat("Max Speed", &flock->max_speed, 100.0f, 500.0f, "%.2f",
                ImGuiSliderFlags_None);
}

i32 main(void) {
  const i32 screen_w = 1600, screen_h = 1200;
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(screen_w, screen_h, "boids");
  SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

  rlImGuiSetup(true);
  ImGuiStyle *style = igGetStyle();
  style->FontScaleDpi = 2.00f;
  style->WindowRounding = style->GrabRounding = style->FrameRounding = 5.0f;
  style->WindowPadding = style->FramePadding = (ImVec2_c){3.0f, 3.0f};
  style->ItemInnerSpacing = style->ItemSpacing = (ImVec2_c){3.0f, 5.0f};
  style->Colors[ImGuiCol_WindowBg] = (ImVec4_c){0.30f, 0.30f, 0.30f, 0.70f};

  igGetIO_Nil()->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  Boid boids[500];
  u32 n_boids = array_count(boids);
  for (u32 i = 0; i < n_boids; i++) {
    boids[i].velocity = (Vector2){100.0f * 2.0f * (pcg32_randomf() - 0.5f),
                                  100.0f * 2.0f * (pcg32_randomf() - 0.5f)};
    boids[i].position =
        (Vector2){pcg32_randomf() * screen_w, pcg32_randomf() * screen_h};
  }

  const f32 margin = 75.0f;
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
      .env_bounds_min = {margin, margin},
      .env_bounds_max = {screen_w - margin, screen_h - margin},
      .debug_draw = true,
      .debug_protected = false,
      .debug_visual = false,
      .debug_env_edge = false,
      .debug_velocity = false,
  };

  RenderTexture2D target = LoadRenderTexture(screen_w, screen_h);
  while (!WindowShouldClose()) {
    update_flock(&flock);
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

      rlImGuiBegin();
      igDockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode,
                              NULL);
      igBegin("Parameters", NULL, ImGuiWindowFlags_AlwaysAutoResize);
      {
        draw_ui(&flock);
      }
      igEnd();
      rlImGuiEnd();
    }
    EndDrawing();
    EndTextureMode();
  }

  rlImGuiShutdown();
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}
