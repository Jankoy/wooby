#include "enemy.h"
#include <raymath.h>

#include "player.h"

static float speed = 80.0f;

void enemy_behavior(entity_t *e) {
  entity_t *p = get_entity(find_entity_from_type(PLAYER));
  const Vector2 delta =
      Vector2Normalize(Vector2Subtract(vector2_from_rectangle(p->rectangle),
                                       vector2_from_rectangle(e->rectangle)));
  const Vector2 pos = Vector2Add(vector2_from_rectangle(e->rectangle),
                                 Vector2Scale(delta, speed * GetFrameTime()));
  e->rectangle =
      (Rectangle){pos.x, pos.y, e->rectangle.width, e->rectangle.height};

  if (CheckCollisionRecs(e->rectangle, p->rectangle))
    player_collision(p, e->type, GetCollisionRec(e->rectangle, p->rectangle));
}
