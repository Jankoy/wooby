#include "enemy.h"
#include <raymath.h>

void enemy_init(entity_t *e) { (void)e; }

static float speed = 90.0f;
static float max_velocity = 6.5f;
static float friction = 0.475f;

void enemy_update(entity_t *e) {
  const entity_t *p = get_entity(find_entity_from_type(E_PLAYER));
  const Vector2 delta =
      Vector2Normalize(Vector2Subtract(p->position, e->position));
  e->velocity =
      Vector2Add(e->velocity, Vector2Scale(delta, speed * GetFrameTime()));
  e->velocity = Vector2Scale(e->velocity, 1.0f - friction);
  e->velocity =
      Vector2Clamp(e->velocity, (Vector2){-max_velocity, -max_velocity},
                   (Vector2){max_velocity, max_velocity});
  move_and_collide(e);
}

void enemy_collide(entity_t *e, entity_t *other,
                   Rectangle collision_rectangle) {
  (void)e;
  (void)other;
  (void)collision_rectangle;
}

void enemy_draw(entity_t *e) {
  const entity_data_t *e_data = get_entity_data(e->type);
  const entity_cache_t *e_cache = get_entity_cache(e->type);
  DrawTexturePro(e_cache->resources[0].texture_data.texture,
                 e_data->resources[0].texture_info.texture_rectangle,
                 REC_FROM_2_VEC2(e->position, e_data->size), (Vector2){0}, 0.0f,
                 WHITE);
}

void enemy_free(entity_t *e) { (void)e; }
