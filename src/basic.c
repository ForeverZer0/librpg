#include "internal.h"

void RPG_BasicSprite_Init(RPGbasic *basic, RPGrenderfunc renderfunc, RPGbatch *batch)
{
    RPG_ASSERT(basic);
    basic->scale     = (RPGvec2){1.0f, 1.0f};
    basic->blend.op  = RPG_BLEND_OP_ADD;
    basic->blend.src = RPG_BLEND_SRC_ALPHA;
    basic->blend.dst = RPG_BLEND_ONE_MINUS_SRC_ALPHA;
    basic->alpha     = 1.0f;
    basic->updated   = RPG_TRUE;
    RPG_Renderable_Init(&basic->renderable, renderfunc, batch);
}

RPG_RESULT RPG_BasicSprite_Update(RPGbasic *basic)
{
    RPG_RETURN_IF_NULL(basic);
    if (basic->flash.duration > 0)
    {
        basic->flash.duration--;
        if (basic->flash.duration == 0)
        {
            memset(&basic->flash.color, 0, sizeof(RPGcolor));
        }
    }
    return RPG_NO_ERROR;
}

DEF_GETTER(BasicSprite, Alpha, RPGbasic, RPGfloat, alpha)
DEF_GETTER(BasicSprite, Hue, RPGbasic, RPGfloat, hue)

RPG_RESULT RPG_BasicSprite_GetColor(RPGbasic *basic, RPGcolor *color)
{
    RPG_RETURN_IF_NULL(basic);
    RPG_RETURN_IF_NULL(color);
    memcpy(color, &basic->color, sizeof(RPGcolor));
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_GetTone(RPGbasic *basic, RPGcolor *tone)
{
    RPG_RETURN_IF_NULL(basic);
    RPG_RETURN_IF_NULL(tone);
    memcpy(tone, &basic->tone, sizeof(RPGtone));
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_GetScale(RPGbasic *basic, RPGfloat *x, RPGfloat *y)
{
    RPG_RETURN_IF_NULL(basic);
    if (x != NULL)
    {
        *x = basic->scale.x;
    }
    if (y != NULL)
    {
        *y = basic->scale.y;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_GetRotation(RPGbasic *basic, RPGfloat *degrees, RPGint *anchorX, RPGint *anchorY)
{
    RPG_RETURN_IF_NULL(basic);
    if (degrees != NULL)
    {
        const float f = 180.0f / RPG_PI;
        *degrees      = f * basic->rotation.radians;
    }
    if (anchorX != NULL)
    {
        *anchorX = basic->rotation.ox;
    }
    if (anchorY != NULL)
    {
        *anchorY = basic->rotation.oy;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_GetBlendMode(RPGbasic *basic, RPG_BLEND *src, RPG_BLEND *dst, RPG_BLEND_OP *op)
{
    RPG_RETURN_IF_NULL(basic);
    if (src != NULL)
    {
        *src = basic->blend.src;
    }
    if (dst != NULL)
    {
        *dst = basic->blend.dst;
    }
    if (op != NULL)
    {
        *op = basic->blend.op;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_GetFlash(RPGbasic *basic, RPGcolor *color, RPGubyte *duration)
{
    RPG_RETURN_IF_NULL(basic);
    if (color != NULL)
    {
        memcpy(color, &basic->flash.color, sizeof(RPGcolor));
    }
    if (duration != NULL)
    {
        *duration = basic->flash.duration;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_GetUserPointer(RPGbasic *basic, void **user)
{
    RPG_RETURN_IF_NULL(basic);
    *user = basic->user;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_SetAlpha(RPGbasic *basic, RPGfloat alpha)
{
    RPG_RETURN_IF_NULL(basic);
    basic->alpha = RPG_CLAMPF(alpha, 0.0f, 1.0f);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_SetHue(RPGbasic *basic, RPGfloat hue)
{
    RPG_RETURN_IF_NULL(basic);
    basic->hue = RPG_CLAMPF(hue, 0.0f, 360.0f);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_SetColor(RPGbasic *basic, RPGcolor *color)
{
    RPG_RETURN_IF_NULL(basic);
    if (color)
    {
        memcpy(&basic->color, color, sizeof(RPGcolor));
    }
    else
    {
        memset(&basic->color, 0, sizeof(RPGcolor));
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_SetTone(RPGbasic *basic, RPGcolor *tone)
{
    RPG_RETURN_IF_NULL(basic);
    if (tone)
    {
        memcpy(&basic->tone, tone, sizeof(RPGtone));
    }
    else
    {
        memset(&basic->tone, 0, sizeof(RPGtone));
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_SetScale(RPGbasic *basic, RPGfloat x, RPGfloat y)
{
    RPG_RETURN_IF_NULL(basic);
    basic->scale.x = x;
    basic->scale.y = y;
    basic->updated = RPG_TRUE;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_SetRotation(RPGbasic *basic, RPGfloat degrees, RPGint anchorX, RPGint anchorY)
{
    RPG_RETURN_IF_NULL(basic);
    const float f                = RPG_PI / 180.0f;
    basic->rotation.radians = f * RPG_CLAMPF(degrees, 0.0f, 360.0f);
    basic->rotation.ox      = anchorX;
    basic->rotation.oy      = anchorY;
    basic->updated          = RPG_TRUE;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_SetUserPointer(RPGbasic *basic, void *user)
{
    RPG_RETURN_IF_NULL(basic);
    basic->user = user;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_SetBlendMode(RPGbasic *basic, RPG_BLEND src, RPG_BLEND dst, RPG_BLEND_OP op)
{
    RPG_RETURN_IF_NULL(basic);
    basic->blend.src = src;
    basic->blend.dst = dst;
    basic->blend.op  = op;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_SetFlash(RPGbasic *basic, RPGcolor *color, RPGubyte duration)
{
    RPG_RETURN_IF_NULL(basic);
    if (color == NULL)
    {
        memset(&basic->flash.color, 0, sizeof(RPGcolor));
        basic->flash.duration = 0;
    }
    else
    {
        memcpy(&basic->flash.color, color, sizeof(RPGcolor));
        basic->flash.duration = duration;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_GetLocation(RPGbasic *basic, RPGint *x, RPGint *y)
{
    RPG_RETURN_IF_NULL(basic);
    if (x != NULL)
    {
        *x = basic->x;
    }
    if (y != NULL)
    {
        *y = basic->y;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_BasicSprite_SetLocation(RPGbasic *basic, RPGint x, RPGint y)
{
    RPG_RETURN_IF_NULL(basic);
    if (x != basic->x || y != basic->y)
    {
        basic->x       = x;
        basic->y       = y;
        basic->updated = RPG_TRUE;
    }
    return RPG_NO_ERROR;
}
