#include "game.h"

#include "entity.h"
#include <raylib.h>

static entity_id_t player_id;
static entity_id_t enemy_uno_id;
static entity_id_t enemy_dos_id;
static entity_id_t enemy_tres_id;

void game_init() {
  player_id = spawn_entity(PLAYER, (Vector2){200.0f, 150.0f});
  enemy_uno_id = spawn_entity(ENEMY, (Vector2){400.0f, 150.0f});
  enemy_dos_id = spawn_entity(ENEMY, (Vector2){500.0f, 150.0f});
  enemy_tres_id = spawn_entity(ENEMY, (Vector2){400.0f, 250.0f});
}

static void game_reset() {
  free_entity(enemy_tres_id);
  free_entity(player_id);
  free_entity(enemy_uno_id);
  free_entity(enemy_dos_id);
  game_init();
}

static bool game_is_over = false;

void game_over() {
  game_is_over = true;
}

void game_restart() {
  game_is_over = false;
  game_reset();
}

void game_update() {
  if (!game_is_over)
    update_entities();
  else if (IsKeyPressed(KEY_R))
      game_restart();
}

void game_draw() {
  if (!game_is_over)
    draw_entities();
  else {
    DrawText("GAME OVER!", 200, 230, 60, BLACK);
    DrawText("Press 'R' to restart!", 280, 290, 20, BLACK);
  }
}
