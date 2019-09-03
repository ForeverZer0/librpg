
#include "internal.h"
#include "tmx/tmx.h"
#include <stdio.h>

#define RPG_LAYER_OFFSET 32

/****************************************************************************************
 * Type Definitions
 ****************************************************************************************/

typedef struct
{
    tmx_tile *tmx; /** The TMX format tile structure. */
    struct
    {
        RPGubyte index;  /** The current animation frame index. */
        RPGdouble delta; /** The target time when the next animation will occur. */
    } anime;             /** Structure containing animation timing. */
} RPGtile;

// Tile Layer
typedef struct
{
    GLuint vao;
    GLuint vbo;
    RPGimage *image;
    RPGuint tileCount;
    RPGtile *tiles;
} RPGtilelayer;

// Image Layer
typedef struct
{
    RPGimage *image;
    GLuint vao;
    GLuint vbo;
} RPGimagelayer;

// Object Layer
typedef struct
{
    int stuff;
} RPGobjectlayer;

// Group Layer
typedef struct
{
    int stuff;
} RPGgrouplayer;

typedef struct
{
    RPGrenderable renderable;
    RPGint type;
    union {
        RPGtilelayer *tile;
        RPGimagelayer *image;
        RPGobjectlayer *object;
        RPGgrouplayer *group;
    } layer;
    tmx_layer *tmx;
    RPGtilemap *parent;
} RPGlayer;

typedef struct RPGtilemap
{
    RPGbasic base;
    RPGbatch layers;
    tmx_map *map;
    RPGfloat pxWidth;
    RPGfloat pxHeight;
    struct
    {
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

typedef struct
{
    RPGvec2 vertices[6];
    RPGmat4 model;
} RPGvertexdata;

/****************************************************************************************
 * Helper Functions
 ****************************************************************************************/

/**
 * @brief Configures the vertex attributes for the currently bound VBO.
 *
 * @param program The tilemap's shader program.
 * @param vao The vertex array object to configure.
 */
static void RPG_Tilemap_SetupVAO(GLuint program, GLuint vao)
{
    // Get the locations for the vertex and model attributes in the vertex shader
    GLint vLoc = glGetAttribLocation(program, "vertex");
    GLint mLoc = glGetAttribLocation(program, "model");

    glBindVertexArray(vao);
    for (int i = 0; i < 6; i++)
    {
        glEnableVertexAttribArray(vLoc + i);
        glVertexAttribPointer(vLoc + i, 4, GL_FLOAT, GL_TRUE, 0, (void *) (sizeof(RPGvec2) * i));
        glVertexAttribDivisor(vLoc + i, 1);
    }

    // Set four vec4 attribute locations to store as a 4x4 matrix
    for (int i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(mLoc + i);
        glVertexAttribPointer(mLoc + i, 4, GL_FLOAT, GL_TRUE, 0, (void *) ((sizeof(RPGvec2) * 6) + (sizeof(RPGvec4) * i)));
        glVertexAttribDivisor(mLoc + i, 1);
    }
}

/**
 * @brief Load function for the TMX library to use for laoding resource images.
 *
 * @param path The path to the resource image.
 * @return void* The created image.
 */
static void *RPG_Tilemap_ImageLoad(const char *path)
{
    if (RPG_FILE_EXISTS(path))
    {
        RPGimage *image;
        if (RPG_Image_CreateFromFile(path, &image) == RPG_NO_ERROR)
        {
            return image;
        }
    }
    return NULL;
}

/**
 * @brief Free function for the TMX library to use for freeing resource images.
 *
 * @param image A pointer to the image to free.
 */
static void RPG_Tilemap_ImageFree(void *image) { RPG_Image_Free(image); }

/**
 * @brief Sets up an array of vertex data for a VAO.
 *
 * @param l The normalized texture coordinate of the left side on the x-axis.
 * @param t The normalized texture coordinate of the right side on the x-axis.
 * @param r The normalized texture coordinate of the top side on the y-axis.
 * @param b The normalized texture coordinate of the bottom side on the y-axis.
 * @param gid The global ID of a tile, or 0 to ignore. Bit-flags for rotation/flipping will be honored.
 * @param vertices The array of vertices the data will be written to.
 */
static void RPG_Tilemap_SetVertices(float l, float t, float r, float b, RPGint gid, RPGvec2 vertices[6])
{
    // Determine placement of vertices based on flag set in GID
    if ((gid & TMX_FLIPPED_VERTICALLY) != 0 && (gid & TMX_FLIPPED_HORIZONTALLY) != 0)
    {
        RPGvec2 v[] = {{r, t}, {l, b}, {r, b}, {r, t}, {l, t}, {l, b}};
        memcpy(vertices, v, sizeof(v));
    }
    else if ((gid & TMX_FLIPPED_HORIZONTALLY) != 0)
    {
        if ((gid & TMX_FLIPPED_DIAGONALLY) != 0)
        {
            RPGvec2 v[] = {{r, b}, {l, t}, {l, b}, {r, b}, {r, t}, {l, t}};
            memcpy(vertices, v, sizeof(v));
        }
        else
        {
            RPGvec2 v[] = {{r, b}, {l, t}, {r, t}, {r, b}, {l, b}, {l, t}};
            memcpy(vertices, v, sizeof(v));
        }
    }
    else if ((gid & TMX_FLIPPED_VERTICALLY) != 0)
    {
        if ((gid & TMX_FLIPPED_DIAGONALLY) != 0)
        {
            RPGvec2 v[] = {{l, t}, {r, b}, {r, t}, {l, t}, {l, b}, {r, b}};
            memcpy(vertices, v, sizeof(v));
        }
        else
        {
            RPGvec2 v[] = {{l, t}, {r, b}, {l, b}, {l, t}, {r, t}, {r, b}};
            memcpy(vertices, v, sizeof(v));
        }
    }
    else
    {
        RPGvec2 v[] = {{l, b}, {r, t}, {l, t}, {l, b}, {r, b}, {r, t}};
        memcpy(vertices, v, sizeof(v));
    }
}

/**
 * @brief Creates and sets up the shader information for this tilemap.
 *
 * @param tilemap A pointer to the tilemap to configure shader for.
 */
static void RPG_Tilemap_CreateShader(RPGtilemap *tilemap)
{
    RPGshader *shader;
    size_t s;
    char *tempv;

    RPG_ReadFile("/home/eric/Desktop/rpg/tilemap.vert", &tempv, &s);  // FIXME: Embed in "assets.c"
    RPG_Shader_Create(tempv, RPG_FRAGMENT_SHADER, NULL, &shader);
    RPG_ASSERT(shader);

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

/****************************************************************************************
 * Render Functions
 ****************************************************************************************/

/**
 * @brief Renders a tile layer.
 *
 * @param layer A pointer to the layer, can be cast to an RPGlayer structure.
 */
static void RPG_Tilemap_RenderTileLayer(void *layer)
{
    RPGlayer *l = (RPGlayer *) layer;

    glUseProgram(l->parent->shader.program);
    RPG_Drawing_SetBlending(l->parent->base.blend.op, l->parent->base.blend.src, l->parent->base.blend.dst);

    glUniform1f(l->parent->shader.alpha, l->parent->base.alpha * (GLfloat) l->tmx->opacity);

    // Bind tileset texture
    RPG_Drawing_BindTexture(l->layer.tile->image->texture, GL_TEXTURE0);
    glBindVertexArray(l->layer.tile->vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, l->layer.tile->tileCount * 6);
    glUseProgram(RPG_GAME->shader.program);
}

/**
 * @brief Renders an image layer.
 *
 * @param layer A pointer to the layer, can be cast to an RPGlayer structure.
 */
static void RPG_Tilemap_RenderImageLayer(void *layer)
{
    RPGlayer *l = (RPGlayer *) layer;

    glUseProgram(l->parent->shader.program);
    RPG_Drawing_SetBlending(l->parent->base.blend.op, l->parent->base.blend.src, l->parent->base.blend.dst);

    glUniform1f(l->parent->shader.alpha, l->parent->base.alpha * (GLfloat) l->tmx->opacity);

    // Bind tileset texture
    RPG_Drawing_BindTexture(l->layer.image->image->texture, GL_TEXTURE0);
    glBindVertexArray(l->layer.image->vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 6);
    glUseProgram(RPG_GAME->shader.program);
}

/**
 * @brief Render function for a tilemap. Does not actually render, as layers are independent, only configures shader uniforms.
 *
 * @param tilemap A pointer to the RPGtilemap layer struct to render.
 */
static void RPG_Tilemap_Render(void *tilemap)
{
    RPGtilemap *t = tilemap;
    // Skip if tilemap is not visible
    if (!t->base.renderable.visible || t->base.alpha < __FLT_EPSILON__)
    {
        return;
    }

    // Activate the tilemap's shader program
    glUseProgram(t->shader.program);

    // Update tilemap model matrix and uniforms if changed since last render
    if (t->base.updated)
    {
        // Set model matrix for tilemap, which is actually a projection matrix for the layers
        RPG_MAT4_ORTHO(t->base.model, 0.0f, RPG_GAME->resolution.width, RPG_GAME->resolution.height, 0.0f, -1.0f, 1.0f);
        // RPG_MAT4_ORTHO(t->base.model, 0.0f, w, h, 0.0f, -1.0f, 1.0f);
        // glUniformMatrix4fv(t->shader.projection, 1, GL_FALSE, (GLfloat *) &t->base.model);
        glUniformMatrix4fv(t->shader.projection, 1, GL_FALSE, (GLfloat *) &RPG_GAME->projection);  // TODO: Update on resolution change?

        // Update any changes to the origin point and clear "dirty" flag
        glUniform2f(t->shader.origin, (float) t->base.ox, (float) t->base.oy);
        t->base.updated = RPG_FALSE;
    }

    // Setup shared shader uniforms
    glUniform1f(t->shader.alpha, t->base.alpha);
    glUniform4f(t->shader.color, t->base.color.x, t->base.color.y, t->base.color.z, t->base.color.w);
    glUniform4f(t->shader.tone, t->base.tone.x, t->base.tone.y, t->base.tone.z, t->base.tone.w);
    glUniform1f(t->shader.hue, t->base.hue);
    glUniform4f(t->shader.flash, t->base.flash.color.x, t->base.flash.color.y, t->base.flash.color.z, t->base.flash.color.w);

    // No actual rendering, just update shader uniforms and ortho if needed
    glUseProgram(RPG_GAME->shader.program);
}

/****************************************************************************************
 * Initialization
 ****************************************************************************************/

static RPGtilelayer *RPG_Tilemap_CreateTileLayer(RPGtilemap *tilemap, tmx_map *map, tmx_layer *layer)
{

    // Initialize tile layer struct and storage for tiles
    RPGtilelayer *tilelayer = RPG_ALLOC(RPGtilelayer);
    tilelayer->tileCount    = map->width * map->height;
    tilelayer->image        = NULL;
    tilelayer->tiles        = RPG_MALLOC(sizeof(RPGtile) * tilelayer->tileCount);
    memset(tilelayer->tiles, 0, sizeof(RPGtile) * tilelayer->tileCount);

    RPGvec2 vertices[6];

    glGenVertexArrays(1, &tilelayer->vao);
    glGenBuffers(1, &tilelayer->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, tilelayer->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RPGvertexdata) * tilelayer->tileCount, NULL, GL_DYNAMIC_DRAW);
    
    RPGvertexdata *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    // Enumerate throught each tile coordinate in the map for this layer
    RPGuint index = 0;
    RPGint count = 0;
    for (RPGuint mapY = 0; mapY < map->height; mapY++)
    {
        for (RPGuint mapX = 0; mapX < map->width; mapX++)
        {

            index = mapX + (mapY * map->width);

            // Get the current tile for this location
            RPGint gid        = layer->content.gids[index];
            tmx_tile *tmxtile = map->tiles[gid & TMX_FLIP_BITS_REMOVAL];

            if (gid == 0)
            {
                memset(&ptr[index], 0, sizeof(RPGvertexdata));
                // Skip if there is no tile here
                continue;
            }

            // Set the image used for this layer (multiple tilesets within one layer not supported)
            if (tilelayer->image == NULL)
            {
                tilelayer->image = tmxtile->tileset->image->resource_image;
            }
            else if (tilelayer->image != tmxtile->tileset->image->resource_image)
            {
                fprintf(stderr, "only one tileset per layer is supported");
            }

            GLfloat tl = RPG_Drawing_GetUV(tmxtile->ul_x, tilelayer->image->width);
            GLfloat tt = RPG_Drawing_GetUV(tmxtile->ul_y, tilelayer->image->height);
            GLfloat tr = RPG_Drawing_GetUV(tmxtile->ul_x + tmxtile->tileset->tile_width - 1, tilelayer->image->width);
            GLfloat tb = RPG_Drawing_GetUV(tmxtile->ul_y + tmxtile->tileset->tile_height - 1, tilelayer->image->height);
            RPG_Tilemap_SetVertices(tl, tt, tr, tb, gid, vertices);

            memcpy(&ptr[index].vertices, vertices, sizeof(vertices));
            RPG_MAT4_SET(ptr[index].model, map->tile_width, 0.0f, 0.0f, 0.0f, 0.0f, map->tile_height, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                         mapX * map->tile_width, mapY * map->tile_height, 0.0f, 1.0f);

            // Store the info in the tile
            tilelayer->tiles[index].tmx = tmxtile;
        }
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);

    RPG_Tilemap_SetupVAO(tilemap->shader.program, tilelayer->vao);
    return tilelayer;
}

static RPGimagelayer *RPG_Tilemap_CreateImageLayer(RPGtilemap *tilemap, tmx_layer *layer)
{

    RPGimagelayer *l = RPG_ALLOC(RPGimagelayer);
    l->image         = layer->content.image->resource_image;
    RPG_ASSERT(l->image);

    RPGvertexdata obj;
    RPG_Tilemap_SetVertices(0.0f, 0.0f, 1.0f, 1.0f, 0, obj.vertices);
    RPG_MAT4_SET(obj.model, l->image->width, 0, 0, 0, 0, l->image->height, 0, 0, 0, 0, 1.0f, 0, layer->offsetx, layer->offsety, 0, 1.0f);

    glGenVertexArrays(1, &l->vao);
    glGenBuffers(1, &l->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, l->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RPGvertexdata), &obj, GL_STATIC_DRAW);

    RPG_Tilemap_SetupVAO(tilemap->shader.program, l->vao);
    return l;
}

static RPGobjectlayer *RPG_Tilemap_CreateObjectLayer(tmx_map *map, tmx_layer *layer) { return NULL; }

static RPGgrouplayer *RPG_Tilemap_CreateGroupLayer(tmx_map *map, tmx_layer *layer) { return NULL; }

static void RPG_Tilemap_Initialize(tmx_map *map, RPGviewport *viewport, RPGtilemap **tilemap)
{
    RPG_ALLOC_ZERO(tm, RPGtilemap);
    RPGbatch *batch = viewport ? &viewport->batch : &RPG_GAME->batch;
    RPG_BasicSprite_Init(&tm->base, RPG_Tilemap_Render, batch);
    RPG_Tilemap_CreateShader(tm);

    // Store the map, count the layers, and allocate memory for them
    tm->map      = map;
    tm->pxWidth  = map->width * map->tile_width;
    tm->pxHeight = map->height * map->tile_height;
    RPG_Batch_Init(&tm->layers);

    // Enumerate each layer
    int index = 0;
    for (tmx_layer *layer = map->ly_head; layer != NULL; layer = layer->next, index++)
    {
        RPG_ALLOC_ZERO(base, RPGlayer);
        base->type = layer->type;
        base->tmx  = layer;
        switch (layer->type)
        {
            case L_LAYER:
            {
                base->layer.tile = RPG_Tilemap_CreateTileLayer(tm, map, layer);
                RPG_Renderable_Init(&base->renderable, RPG_Tilemap_RenderTileLayer, batch);
                break;
            }
            case L_IMAGE:
            {
                base->layer.image = RPG_Tilemap_CreateImageLayer(tm, layer);
                RPG_Renderable_Init(&base->renderable, RPG_Tilemap_RenderImageLayer, batch);
                break;
            }
            case L_GROUP:
            {
                base->layer.object = RPG_Tilemap_CreateObjectLayer(map, layer);
                break;
            }
            case L_OBJGR:
            {
                base->layer.group = RPG_Tilemap_CreateGroupLayer(map, layer);
                break;
            }
            default:
            {
                fprintf(stderr, "invalid layer type");
                break;
            }
        }
        base->parent             = tm;
        base->renderable.visible = (RPGbool) layer->visible;
        base->renderable.z       = index * RPG_LAYER_OFFSET;
        RPG_Batch_Add(&tm->layers, &base->renderable);
    }
    *tilemap = tm;
}

tmx_resource_manager *TEST;  // FIXME:

RPG_RESULT RPG_Tilemap_Create(const void *buffer, RPGsize size, RPGviewport *viewport, RPGtilemap **tilemap)
{

    if (tmx_alloc_func == NULL)
    {
        tmx_alloc_func    = RPG_REALLOC;
        tmx_free_func     = RPG_FREE;
        tmx_img_load_func = RPG_Tilemap_ImageLoad;
        tmx_img_free_func = RPG_Tilemap_ImageFree;
    }
    tmx_map *map = tmx_rcmgr_load_buffer(TEST, buffer, (int) size);
    // tmx_map *map = tmx_load_buffer(buffer, (int) size);
    if (map == NULL)
    {
        return RPG_ERR_UNKNOWN;
    }
    RPG_Tilemap_Initialize(map, viewport, tilemap);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_CreateFromFile(const char *path, RPGviewport *viewport, RPGtilemap **tilemap)
{
    RPG_ENSURE_FILE(path);

    if (TEST == NULL)
    {
        TEST              = tmx_make_resource_manager();
        tmx_alloc_func    = RPG_REALLOC;
        tmx_free_func     = RPG_FREE;
        tmx_img_load_func = RPG_Tilemap_ImageLoad;
        tmx_img_free_func = RPG_Tilemap_ImageFree;
    }
    tmx_map *map = tmx_rcmgr_load(TEST, path);
    // tmx_map *map = tmx_load(path);
    if (map == NULL)
    {
        return RPG_ERR_UNKNOWN;
    }
    RPG_Tilemap_Initialize(map, viewport, tilemap);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_Free(RPGtilemap *tilemap)
{
    if (tilemap != NULL)
    {

        // TODO:
        RPG_FREE(tilemap);
    }
    return RPG_NO_ERROR;
}

/****************************************************************************************
 * Updating
 ****************************************************************************************/

static void RPG_Tilemap_UpdateTileLayer(RPGtilemap *map, RPGtilelayer *layer, tmx_layer *tmx)
{

    // Declare variable storage;
    tmx_tile *next;
    tmx_anim_frame *frame;
    RPGtile *tile;
    RPGvertexdata *ptr;
    RPGbool bound = RPG_FALSE;

    // Get the current time
    RPGdouble time = glfwGetTime();

    // Enumerate through tiles of layer
    for (RPGuint i = 0; i < layer->tileCount; i++)
    {
        tile = &layer->tiles[i];

        // If tile is defined and is time has been reached for a tile change
        if (tile->tmx && tile->tmx->animation_len > 0 && tile->anime.delta < time)
        {

            // Increment the current index, and get the next frame
            tile->anime.index = (tile->anime.index + 1) % tile->tmx->animation_len;
            frame             = &tile->tmx->animation[tile->anime.index];
            next              = &tile->tmx->tileset->tiles[frame->tile_id];

            // left/top/right/bottom in normalized texture coordinates
            GLfloat l = RPG_Drawing_GetUV(next->ul_x, layer->image->width);
            GLfloat t = RPG_Drawing_GetUV(next->ul_y, layer->image->height);
            GLfloat r = RPG_Drawing_GetUV(next->ul_x + next->tileset->tile_width - 1, layer->image->width);
            GLfloat b = RPG_Drawing_GetUV(next->ul_y + next->tileset->tile_height - 1, layer->image->height);

            // Buffer the new data, and set the next target time for an animation
            tile->anime.delta = time + (frame->duration * 0.001);
            if (!bound)
            {
                // Lazy-bind the buffer only if needed
                glBindBuffer(GL_ARRAY_BUFFER, layer->vbo);
                ptr   = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                bound = RPG_TRUE;
            }

            // Update the vertices directly in the VBO
            RPG_Tilemap_SetVertices(l, t, r, b, next->id, ptr[i].vertices);
        }
    }
    if (bound)
    {
        // Unmap our pointer if buffer was bound
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

RPG_RESULT RPG_Tilemap_Update(RPGtilemap *tilemap)
{
    RPG_RETURN_IF_NULL(tilemap);
    RPG_BasicSprite_Update(&tilemap->base);

    RPGlayer *layer;
    for (RPGuint i = 0; i < tilemap->layers.total; i++)
    {
        layer = (RPGlayer *) tilemap->layers.items[i];
        if (layer->type == L_LAYER)
        {
            RPG_Tilemap_UpdateTileLayer(tilemap, layer->layer.tile, layer->tmx);
        }
    }

    return RPG_NO_ERROR;
}

/****************************************************************************************
 * Public Getters/Setters
 ****************************************************************************************/

RPG_RESULT RPG_Tilemap_GetSize(RPGtilemap *tilemap, RPGint *width, RPGint *height)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (width != NULL)
    {
        *width = tilemap->map->width;
    }
    if (height != NULL)
    {
        *height = tilemap->map->height;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetTileSize(RPGtilemap *tilemap, RPGint *width, RPGint *height)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (width != NULL)
    {
        *width = tilemap->map->tile_width;
    }
    if (height != NULL)
    {
        *height = tilemap->map->tile_height;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetTileCount(RPGtilemap *tilemap, RPGuint *count)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (count != NULL)
    {
        *count = tilemap->map->tilecount;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetOrigin(RPGtilemap *tilemap, RPGint *x, RPGint *y)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (x != NULL)
    {
        *x = tilemap->base.ox;
    }
    if (y != NULL)
    {
        *y = tilemap->base.oy;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_SetOrigin(RPGtilemap *tilemap, RPGint x, RPGint y)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (tilemap->base.ox != x || tilemap->base.oy != y)
    {
        tilemap->base.ox      = x;
        tilemap->base.oy      = y;
        tilemap->base.updated = RPG_TRUE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetLayerCount(RPGtilemap *tilemap, RPGint *count)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (count != NULL)
    {
        *count = tilemap->layers.total;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetLayerAlpha(RPGtilemap *tilemap, RPGint layer, RPGfloat *alpha)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (layer < 0 || layer >= tilemap->layers.total)
    {
        return RPG_ERR_OUT_OF_RANGE;
    }
    if (alpha != NULL)
    {
        RPGlayer *l = (RPGlayer *) tilemap->layers.items[layer];
        *alpha      = (RPGfloat) l->tmx->opacity;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_SetLayerAlpha(RPGtilemap *tilemap, RPGint layer, RPGfloat alpha)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (layer < 0 || layer >= tilemap->layers.total)
    {
        return RPG_ERR_OUT_OF_RANGE;
    }
    RPGlayer *l     = (RPGlayer *) tilemap->layers.items[layer];
    l->tmx->opacity = RPG_CLAMPF(alpha, 0.0f, 1.0f);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetLayerVisible(RPGtilemap *tilemap, RPGint layer, RPGbool *visible)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (layer < 0 || layer >= tilemap->layers.total)
    {
        return RPG_ERR_OUT_OF_RANGE;
    }
    if (visible != NULL)
    {
        *visible = tilemap->layers.items[layer]->visible;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_SetLayerVisible(RPGtilemap *tilemap, RPGint layer, RPGbool visible)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (layer < 0 || layer >= tilemap->layers.total)
    {
        return RPG_ERR_OUT_OF_RANGE;
    }
    tilemap->layers.items[layer]->visible = visible;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_GetLayerZ(RPGtilemap *tilemap, RPGint layer, RPGint *z)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (layer < 0 || layer >= tilemap->layers.total)
    {
        return RPG_ERR_OUT_OF_RANGE;
    }
    if (z != NULL)
    {
        *z = tilemap->layers.items[layer]->z;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Tilemap_SetLayerZ(RPGtilemap *tilemap, RPGint layer, RPGint z)
{
    RPG_RETURN_IF_NULL(tilemap);
    if (layer < 0 || layer >= tilemap->layers.total)
    {
        return RPG_ERR_OUT_OF_RANGE;
    }
    tilemap->layers.items[layer]->z = z;
    return RPG_NO_ERROR;
}