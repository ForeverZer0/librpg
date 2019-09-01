
#include "internal.h"
#include "tmx/tmx.h"
#include <stdio.h>

#define LAYOUT_VERTEX 0
#define LAYOUT_MODEL 1

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

    GLuint VAO;
    GLuint VBO;
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
    struct {
        RPGint left;
        RPGint top;
        RPGint right;
        RPGint bottom;
    } visible;
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
    char *tempv;
    RPG_ReadFile("/home/eric/Desktop/rpg/tilemap.vert", &tempv, &s);


    RPG_RESULT r = RPG_Shader_Create(tempv, RPG_FRAGMENT_SHADER, NULL, &shader);


    RPG_ASSERT(r == RPG_NO_ERROR);
    tilemap->program = shader->program; 
    RPG_FREE(shader);
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

        t->visible.left = t->base.ox / t->map->tile_width;
        t->visible.top = t->base.oy / t->map->tile_height; // FIXME: Account for when negative
        t->visible.right = t->visible.left + (RPG_GAME->resolution.width / t->map->tile_width);
        t->visible.bottom = t->visible.top + (RPG_GAME->resolution.height / t->map->tile_height);

        RPG_MAT4_ORTHO(t->base.model, 0.0f + t->base.ox, RPG_GAME->resolution.width, RPG_GAME->resolution.height, 0.0f + t->base.oy, -1.0f, 1.0f);
        glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat *) &t->base.model);


        t->base.updated = RPG_FALSE;
    }

    glUniform1f(glGetUniformLocation(t->program, "alpha"), t->base.alpha);
    // glUniform4f(glGetUniformLocation(t->shader.program, "color"), 0.0f, 0.0f, 0.0f, 0.0f);
    // glUniform4f(glGetUniformLocation(t->shader.program, "tone"), 0.0f, 0.0f, 0.0f, 0.0f);
    // glUniform1f(glGetUniformLocation(t->shader.program, "alpha"), 1.0f);
    // glUniform1f(glGetUniformLocation(t->shader.program, "hue"), 0.0f);
    // glUniform4f(glGetUniformLocation(t->shader.program, "flash"), 0.0f, 0.0f, 0.0f, 0.0f);
    RPG_Drawing_SetBlending(t->base.blend.op, t->base.blend.src, t->base.blend.dst);


    

    for (RPGuint i = 0; i < t->layerCount; i++) {
        RPGlayer *layer = &t->layers[i];
        if (layer->type != L_LAYER) {
            continue;
        }

        RPGtilelayer *l = layer->layer.tile;

        // glUniformMatrix4fv(glGetUniformLocation(t->shader.program, "model"), 1, GL_FALSE, (GLfloat *) &l->model);
        RPG_Drawing_BindTexture(l->image->texture, GL_TEXTURE0);
    
        for (RPGuint mapY = t->visible.top; mapY < t->visible.top + t->visible.bottom + 2; mapY++) {
            for (RPGuint mapX = t->visible.left; mapX < t->visible.left + t->visible.right + 2; mapX++) {
                RPGuint tileIndex = mapX + (mapY * t->map->width);
                if (l->tiles[tileIndex].vao) {
                    glBindVertexArray(l->tiles[tileIndex].vao);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }

            }
        }
    }
    glUseProgram(RPG_GAME->shader.program);
}

#if 1

static RPGtilelayer *RPG_Tilemap_CreateTileLayer(RPGtilemap *tilemap, tmx_map *map, tmx_layer *layer) {
    // Initialize tile layer struct and storage for tiles
    RPGtilelayer *tilelayer = RPG_ALLOC(RPGtilelayer);
    tilelayer->tileCount = map->width * map->height;
    tilelayer->image = NULL;
    tilelayer->tiles = RPG_MALLOC(sizeof(RPGtile) * tilelayer->tileCount);
    memset(tilelayer->tiles, 0, sizeof(RPGtile) * tilelayer->tileCount);

    RPGvec2 offsets[tilelayer->tileCount];
    RPGmat4 models[tilelayer->tileCount];

    RPGuint dataSize = VERTICES_SIZE + sizeof(RPGmat4);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBufferData(GL_ARRAY_BUFFER, dataSize * tilelayer->tileCount, NULL, GL_STATIC_DRAW);

    // Get the locations for the vertex and model attributes in the vertex shader
    GLint vLoc = glGetAttribLocation(tilemap->program, "vertex");
    GLint oLoc = glGetAttribLocation(tilemap->program, "offset");
    GLint mLoc = glGetAttribLocation(tilemap->program, "model");

    RPGuint index = 0, offset = 0;
    for (RPGuint mapY = 0; mapY < map->height; mapY++) {
        for (RPGuint mapX = 0; mapX < map->width; mapX++, index++, offset += dataSize) {

            // Get the current tile for this location
            RPGint gid = layer->content.gids[index];
            tmx_tile *tmxtile = map->tiles[gid & TMX_FLIP_BITS_REMOVAL];

            if (gid == 0) {
                // Skip if there is no tile here
                continue;
            }
            // Calculate the left/top/right/bottom of tile source in normalized coordinates 
            GLfloat tl = tmxtile->ul_x / (GLfloat) tilelayer->image->width;
            GLfloat tt = tmxtile->ul_y / (GLfloat) tilelayer->image->height;
            GLfloat tr = tl + (tmxtile->tileset->tile_width / (GLfloat) tilelayer->image->width);
            GLfloat tb = tt + (tmxtile->tileset->tile_height / (GLfloat) tilelayer->image->height);
            RPG_Tilemap_SetVertices(tl, tt, tr, tb, gid, vbo, RPG_FALSE);

            RPGvec4 vertices[] = {
                { 0.0f, 1.0f, tl, tb }, 
                { 1.0f, 0.0f, tr, tt }, 
                { 0.0f, 0.0f, tl, tt },
                { 0.0f, 1.0f, tl, tb }, 
                { 1.0f, 1.0f, tr, tb }, 
                { 1.0f, 0.0f, tr, tt }
            };

            //  Set a model matrix for the tile map coordinates
            RPGmat4 model;
            RPG_MAT4_SET(model,
                map->tile_width, 0.0f, 0.0f, 0.0f,
                0.0f, map->tile_height, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                mapX * map->tile_width, mapY * map->tile_height, 0.0f, 1.0f
            );

            glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vertices), vertices);
            glBufferSubData(GL_ARRAY_BUFFER, offset + sizeof(vertices), sizeof(RPGmat4), &model);
        }
    }
}

#else

static RPGtilelayer *RPG_Tilemap_CreateTileLayer(RPGtilemap *tilemap, tmx_map *map, tmx_layer *layer) {

    // Initialize tile layer struct and storage for tiles
    RPGtilelayer *tilelayer = RPG_ALLOC(RPGtilelayer);
    tilelayer->tileCount = map->width * map->height;
    tilelayer->image = NULL;
    tilelayer->tiles = RPG_MALLOC(sizeof(RPGtile) * tilelayer->tileCount);
    memset(tilelayer->tiles, 0, sizeof(RPGtile) * tilelayer->tileCount);

    // Get the locations for the vertex and model attributes in the vertex shader
    GLint vLoc = glGetAttribLocation(tilemap->program, "vertex");
    GLint mLoc = glGetAttribLocation(tilemap->program, "model");

    // Enumerate throught each tile coordinate in the map for this layer
    RPGuint index = 0;
    for (RPGuint mapY = 0; mapY < map->height; mapY++) {
        for (RPGuint mapX = 0; mapX < map->width; mapX++, index++) {

            // Get the current tile for this location
            RPGint gid = layer->content.gids[index];
            tmx_tile *tmxtile = map->tiles[gid & TMX_FLIP_BITS_REMOVAL];

            if (gid == 0) {
                // Skip if there is no tile here
                continue;
            }

            // Set the image used for this layer (multiple tilesets within one layer not supported)
            if (tilelayer->image == NULL) {
                tilelayer->image = tmxtile->tileset->image->resource_image;
            } else if (tilelayer->image != tmxtile->tileset->image->resource_image) {
                fprintf(stderr, "only one tileset per layer is supported");
            }

            // Generate a buffer and vertex array for this tile
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

            //  Set a model matrix for the tile map coordinates
            RPGmat4 model;
            RPG_MAT4_SET(model,
                map->tile_width, 0.0f, 0.0f, 0.0f,
                0.0f, map->tile_height, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                mapX * map->tile_width, mapY * map->tile_height, 0.0f, 1.0f
            );
            glBufferSubData(GL_ARRAY_BUFFER, VERTICES_SIZE, sizeof(RPGmat4), &model);

            // Define how the data and vertices should be interpreted in the vertex shader
            glBindVertexArray(vao);
            glEnableVertexAttribArray(vLoc);
            glVertexAttribPointer(vLoc, 4, GL_FLOAT, GL_FALSE, VERTICES_STRIDE, NULL);
            glVertexAttribDivisor(vLoc, 0);

            // Set four vec4 attribute locations to store as a 4x4 matrix
            for (int i = 0; i < 4; i++) {
                glEnableVertexAttribArray(mLoc + i);
                glVertexAttribPointer(mLoc + i, 4, GL_FLOAT, GL_FALSE, sizeof(RPGmat4), (void*)(VERTICES_SIZE + sizeof(RPGvec4) * i));
                glVertexAttribDivisor(mLoc + i, 1);
            }

            // Store the info in the tile
            tilelayer->tiles[index].vao = vao;
            tilelayer->tiles[index].vbo = vbo;
            tilelayer->tiles[index].gid = gid;
            tilelayer->tiles[index].tmx = tmxtile;
        }
    }

    return tilelayer;
}


#endif

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

RPG_RESULT RPG_Tilemap_Create(const void *buffer, RPGsize size, RPGviewport *viewport, RPGtilemap **tilemap) {

    if (tmx_alloc_func == NULL) {
        tmx_alloc_func    = RPG_REALLOC;
        tmx_free_func     = RPG_FREE;
        tmx_img_load_func = RPG_Tilemap_ImageLoad;
        tmx_img_free_func = RPG_Tilemap_ImageFree;
    } 

    tmx_map *map = tmx_load_buffer(buffer, (int) size);
    if (map == NULL) {
        return RPG_ERR_UNKNOWN;
    }
    RPG_Tilemap_Initialize(map, viewport, tilemap);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_CreateFromFile(const char *path, RPGviewport *viewport, RPGtilemap **tilemap) {
    RPG_ENSURE_FILE(path);

    if (tmx_alloc_func == NULL) {
        tmx_alloc_func    = RPG_REALLOC;
        tmx_free_func     = RPG_FREE;
        tmx_img_load_func = RPG_Tilemap_ImageLoad;
        tmx_img_free_func = RPG_Tilemap_ImageFree;
    } 

    tmx_map *map = tmx_load(path);
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
            next = &tile->tmx->tileset->tiles[frame->tile_id];

            // left/top/right/bottom in normalized texture coordinates
            GLfloat l = next->ul_x / (GLfloat) layer->image->width;
            GLfloat t = next->ul_y / (GLfloat) layer->image->height;
            GLfloat r = l + (next->tileset->tile_width / (GLfloat)  layer->image->width);
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
        tilemap->base.ox = x;
        tilemap->base.oy = y;
        tilemap->base.updated = RPG_TRUE;
    }
    return RPG_NO_ERROR;
}