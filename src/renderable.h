//
// Created by eric on 8/23/19.
//

#ifndef OPEN_RPG_RENDERABLE_H
#define OPEN_RPG_RENDERABLE_H 1

#include "internal.h"
#include "rpg.h"
#include "game.h"

/**
 * @brief Base structure for objects that can be rendered. MUST BE FIRST FIELD IN IMPLEMENTATIONS!
 */
typedef struct RPGrenderable {
    RPGint z;        /** The position of the object on the z-axis. */
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
        RPG_BLEND_OP op;
        RPG_BLEND src;
        RPG_BLEND dst;
    } blend;              /** The blending factors to apply during rendering. */
    RPGrenderfunc render; /** The function to call when the object needs rendered. */
    RPGmat4 model;        /** The model matrix for the object. */
    RPGgame *game;        /** The host game */
    void *user;           /** Arbitrary user-defined pointer to store with this instance */
} RPGrenderable;

#define RPG_BASE_UNIFORMS(r)                                                                                                               \
    glUniform4f(r.game->shader.color, r.color.x, r.color.y, r.color.z, r.color.w);                                                         \
    glUniform4f(r.game->shader.tone, r.tone.x, r.tone.y, r.tone.z, r.tone.w);                                                              \
    glUniform1f(r.game->shader.alpha, r.alpha);                                                                                            \
    glUniform1f(r.game->shader.hue, r.hue);                                                                                                \
    glUniform4f(r.game->shader.flash, r.flash.color.x, r.flash.color.y, r.flash.color.z, r.flash.color.w);                                 \
    glUniformMatrix4fv(r.game->shader.model, 1, GL_FALSE, (GLfloat *)&r.model);                                                            \
    glBlendEquation(r.blend.op);                                                                                                           \
    glBlendFunc(r.blend.src, r.blend.dst)

void RPG_Renderable_Init(RPGgame *game, RPGrenderable *renderable, RPGrenderfunc renderfunc);

#endif /* OPEN_RPG_RENDERABLE_H */
