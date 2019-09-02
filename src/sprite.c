#include "internal.h"

static void RPG_Sprite_Render(void *sprite)
{
    RPGsprite *s = sprite;
    if (!s->base.visible || s->base.alpha < __FLT_EPSILON__ || s->image == NULL)
    {
        // No-op if sprite won't be visible
        return;
    }
    if (s->base.updated)
    {
        GLint x = s->base.x + s->base.ox;
        GLint y = s->base.y + s->base.oy;
        if (s->viewport != NULL)
        {
            x += s->viewport->base.ox;
            y += s->viewport->base.oy;
        }
        GLfloat sx  = s->base.scale.x * s->rect.w;
        GLfloat sy  = s->base.scale.y * s->rect.h;
        GLfloat cos = cosf(s->base.rotation.radians);
        GLfloat sin = sinf(s->base.rotation.radians);
        RPG_MAT4_SET(s->base.model, sx * cos, sx * sin, 0.0f, 0.0f, sy * -sin, sy * cos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                     (s->base.rotation.ox * (1.0f - cos) + s->base.rotation.oy * sin) + x,
                     (s->base.rotation.oy * (1.0f - cos) - s->base.rotation.ox * sin) + y, 0.0f, 1.0f);
        s->base.updated = RPG_FALSE;
    }
    RPG_BASE_UNIFORMS(s->base);
    RPG_RENDER_TEXTURE(s->image->texture, s->vao);
}

RPG_RESULT RPG_Sprite_Free(RPGsprite *sprite)
{
    RPG_RETURN_IF_NULL(sprite);
    if (sprite->base.parent)
    {
        RPG_Batch_DeleteItem(sprite->base.parent, &sprite->base);
    }
    glDeleteVertexArrays(1, &sprite->vao);
    glDeleteBuffers(1, &sprite->vbo);
    RPG_FREE(sprite);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sprite_Create(RPGviewport *viewport, RPGsprite **sprite)
{
    RPG_ALLOC_ZERO(s, RPGsprite);
    RPGbatch *batch = viewport ? &viewport->batch : &RPG_GAME->batch;
    RPG_Renderable_Init(&s->base, RPG_Sprite_Render, batch);

    // Generate VAO/VBO
    glGenVertexArrays(1, &s->vao);
    glBindVertexArray(s->vao);
    glGenBuffers(1, &s->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s->vbo);
    glBufferData(GL_ARRAY_BUFFER, VERTICES_SIZE, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VERTICES_STRIDE, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    *sprite = s;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sprite_GetViewport(RPGsprite *sprite, RPGviewport **viewport)
{
    RPG_RETURN_IF_NULL(sprite);
    if (viewport != NULL)
    {
        *viewport = sprite->viewport;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sprite_GetImage(RPGsprite *sprite, RPGimage **image)
{
    RPG_RETURN_IF_NULL(sprite);
    if (image != NULL)
    {
        *image = sprite->image;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sprite_SetImage(RPGsprite *sprite, RPGimage *image)
{
    RPG_RETURN_IF_NULL(sprite);
    sprite->image = image;
    if (image)
    {
        RPG_Sprite_SetSourceBounds(sprite, 0, 0, image->width, image->height);
    }
    else
    {
        sprite->rect = (RPGrect){0, 0, 0, 0};
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sprite_GetSourceRect(RPGsprite *sprite, RPGrect *rect)
{
    RPG_RETURN_IF_NULL(sprite);
    if (rect != NULL)
    {
        *rect = sprite->rect;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sprite_GetSourceBounds(RPGsprite *sprite, RPGint *x, RPGint *y, RPGint *width, RPGint *height)
{
    RPG_RETURN_IF_NULL(sprite);
    if (x != NULL)
    {
        *x = sprite->rect.x;
    }
    if (y != NULL)
    {
        *y = sprite->rect.y;
    }
    if (width != NULL)
    {
        *width = sprite->rect.w;
    }
    if (height != NULL)
    {
        *height = sprite->rect.h;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sprite_SetSourceBounds(RPGsprite *sprite, RPGint x, RPGint y, RPGint w, RPGint h)
{
    RPG_RETURN_IF_NULL(sprite);
    if (sprite->image == NULL)
    {
        return RPG_NO_ERROR;
    }

    sprite->rect.x = x;
    sprite->rect.y = y;
    sprite->rect.w = w;
    sprite->rect.h = h;

    GLfloat l = (GLfloat) x / sprite->image->width;
    GLfloat t = (GLfloat) y / sprite->image->height;
    GLfloat r = l + ((GLfloat) w / sprite->image->width);
    GLfloat b = t + ((GLfloat) h / sprite->image->height);

    glBindBuffer(GL_ARRAY_BUFFER, sprite->vbo);
    GLfloat vertices[VERTICES_COUNT] = {0.0f, 1.0f, l, b, 1.0f, 0.0f, r, t, 0.0f, 0.0f, l, t,
                                        0.0f, 1.0f, l, b, 1.0f, 1.0f, r, b, 1.0f, 0.0f, r, t};
    glBufferSubData(GL_ARRAY_BUFFER, 0, VERTICES_SIZE, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    sprite->base.updated = RPG_TRUE;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sprite_SetSourceRect(RPGsprite *sprite, RPGrect *rect)
{
    RPG_RETURN_IF_NULL(rect);
    return RPG_Sprite_SetSourceBounds(sprite, rect->x, rect->y, rect->w, rect->h);
}

RPG_RESULT RPG_Sprite_GetOrigin(RPGsprite *sprite, RPGint *x, RPGint *y)
{
    RPG_RETURN_IF_NULL(sprite);
    if (x != NULL)
    {
        *x = sprite->base.ox;
    }
    if (y != NULL)
    {
        *y = sprite->base.oy;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sprite_SetOrigin(RPGsprite *sprite, RPGint x, RPGint y)
{
    RPG_RETURN_IF_NULL(sprite);
    if (sprite->base.ox != x || sprite->base.oy != y)
    {
        sprite->base.ox      = x;
        sprite->base.oy      = y;
        sprite->base.updated = RPG_TRUE;
    }
    return RPG_NO_ERROR;
}