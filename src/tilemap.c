#ifndef MEH

#include "internal.h"
#include "tmx/tmx.h"
#include <stdio.h>

tmx_resource_manager *cache;

// Tile Layer
typedef struct {
    GLuint vao;
    GLuint modelVBO;
    GLuint vertexVBO;
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

    RPG_RESULT r = RPG_Shader_Create(v, RPG_FRAGMENT_SHADER, NULL, &shader);
    RPG_ASSERT(r == RPG_NO_ERROR);
    tilemap->shader.program = shader->program;  // TODO: Free
}

static void RPG_Tilemap_Render(void *tilemap) {
    RPGtilemap *t = tilemap;
    if (!t->base.visible || t->base.alpha < __FLT_EPSILON__) {
        return;
    }

    glUseProgram(t->shader.program);
    glUniform4f(glGetUniformLocation(t->shader.program, "color"), 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform4f(glGetUniformLocation(t->shader.program, "tone"), 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform1f(glGetUniformLocation(t->shader.program, "alpha"), 1.0f);
    glUniform1f(glGetUniformLocation(t->shader.program, "hue"), 0.0f);
    glUniform4f(glGetUniformLocation(t->shader.program, "flash"), 0.0f, 0.0f, 0.0f, 0.0f);
    glUniformMatrix4fv(glGetUniformLocation(t->shader.program, "model"), 1, GL_FALSE, (GLfloat *) &t->base.model);
    glUniformMatrix4fv(glGetUniformLocation(t->shader.program, "projection"), 1, GL_FALSE, (GLfloat *) &RPG_GAME->projection);
    glBlendEquation(t->base.blend.op);
    glBlendFunc(t->base.blend.src, t->base.blend.dst);

    for (RPGuint i = 0; i < t->layerCount; i++) {
        RPGlayer *layer = &t->layers[i];
        if (layer->type != L_LAYER) {
            continue;
        }
        RPGtilelayer *l = layer->layer.tile;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, l->image->texture);
        glBindVertexArray(l->vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, l->tileCount);
    }
    glUseProgram(RPG_GAME->shader.program);
}

static RPGtilelayer *RPG_Tilemap_CreateTileLayer(RPGtilemap *tilemap, tmx_map *map, tmx_layer *layer) {
    // Create a new layer object
    RPG_ALLOC_ZERO(l, RPGtilelayer);
    l->tileCount = map->width * map->height;
    l->image     = NULL;

    RPGfloat vertices[l->tileCount][VERTICES_COUNT];
    RPGmat4 models[l->tileCount];

    RPGuint i;
    RPGint cell, gid;
    for (RPGuint y = 0; y < map->height; y++) {
        for (RPGuint x = 0; x < map->width; x++) {
            i = x + (y * map->width);

            cell = layer->content.gids[i];
            gid  = cell & TMX_FLIP_BITS_REMOVAL;

            if (gid == 0) {
                memset(&vertices[i][0], 0, VERTICES_SIZE);
                memset(&models[i], 0, sizeof(RPGmat4));
                continue;
            }
            tmx_tile *tile = map->tiles[gid];
            RPG_ASSERT(tile);

            if (l->image == NULL) {
                l->image = tile->tileset->image->resource_image;
            }
            RPG_ASSERT(l->image);

            // Calculate normalized width and height for a tile
            GLfloat tw = (map->tile_width / (GLfloat) l->image->width) / map->tile_width;
            GLfloat th = (map->tile_height / (GLfloat) l->image->height) / map->tile_height;

            // Define left, top, right, and bottom of the tile in normalized coordinates
            GLfloat l = tile->ul_x * tw;
            GLfloat t = tile->ul_y * th;
            GLfloat r = l + tw;
            GLfloat b = t + th;

            RPGfloat mx = x * map->tile_width;
            RPGfloat my = y * map->tile_height;

            // Define vertices and model matrix for this tile
            RPGfloat v[VERTICES_COUNT] = 
            {
                0.0f, 1.0f, l, b, 
                1.0f, 0.0f, r, t, 
                0.0f, 0.0f, l, t, 
                0.0f, 1.0f, l, b, 
                1.0f, 1.0f, r, b, 
                1.0f, 0.0f, r, t}
            ;

            memcpy(&vertices[i][0], v, VERTICES_SIZE);
            RPG_MAT4_SET(models[i], map->tile_width, 0.0f, 0.0f, 0.0f, 0.0f, map->tile_height, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, mx, my, 0.0f, 1.0f);
        }
    }

    // Create and bind a VAO/VBO for this layer
    
    glGenVertexArrays(1, &l->vao);
    glBindVertexArray(l->vao);  
    // Vertices
    GLint vertexLocation = glGetAttribLocation(tilemap->shader.program, "vertex");
    glGenBuffers(1, &l->vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, l->vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0][0], GL_STATIC_DRAW); 
    glVertexAttribPointer(vertexLocation, 4, GL_FLOAT, GL_FALSE, VERTICES_STRIDE, NULL);
    glEnableVertexAttribArray(vertexLocation);
    glVertexAttribDivisor(vertexLocation, 1);

    // Models
    GLint matLocation = glGetAttribLocation(tilemap->shader.program, "model");
    RPG_ASSERT(matLocation != - 1);
    glGenBuffers(1, &l->modelVBO);
    glBindBuffer(GL_ARRAY_BUFFER, l->modelVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(models), &models[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(matLocation);
    for (int i = 0; i < 4; i++) {
        // Set up the vertex attribute
        glVertexAttribPointer(matLocation + i, 4, GL_FLOAT, GL_FALSE, sizeof(RPGmat4), (void *)(sizeof(RPGvec4) * i));
        // Enable it
        glEnableVertexAttribArray(matLocation + i);
        // Make it instanced
        glVertexAttribDivisor(matLocation + i, 1);
    }
    return l;
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