#include "entity.h"
#include "data.h"
#include "nob.h"
#include <raymath.h>

#include "behaviors/enemy.h"
#include "behaviors/player.h"

static entity_behavior_t player_behaviors[] = {
    {
        .type = BEHAVIOR_UPDATE,
        .func = player_update,
        .is_active = true,
    },
    {
        .type = BEHAVIOR_COLLIDE,
        .func = player_collision,
        .is_active = true,
    },
};

static entity_behavior_t enemy_behaviors[] = {
    {
        .type = BEHAVIOR_UPDATE,
        .func = enemy_update,
        .is_active = true,
    },
};

static entity_data_t data_lookup[] = {
    [PLAYER] =
        {
            .texture_path = "resources/player.png",
            .texture_rectangle = {0.0f, 0.0f, 256.0f, 256.0f},
            .size = {64.0f, 64.0f},
            .behaviors = player_behaviors,
            .behavior_count = NOB_ARRAY_LEN(player_behaviors),
        },
    [ENEMY] =
        {
            .texture_path = "resources/enemy.png",
            .texture_rectangle = {0.0f, 0.0f, 256.0f, 256.0f},
            .size = {64.0f, 64.0f},
            .behaviors = enemy_behaviors,
            .behavior_count = NOB_ARRAY_LEN(enemy_behaviors),
        },
};

typedef struct {
  bool texture_loaded;
  Texture texture;
} entity_cache_t;

static entity_cache_t resource_cache[] = {
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
  entity_data_t e_data = data_lookup[type];

  if (!resource_cache[type].texture_loaded) {
    size_t texture_size;
    void *texture_data = load_resource_data(e_data.texture_path, &texture_size);
    if (!texture_data)
      exit(1);
    Image image = LoadImageFromMemory(GetFileExtension(e_data.texture_path),
                                      texture_data, (int)texture_size);
    Texture texture = LoadTextureFromImage(image);
    free_resource_data(texture_data);
    resource_cache[type].texture_loaded = true;
    resource_cache[type].texture = texture;
  }

  entity_t e = {
      .id = entities.count + 1,
      .type = type,
      .position = position,
      .size = {e_data.size.x, e_data.size.y},
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
entity_data_t get_entity_data(entity_type_t type) { return data_lookup[type]; }

static bool entities_should_update = true;

bool are_entities_updating() { return entities_should_update; }
void start_updating_entities() { entities_should_update = true; }
void stop_updating_entities() { entities_should_update = false; }

static const entity_behavior_t *
get_behavior_if_exists(entity_type_t type, entity_behavior_type_t behavior) {
  for (const entity_behavior_t *b = data_lookup[type].behaviors;
       b < data_lookup[type].behaviors + data_lookup[type].behavior_count; ++b)
    if (b->type == behavior)
      return b;
  return NULL;
}

void move_and_collide(entity_t *e) {
  e->position = Vector2Add(e->position, e->velocity);
  const Rectangle e_rectangle = REC_FROM_2_VEC2(e->position, e->size);
  const entity_behavior_t *collide_behavior =
      get_behavior_if_exists(e->type, BEHAVIOR_COLLIDE);
  if (collide_behavior && collide_behavior->is_active) {
    for (size_t i = 0; i < entities.count; ++i) {
      if (&entities.items[i] == e)
        continue;
      const Rectangle other_rectangle =
          REC_FROM_2_VEC2(entities.items[i].position, entities.items[i].size);
      if (CheckCollisionRecs(e_rectangle, other_rectangle)) {
        ((entity_collide_behavior_t)collide_behavior->func)(
            e, &entities.items[i],
            GetCollisionRec(REC_FROM_2_VEC2(e->position, e->size),
                            REC_FROM_2_VEC2(entities.items[i].position,
                                            entities.items[i].size)));
      }
    }
  }
}

void update_entities() {
  if (entities_should_update) {
    for (entity_t *e = entities.items;
         (size_t)(e - entities.items) < entities.count; ++e) {
      const entity_behavior_t *update_behavior =
          get_behavior_if_exists(e->type, BEHAVIOR_UPDATE);
      if (update_behavior)
        if (update_behavior->is_active)
          ((entity_update_behavior_t)update_behavior->func)(e);
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
    entity_data_t e_data = data_lookup[e->type];
    DrawTexturePro(resource_cache[e->type].texture, e_data.texture_rectangle,
                   REC_FROM_2_VEC2(e->position, e->size), (Vector2){0},
                   e->rotation, e->tint);
  }
  if (!p) {
    nob_log(NOB_ERROR, "Player could not be found");
    exit(1);
  }
  entity_data_t p_data = data_lookup[p->type];
  DrawTexturePro(resource_cache[p->type].texture, p_data.texture_rectangle,
                 REC_FROM_2_VEC2(p->position, p->size), (Vector2){0},
                 p->rotation, p->tint);
}
