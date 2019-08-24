#include "renderable.h"
#include "glad.h"
#include "internal.h"
#include "rpg.h"

void RPG_Renderable_Init(RPGgame *game, RPGrenderable *renderable, RPGrenderfunc renderfunc, RPGbatch *batch) {
    RPG_ASSERT(game);
    RPG_ASSERT(renderable);
    RPG_ASSERT(renderfunc);
    renderable->scale     = (RPGvec2){1.0f, 1.0f};
    renderable->blend.op  = RPG_BLEND_OP_ADD;
    renderable->blend.src = RPG_BLEND_SRC_ALPHA;
    renderable->blend.dst = RPG_BLEND_ONE_MINUS_SRC_ALPHA;
    renderable->visible   = RPG_TRUE;
    renderable->alpha     = 1.0f;
    renderable->updated   = RPG_TRUE;
    renderable->render    = renderfunc;
    renderable->game      = game;
    renderable->batch     = batch;
    RPG_Batch_Add(batch, renderable);
}

RPG_RESULT RPG_Renderable_Update(RPGrenderable *renderable) {
    RPG_RETURN_IF_NULL(renderable);
    if (renderable->flash.duration > 0) {
        renderable->flash.duration--;
        if (renderable->flash.duration == 0) {
            memset(&renderable->flash.color, 0, sizeof(RPGcolor));
        }
    }
    return RPG_NO_ERROR;
}

DEF_GETTER(Renderable, Z, RPGrenderable, RPGint, z)
DEF_GETTER(Renderable, Alpha, RPGrenderable, RPGfloat, alpha)
DEF_GETTER(Renderable, Hue, RPGrenderable, RPGfloat, hue)
DEF_PARAM(Renderable, Visible, RPGrenderable, RPGbool, visible)

RPG_RESULT RPG_Renderable_GetOrigin(RPGrenderable *renderable, RPGint *x, RPGint *y) {
    RPG_RETURN_IF_NULL(renderable);
    if (x != NULL) {
        *x = renderable->ox;
    }
    if (y != NULL) {
        *y = renderable->oy;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_GetColor(RPGrenderable *renderable, RPGcolor *color) {
    RPG_RETURN_IF_NULL(renderable);
    RPG_RETURN_IF_NULL(color);
    memcpy(color, &renderable->color, sizeof(RPGcolor));
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_GetTone(RPGrenderable *renderable, RPGcolor *tone) {
    RPG_RETURN_IF_NULL(renderable);
    RPG_RETURN_IF_NULL(tone);
    memcpy(tone, &renderable->tone, sizeof(RPGtone));
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_GetScale(RPGrenderable *renderable, RPGfloat *x, RPGfloat *y) {
    RPG_RETURN_IF_NULL(renderable);
    if (x != NULL) {
        *x = renderable->scale.x;
    }
    if (y != NULL) {
        *y = renderable->scale.y;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_GetRotation(RPGrenderable *renderable, RPGfloat *degrees, RPGint *anchorX, RPGint *anchorY) {
    RPG_RETURN_IF_NULL(renderable);
    if (degrees != NULL) {
        const float f = 180.0f / RPG_PI;
        *degrees      = f * renderable->rotation.radians;
    }
    if (anchorX != NULL) {
        *anchorX = renderable->rotation.ox;
    }
    if (anchorY != NULL) {
        *anchorY = renderable->rotation.oy;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_GetBlendMode(RPGrenderable *renderable, RPG_BLEND *src, RPG_BLEND *dst, RPG_BLEND_OP *op) {
    RPG_RETURN_IF_NULL(renderable);
    if (src != NULL) {
        *src = renderable->blend.src;
    }
    if (dst != NULL) {
        *dst = renderable->blend.dst;
    }
    if (op != NULL) {
        *op = renderable->blend.op;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_GetFlash(RPGrenderable *renderable, RPGcolor *color, RPGubyte *duration) {
    RPG_RETURN_IF_NULL(renderable);
    if (color != NULL) {
        memcpy(color, &renderable->flash.color, sizeof(RPGcolor));
    }
    if (duration != NULL) {
        *duration = renderable->flash.duration;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_GetUserPointer(RPGrenderable *renderable, void **user) {
    RPG_RETURN_IF_NULL(renderable);
    *user = renderable->user;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetZ(RPGrenderable *renderable, RPGint z) {
    RPG_RETURN_IF_NULL(renderable);
    if (z != renderable->z) {
        renderable->z              = z;
        renderable->batch->updated = RPG_TRUE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetOrigin(RPGrenderable *renderable, RPGint x, RPGint y) {
    RPG_RETURN_IF_NULL(renderable);
    if (renderable->ox != x || renderable->oy != y) {
        renderable->ox      = x;
        renderable->oy      = y;
        renderable->updated = RPG_TRUE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetAlpha(RPGrenderable *renderable, RPGfloat alpha) {
    RPG_RETURN_IF_NULL(renderable);
    renderable->alpha = RPG_CLAMPF(alpha, 0.0f, 1.0f);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetHue(RPGrenderable *renderable, RPGfloat hue) {
    RPG_RETURN_IF_NULL(renderable);
    renderable->hue = RPG_CLAMPF(hue, 0.0f, 360.0f);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetColor(RPGrenderable *renderable, RPGcolor *color) {
    RPG_RETURN_IF_NULL(renderable);
    if (color) {
        memcpy(&renderable->color, color, sizeof(RPGcolor));
    } else {
        memset(&renderable->color, 0, sizeof(RPGcolor));
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetTone(RPGrenderable *renderable, RPGcolor *tone) {
    RPG_RETURN_IF_NULL(renderable);
    if (tone) {
        memcpy(&renderable->tone, tone, sizeof(RPGtone));
    } else {
        memset(&renderable->tone, 0, sizeof(RPGtone));
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetScale(RPGrenderable *renderable, RPGfloat x, RPGfloat y) {
    RPG_RETURN_IF_NULL(renderable);
    renderable->scale.x = x;
    renderable->scale.y = y;
    renderable->updated = RPG_TRUE;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetRotation(RPGrenderable *renderable, RPGfloat degrees, RPGint anchorX, RPGint anchorY) {
    RPG_RETURN_IF_NULL(renderable);
    const float f                = RPG_PI / 180.0f;
    renderable->rotation.radians = f * RPG_CLAMPF(degrees, 0.0f, 360.0f);
    renderable->rotation.ox      = anchorX;
    renderable->rotation.oy      = anchorY;
    renderable->updated          = RPG_TRUE;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetUserPointer(RPGrenderable *renderable, void *user) {
    RPG_RETURN_IF_NULL(renderable);
    renderable->user = user;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetBlendMode(RPGrenderable *renderable, RPG_BLEND src, RPG_BLEND dst, RPG_BLEND_OP op) {
    RPG_RETURN_IF_NULL(renderable);
    renderable->blend.src = src;
    renderable->blend.dst = dst;
    renderable->blend.op  = op;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetFlash(RPGrenderable *renderable, RPGcolor *color, RPGubyte duration) {
    RPG_RETURN_IF_NULL(renderable);
    if (color == NULL) {
        memset(&renderable->flash.color, 0, sizeof(RPGcolor));
        renderable->flash.duration = 0;
    } else {
        memcpy(&renderable->flash.color, color, sizeof(RPGcolor));
        renderable->flash.duration = duration;
    }
    return RPG_NO_ERROR;
}