#include "entity.h"
#include "data.h"
#include "nob.h"
#include <raymath.h>

Vector2 vector2_from_rectangle(Rectangle r) { return (Vector2){r.x, r.y}; }

#include "behaviors/enemy.h"
#include "behaviors/player.h"

static entity_data_t entity_data_lookup_table[] = {
    [PLAYER] =
        {
            .texture_path = "resources/player.png",
            .texture_rectangle = {0.0f, 0.0f, 256.0f, 256.0f},
            .size = {64.0f, 64.0f},
            .behavior = player_behavior,
            .collision = player_collision,
        },
    [ENEMY] =
        {
            .texture_path = "resources/enemy.png",
            .texture_rectangle = {0.0f, 0.0f, 256.0f, 256.0f},
            .size = {64.0f, 64.0f},
            .behavior = enemy_behavior,
            .collision = NULL,
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

size_t find_entity_from_id(entity_id_t id) {
  size_t index = entities.count;
  for (size_t i = 0; i < entities.count; ++i)
    if (entities.items[i].id == id)
      index = i;
  return index;
}

size_t find_entity_from_type(entity_type_t type) {
  size_t index = entities.count;
  for (size_t i = 0; i < entities.count; ++i)
    if (entities.items[i].type == type)
      index = i;
  return index;
}

entity_id_t spawn_entity(entity_type_t type, Vector2 position) {
  entity_data_t e_data = entity_data_lookup_table[type];

  if (!entity_resource_cache[type].texture_loaded) {
    size_t texture_size;
    void *texture_data = load_resource_data(e_data.texture_path, &texture_size);
    if (!texture_data)
      exit(1);
    Image image = LoadImageFromMemory(GetFileExtension(e_data.texture_path),
                                      texture_data, (int)texture_size);
    Texture texture = LoadTextureFromImage(image);
    free_resource_data(texture_data);
    entity_resource_cache[type].texture_loaded = true;
    entity_resource_cache[type].texture = texture;
  }

  entity_t e = {
      .id = entities.count + 1,
      .type = type,
      .rectangle = {.x = position.x,
                    .y = position.y,
                    .width = e_data.size.x,
                    .height = e_data.size.y},
      .velocity = {0},
      .rotation = 0.0f,
      .tint = WHITE,
  };

  nob_da_append(&entities, e);

  return e.id;
}

void free_entity(entity_id_t id) {
  size_t index = find_entity_from_id(id);
  if (index >= entities.count)
    return;
  for (size_t i = index; i < entities.count - 1; ++i)
    entities.items[i] = entities.items[i + 1];
  entities.count -= 1;
}

entity_t *get_entity(size_t index) { return entities.items + index; }

static bool entities_should_update = true;

bool are_entities_updating() { return entities_should_update; }
void start_updating_entities() { entities_should_update = true; }
void stop_updating_entities() { entities_should_update = false; }

void update_entities() {
  if (entities_should_update) {
    for (entity_t *e = entities.items;
         (size_t)(e - entities.items) < entities.count; ++e) {
      entity_behavior_t behavior = entity_data_lookup_table[e->type].behavior;
      if (behavior)
        behavior(e);
    }
  }
}

void draw_entities() {
  entity_t *p = NULL;
  for (entity_t *e = entities.items; e < entities.items + entities.count; ++e) {
    if (e->type == PLAYER) {
      p = e;
      continue;
    }
    entity_data_t e_data = entity_data_lookup_table[e->type];
    DrawTexturePro(entity_resource_cache[e->type].texture,
                   e_data.texture_rectangle, e->rectangle, (Vector2){0},
                   e->rotation, e->tint);
  }
  if (!p) {
    nob_log(NOB_ERROR, "Player could not be found");
    exit(1);
  }
  entity_data_t p_data = entity_data_lookup_table[p->type];
  DrawTexturePro(entity_resource_cache[p->type].texture,
                 p_data.texture_rectangle, p->rectangle, (Vector2){0},
                 p->rotation, p->tint);
}
