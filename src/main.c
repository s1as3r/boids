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

Color _imcolor_to_rl(const ImVec4_c *color);
ImVec4_c _rlcolor_to_im(const Color *color);
void draw_ui_single_flock(Flock *flock);
void draw_ui(Flock *flocks, u64 n);

i32 main(void) {
  const i32 screen_w = 1600, screen_h = 1000;
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

  ImGuiIO *io = igGetIO_Nil();
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  const f32 margin = 75.0f;
  Vector2 env_bounds_min = {margin, margin};
  Vector2 env_bounds_max = {screen_w - margin, screen_h - margin};

  Flock flocks[3] = {
      flock_init(0, 100, WHITE, env_bounds_min, env_bounds_max),
      flock_init(1, 100, BLUE, env_bounds_min, env_bounds_max),
      flock_init(2, 100, GREEN, env_bounds_min, env_bounds_max),
  };
  u64 n_flocks = array_count(flocks);

  RenderTexture2D target = LoadRenderTexture(screen_w, screen_h);
  while (!WindowShouldClose()) {
    for (u32 i = 0; i < n_flocks; i++) {
      flock_update(&flocks[i]);
    }
    BeginTextureMode(target);
    {
      ClearBackground(BLACK);
      for (u32 i = 0; i < n_flocks; i++) {
        flock_draw(&flocks[i]);
      }
    }
    EndTextureMode();

    BeginDrawing();
    {
      DrawTexturePro(target.texture,
                     (Rectangle){.x = 0,
                                 .y = 0,
                                 .width = (f32)target.texture.width,
                                 .height = (f32)-target.texture.height},
                     (Rectangle){.x = 0.0,
                                 .y = 0.0,
                                 .width = (f32)GetScreenWidth(),
                                 .height = (f32)GetScreenHeight()},
                     ZERO_VECTOR2, 0.0f, WHITE);
      DrawFPS(10, 10);

      rlImGuiBegin();
      igDockSpaceOverViewport(0, NULL, ImGuiDockNodeFlags_PassthruCentralNode,
                              NULL);
      draw_ui(flocks, n_flocks);
      rlImGuiEnd();
    }
    EndDrawing();
  }

  for (u64 i = 0; i < n_flocks; i++) {
    flock_deinit(flocks[i]);
  }
  rlImGuiShutdown();
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}

Color _imcolor_to_rl(const ImVec4_c *color) {
  return (Color){
      .r = (u8)(color->x * 255),
      .g = (u8)(color->y * 255),
      .b = (u8)(color->z * 255),
      .a = (u8)(color->w * 255),
  };
}

ImVec4_c _rlcolor_to_im(const Color *color) {
  return (ImVec4_c){
      .x = ((f32)color->r / 255.0f),
      .y = ((f32)color->g / 255.0f),
      .z = ((f32)color->b / 255.0f),
      .w = ((f32)color->a / 255.0f),
  };
}

void draw_ui_single_flock(Flock *flock) {
  if (igTreeNode_Str("Debug Draw##dbg")) {
    igCheckbox("Enable##dbg", &flock->debug_draw.enabled);
    if (!flock->debug_draw.enabled) {
      igBeginDisabled(true);
    }
    igCheckbox("Protected Radius##dbg", &flock->debug_draw.protected);
    igCheckbox("Visual Radius##dbg", &flock->debug_draw.visual);
    igCheckbox("Env Edges##dbg", &flock->debug_draw.env_edge);
    igCheckbox("Velocity##dbg", &flock->debug_draw.velocity);
    if (!flock->debug_draw.enabled) {
      igEndDisabled();
    }
    igSeparator();
    igTreePop();
  }
  ImVec4_c color = _rlcolor_to_im(&flock->color);
  igColorEdit3("Color", (float *)&color, ImGuiColorEditFlags_None);
  flock->color = _imcolor_to_rl(&color);

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
  igCheckbox("Mouse Influence", &flock->is_influenced_by_mouse);
  igSeparator();
}

void draw_ui(Flock *flocks, u64 n) {
  Flock *flock;
  for (u64 i = 0; i < n; i++) {
    flock = &flocks[i];
    igBegin(TextFormat("Flock %d", i), NULL, ImGuiWindowFlags_AlwaysAutoResize);
    draw_ui_single_flock(flock);
    igEnd();
  }
}
