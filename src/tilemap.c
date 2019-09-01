#ifndef MEH

/*


*/


#include "internal.h"
#include "tmx/tmx.h"
#include <stdio.h>

tmx_resource_manager *cache;

typedef struct {
    GLuint vao;
    GLuint vbo;
} RPGtile;

// Tile Layer
typedef struct {
    RPGimage *image;
    RPGuint tileCount;
    RPGtile *tiles;
} RPGtilelayer;

// Image Layer
typedef struct {
    int stuff;
} RPGimagelayer;

// Object Layer
typedef struct {
    int stuff;
} RPGobjectlayer;

// Group Layer
typedef struct {
    int stuff;
} RPGgrouplayer;

typedef struct {
    RPGint type;
    union {
        RPGtilelayer *tile;
        RPGimagelayer *image;
        RPGobjectlayer *object;
        RPGgrouplayer *group;
    } layer;
} RPGlayer;

typedef struct RPGtilemap {
    RPGrenderable base;
    RPGmat4 projection;
    tmx_map *map;
    RPGuint layerCount;
    RPGlayer *layers;
    struct {
        RPGuint program;
        RPGint projection;
        RPGint model;
    } shader;
    void *user;
} RPGtilemap;




static void RPG_Tilemap_CreateShader(RPGtilemap *tilemap) {

    RPGshader *shader;

    size_t s;
    char *tempv, *tempf;
    RPG_ReadFile("/home/eric/Desktop/rpg/tilemap.vert", &tempv, &s);
    RPG_ReadFile("/home/eric/Desktop/rpg/tilemap.frag", &tempf, &s);

    RPG_RESULT r = RPG_Shader_Create(tempv, tempf, NULL, &shader);


    RPG_ASSERT(r == RPG_NO_ERROR);
    tilemap->shader.program = shader->program; 
    RPG_FREE(shader);
    if (tempf != NULL) RPG_FREE(tempf); 
    if (tempv != NULL) RPG_FREE(tempv); 
}

static void RPG_Tilemap_Render(void *tilemap) {
    RPGtilemap *t = tilemap;
    if (!t->base.visible || t->base.alpha < __FLT_EPSILON__) {
        return;
    }

    glUseProgram(t->shader.program);
    // glUniform4f(glGetUniformLocation(t->shader.program, "color"), 0.0f, 0.0f, 0.0f, 0.0f);
    // glUniform4f(glGetUniformLocation(t->shader.program, "tone"), 0.0f, 0.0f, 0.0f, 0.0f);
    // glUniform1f(glGetUniformLocation(t->shader.program, "alpha"), 1.0f);
    // glUniform1f(glGetUniformLocation(t->shader.program, "hue"), 0.0f);
    // glUniform4f(glGetUniformLocation(t->shader.program, "flash"), 0.0f, 0.0f, 0.0f, 0.0f);
    
    glUniformMatrix4fv(glGetUniformLocation(t->shader.program, "projection"), 1, GL_FALSE, (GLfloat *) &RPG_GAME->projection);
    glBlendEquation(t->base.blend.op);
    glBlendFunc(t->base.blend.src, t->base.blend.dst);

    for (RPGuint i = 0; i < t->layerCount; i++) {
        RPGlayer *layer = &t->layers[i];
        if (layer->type != L_LAYER) {
            continue;
        }

        RPGtilelayer *l = layer->layer.tile;

        // glUniformMatrix4fv(glGetUniformLocation(t->shader.program, "model"), 1, GL_FALSE, (GLfloat *) &l->model);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, l->image->texture);
    
        for (RPGuint ti = 0; ti < l->tileCount; ti++) {
            if (l->tiles[ti].vao) {
                glBindVertexArray(l->tiles[ti].vao);
                glDrawArraysInstanced(GL_TRIANGLES, 0, 6, l->tileCount);
            }
                // glBindVertexArray(l->tiles[ti].vao);
                // glDrawArraysInstanced(GL_TRIANGLES, 0, 6, l->tileCount);
        }
    }
    glUseProgram(RPG_GAME->shader.program);
}

static RPGtilelayer *RPG_Tilemap_CreateTileLayer(RPGtilemap *tilemap, tmx_map *map, tmx_layer *layer) {

    RPGtilelayer *tilelayer = RPG_ALLOC(RPGtilelayer);
    tilelayer->tileCount = map->width * map->height;
    tilelayer->image = NULL;
    tilelayer->tiles = RPG_MALLOC(sizeof(RPGtile) * tilelayer->tileCount);

    GLint vLoc = glGetAttribLocation(tilemap->shader.program, "vertex");
    GLint mLoc = glGetAttribLocation(tilemap->shader.program, "model");

    RPGuint index = 0;
    for (RPGuint mapY = 0; mapY < map->height; mapY++) {
        for (RPGuint mapX = 0; mapX < map->width; mapX++, index++) {

            RPGint gid = layer->content.gids[index];
            tmx_tile *tmxtile = map->tiles[gid & TMX_FLIP_BITS_REMOVAL];

            if (gid == 0) {
                memset(&tilelayer->tiles[index], 0, sizeof(RPGtile));
                continue;
            }

            if (tilelayer->image == NULL) {
                tilelayer->image = tmxtile->tileset->image->resource_image;
            } else if (tilelayer->image != tmxtile->tileset->image->resource_image) {
                fprintf(stderr, "only one tileset per layer is supported");
            }

            GLuint vao, vbo;
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, VERTICES_SIZE + sizeof(RPGmat4), NULL, GL_STATIC_DRAW);

            // Calculate the left/top/right/bottom of tile source in normalized coordinates 
            GLfloat tl = tmxtile->ul_x / (GLfloat) tilelayer->image->width;
            GLfloat tt = tmxtile->ul_y / (GLfloat) tilelayer->image->height;
            GLfloat tr = tl + (tmxtile->tileset->tile_width / (GLfloat) tilelayer->image->width);
            GLfloat tb = tt + (tmxtile->tileset->tile_height / (GLfloat) tilelayer->image->height);

            GLfloat vertices[VERTICES_COUNT] = 
            {
                0.0f, 1.0f, tl, tb, 
                1.0f, 0.0f, tr, tt, 
                0.0f, 0.0f, tl, tt,
                0.0f, 1.0f, tl, tb, 
                1.0f, 1.0f, tr, tb, 
                1.0f, 0.0f, tr, tt
            };
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

            RPGmat4 model;
            RPG_MAT4_SET(model,
                map->tile_width, 0.0f, 0.0f, 0.0f,
                0.0f, map->tile_height, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                mapX * map->tile_width, mapY * map->tile_height, 0.0f, 1.0f
            );
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(RPGmat4), &model);


            glBindVertexArray(vao);
            glEnableVertexAttribArray(vLoc);
            glVertexAttribPointer(vLoc, 4, GL_FLOAT, GL_FALSE, VERTICES_STRIDE, NULL);
            glVertexAttribDivisor(vLoc, 0);

            for (int i = 0; i < 4; i++) {
                glEnableVertexAttribArray(mLoc + i);
                glVertexAttribPointer(mLoc + i, 4, GL_FLOAT, GL_FALSE, sizeof(RPGmat4), (void*)(VERTICES_SIZE + sizeof(RPGvec4) * i));
                glVertexAttribDivisor(mLoc + i, 1);
            }

            tilelayer->tiles[index].vao = vao;
            tilelayer->tiles[index].vbo = vbo;
        }
    }

    return tilelayer;
}

static RPGimagelayer *RPG_Tilemap_CreateImageLayer(tmx_map *map, tmx_layer *layer) {

    return NULL;
}

static RPGobjectlayer *RPG_Tilemap_CreateObjectLayer(tmx_map *map, tmx_layer *layer) {

    return NULL;
}

static RPGgrouplayer *RPG_Tilemap_CreateGroupLayer(tmx_map *map, tmx_layer *layer) {

    return NULL;
}

static void RPG_Tilemap_Initialize(tmx_map *map, RPGviewport *viewport, RPGtilemap **tilemap) {
    RPG_ALLOC_ZERO(tm, RPGtilemap);
    RPGbatch *batch = viewport ? &viewport->batch : &RPG_GAME->batch;
    RPG_Renderable_Init(&tm->base, RPG_Tilemap_Render, batch);
    RPG_Tilemap_CreateShader(tm);  // TODO:

    // Store the map, count the layers, and allocate memory for them
    tm->map = map;
    for (tmx_layer *layer = map->ly_head; layer != NULL; layer = layer->next, tm->layerCount++) {
    }
    tm->layers = RPG_ALLOC_N(RPGlayer, tm->layerCount);

    // Enumerate each layer
    int index = 0;
    for (tmx_layer *layer = map->ly_head; layer != NULL; layer = layer->next, index++) {
        RPGlayer *base = &tm->layers[index];
        base->type     = layer->type;

        switch (layer->type) {
            case L_LAYER: {
                base->layer.tile = RPG_Tilemap_CreateTileLayer(tm, map, layer);
                break;
            }
            case L_IMAGE: {
                base->layer.image = RPG_Tilemap_CreateImageLayer(map, layer);
                break;
            }
            case L_GROUP: {
                base->layer.object = RPG_Tilemap_CreateObjectLayer(map, layer);
                break;
            }
            case L_OBJGR: {
                base->layer.group = RPG_Tilemap_CreateGroupLayer(map, layer);
                break;
            }
            default: {
                fprintf(stderr, "invalid layer type");
                break;
            }
        }
    }
    RPG_MAT4_ORTHO(tm->projection, 0.0f, map->width * map->tile_width, map->height * map->tile_height,  0.0f, -1.0f, 1.0f);
    // tm->projection = RPG_GAME->projection;
    *tilemap = tm;
}

// Below here is pretty standard, can probably reuse most of it

static void *RPG_Tilemap_ImageLoad(const char *path) {
    if (RPG_FILE_EXISTS(path)) {
        RPGimage *image;
        if (RPG_Image_CreateFromFile(path, &image) == RPG_NO_ERROR) {
            return image;
        }
    }
    return NULL;
}

static void RPG_Tilemap_ImageFree(void *image) { RPG_Image_Free(image); }

static void RPG_Tilemap_Init(void) {
    // if (cache != NULL) {
    //     return;
    // }
    tmx_alloc_func    = RPG_REALLOC;
    tmx_free_func     = RPG_FREE;
    tmx_img_load_func = RPG_Tilemap_ImageLoad;
    tmx_img_free_func = RPG_Tilemap_ImageFree;
    cache             = tmx_make_resource_manager();
}

RPG_RESULT RPG_Tilemap_Create(const void *buffer, RPGsize size, RPGviewport *viewport, RPGtilemap **tilemap) {
    RPG_Tilemap_Init();
    tmx_map *map = tmx_rcmgr_load_buffer(cache, buffer, (int) size);
    if (map == NULL) {
        return RPG_ERR_UNKNOWN;
    }
    RPG_Tilemap_Initialize(map, viewport, tilemap);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_CreateFromFile(const char *path, RPGviewport *viewport, RPGtilemap **tilemap) {
    RPG_ENSURE_FILE(path);
    RPG_Tilemap_Init();
    tmx_map *map = tmx_rcmgr_load(cache, path);
    if (map == NULL) {
        return RPG_ERR_UNKNOWN;
    }
    RPG_Tilemap_Initialize(map, viewport, tilemap);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_Free(RPGtilemap *tilemap) {
    if (tilemap != NULL) {

        // TODO:
        RPG_FREE(tilemap);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetSize(RPGtilemap *tilemap, RPGint *width, RPGint *height) {
    RPG_RETURN_IF_NULL(tilemap);
    if (width != NULL) {
        *width = tilemap->map->width;
    }
    if (height != NULL) {
        *height = tilemap->map->height;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetTileSize(RPGtilemap *tilemap, RPGint *width, RPGint *height) {
    RPG_RETURN_IF_NULL(tilemap);
    if (width != NULL) {
        *width = tilemap->map->tile_width;
    }
    if (height != NULL) {
        *height = tilemap->map->tile_height;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_ClearCache(void) {
    if (cache != NULL) {
        tmx_free_resource_manager(cache);
        cache = tmx_make_resource_manager();
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetTileCount(RPGtilemap *tilemap, RPGuint *count) {
    RPG_RETURN_IF_NULL(tilemap);
    if (count != NULL) {
        *count = tilemap->map->tilecount;
    }
    return RPG_NO_ERROR;
}

#endif /* MEH */