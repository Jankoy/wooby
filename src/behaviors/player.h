#ifndef PLAYER_H_
#define PLAYER_H_

#include "../entity.h"

void player_collision(entity_t *e, entity_t *o, Rectangle collision_rectangle);
void player_update(entity_t *e);

#endif // PLAYER_H_
