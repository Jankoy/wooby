#include "entity.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include <raylib.h>

int main(void) {
  SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
  InitWindow(800, 600, "Test");
  SetTargetFPS(60);

  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  luaL_dofile(L, "resources/scripts/test.lua");

  spawn_entity(PLAYER, (Vector2){200.0f, 150.0f}, 0.25f);
  spawn_entity(ENEMY, (Vector2){400.0f, 150.0f}, 0.25f);

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
