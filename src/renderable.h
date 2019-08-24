#ifndef OPEN_RPG_RENDERABLE_H
#define OPEN_RPG_RENDERABLE_H 1

#include "game.h"
#include "internal.h"
#include "rpg.h"

#define VERTICES_COUNT 24
#define VERTICES_SIZE (sizeof(RPGfloat) * VERTICES_COUNT)
#define VERTICES_STRIDE (sizeof(RPGfloat) * 4)

typedef void (*RPGrenderfunc)(void *renderable);

typedef struct RPGimage {
    RPGint width;
    RPGint height;
    GLuint texture;
    GLuint fbo;
    RPGgame *game;
    void *user;
} RPGimage;

/**
 * @brief Base structure for objects that can be rendered.
 */
typedef struct RPGrenderable {
    RPGint x;        /** The location of the sprite on the x-axis. */
    RPGint y;        /** The location of the sprite on the y-axis. */
    RPGint z;        /** The position of the sprite on the z-axis. */
    RPGint ox;       /** The origin point on the x-axis, context-dependent definition. */
    RPGint oy;       /** The origin point on the y-axis, context-dependent definition. */
    RPGbool updated; /** Flag indicating if the model matrix needs updated to reflect changes. */
    RPGbool visible; /** Flag indicating if object should be rendered. */
    RPGfloat alpha;  /** The opacity level to be rendered at in the range of 0.0 to 1.0. */
    RPGcolor color;  /** The color to blended when rendered. */
    RPGtone tone;    /** The tone to apply when rendered. */
    RPGfloat hue;    /** The amount of hue to apply, in degrees. */
    RPGvec2 scale;   /** The amount of scale to apply when rendered. */
    struct {
        RPGcolor color;    /** The color to use for the flash effect. */
        RPGubyte duration; /** The number of remaining frames to apply the flash effect. */
    } flash;               /** The flash effect to apply when rendered. */
    struct {
        RPGfloat radians; /** The amount of rotation, in radians. */
        RPGint ox;        /** The anchor point on the x-axis to rotate around, relative to the sprite. */
        RPGint oy;        /** The anchor point on the y-axis to rotate around, relative to the sprite. */
    } rotation;
    struct {
        RPG_BLEND_OP op;  /** The equation used for combining the source and destination factors. */
        RPG_BLEND src;    /** The factor to be used for the source pixel color. */
        RPG_BLEND dst;    /** The factor ot used for the destination pixel color. */
    } blend;              /** The blending factors to apply during rendering. */
    RPGrenderfunc render; /** The function to call when the object needs rendered. */
    RPGmat4 model;        /** The model matrix for the object. */
    RPGgame *game;        /** The host game */
    RPGbatch *parent;     /** Pointer to the rendering batch the object is contained within */
    void *user;           /** Arbitrary user-defined pointer to store with this instance */
} RPGrenderable;

/**
 * @brief Contains the information required to render an arbitrary image on-screen.
 */
typedef struct RPGsprite {
    RPGrenderable base;    /** The base renderable object, MUST BE FIRST FIELD IN THE STRUCTURE! */
    RPGimage *img;         /** A pointer ot the sprite's image, or NULL. */
    RPGviewport *viewport; /** A pointer to the sprite's viewport, or NULL. */
    RPGrect rect;          /** The source rectangle of the sprite's image. */
    GLuint vbo;            /** The Vertex Buffer Object bound to this sprite. */
    GLuint vao;            /** The Vertex Array Object bound to this sprite. */
} RPGsprite;

/**
 * @brief A container for sprites that is drawn in its own independent batch with its own projection.
 */
typedef struct RPGviewport {
    RPGrenderable base; /** The base renderable object, MUST BE FIRST FIELD IN THE STRUCTURE! */
    RPGint width;       /** The dimension of the viewport, in pixels, on the x-axis. */
    RPGint height;      /** The dimension of the viewport, in pixels, on the y-axis. */
    RPGbatch batch;     /** A collection containing pointers to the sprites within this viewport. */
    GLuint fbo;         /** The Framebuffer Object for rendering to the viewport. */
    GLuint texture;     /** Texture with the viewport batch rendered onto it. */
    RPGmat4 projection; /** The projection matrix for the viewport. */
    GLuint vbo;         /** The Vertex Buffer Object bound to this viewport. */
    GLuint vao;         /** The Vertex Array Object bound to this viewport. */
} RPGviewport;

#define RPG_BASE_UNIFORMS(r)                                                                                                               \
    glUniform4f(r.game->shader.color, r.color.x, r.color.y, r.color.z, r.color.w);                                                         \
    glUniform4f(r.game->shader.tone, r.tone.x, r.tone.y, r.tone.z, r.tone.w);                                                              \
    glUniform1f(r.game->shader.alpha, r.alpha);                                                                                            \
    glUniform1f(r.game->shader.hue, r.hue);                                                                                                \
    glUniform4f(r.game->shader.flash, r.flash.color.x, r.flash.color.y, r.flash.color.z, r.flash.color.w);                                 \
    glUniformMatrix4fv(r.game->shader.model, 1, GL_FALSE, (GLfloat *) &r.model);                                                           \
    glBlendEquation(r.blend.op);                                                                                                           \
    glBlendFunc(r.blend.src, r.blend.dst)

void RPG_Renderable_Init(RPGgame *game, RPGrenderable *renderable, RPGrenderfunc renderfunc, RPGbatch *batch);

#endif /* OPEN_RPG_RENDERABLE_H */
