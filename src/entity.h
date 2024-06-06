#ifndef ENTITY_H_
#define ENTITY_H_

#include <raylib.h>
#include <stddef.h>
#include <stdint.h>

#define VEC2_FROM_REC_1(r) ((Vector2){(r).x, (r).y})
#define VEC2_FROM_REC_2(r) ((Vector2){(r).width, (r).height})
#define REC_FROM_2_VEC2(v1, v2) ((Rectangle){(v1).x, (v1).y, (v2).x, (v2).y})

typedef size_t entity_id_t;
typedef enum { E_PLAYER, E_ENEMY } entity_type_t;

typedef struct {
  entity_id_t id;
  entity_type_t type;
  Vector2 position, velocity;
  void *data;
} entity_t;

typedef struct {
  void (*init)(entity_t *);
  void (*update)(entity_t *);
  void (*collide)(entity_t *, entity_t *, Rectangle);
  void (*draw)(entity_t *);
  void (*free)(entity_t *);
} entity_vtable_t;

typedef enum { RES_NULL, RES_TEXTURE } resource_type_t;

typedef struct {
  const char *texture_path;
  const Rectangle texture_rectangle;
  const TextureFilter texture_filter;
} texture_info_t;

typedef struct {
  resource_type_t type;
  union {
    texture_info_t texture_info;
  };
} resource_info_t;

#define RES_CAP 8

typedef struct {
  const entity_vtable_t vtable;
  const Vector2 size;
  const resource_info_t resources[RES_CAP];
} entity_data_t;

typedef struct {
  Texture texture;
} texture_resource_data_t;

typedef struct {
  resource_type_t type;
  union {
    texture_resource_data_t texture_data;
  };
} resource_data_t;

typedef struct {
  bool resources_loaded;
  resource_data_t resources[RES_CAP];
} entity_cache_t;

size_t find_entity_from_id(entity_id_t id);
size_t find_entity_from_type(entity_type_t type);

entity_id_t spawn_entity(entity_type_t type, Vector2 position);
void free_entity(entity_id_t id);

const entity_t *get_entity(size_t index);
const entity_data_t *get_entity_data(entity_type_t type);
const entity_cache_t *get_entity_cache(entity_type_t type);

void move_and_collide(entity_t *e);

void update_entities();
void draw_entities();

#endif // ENTITY_H_
