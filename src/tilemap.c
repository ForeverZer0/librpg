
#include "internal.h"
#include "tmx/tmx.h"
#include <stdio.h>

static inline float getUV(float t, float dim) {
    return (t + 0.5f) / dim; // FIXME:
}

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

    GLuint vao;
    GLuint vbo;
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
    RPGfloat pxWidth;
    RPGfloat pxHeight;
    struct {
        GLuint program;
        GLint color;
        GLint tone;
        GLint flash;
        GLint alpha;
        GLint projection;
        GLint hue;
        GLint origin;
    } shader;
    void *user;
} RPGtilemap;

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

static void RPG_Tilemap_SetVertices(float l, float t, float r, float b, RPGint gid, GLuint vbo, RPGbool bind) {
    if (bind) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }
    // Determine placement of vertices based on flag set in GID
    if ((gid & TMX_FLIPPED_VERTICALLY) != 0 && (gid & TMX_FLIPPED_HORIZONTALLY) != 0) {
        RPGvec4 elements[6] = {{0.0f, 1.0f, r, t}, {1.0f, 0.0f, l, b}, {0.0f, 0.0f, r, b},
                               {0.0f, 1.0f, r, t}, {1.0f, 1.0f, l, t}, {1.0f, 0.0f, l, b}};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(elements), elements);
    } else if ((gid & TMX_FLIPPED_HORIZONTALLY) != 0) {
        RPGvec4 elements[6] = {{0.0f, 1.0f, r, b}, {1.0f, 0.0f, l, t}, {0.0f, 0.0f, r, t},
                               {0.0f, 1.0f, r, b}, {1.0f, 1.0f, l, b}, {1.0f, 0.0f, l, t}};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(elements), elements);
    } else if ((gid & TMX_FLIPPED_VERTICALLY) != 0) {
        RPGvec4 elements[6] = {{0.0f, 1.0f, l, t}, {1.0f, 0.0f, r, b}, {0.0f, 0.0f, l, b},
                               {0.0f, 1.0f, l, t}, {1.0f, 1.0f, r, t}, {1.0f, 0.0f, r, b}};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(elements), elements);
    } else if ((gid & TMX_FLIPPED_DIAGONALLY) != 0) {
        // TODO: Implement diagonal?
    } else {
        RPGvec4 elements[6] = {{0.0f, 1.0f, l, b}, {1.0f, 0.0f, r, t}, {0.0f, 0.0f, l, t},
                               {0.0f, 1.0f, l, b}, {1.0f, 1.0f, r, b}, {1.0f, 0.0f, r, t}};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(elements), elements);
    }
}

static void RPG_Tilemap_CreateShader(RPGtilemap *tilemap) {

    RPGshader *shader;

    size_t s;
    char *tempv;
    RPG_ReadFile("/home/eric/Desktop/rpg/tilemap.vert", &tempv, &s);
    RPG_Shader_Create(tempv, RPG_FRAGMENT_SHADER, NULL, &shader);
    RPG_ASSERT(shader);

    // FIXME: Shared shader for all tilemaps
    tilemap->shader.program    = shader->program;
    tilemap->shader.projection = glGetUniformLocation(shader->program, "projection");
    tilemap->shader.alpha      = glGetUniformLocation(shader->program, "alpha");
    tilemap->shader.color      = glGetUniformLocation(shader->program, "color");
    tilemap->shader.tone       = glGetUniformLocation(shader->program, "tone");
    tilemap->shader.flash      = glGetUniformLocation(shader->program, "flash");
    tilemap->shader.hue        = glGetUniformLocation(shader->program, "hue");
    tilemap->shader.origin     = glGetUniformLocation(shader->program, "origin");

    RPG_FREE(shader);
    if (tempv != NULL)
        RPG_FREE(tempv);
}

static void RPG_Tilemap_RenderTileLayer(RPGtilemap *tilemap, RPGtilelayer *layer) {
    // Bind tileset texture
    RPG_Drawing_BindTexture(layer->image->texture, GL_TEXTURE0);
    glBindVertexArray(layer->vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, layer->tileCount * 6);
}

static void RPG_Tilemap_Render(void *tilemap) {
    RPGtilemap *t = tilemap;
    // Skip if tilemap is not visible
    if (!t->base.visible || t->base.alpha < __FLT_EPSILON__) {
        return;
    }

    // Activate the tilemap's shader program
    glUseProgram(t->shader.program);

    // Update tilemap model matrix and uniforms if changed since last render
    if (t->base.updated) {
        // Set model matrix for tilemap, which is actually a projection matrix for the layers
        RPG_MAT4_ORTHO(t->base.model, 0.0f, RPG_GAME->resolution.width, RPG_GAME->resolution.height, 0.0f, -1.0f, 1.0f);
        // RPG_MAT4_ORTHO(t->base.model, 0.0f, w, h, 0.0f, -1.0f, 1.0f);
        // glUniformMatrix4fv(t->shader.projection, 1, GL_FALSE, (GLfloat *) &t->base.model);
        glUniformMatrix4fv(t->shader.projection, 1, GL_FALSE, (GLfloat *) &RPG_GAME->projection);

        // Update any changes to the origin point and clear "dirty" flag
        glUniform2f(t->shader.origin, (float) t->base.ox, (float) t->base.oy);
        t->base.updated = RPG_FALSE;
    }

    glUniform1f(t->shader.alpha, t->base.alpha);
    glUniform4f(t->shader.color, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform4f(t->shader.tone, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform1f(t->shader.alpha, 1.0f);
    glUniform1f(t->shader.hue, 0.0f);
    glUniform4f(t->shader.flash, 0.0f, 0.0f, 0.0f, 0.0f);
    RPG_Drawing_SetBlending(t->base.blend.op, t->base.blend.src, t->base.blend.dst);

    for (RPGuint i = 0; i < t->layerCount; i++) {
        RPGlayer *layer = &t->layers[i];

        // Skip if layer is not visible
        if (!layer->tmx->visible)
            continue;

        // Branch defpending on the type of layer
        switch (layer->type) {
            case L_LAYER: 
                RPG_Tilemap_RenderTileLayer(t, layer->layer.tile);
                break;
            case L_IMAGE:

                break;
            case L_GROUP:

                break;
            case L_OBJGR:

                break;
            default:
                fprintf(stderr, "unknown layer type");
                break;
        }
    }
    glUseProgram(RPG_GAME->shader.program);
}

typedef struct {
    RPGvec2 vertices[6];
    RPGmat4 model;
} TILE;

static RPGtilelayer *RPG_Tilemap_CreateTileLayer(RPGtilemap *tilemap, tmx_map *map, tmx_layer *layer) {

    // Initialize tile layer struct and storage for tiles
    RPGtilelayer *tilelayer = RPG_ALLOC(RPGtilelayer);
    tilelayer->tileCount    = map->width * map->height;
    tilelayer->image        = NULL;
    tilelayer->tiles        = RPG_MALLOC(sizeof(RPGtile) * tilelayer->tileCount);
    memset(tilelayer->tiles, 0, sizeof(RPGtile) * tilelayer->tileCount);

    // Get the locations for the vertex and model attributes in the vertex shader
    GLint vLoc = glGetAttribLocation(tilemap->shader.program, "vertex");
    GLint mLoc = glGetAttribLocation(tilemap->shader.program, "model");

    glGenVertexArrays(1, &tilelayer->vao);
    glGenBuffers(1, &tilelayer->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, tilelayer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TILE) * tilelayer->tileCount, NULL, GL_STATIC_DRAW);
    TILE *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    // Enumerate throught each tile coordinate in the map for this layer
    RPGuint index = 0;
    for (RPGuint mapY = 0; mapY < map->height; mapY++) {
        for (RPGuint mapX = 0; mapX < map->width; mapX++) {

            index = mapX + (mapY * map->width);

            // Get the current tile for this location
            RPGint gid        = layer->content.gids[index];
            tmx_tile *tmxtile = map->tiles[gid & TMX_FLIP_BITS_REMOVAL];

            if (gid == 0) {
                memset(&ptr[index], 0, sizeof(TILE));
                // Skip if there is no tile here
                continue;
            }

            // Set the image used for this layer (multiple tilesets within one layer not supported)
            if (tilelayer->image == NULL) {
                tilelayer->image = tmxtile->tileset->image->resource_image;
            } else if (tilelayer->image != tmxtile->tileset->image->resource_image) {
                fprintf(stderr, "only one tileset per layer is supported");
            }

        
            GLfloat tl = getUV(tmxtile->ul_x, tilelayer->image->width);
            GLfloat tt = getUV(tmxtile->ul_y, tilelayer->image->height);
            GLfloat tr = getUV(tmxtile->ul_x + tmxtile->tileset->tile_width - 1, tilelayer->image->width);
            GLfloat tb = getUV(tmxtile->ul_y + tmxtile->tileset->tile_height - 1, tilelayer->image->height);
        
        
            // Calculate the left/top/right/bottom of tile source in normalized coordinates
            // GLfloat tl = tmxtile->ul_x / (GLfloat) tilelayer->image->width;
            // GLfloat tt = tmxtile->ul_y / (GLfloat) tilelayer->image->height;
            // GLfloat tr = tl + (tmxtile->tileset->tile_width / (GLfloat) tilelayer->image->width);
            // GLfloat tb = tt + (tmxtile->tileset->tile_height / (GLfloat) tilelayer->image->height);

            RPGvec2 vertices[6] = 
            {
                {tl, tb}, 
                {tr, tt}, 
                {tl, tt},
                {tl, tb}, 
                {tr, tb}, 
                {tr, tt}
            };
            memcpy(&ptr[index].vertices, vertices, sizeof(vertices));
            RPG_MAT4_SET(ptr[index].model, 
                map->tile_width, 0.0f, 0.0f, 0.0f, 
                0.0f, map->tile_height, 0.0f, 0.0f, 
                0.0f, 0.0f, 1.0f, 0.0f,
                mapX * map->tile_width, mapY * map->tile_height, 0.0f, 1.0f
            );


            // Store the info in the tile
            tilelayer->tiles[index].gid = gid;
            tilelayer->tiles[index].tmx = tmxtile;
        }
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindVertexArray(tilelayer->vao);

    for (int i = 0; i < 6; i++) {
        glEnableVertexAttribArray(vLoc + i);
        glVertexAttribPointer(vLoc + i, 4, GL_FLOAT, GL_TRUE, 0, (void *) (sizeof(RPGvec2) * i));
        glVertexAttribDivisor(vLoc + i, 1);
    }

    // Set four vec4 attribute locations to store as a 4x4 matrix
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(mLoc + i);
        glVertexAttribPointer(mLoc + i, 4, GL_FLOAT, GL_TRUE, 0, (void *) ((sizeof(RPGvec2) * 6) + (sizeof(RPGvec4) * i)));
        glVertexAttribDivisor(mLoc + i, 1);
    }
    return tilelayer;
}

static RPGimagelayer *RPG_Tilemap_CreateImageLayer(tmx_map *map, tmx_layer *layer) { return NULL; }

static RPGobjectlayer *RPG_Tilemap_CreateObjectLayer(tmx_map *map, tmx_layer *layer) { return NULL; }

static RPGgrouplayer *RPG_Tilemap_CreateGroupLayer(tmx_map *map, tmx_layer *layer) { return NULL; }

static void RPG_Tilemap_Initialize(tmx_map *map, RPGviewport *viewport, RPGtilemap **tilemap) {

    RPG_ALLOC_ZERO(tm, RPGtilemap);
    RPGbatch *batch = viewport ? &viewport->batch : &RPG_GAME->batch;
    RPG_Renderable_Init(&tm->base, RPG_Tilemap_Render, batch);
    RPG_Tilemap_CreateShader(tm);  // TODO:

    // Store the map, count the layers, and allocate memory for them
    tm->map = map;
    tm->pxWidth = map->width * map->tile_width;
    tm->pxHeight = map->height * map->tile_height;
    for (tmx_layer *layer = map->ly_head; layer != NULL; layer = layer->next, tm->layerCount++) {
    }
    tm->layers = RPG_ALLOC_N(RPGlayer, tm->layerCount);

    // Enumerate each layer
    int index = 0;
    for (tmx_layer *layer = map->ly_head; layer != NULL; layer = layer->next, index++) {
        RPGlayer *base = &tm->layers[index];
        base->type     = layer->type;
        base->tmx      = layer;

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

tmx_resource_manager *TEST;


RPG_RESULT RPG_Tilemap_Create(const void *buffer, RPGsize size, RPGviewport *viewport, RPGtilemap **tilemap) {

    if (tmx_alloc_func == NULL) {
        tmx_alloc_func    = RPG_REALLOC;
        tmx_free_func     = RPG_FREE;
        tmx_img_load_func = RPG_Tilemap_ImageLoad;
        tmx_img_free_func = RPG_Tilemap_ImageFree;
    }
    tmx_map *map = tmx_rcmgr_load_buffer(TEST, buffer, (int) size);
    // tmx_map *map = tmx_load_buffer(buffer, (int) size);
    if (map == NULL) {
        return RPG_ERR_UNKNOWN;
    }
    RPG_Tilemap_Initialize(map, viewport, tilemap);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_CreateFromFile(const char *path, RPGviewport *viewport, RPGtilemap **tilemap) {
    RPG_ENSURE_FILE(path);

    if (TEST == NULL) {
        TEST = tmx_make_resource_manager();
        tmx_alloc_func    = RPG_REALLOC;
        tmx_free_func     = RPG_FREE;
        tmx_img_load_func = RPG_Tilemap_ImageLoad;
        tmx_img_free_func = RPG_Tilemap_ImageFree;
    }
    tmx_map *map = tmx_rcmgr_load(TEST, path);
    // tmx_map *map = tmx_load(path);
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
    return;  // TODO:!!!!!!!!!!!!!!!!!!!!!
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
            frame             = &tile->tmx->animation[tile->anime.index];
            next              = &tile->tmx->tileset->tiles[frame->tile_id];

            // left/top/right/bottom in normalized texture coordinates
            GLfloat l = next->ul_x / (GLfloat) layer->image->width;
            GLfloat t = next->ul_y / (GLfloat) layer->image->height;
            GLfloat r = l + (next->tileset->tile_width / (GLfloat) layer->image->width);
            GLfloat b = t + (next->tileset->tile_height / (GLfloat) layer->image->height);

            // Buffer the new data, and set the next target time for an animation
            tile->anime.delta = time + (frame->duration * 0.001);
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

RPG_RESULT RPG_Tilemap_GetTileCount(RPGtilemap *tilemap, RPGuint *count) {
    RPG_RETURN_IF_NULL(tilemap);
    if (count != NULL) {
        *count = tilemap->map->tilecount;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetOrigin(RPGtilemap *tilemap, RPGint *x, RPGint *y) {
    RPG_RETURN_IF_NULL(tilemap);
    if (x != NULL) {
        *x = tilemap->base.ox;
    }
    if (y != NULL) {
        *y = tilemap->base.oy;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_SetOrigin(RPGtilemap *tilemap, RPGint x, RPGint y) {
    RPG_RETURN_IF_NULL(tilemap);
    if (tilemap->base.ox != x || tilemap->base.oy != y) {
        tilemap->base.ox      = x;
        tilemap->base.oy      = y;
        tilemap->base.updated = RPG_TRUE;
    }
    return RPG_NO_ERROR;
}