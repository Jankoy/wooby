#include "entity.h"
#include <raylib.h>

int main(void) {
  InitWindow(800, 600, "Test");
  SetTargetFPS(60);

  spawn_entity(PLAYER, (Vector2){200.0f, 150.0f}, 0.25f);

  while (!WindowShouldClose()) {
    update_entities();
    BeginDrawing();
    ClearBackground(GRAY);
    draw_entities();
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
