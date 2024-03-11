#include "entity.h"
#define NOB_IMPLEMENTATION
#include "nob.h"
#include <raylib.h>

int main(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(800, 600, "Wooby");
  SetTargetFPS(60);

  entity_id_t e1 = spawn_entity(PLAYER, (Vector2){200.0f, 150.0f});
  entity_id_t e2 = spawn_entity(ENEMY, (Vector2){400.0f, 150.0f});
  entity_id_t e3 = spawn_entity(ENEMY, (Vector2){500.0f, 150.0f});
  entity_id_t e4 = spawn_entity(ENEMY, (Vector2){400.0f, 250.0f});

  while (!WindowShouldClose()) {
    update_entities();
    if (!are_entities_updating())
      if (IsKeyPressed(KEY_R)) {
        free_entity(e1);
        free_entity(e2);
        free_entity(e3);
        free_entity(e4);
        e1 = spawn_entity(PLAYER, (Vector2){200.0f, 150.0f});
        e2 = spawn_entity(ENEMY, (Vector2){400.0f, 150.0f});
        e3 = spawn_entity(ENEMY, (Vector2){500.0f, 150.0f});
        e4 = spawn_entity(ENEMY, (Vector2){400.0f, 250.0f});
        start_updating_entities();
      }
    BeginDrawing();
    ClearBackground(GRAY);
    draw_entities();
    if (!are_entities_updating()) {
      DrawText("GAME OVER!", 200, 230, 60, BLACK);
      DrawText("Press 'R' to restart!", 280, 290, 20, BLACK);
    }
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
