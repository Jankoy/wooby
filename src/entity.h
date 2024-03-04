#ifndef ENTITY_H_
#define ENTITY_H_

#include "types.h"
#include <raylib.h>

typedef enum {
  PLAYER,
  ENEMY,
} entity_type_t;

typedef size_t entity_id_t;

typedef struct {
  entity_id_t id;
  entity_type_t type;
  Vector2 position;
  Vector2 velocity;
  float rotation;
  float scale;
  Color tint;
} entity_t;

typedef struct {
  const char *texture_path;
  const char *behavior_path;
} entity_data_t;

entity_id_t spawn_entity(entity_type_t type, Vector2 position, float scale);
void free_entity(entity_id_t id);

void update_entities();
void draw_entities();

#endif // ENTITY_H_
