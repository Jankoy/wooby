#include "entity.h"
#include "data.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "nob.h"
#include <raymath.h>

static bool check_lua(lua_State *L, int r) {
  if (r != LUA_OK) {
    fprintf(stderr, "[LUA ERROR]: %s\n", lua_tostring(L, -1));
    return false;
  }
  return true;
}

static entity_data_t entity_data_lookup_table[] = {
    [PLAYER] =
        {
            .texture_path = "resources/assets/player.png",
            .behavior_path = NULL,
        },
    [ENEMY] =
        {
            .texture_path = "resources/assets/enemy.png",
            .behavior_path = "resources/scripts/enemy.lua",
        },
};

typedef struct {
  bool texture_loaded;
  Texture texture;
  bool behavior_loaded;
  lua_State *L;
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

  if (!entity_resource_cache[type].behavior_loaded) {
    if (e_data.behavior_path) {
      size_t behavior_size;
      void *behavior_data = load_resource(e_data.behavior_path, &behavior_size);
      if (!behavior_data)
        exit(1);
      lua_State *L = luaL_newstate();
      luaL_openlibs(L);
      if (!check_lua(L, luaL_dostring(L, behavior_data)))
        exit(1);
      free_resource(behavior_data);
      entity_resource_cache[type].behavior_loaded = true;
      entity_resource_cache[type].L = L;
    }
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

static void push_entity(lua_State *L, entity_t *e) {
  lua_newtable(L);
  lua_pushinteger(L, e->id);
  lua_setfield(L, -2, "id");
  lua_pushinteger(L, e->type);
  lua_setfield(L, -2, "type");
  lua_newtable(L);
  lua_pushnumber(L, e->position.x);
  lua_setfield(L, -2, "x");
  lua_pushnumber(L, e->position.y);
  lua_setfield(L, -2, "y");
  lua_setfield(L, -2, "position");
  lua_newtable(L);
  lua_pushnumber(L, e->velocity.x);
  lua_setfield(L, -2, "x");
  lua_pushnumber(L, e->velocity.y);
  lua_setfield(L, -2, "y");
  lua_setfield(L, -2, "velocity");
  lua_pushnumber(L, e->rotation);
  lua_setfield(L, -2, "rotation");
  lua_pushnumber(L, e->scale);
  lua_setfield(L, -2, "scale");
  lua_newtable(L);
  lua_pushinteger(L, e->tint.r);
  lua_setfield(L, -2, "r");
  lua_pushinteger(L, e->tint.g);
  lua_setfield(L, -2, "g");
  lua_pushinteger(L, e->tint.b);
  lua_setfield(L, -2, "b");
  lua_pushinteger(L, e->tint.a);
  lua_setfield(L, -2, "a");
  lua_setfield(L, -2, "tint");
}

static entity_t to_entity(lua_State *L, int index) {
  entity_t e = {0};
  lua_getfield(L, index, "id");
  e.id = lua_tointeger(L, -1);
  lua_pop(L, 1);
  lua_getfield(L, index, "type");
  e.type = lua_tointeger(L, -1);
  lua_pop(L, 1);
  lua_getfield(L, index, "position");
  lua_getfield(L, -1, "x");
  e.position.x = lua_tonumber(L, -1);
  lua_pop(L, 1);
  lua_getfield(L, -1, "y");
  e.position.y = lua_tonumber(L, -1);
  lua_pop(L, 2);
  lua_getfield(L, index, "velocity");
  lua_getfield(L, -1, "x");
  e.velocity.x = lua_tonumber(L, -1);
  lua_pop(L, 1);
  lua_getfield(L, -1, "y");
  e.velocity.y = lua_tonumber(L, -1);
  lua_pop(L, 2);
  lua_getfield(L, index, "rotation");
  e.rotation = lua_tonumber(L, -1);
  lua_pop(L, 1);
  lua_getfield(L, index, "scale");
  e.scale = lua_tonumber(L, -1);
  lua_pop(L, 1);
  lua_getfield(L, index, "tint");
  lua_getfield(L, -1, "r");
  e.tint.r = lua_tointeger(L, -1);
  lua_pop(L, 1);
  lua_getfield(L, -1, "g");
  e.tint.g = lua_tointeger(L, -1);
  lua_pop(L, 1);
  lua_getfield(L, -1, "b");
  e.tint.b = lua_tointeger(L, -1);
  lua_pop(L, 1);
  lua_getfield(L, -1, "a");
  e.tint.a = lua_tointeger(L, -1);
  lua_pop(L, 2);
  return e;
}

void update_entities() {
  for (entity_t *e = entities.items;
       (size_t)(e - entities.items) < entities.count; ++e) {
    if (e->type == PLAYER) {
      const Vector2 delta =
          Vector2Normalize((Vector2){IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT),
                                     IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)});
      e->position =
          Vector2Add(e->position, Vector2Scale(delta, 180.0f * GetFrameTime()));
    } else {
      lua_State *L = entity_resource_cache[e->type].L;
      lua_getglobal(L, "_update");
      if (!lua_isfunction(L, -1)) {
        fprintf(stderr, "[LUA ERROR]: Global \"_update\" must be a function\n");
        exit(1);
      }
      push_entity(L, e);
      if (!check_lua(L, lua_pcall(L, 1, 1, 0)))
        exit(1);
      *e = to_entity(L, -1);
      lua_pop(L, 1);
    }
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
