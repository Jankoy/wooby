#include "enemy.h"
#include <raymath.h>

#include "player.h"

void enemy_collide(entity_t *e, entity_t *o, Rectangle collision_rectangle) {
  (void)e;
  (void)collision_rectangle;

  if (o->type == PLAYER)
    player_death();
}

static float speed = 80.0f;
static float max_velocity = 5.5f;
static float friction = 0.475f;

void enemy_update(entity_t *e) {
  entity_t *p = get_entity(find_entity_from_type(PLAYER));
  const Vector2 delta =
      Vector2Normalize(Vector2Subtract(p->position, e->position));
  e->velocity =
      Vector2Add(e->velocity, Vector2Scale(delta, speed * GetFrameTime()));
  e->velocity =
      Vector2Clamp(e->velocity, (Vector2){-max_velocity, -max_velocity},
                   (Vector2){max_velocity, max_velocity});
  if (!Vector2Equals(e->velocity, Vector2Zero())) {
    move_and_collide(e);
    e->velocity = Vector2Scale(e->velocity, 1.0f - friction);
  }
}
