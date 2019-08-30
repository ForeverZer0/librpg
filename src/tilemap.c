#ifndef MEH

#include "internal.h"
#include "tmx/tmx.h"
#include <stdio.h>

tmx_resource_manager *cache;


// Tile Layer
typedef struct {
    GLuint vao;
    GLuint buffer;
    RPGimage *image;
    RPGuint tileCount;
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
    void *v, *f;
    RPG_ReadFile("/home/eric/Desktop/rpg/temp.glsl", &v, &s);
    RPG_ReadFile("/home/eric/Desktop/rpg/temp.frag", &f, &s);
    RPG_Shader_Create(v, f, NULL, &shader);


    // RPG_Shader_Create(RPG_TILEMAP_VERTEX, RPG_TILEMAP_FRAGMENT, NULL, &shader);

    tilemap->shader.program = shader->program;  // TODO: Free

    tilemap->shader.projection = glGetUniformLocation(shader->program, "projection");
    glUseProgram(tilemap->shader.program);
    glUniformMatrix4fv(tilemap->shader.projection, 1, GL_FALSE, (GLfloat *) &RPG_GAME->projection);
    glUseProgram(RPG_GAME->shader.program);
}

static void RPG_Tilemap_Render(void *tilemap) {
    RPGtilemap *t = tilemap;

    RPGtilemap *v = t;
    if (!t->base.visible || t->base.alpha < __FLT_EPSILON__) {
        return;
    }
    
    for (RPGuint i = 0; i < t->layerCount; i++) {
        RPGlayer *layer = &t->layers[i];
        if (layer->type != L_LAYER) {
            continue;
        }
        RPGtilelayer *l = layer->layer.tile;

        // glUseProgram(t->shader.program);
        // glUniform2i(glGetUniformLocation(t->shader.program, "mapSize"), (int) t->map->width, (int) t->map->height);
        // glUniform2f(glGetUniformLocation(t->shader.program, "tileSize"), 16.0f, 16.0f);
        // glUniform2i(glGetUniformLocation(t->shader.program, "imageSize"), 576, 416);
        // glUniform2f(glGetUniformLocation(t->shader.program, "origin"), 0.0f, 0.0f);


        // glActiveTexture(GL_TEXTURE0);                                                                                                          
        glBindTexture(GL_TEXTURE_2D, l->image->texture);                                                                                          
        glBindVertexArray(l->vao);                     
        glMultiDrawArrays(GL_TRIANGLES, 0, 6, l->tileCount);                                                                                           
        // glDrawArraysInstanced(GL_TRIANGLES, 0, 6, l->tileCount);                                                                                                      
        glBindVertexArray(0);

        // glUseProgram(RPG_GAME->shader.program);
    }

}

static RPGimagelayer *RPG_Tilemap_CreateImageLayer(tmx_map *map, tmx_layer *layer) {
    // TODO:
    return NULL;
}

static RPGobjectlayer *RPG_Tilemap_CreateObjectLayer(tmx_map *map, tmx_layer *layer) {
    // TODO:
    return NULL;
}

static RPGgrouplayer *RPG_Tilemap_CreateGroupLayer(tmx_map *map, tmx_layer *layer) {
    // TODO:
    return NULL;
}

static RPGtilelayer *RPG_Tilemap_CreateTileLayer(tmx_map *map, tmx_layer *layer) {
    // Create a new layer object
    RPG_ALLOC_ZERO(l, RPGtilelayer);
    l->tileCount = map->width * map->height;

    // Create storage for a model matrix and VAO for each tile.
    GLfloat vertices[l->tileCount][VERTICES_COUNT];
    l->image = NULL;


    RPGuint i = 0;
    RPGint cell, gid;
    tmx_tile *tmxTile;
    for (RPGuint y = 0; y < map->height; y++) {
        for (RPGuint x = 0; x < map->width; x++, i = x + (y * map->width)) {

            cell = layer->content.gids[i];
            gid = cell & TMX_FLIP_BITS_REMOVAL;
            if (gid == 0) {
                memset(&vertices[i], 0, VERTICES_SIZE);
                continue;
            }

     
            tmxTile = map->tiles[gid];

            RPG_ASSERT(tmxTile);

            if (l->image == NULL) {
                l->image = tmxTile->tileset->image->resource_image;
            }
            RPG_ASSERT(l->image);

            // Calculate normalized dimensions for a tile
            RPGfloat tw = (map->tile_width / (GLfloat) l->image->width) / map->tile_width;
            RPGfloat th = (map->tile_height / (GLfloat) l->image->height) / map->tile_height;

            // left, right, top, bottom of texture
            GLfloat tl = tmxTile->ul_x * tw;
            GLfloat tt = tmxTile->ul_y * th;
            GLfloat tr = tl + tw;
            GLfloat tb = tt + th;

            // left, right, top, bottom of tile on map
            GLfloat ml = x * map->tile_width;
            GLfloat mt = y * map->tile_height;
            GLfloat mr = ml + map->tile_width;
            GLfloat mb = mt + map->tile_height;

            GLfloat v[VERTICES_COUNT] =
            {
                ml, mb, tl, tb, 
                mr, mt, tr, tt, 
                ml, mt, tl, tt,
                ml, mb, tl, tb, 
                mr, mb, tr, tb, 
                mr, mt, tr, tt
            };
            memcpy(&vertices[i], v, VERTICES_SIZE);
        }
    }

    glGenVertexArrays(1, &l->vao);
    glGenBuffers(1, &l->buffer);
    glBindVertexArray(l->vao);
    glBindBuffer(GL_ARRAY_BUFFER, l->buffer);

    glBufferData(GL_ARRAY_BUFFER, VERTICES_SIZE * l->tileCount, &vertices[0][0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VERTICES_STRIDE, NULL);

    glEnableVertexAttribArray(0);

    glVertexAttribDivisor(0, 1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return l;
}



static void RPG_Tilemap_Initialize(tmx_map *map, RPGviewport *viewport, RPGtilemap **tilemap) {

    RPG_ALLOC_ZERO(tm, RPGtilemap);
    RPGbatch *batch = viewport ? &viewport->batch : &RPG_GAME->batch;
    RPG_Renderable_Init(&tm->base, RPG_Tilemap_Render, batch);

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
                base->layer.tile = RPG_Tilemap_CreateTileLayer(map, layer);
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

    RPG_Tilemap_CreateShader(tm);  // TODO:

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

// struct _tmx_map { /* <map> (Head of the data structure) */
// 	enum tmx_map_orient orient;

// 	enum tmx_stagger_index stagger_index;
// 	enum tmx_stagger_axis stagger_axis;
// 	int hexsidelength;

// 	unsigned int backgroundcolor; /* bytes : RGB */
// 	enum tmx_map_renderorder renderorder;

// 	tmx_properties *properties;
// 	tmx_tileset_list *ts_head;
// 	tmx_layer *ly_head;

// 	tmx_tile **tiles; /* GID indexed tile array (array of pointers to tmx_tile) */

// 	tmx_user_data user_data;
// };

#endif /* MEH */