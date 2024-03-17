#ifndef ENEMY_H_
#define ENEMY_H_

#include "../entity.h"

void enemy_collide(entity_t *e, entity_t *o, Rectangle collision_rectangle);
void enemy_update(entity_t *e);

#endif // ENEMY_H_
