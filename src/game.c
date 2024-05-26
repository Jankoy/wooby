#include "game.h"

#include "entity.h"

#include "raygui.h"
#include <raylib.h>
#include <raymath.h>

static entity_id_t player_id;

#define ENEMY_CAP 512
static entity_id_t enemies[ENEMY_CAP];
static size_t enemy_count = 0;

static void spawn_enemy(Vector2 position) {
  if (enemy_count >= ENEMY_CAP)
    return;
  enemies[enemy_count++] = spawn_entity(E_ENEMY, position);
}

void game_init() {
  player_id = spawn_entity(E_PLAYER, (Vector2){200.0f, 150.0f});
  spawn_enemy((Vector2){350.0f, 250.0f});
  spawn_enemy((Vector2){450.0f, 250.0f});
  spawn_enemy((Vector2){350.0f, 350.0f});
  spawn_enemy((Vector2){450.0f, 350.0f});
}

static void enemies_free() {
  for (size_t i = 0; i < enemy_count && i < ENEMY_CAP; ++i)
    free_entity(enemies[i]);
  enemy_count = 0;
}

void game_free() {
  enemies_free();
  free_entity(player_id);
}

static void game_reset() {
  game_free();
  game_init();
}

static bool game_is_over = false;
static bool show_debug_menu = false;

void game_over() { game_is_over = true; }

void game_restart() {
  game_is_over = false;
  game_reset();
}

static Rectangle debug_menu_bounds = {40.0f, 40.0f, 260.0f, 180.0f};
static bool debug_menu_was_dragged = false;
static Vector2 debug_menu_drag_offset = {0};

static void debug_menu_update() {
  const Vector2 mouse_position = GetMousePosition();
  const bool is_dragging =
      (debug_menu_was_dragged && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) ||
      (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
       CheckCollisionPointRec(
           mouse_position, (Rectangle){debug_menu_bounds.x, debug_menu_bounds.y,
                                       debug_menu_bounds.width, 24.0f}));
  if (debug_menu_was_dragged) {
    const Vector2 new_pos = Vector2Clamp(
        Vector2Subtract(mouse_position, debug_menu_drag_offset), (Vector2){0},
        (Vector2){GetScreenWidth() - debug_menu_bounds.width,
                  GetScreenHeight() - debug_menu_bounds.height});
    debug_menu_bounds.x = new_pos.x;
    debug_menu_bounds.y = new_pos.y;
  }

  if (is_dragging && !debug_menu_was_dragged)
    debug_menu_drag_offset = Vector2Subtract(
        mouse_position, (Vector2){debug_menu_bounds.x, debug_menu_bounds.y});

  debug_menu_was_dragged = is_dragging;
}

void game_update() {
  if (IsKeyPressed(KEY_R))
    game_restart();

  if (IsKeyPressed(KEY_GRAVE))
    show_debug_menu = !show_debug_menu;

  if (show_debug_menu)
    debug_menu_update();

  if (game_is_over)
    return;

  update_entities();
}

static void debug_menu_draw() {
  show_debug_menu = !GuiWindowBox(debug_menu_bounds, "Debug Menu");
  const Vector2 button_size = {90.0f, 30.0f};
  Vector2 button_pos = {debug_menu_bounds.x + 10.0f,
                        debug_menu_bounds.y + 34.0f};
  if (GuiButton(
          (Rectangle){button_pos.x, button_pos.y, button_size.x, button_size.y},
          "Spawn Enemy"))
    spawn_enemy((Vector2){(float)GetScreenWidth() / 2.0f - 28.0f,
                          (float)GetScreenHeight() / 2.0f - 28.0f});
  button_pos.y += button_size.y + 10.0f;
  if (GuiButton(
          (Rectangle){button_pos.x, button_pos.y, button_size.x, button_size.y},
          "Kill Enemies"))
    enemies_free();
}

void game_draw() {
  if (!game_is_over) {
    draw_entities();
    // DrawFPS(10, 10);
    // DrawText(TextFormat("Enemy Count: %zu", enemy_count), 10, 30, 20, BLACK);
  } else {
    DrawText("GAME OVER!", 200, 230, 60, BLACK);
    DrawText("Press 'R' to restart!", 280, 290, 20, BLACK);
  }

  if (show_debug_menu)
    debug_menu_draw();
}
