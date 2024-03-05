#include "entity.h"
#include "data.h"
#include "nob.h"
#include <raymath.h>

static entity_data_t entity_data_lookup_table[] = {
    [PLAYER] =
        {
            .texture_path = "resources/assets/player.png",
            .behavior = NULL,
        },
    [ENEMY] =
        {
            .texture_path = "resources/assets/enemy.png",
            .behavior = NULL,
        },
};

typedef struct {
  bool texture_loaded;
  Texture texture;
} entity_cache_t;

static entity_cache_t entity_resource_cache[] = {
    [PLAYER] = {0},
    [ENEMY] = {0},
};

static struct {
  entity_t *items;
  size_t count;
  size_t capacity;
} entities = {0};

entity_id_t spawn_entity(entity_type_t type, Vector2 position, float scale) {
  entity_data_t e_data = entity_data_lookup_table[type];

  if (!entity_resource_cache[type].texture_loaded) {
    size_t texture_size;
    void *texture_data = load_resource(e_data.texture_path, &texture_size);
    if (!texture_data)
      exit(1);
    Image image = LoadImageFromMemory(GetFileExtension(e_data.texture_path),
                                      texture_data, (int)texture_size);
    Texture texture = LoadTextureFromImage(image);
    free_resource(texture_data);
    entity_resource_cache[type].texture_loaded = true;
    entity_resource_cache[type].texture = texture;
  }

  entity_t e = {
      .id = entities.count + 1,
      .type = type,
      .position = position,
      .velocity = {0},
      .rotation = 0.0f,
      .scale = scale,
      .tint = WHITE,
  };

  nob_da_append(&entities, e);

  return e.id;
}

static size_t find_entity_from_id(entity_id_t id) {
  size_t index = entities.count;
  for (size_t i = 0; i < entities.count; ++i)
    if (entities.items[i].id == id)
      index = i;
  return index;
}

static size_t find_entity_from_type(entity_type_t type) {
  size_t index = entities.count;
  for (size_t i = 0; i < entities.count; ++i)
    if (entities.items[i].type == type)
      index = i;
  return index;
}

void free_entity(entity_id_t id) {
  size_t index = find_entity_from_id(id);
  if (index >= entities.count)
    return;
  for (size_t i = index; i < entities.count - 1; ++i)
    entities.items[i] = entities.items[i + 1];
  entities.count -= 1;
}

void update_entities() {
  for (entity_t *e = entities.items;
       (size_t)(e - entities.items) < entities.count; ++e) {
    entity_behavior_t behavior = entity_data_lookup_table[e->type].behavior;
    if (behavior)
      behavior(e);
  }
}

void draw_entities() {
  entity_t *player = NULL;
  for (entity_t *e = entities.items; e < entities.items + entities.count; ++e) {
    if (e->type == PLAYER) {
      player = e;
      continue;
    }
    DrawTextureEx(entity_resource_cache[e->type].texture, e->position,
                  e->rotation, e->scale, e->tint);
  }
  if (!player) {
    nob_log(NOB_ERROR, "Player could not be found");
    exit(1);
  }
  DrawTextureEx(entity_resource_cache[player->type].texture, player->position,
                player->rotation, player->scale, player->tint);
}
