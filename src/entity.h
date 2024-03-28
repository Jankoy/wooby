#ifndef ENTITY_H_
#define ENTITY_H_

#include <raylib.h>
#include <stddef.h>
#include <stdint.h>

#define VEC2_FROM_REC_1(r) ((Vector2){(r).x, (r).y})
#define VEC2_FROM_REC_2(r) ((Vector2){(r).width, (r).height})
#define REC_FROM_2_VEC2(v1, v2) ((Rectangle){(v1).x, (v1).y, (v2).x, (v2).y})

typedef size_t entity_id_t;
typedef enum { PLAYER, ENEMY } entity_type_t;

typedef struct {
  entity_id_t id;
  entity_type_t type;
  Vector2 position;
  Vector2 size;
  Vector2 velocity;
  float rotation;
  Color tint;
} entity_t;

typedef enum { BEHAVIOR_UPDATE, BEHAVIOR_COLLIDE } entity_behavior_type_t;

typedef void (*entity_update_behavior_t)(entity_t *);
typedef void (*entity_collide_behavior_t)(entity_t *, entity_t *, Rectangle);

typedef struct {
  entity_behavior_type_t type;
  void *func;
  bool is_active;
} entity_behavior_t;

typedef struct {
  const char *texture_path;
  const Rectangle texture_rectangle;
  const Vector2 size;
  const entity_behavior_t *behaviors;
  const size_t behavior_count;
} entity_data_t;

size_t find_entity_from_id(entity_id_t id);
size_t find_entity_from_type(entity_type_t type);

entity_id_t spawn_entity(entity_type_t type, Vector2 position);
void free_entity(entity_id_t id);

entity_t *get_entity(size_t index);
entity_data_t get_entity_data(entity_type_t type);

void move_and_collide(entity_t *e);

void update_entities();
void draw_entities();

#endif // ENTITY_H_
