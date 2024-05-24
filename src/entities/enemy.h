#ifndef ENEMY_H_
#define ENEMY_H_

#include "../entity.h"

void enemy_init(entity_t *e);
void enemy_update(entity_t *e);
void enemy_collide(entity_t *e, entity_t *other, Rectangle collision_rectangle);
void enemy_draw(entity_t *e);
void enemy_free(entity_t *e);

#endif // ENEMY_H_
