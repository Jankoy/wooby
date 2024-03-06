#ifndef ENTITY_H_
#define ENTITY_H_

#include <raylib.h>
#include <stddef.h>
#include <stdint.h>

Vector2 vector2_from_rectangle(Rectangle r);

typedef enum {
  PLAYER,
  ENEMY,
} entity_type_t;

typedef size_t entity_id_t;

typedef struct {
  entity_id_t id;
  entity_type_t type;
  Rectangle rectangle;
  Vector2 velocity;
  float rotation;
  Color tint;
} entity_t;

typedef void (*entity_behavior_t)(entity_t *);

typedef struct {
  const char *texture_path;
  const Rectangle texture_rectangle;
  const Vector2 size;
  const entity_behavior_t behavior;
} entity_data_t;

size_t find_entity_from_id(entity_id_t id);
size_t find_entity_from_type(entity_type_t type);

entity_id_t spawn_entity(entity_type_t type, Vector2 position);
void free_entity(entity_id_t id);

entity_t *get_entity(size_t index);

bool are_entities_updating();
void start_updating_entities();
void stop_updating_entities();

void update_entities();
void draw_entities();

#endif // ENTITY_H_
