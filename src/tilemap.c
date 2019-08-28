#include "game.h"
#include "internal.h"
#include "tmx/tmx.h"

tmx_resource_manager *rcm;

typedef struct RPGtilemap RPGtilemap;

typedef struct RPGtilemap {
    RPGrenderable base;
    tmx_map *map;
    void *user;
} RPGtilemap;

static void RPG_Tilemap_Render(void *tilemap) {

}

static void RPG_Tilemap_Init(tmx_map *map, RPGviewport *viewport, RPGtilemap **tilemap) {
    RPG_ALLOC_ZERO(t, RPGtilemap);
    RPGbatch *batch = viewport ? &viewport->batch : &RPG_GAME->batch;
    RPG_Renderable_Init(&t->base, RPG_Tilemap_Render, batch);

    t->map = map;


    *tilemap = t;
}


RPG_RESULT RPG_Tilemap_Create(const void *buffer, RPGsize size, RPGviewport *viewport, RPGtilemap **tilemap) {
    if (rcm == NULL) {
        rcm = tmx_make_resource_manager();
    }
    tmx_map *map = tmx_rcmgr_load_buffer(rcm, buffer, (int) size);
    if (map == NULL) {

        // TODO:
    }
    RPG_Tilemap_Init(map, viewport, tilemap);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_CreateFromFile(const char *path, RPGviewport *viewport, RPGtilemap **tilemap) {
    if (rcm == NULL) {
        rcm = tmx_make_resource_manager();
    }
    tmx_map *map = tmx_rcmgr_load(rcm, path);
    if (map == NULL) {

        // TODO:
    }
    RPG_Tilemap_Init(map, viewport, tilemap);
    return RPG_NO_ERROR;
}