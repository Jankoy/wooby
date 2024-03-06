#include "player.h"
#include <raymath.h>

void player_collision(entity_t *e, entity_type_t type,
                      Rectangle collision_rectangle) {
  const float area = e->rectangle.width * e->rectangle.height;
  const float collision_area =
      collision_rectangle.width * collision_rectangle.height;

  if (collision_area / area >= 0.14f && type == ENEMY)
    stop_updating_entities();
}

static float speed = 180.0f;

void player_behavior(entity_t *e) {
  const Vector2 delta =
      Vector2Normalize((Vector2){IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT),
                                 IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)});
  const Vector2 pos = Vector2Add(vector2_from_rectangle(e->rectangle),
                                 Vector2Scale(delta, speed * GetFrameTime()));
  e->rectangle =
      (Rectangle){pos.x, pos.y, e->rectangle.width, e->rectangle.height};
}
