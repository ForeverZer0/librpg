#ifndef OPEN_RPG_INTERNAL_H
#define OPEN_RPG_INTERNAL_H 1

#include "glad.h"
#include "platform.h"
#include <string.h>

#define RPG_ENSURE_FILE(filename)                                                                                                          \
    if (!RPG_FILE_EXISTS(filename))                                                                                                        \
    return RPG_ERR_FILE_NOT_FOUND

#define RPG_RETURN_IF_NULL(ptr)                                                                                                            \
    if (ptr == NULL)                                                                                                                       \
    return RPG_ERR_INVALID_POINTER

static inline int maxi(int i1, int i2) { return i1 > i2 ? i1 : i2; }

static inline int mini(int i1, int i2) { return i1 < i2 ? i1 : i2; }

#define RPG_CLAMPF(v, min, max) (fmaxf(min, fminf(max, v)))

#define RPG_CLAMPI(v, min, max) (maxi(min, mini(v, max)))

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
        alGetEffectfv(fx->effect, param, (RPGfloat *)value);                                                                               \
        return RPG_NO_ERROR;                                                                                                               \
    }                                                                                                                                      \
    RPG_RESULT RPG_##type##_Set##name(RPGaudiofx *fx, RPGvec3 *value) {                                                                    \
        RPG_RETURN_IF_NULL(fx);                                                                                                            \
        alEffectfv(fx->effect, param, (RPGfloat *)value);                                                                                  \
        if (alGetError())                                                                                                                  \
            return RPG_ERR_INVALID_POINTER;                                                                                                \
        RPG_Audio_UpdateEffectChange(fx);                                                                                                  \
        return RPG_NO_ERROR;                                                                                                               \
    }

#define DEF_FX_CREATE(name, type)                                                                                                          \
    RPG_RESULT RPG_##name##_Create(RPGaudiofx **fx) { return RPG_Audio_CreateEffect(type, fx); }


#define RPG_ALLOC(type) ((type *)RPG_MALLOC(sizeof(type)))
#define RPG_ALLOC_N(type, n) ((type *)RPG_MALLOC(sizeof(type) * n))
#define RPG_ALLOC_ZERO(var, type)                                                                                                          \
    type *var = RPG_ALLOC(type);                                                                                                           \
    memset(var, 0, sizeof(type))


#endif /* OPEN_RPG_INTERNAL_H */