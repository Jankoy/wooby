#ifndef PLAYER_H_
#define PLAYER_H_

#include "../entity.h"

void player_collision(entity_t *e, Rectangle collision_rectangle);
void player_behavior(entity_t *e);

#endif // PLAYER_H_
