#include "entity.h"
#include "data.h"
#include "nob.h"
#include <raymath.h>

#include "entities/enemy.h"
#include "entities/player.h"

static const entity_data_t data_lookup[] = {
    [E_PLAYER] =
        {
            .vtable =
                {
                    .init = player_init,
                    .update = player_update,
                    .collide = player_collide,
                    .draw = player_draw,
                    .free = player_free,
                },
            .size = {56.0f, 56.0f},
            .resources =
                {
                    {
                        .type = RES_TEXTURE,
                        .texture_info =
                            {
                                .texture_path = "resources/player.png",
                                .texture_rectangle = {0.0f, 0.0f, 256.0f,
                                                      256.0f},
                                .texture_filter = TEXTURE_FILTER_ANISOTROPIC_8X,
                            },
                    },
                },
        },
    [E_ENEMY] =
        {
            .resources =
                {
                    {
                        .type = RES_TEXTURE,
                        .texture_info =
                            {
                                .texture_path = "resources/enemy.png",
                                .texture_rectangle = {0.0f, 0.0f, 256.0f,
                                                      256.0f},
                                .texture_filter = TEXTURE_FILTER_ANISOTROPIC_8X,
                            },
                    },
                },
            .size = {56.0f, 56.0f},
            .vtable =
                {
                    .init = enemy_init,
                    .update = enemy_update,
                    .collide = enemy_collide,
                    .draw = enemy_draw,
                    .free = enemy_free,
                },
        },
};

static entity_cache_t resource_cache[] = {
    [E_PLAYER] = {0},
    [E_ENEMY] = {0},
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
  const entity_data_t e_data = data_lookup[type];

  if (!resource_cache[type].resources_loaded) {
    for (size_t i = 0; i < RES_CAP && e_data.resources[i].type != RES_NULL;
         ++i) {
      switch (e_data.resources[i].type) {
      case RES_TEXTURE: {
        const texture_info_t texture_info = e_data.resources[i].texture_info;
        size_t texture_size;
        void *texture_data =
            load_resource_data(texture_info.texture_path, &texture_size);
        if (!texture_data)
          exit(1);
        Image image =
            LoadImageFromMemory(GetFileExtension(texture_info.texture_path),
                                texture_data, (int)texture_size);
        Texture texture = LoadTextureFromImage(image);
        free_resource_data(texture_data);
        resource_cache[type].resources[i].type = RES_TEXTURE;
        resource_cache[type].resources[i].texture_data.texture = texture;
      } break;
      default:
        break;
      }
    }
    resource_cache[type].resources_loaded = true;
  }

  entity_t e = {
      .id = entities.count + 1,
      .type = type,
      .position = position,
      .velocity = {0},
      .data = NULL,
  };

  e_data.vtable.init(&e);

  nob_da_append(&entities, e);

  return e.id;
}

void free_entity(entity_id_t id) {
  size_t index = find_entity_from_id(id);
  if (index >= entities.count)
    return;
  entity_t *e = &entities.items[index];
  data_lookup[e->type].vtable.free(e);
  for (size_t i = index; i < entities.count - 1; ++i)
    entities.items[i] = entities.items[i + 1];
  --entities.count;
}

const entity_t *get_entity(size_t index) { return entities.items + index; }
const entity_data_t *get_entity_data(entity_type_t type) {
  return &data_lookup[type];
}
const entity_cache_t *get_entity_cache(entity_type_t type) {
  return &resource_cache[type];
}

typedef struct {
  entity_t *e;
  Vector2 dst;
} entity_move_t;

static struct {
  entity_move_t *items;
  size_t count;
  size_t capacity;
} entity_moves = {0};

void move_and_collide(entity_t *e) {
  nob_da_append(
      &entity_moves,
      ((entity_move_t){.e = e, .dst = Vector2Add(e->position, e->velocity)}));
}

static void resolve_moves() {
  for (entity_move_t *move = entity_moves.items + entity_moves.count - 1;
       move >= entity_moves.items; --move) {
    move->e->position = move->dst;
    const entity_data_t e_data = data_lookup[move->e->type];
    const Rectangle e_rectangle =
        REC_FROM_2_VEC2(move->e->position, e_data.size);
    for (size_t i = 0; i < entities.count; ++i) {
      if (&entities.items[i] == move->e)
        continue;
      const entity_data_t other_data = data_lookup[entities.items[i].type];
      const Rectangle other_rectangle =
          REC_FROM_2_VEC2(entities.items[i].position, other_data.size);
      if (CheckCollisionRecs(e_rectangle, other_rectangle)) {
        const Rectangle collision_rectangle =
            GetCollisionRec(e_rectangle, other_rectangle);

        e_data.vtable.collide(move->e, &entities.items[i], collision_rectangle);
        other_data.vtable.collide(&entities.items[i], move->e,
                                  collision_rectangle);

        enum { LEFT, RIGHT, TOP, BOTTOM } direction = LEFT;
        float correction = collision_rectangle.width;

        if (collision_rectangle.x <= move->e->position.x)
          direction = RIGHT;

        if (collision_rectangle.height < correction) {
          correction = collision_rectangle.height;
          if (collision_rectangle.y > move->e->position.y)
            direction = TOP;
          else if (collision_rectangle.y <= move->e->position.y)
            direction = BOTTOM;
        }

        switch (direction) {
        case LEFT:
          move->e->position.x -= correction;
          break;
        case RIGHT:
          move->e->position.x += correction;
          break;
        case TOP:
          move->e->position.y -= correction;
          break;
        case BOTTOM:
          move->e->position.y += correction;
          break;
        }
      }
    }
    entity_moves.count = 0;
  }
}

void update_entities() {
  for (entity_t *e = entities.items;
       (size_t)(e - entities.items) < entities.count; ++e)
    data_lookup[e->type].vtable.update(e);
  resolve_moves();
}

void draw_entities() {
  for (entity_t *e = entities.items; e < entities.items + entities.count; ++e) {
    entity_data_t e_data = data_lookup[e->type];
    e_data.vtable.draw(e);
  }
}
