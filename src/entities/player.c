#include "player.h"

#include "../game.h"
#include "../nob.h"
#include <raymath.h>

static float speed = 115.0f;
static float max_velocity = 6.5f;
static float friction = 0.3f;

void player_init(entity_t *e) { (void)e; }

void player_update(entity_t *e) {
  const Vector2 delta =
      Vector2Normalize((Vector2){(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) -
                                     (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)),
                                 (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) -
                                     (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))});
  e->velocity =
      Vector2Add(e->velocity, Vector2Scale(delta, speed * GetFrameTime()));
  e->velocity = Vector2Scale(e->velocity, 1.0f - friction);
  e->velocity =
      Vector2Clamp(e->velocity, (Vector2){-max_velocity, -max_velocity},
                   (Vector2){max_velocity, max_velocity});
  move_and_collide(e);
}

void player_collide(entity_t *e, entity_t *other, Rectangle rectangle) {
  (void)e;
  if (other->type == E_ENEMY &&
      (rectangle.width > 8.0f || rectangle.height > 8.0f))
    game_over();
}

void player_draw(entity_t *e) {
  const entity_data_t *e_data = get_entity_data(e->type);
  const entity_cache_t *e_cache = get_entity_cache(e->type);
  DrawTexturePro(e_cache->resources[0].texture_data.texture,
                 e_data->resources[0].texture_info.texture_rectangle,
                 REC_FROM_2_VEC2(e->position, e_data->size), (Vector2){0}, 0.0f,
                 WHITE);
}

void player_free(entity_t *e) { (void)e; }
