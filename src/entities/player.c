#include "player.h"

#include <raymath.h>
#include "../game.h"
#include "../nob.h"

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

void player_death(void) { game_over(); }
