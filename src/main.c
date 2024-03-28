#include "game.h"
#define NOB_IMPLEMENTATION
#include "nob.h"
#include <raylib.h>

int main(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(800, 600, "Wooby");
  SetTargetFPS(60);

  game_init();

  while (!WindowShouldClose()) {
    game_update();
    BeginDrawing();
    ClearBackground(GRAY);
    game_draw();
    EndDrawing();
  }
  
  CloseWindow();
  return 0;
}
