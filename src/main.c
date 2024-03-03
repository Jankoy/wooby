#include <raylib.h>

int main(void) {
  InitWindow(800, 600, "Test");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(GRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
