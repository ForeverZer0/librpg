#ifndef OPEN_RPG_INTERNAL_H
#define OPEN_RPG_INTERNAL_H 1

#include "platform.h"
#include <math.h>
#include <string.h>

extern const char *RPG_VERTEX_SHADER;
extern const char *RPG_FRAGMENT_SHADER;
extern const char *RPG_FONT_VERTEX;
extern const char *RPG_FONT_FRAGMENT;

#define RPG_PI 3.14159274f

#define RPG_ENSURE_FILE(filename)                                                                                                          \
    if (!RPG_FILE_EXISTS(filename))                                                                                                        \
    return RPG_ERR_FILE_NOT_FOUND

#define RPG_RETURN_IF_NULL(ptr)                                                                                                            \
    if (ptr == NULL)                                                                                                                       \
    return RPG_ERR_INVALID_POINTER

static inline int imax(int i1, int i2) { return i1 > i2 ? i1 : i2; }

static inline int imin(int i1, int i2) { return i1 < i2 ? i1 : i2; }

#define RPG_CLAMPF(v, min, max) (fmaxf(min, fminf(max, v)))

#define RPG_CLAMPI(v, min, max) (imax(min, imin(v, max)))

#define DEF_FX_PARAM_F(type, name, param, min, max)                                                                                        \
    RPG_RESULT RPG_##type##_Get##name(RPGaudiofx *fx, RPGfloat *value) {                                                                   \
        RPG_RETURN_IF_NULL(fx);                                                                                                            \
        RPG_RETURN_IF_NULL(value);                                                                                                         \
        alGetEffectf(fx->effect, param, value);                                                                                            \
        return RPG_NO_ERROR;                                                                                                               \
    }                                                                                                                                      \
    RPG_RESULT RPG_##type##_Set##name(RPGaudiofx *fx, RPGfloat value) {                                                                    \
        RPG_RETURN_IF_NULL(fx);                                                                                                            \
        alEffectf(fx->effect, param, RPG_CLAMPF(value, min, max));                                                                         \
        if (alGetError())                                                                                                                  \
            return RPG_ERR_INVALID_POINTER;                                                                                                \
        RPG_Audio_UpdateEffectChange(fx);                                                                                                  \
        return RPG_NO_ERROR;                                                                                                               \
    }

#define DEF_FX_PARAM_I(type, name, param, min, max)                                                                                        \
    RPG_RESULT RPG_##type##_Get##name(RPGaudiofx *fx, RPGint *value) {                                                                     \
        RPG_RETURN_IF_NULL(fx);                                                                                                            \
        RPG_RETURN_IF_NULL(value);                                                                                                         \
        alGetEffecti(fx->effect, param, value);                                                                                            \
        return RPG_NO_ERROR;                                                                                                               \
    }                                                                                                                                      \
    RPG_RESULT RPG_##type##_Set##name(RPGaudiofx *fx, RPGint value) {                                                                      \
        RPG_RETURN_IF_NULL(fx);                                                                                                            \
        alEffecti(fx->effect, param, RPG_CLAMPI(value, min, max));                                                                         \
        if (alGetError())                                                                                                                  \
            return RPG_ERR_INVALID_POINTER;                                                                                                \
        RPG_Audio_UpdateEffectChange(fx);                                                                                                  \
        return RPG_NO_ERROR;                                                                                                               \
    }

#define DEF_FX_PARAM_V(type, name, param)                                                                                                  \
    RPG_RESULT RPG_##type##_Get##name(RPGaudiofx *fx, RPGvec3 *value) {                                                                    \
        RPG_RETURN_IF_NULL(fx);                                                                                                            \
        RPG_RETURN_IF_NULL(value);                                                                                                         \
        alGetEffectfv(fx->effect, param, (RPGfloat *) value);                                                                              \
        return RPG_NO_ERROR;                                                                                                               \
    }                                                                                                                                      \
    RPG_RESULT RPG_##type##_Set##name(RPGaudiofx *fx, RPGvec3 *value) {                                                                    \
        RPG_RETURN_IF_NULL(fx);                                                                                                            \
        alEffectfv(fx->effect, param, (RPGfloat *) value);                                                                                 \
        if (alGetError())                                                                                                                  \
            return RPG_ERR_INVALID_POINTER;                                                                                                \
        RPG_Audio_UpdateEffectChange(fx);                                                                                                  \
        return RPG_NO_ERROR;                                                                                                               \
    }

#define DEF_FX_CREATE(name, type)                                                                                                          \
    RPG_RESULT RPG_##name##_Create(RPGaudiofx **fx) { return RPG_Audio_CreateEffect(type, fx); }

#define RPG_ALLOC(type) ((type *) RPG_MALLOC(sizeof(type)))
#define RPG_ALLOC_N(type, n) ((type *) RPG_MALLOC(sizeof(type) * n))
#define RPG_ALLOC_ZERO(var, type)                                                                                                          \
    type *var = RPG_ALLOC(type);                                                                                                           \
    memset(var, 0, sizeof(type))

#define RPG_MAT4_ORTHO(mat4, left, right, top, bottom, near, far)                                                                          \
    mat4.m11 = 2.0f / (right - left);                                                                                                      \
    mat4.m12 = mat4.m13 = mat4.m14 = 0.0f;                                                                                                 \
    mat4.m22                       = 2.0f / (top - bottom);                                                                                \
    mat4.m21 = mat4.m23 = mat4.m24 = 0.0f;                                                                                                 \
    mat4.m33                       = 1.0f / (near - far);                                                                                  \
    mat4.m31 = mat4.m32 = mat4.m34 = 0.0f;                                                                                                 \
    mat4.m41                       = (left + right) / (RPGfloat)(left - right);                                                            \
    mat4.m42                       = (top + bottom) / (RPGfloat)(bottom - top);                                                            \
    mat4.m43                       = near / (RPGfloat)(near - far);                                                                        \
    mat4.m44                       = 1.0f

#define RPG_MAT4_SET(mat4, _m11, _m12, _m13, _m14, _m21, _m22, _m23, _m24, _m31, _m32, _m33, _m34, _m41, _m42, _m43, _m44)                 \
    mat4.m11 = _m11;                                                                                                                       \
    mat4.m12 = _m12;                                                                                                                       \
    mat4.m13 = _m13;                                                                                                                       \
    mat4.m14 = _m14;                                                                                                                       \
    mat4.m21 = _m21;                                                                                                                       \
    mat4.m22 = _m22;                                                                                                                       \
    mat4.m23 = _m23;                                                                                                                       \
    mat4.m24 = _m24;                                                                                                                       \
    mat4.m31 = _m31;                                                                                                                       \
    mat4.m32 = _m32;                                                                                                                       \
    mat4.m33 = _m33;                                                                                                                       \
    mat4.m34 = _m34;                                                                                                                       \
    mat4.m41 = _m41;                                                                                                                       \
    mat4.m42 = _m42;                                                                                                                       \
    mat4.m43 = _m43;                                                                                                                       \
    mat4.m44 = _m44

#define RPG_RENDER_TEXTURE(TEXTURE, VAO)                                                                                                   \
    glActiveTexture(GL_TEXTURE0);                                                                                                          \
    glBindTexture(GL_TEXTURE_2D, TEXTURE);                                                                                                 \
    glBindVertexArray(VAO);                                                                                                                \
    glDrawArrays(GL_TRIANGLES, 0, 6);                                                                                                      \
    glBindVertexArray(0)

#define DEF_GETTER(name, param, objtype, paramtype, field)                                                                                 \
    RPG_RESULT RPG_##name##_Get##param(objtype *obj, paramtype *value) {                                                                   \
        RPG_RETURN_IF_NULL(obj);                                                                                                           \
        if (value != NULL) {                                                                                                               \
            *value = obj->field;                                                                                                           \
        }                                                                                                                                  \
        return RPG_NO_ERROR;                                                                                                               \
    }

#define DEF_SETTER(name, param, objtype, paramtype, field)                                                                                 \
    RPG_RESULT RPG_##name_Set##param(objtype *obj, paramtype value) {                                                                      \
        RPG_RETURN_IF_NULL(obj);                                                                                                           \
        obj->field = value;                                                                                                                \
        return RPG_NO_ERROR;                                                                                                               \
    }

#define DEF_PARAM(name, param, objtype, paramtype, field)                                                                                  \
    DEF_GETTER(name, param, objtype, paramtype, field)                                                                                     \
    DEF_SETTER(name, param, objtype, paramtype, field)

#define RPG_CHECK_DIMENSIONS(w, h)                                                                                                         \
    if ((w) < 1 || (h) < 1)                                                                                                                \
    return RPG_ERR_OUT_OF_RANGE

/**
 * @brief Resets the clear color back to the user-defined value.
 */
#define RPG_RESET_BACK_COLOR() glClearColor(RPG_GAME->color.x, RPG_GAME->color.y, RPG_GAME->color.z, RPG_GAME->color.w)

/**
 * @brief Resets the primary viewport to fit the window correctly.
 */
#define RPG_RESET_VIEWPORT() RPG_VIEWPORT(RPG_GAME->bounds.x, RPG_GAME->bounds.y, RPG_GAME->bounds.w, RPG_GAME->bounds.h)

/**
 * @brief Resets the primary projection matrix.
 */
#define RPG_RESET_PROJECTION() glUniformMatrix4fv(RPG_GAME->shader.projection, 1, GL_FALSE, (GLfloat *) &RPG_GAME->projection)

/**
 * @brief Sets the viewport and scissor rectangle of the primary viewport.
 */
#define RPG_VIEWPORT(x, y, w, h)                                                                                                           \
    glViewport(x, y, w, h);                                                                                                                \
    glScissor(x, y, w, h)

#define RPG_ENSURE_FBO(img)                                                                                                                \
    if (img->fbo == 0) {                                                                                                                   \
        glGenFramebuffers(1, &img->fbo);                                                                                                   \
        glBindFramebuffer(GL_FRAMEBUFFER, img->fbo);                                                                                       \
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img->texture, 0);                                      \
    } else                                                                                                                                 \
        glBindFramebuffer(GL_FRAMEBUFFER, img->fbo)

#define RPG_BIND_FBO(img, x, y, w, h)                                                                                                      \
    RPG_ENSURE_FBO(img);                                                                                                                   \
    RPGmat4 m;                                                                                                                             \
    RPG_MAT4_ORTHO(m, 0.0f, w, 0.0f, h, -1.0f, 1.0f);                                                                                      \
    glUniformMatrix4fv(RPG_GAME->shader.projection, 1, GL_FALSE, (RPGfloat *) &m);                                                         \
    RPG_VIEWPORT(x, y, w, h)

#define RPG_UNBIND_FBO(img)                                                                                                                \
    glBindFramebuffer(GL_FRAMEBUFFER, 0);                                                                                                  \
    RPG_RESET_PROJECTION();                                                                                                                \
    RPG_RESET_VIEWPORT()

#define RPG_FONT_DEFAULT_COLOR                                                                                                             \
    (RPGcolor) { 1.0f, 1.0f, 1.0f, 1.0f }
#define RPG_FONT_DEFAULT_SIZE 32

#endif /* OPEN_RPG_INTERNAL_H */