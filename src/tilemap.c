#ifndef MEH

/*


*/


#include "internal.h"
#include "tmx/tmx.h"
#include <stdio.h>

#define LAYOUT_VERTEX 0
#define LAYOUT_MODEL 1


tmx_resource_manager *cache;

typedef struct {
    GLuint vao;
    GLuint vbo;
    RPGint gid;
    tmx_tile *tmx;
    struct {
        RPGubyte index;
        RPGdouble delta;
    } anime;
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
    tmx_layer *tmx;
} RPGlayer;

typedef struct RPGtilemap {
    RPGrenderable base;
    tmx_map *map;
    RPGuint layerCount;
    RPGlayer *layers;
    RPGuint program;
    RPGvec2 size;
    void *user;
} RPGtilemap;


static void RPG_Tilemap_SetVertices(float l, float t, float r, float b, RPGint gid, GLuint vbo, RPGbool bind) {
    if (bind) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }
    // Determine placement of vertices based on flag set in GID
    if ((gid & TMX_FLIPPED_VERTICALLY) != 0 && (gid & TMX_FLIPPED_HORIZONTALLY) != 0) {
        RPGvec4 elements[6] = {
            { 0.0f, 1.0f, r, t }, 
            { 1.0f, 0.0f, l, b }, 
            { 0.0f, 0.0f, r, b },
            { 0.0f, 1.0f, r, t }, 
            { 1.0f, 1.0f, l, t }, 
            { 1.0f, 0.0f, l, b }
        };
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(elements), elements);
    } else if ((gid & TMX_FLIPPED_HORIZONTALLY) != 0) {
        RPGvec4 elements[6] = {
            { 0.0f, 1.0f, r, b }, 
            { 1.0f, 0.0f, l, t }, 
            { 0.0f, 0.0f, r, t },
            { 0.0f, 1.0f, r, b }, 
            { 1.0f, 1.0f, l, b }, 
            { 1.0f, 0.0f, l, t }
        };
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(elements), elements);
    } else if ((gid & TMX_FLIPPED_VERTICALLY) != 0) {
        RPGvec4 elements[6] = {
            { 0.0f, 1.0f, l, t }, 
            { 1.0f, 0.0f, r, b }, 
            { 0.0f, 0.0f, l, b },
            { 0.0f, 1.0f, l, t }, 
            { 1.0f, 1.0f, r, t }, 
            { 1.0f, 0.0f, r, b }
        };
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(elements), elements);
    } else if ((gid & TMX_FLIPPED_DIAGONALLY) != 0) {
        // TODO: Implement diagonal?
    } else {
        RPGvec4 elements[6] = {
            { 0.0f, 1.0f, l, b }, 
            { 1.0f, 0.0f, r, t }, 
            { 0.0f, 0.0f, l, t },
            { 0.0f, 1.0f, l, b }, 
            { 1.0f, 1.0f, r, b }, 
            { 1.0f, 0.0f, r, t }
        };
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(elements), elements);
    }
}


static void RPG_Tilemap_CreateShader(RPGtilemap *tilemap) {

    RPGshader *shader;

    size_t s;
    char *tempv, *tempf;
    RPG_ReadFile("/home/eric/Desktop/rpg/tilemap.vert", &tempv, &s);
    // RPG_ReadFile("/home/eric/Desktop/rpg/tilemap.frag", &tempf, &s);

    RPG_RESULT r = RPG_Shader_Create(tempv, RPG_FRAGMENT_SHADER, NULL, &shader);


    RPG_ASSERT(r == RPG_NO_ERROR);
    tilemap->program = shader->program; 
    RPG_FREE(shader);
    // if (tempf != NULL) RPG_FREE(tempf); 
    if (tempv != NULL) RPG_FREE(tempv); 
}

static void RPG_Tilemap_Render(void *tilemap) {
    RPGtilemap *t = tilemap;
    if (!t->base.visible || t->base.alpha < __FLT_EPSILON__) {
        return;
    }

    glUseProgram(t->program);

    if (t->base.updated) {

        GLint loc = glGetUniformLocation(t->program, "projection");

        RPG_MAT4_ORTHO(t->base.model, 0.0f, RPG_GAME->resolution.width, RPG_GAME->resolution.height, 0.0f, -1.0f, 1.0f);
        glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *) &t->base.model);




        t->base.updated = RPG_FALSE;
    }

    glUniform1f(glGetUniformLocation(t->program, "alpha"), t->base.alpha);
    // glUniform4f(glGetUniformLocation(t->shader.program, "color"), 0.0f, 0.0f, 0.0f, 0.0f);
    // glUniform4f(glGetUniformLocation(t->shader.program, "tone"), 0.0f, 0.0f, 0.0f, 0.0f);
    // glUniform1f(glGetUniformLocation(t->shader.program, "alpha"), 1.0f);
    // glUniform1f(glGetUniformLocation(t->shader.program, "hue"), 0.0f);
    // glUniform4f(glGetUniformLocation(t->shader.program, "flash"), 0.0f, 0.0f, 0.0f, 0.0f);

   

    

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
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
    }
    glUseProgram(RPG_GAME->shader.program);
}

static RPGtilelayer *RPG_Tilemap_CreateTileLayer(RPGtilemap *tilemap, tmx_map *map, tmx_layer *layer) {

    RPGtilelayer *tilelayer = RPG_ALLOC(RPGtilelayer);
    tilelayer->tileCount = map->width * map->height;
    tilelayer->image = NULL;
    tilelayer->tiles = RPG_MALLOC(sizeof(RPGtile) * tilelayer->tileCount);
    memset(tilelayer->tiles, 0, sizeof(RPGtile) * tilelayer->tileCount);

    GLint vLoc = glGetAttribLocation(tilemap->program, "vertex");
    GLint mLoc = glGetAttribLocation(tilemap->program, "model");

    RPGuint index = 0;
    for (RPGuint mapY = 0; mapY < map->height; mapY++) {
        for (RPGuint mapX = 0; mapX < map->width; mapX++, index++) {

            RPGint gid = layer->content.gids[index];
            tmx_tile *tmxtile = map->tiles[gid & TMX_FLIP_BITS_REMOVAL];

            if (gid == 0) {
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
            RPG_Tilemap_SetVertices(tl, tt, tr, tb, gid, vbo, RPG_FALSE);


            RPGmat4 model;
            RPG_MAT4_SET(model,
                map->tile_width, 0.0f, 0.0f, 0.0f,
                0.0f, map->tile_height, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                mapX * map->tile_width, mapY * map->tile_height, 0.0f, 1.0f
            );
            glBufferSubData(GL_ARRAY_BUFFER, VERTICES_SIZE, sizeof(RPGmat4), &model);


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
            tilelayer->tiles[index].gid = gid;
            tilelayer->tiles[index].tmx = tmxtile;
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
        base->tmx = layer;

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

void RPG_Tilemap_UpdateTileLayer(RPGtilemap *map, RPGtilelayer *layer, tmx_layer *tmx) {
    
    // Declare variable storage;
    tmx_tile *next;
    tmx_anim_frame *frame;
    RPGtile *tile;

    // Get the current time
    RPGdouble time = glfwGetTime();

    // Enumerate through tiles of layer
    for (RPGuint i = 0; i < layer->tileCount; i++) {
        tile = &layer->tiles[i];

        // If tile is defined and is time has been reached for a tile change
        if (tile->tmx && tile->tmx->animation_len > 0 && tile->anime.delta < time) {
            
            // Increment the current index, and get the next frame
            tile->anime.index = (tile->anime.index + 1) % tile->tmx->animation_len;
            frame = &tile->tmx->animation[tile->anime.index];
            // Set the next target time for an animation
            tile->anime.delta = time + (frame->duration * 0.001);
            
            // Get the GID for the next tile in the animation
            next = &tile->tmx->tileset->tiles[frame->tile_id];

            // left/top/right/bottom in normalized texture coordinates
            GLfloat l = next->ul_x / (GLfloat) layer->image->width;
            GLfloat t = next->ul_y / (GLfloat) layer->image->height;
            GLfloat r = l + (next->tileset->tile_width / (GLfloat)  layer->image->width);
            GLfloat b = t + (next->tileset->tile_height / (GLfloat) layer->image->height);

            // Buffer the new data
            RPG_Tilemap_SetVertices(l, t, r, b, next->id, tile->vbo, RPG_TRUE);
        }
    }
}

RPG_RESULT RPG_Tilemap_Update(RPGtilemap *tilemap) {
    RPG_RETURN_IF_NULL(tilemap);

    RPGlayer *layer;
    for (RPGuint i = 0; i < tilemap->layerCount; i++) {
        layer = &tilemap->layers[i];
        if (layer->type == L_LAYER) {
            RPG_Tilemap_UpdateTileLayer(tilemap, layer->layer.tile, layer->tmx);
        }

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