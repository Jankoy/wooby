#ifndef PLAYER_H_
#define PLAYER_H_

#include "../entity.h"

void player_init(entity_t *e);
void player_update(entity_t *e);
void player_collide(entity_t *e, entity_t *other, Rectangle rectangle);
void player_draw(entity_t *e);
void player_free(entity_t *e);

#endif // PLAYER_H_
