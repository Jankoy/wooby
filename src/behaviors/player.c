#include "player.h"
#include <raymath.h>

#include "../nob.h"

void player_collision(entity_t *e, entity_t *o, Rectangle collision_rectangle) {
  (void)e;
  (void)collision_rectangle;
  if (o->type == ENEMY)
    stop_updating_entities();
}

static float speed = 135.0f;
static float max_velocity = 6.5f;
static float friction = 0.475f;

void player_update(entity_t *e) {
  const Vector2 delta =
      Vector2Normalize((Vector2){IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT),
                                 IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)});
  e->velocity =
      Vector2Add(e->velocity, Vector2Scale(delta, speed * GetFrameTime()));
  e->velocity =
      Vector2Clamp(e->velocity, (Vector2){-max_velocity, -max_velocity},
                   (Vector2){max_velocity, max_velocity});
  move_and_collide(e);
  e->velocity = Vector2Scale(e->velocity, 1.0f - friction);
}
