#include "internal.h"

static void RPG_Plane_Render(void *plane)
{
    RPGplane *p = plane;
    if (!p->base.visible || p->base.alpha < __FLT_EPSILON__ || p->image == NULL)
    {
        return;
    }

    if (p->updateVAO)
    {
        GLfloat l = ((GLfloat) p->base.ox / p->image->width) * p->zoom.x;
        GLfloat t = ((GLfloat) p->base.oy / p->image->height) * p->zoom.y;
        GLfloat r = l + (((GLfloat) p->width / p->image->width) * p->zoom.x);
        GLfloat b = t + (((GLfloat) p->height / p->image->height) * p->zoom.y);
        glBindBuffer(GL_ARRAY_BUFFER, p->vbo);
        float vertices[VERTICES_COUNT] = {0.0f, 1.0f, l, b, 1.0f, 0.0f, r, t, 0.0f, 0.0f, l, t,
                                          0.0f, 1.0f, l, b, 1.0f, 1.0f, r, b, 1.0f, 0.0f, r, t};
        glBufferSubData(GL_ARRAY_BUFFER, 0, VERTICES_SIZE, vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        p->updateVAO = RPG_FALSE;
    }

    if (p->base.updated)
    {
        GLint x = p->base.x;
        GLint y = p->base.y;
        if (p->viewport != NULL)
        {
            x += p->viewport->base.ox;
            y += p->viewport->base.oy;
        }

        GLfloat sx  = p->base.scale.x * p->width;
        GLfloat sy  = p->base.scale.y * p->height;
        GLfloat cos = cosf(p->base.rotation.radians);
        GLfloat sin = sinf(p->base.rotation.radians);
        RPG_MAT4_SET(p->base.model, sx * cos, sx * sin, 0.0f, 0.0f, sy * -sin, sy * cos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                     (p->base.rotation.ox * (1.0f - cos) + p->base.rotation.oy * sin) + x,
                     (p->base.rotation.oy * (1.0f - cos) - p->base.rotation.ox * sin) + y, 0.0f, 1.0f);

        p->base.updated = RPG_FALSE;
    }

    RPG_BASE_UNIFORMS(p->base);
    glBindSampler(0, p->sampler);
    RPG_RENDER_TEXTURE(p->image->texture, p->vao);
    glBindSampler(0, 0);
}

RPG_RESULT RPG_Plane_Create(RPGviewport *viewport, RPGplane **plane)
{
    RPG_ALLOC_ZERO(p, RPGplane);
    RPGbatch *batch = viewport ? &viewport->batch : &RPG_GAME->batch;
    RPG_Renderable_Init(&p->base, RPG_Plane_Render, batch);

    // Set initial values
    if (viewport)
    {
        p->width  = viewport->width;
        p->height = viewport->height;
    }
    else
    {
        p->width  = RPG_GAME->resolution.width;
        p->height = RPG_GAME->resolution.height;
    }
    p->zoom.x    = 1.0f;
    p->zoom.y    = 1.0f;
    p->updateVAO = RPG_TRUE;

    // Generate sampler
    glGenSamplers(1, &p->sampler);
    glSamplerParameteri(p->sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(p->sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(p->sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(p->sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Generate VAO/VBO
    glGenVertexArrays(1, &p->vao);
    glBindVertexArray(p->vao);
    glGenBuffers(1, &p->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, p->vbo);
    glBufferData(GL_ARRAY_BUFFER, VERTICES_SIZE, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    *plane = p;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_Free(RPGplane *plane)
{
    RPG_RETURN_IF_NULL(plane);
    if (plane->base.parent)
    {
        RPG_Batch_DeleteItem(plane->base.parent, &plane->base);
    }
    glDeleteVertexArrays(1, &plane->vao);
    glDeleteBuffers(1, &plane->vbo);
    glDeleteSamplers(1, &plane->sampler);
    RPG_FREE(plane);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_GetViewport(RPGplane *plane, RPGviewport **viewport)
{
    RPG_RETURN_IF_NULL(plane);
    RPG_RETURN_IF_NULL(viewport);
    *viewport = plane->viewport;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_GetRect(RPGplane *plane, RPGrect *rect)
{
    RPG_RETURN_IF_NULL(plane);
    if (rect != NULL)
    {
        rect->x = plane->base.x;
        rect->y = plane->base.y;
        rect->w = plane->width;
        rect->h = plane->height;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_SetRect(RPGplane *plane, RPGrect *rect)
{
    RPG_RETURN_IF_NULL(plane);
    RPG_RETURN_IF_NULL(rect);

    plane->base.x       = rect->x;
    plane->base.y       = rect->y;
    plane->width        = rect->w;
    plane->height       = rect->h;
    plane->base.updated = RPG_TRUE;

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_GetBounds(RPGplane *plane, RPGint *x, RPGint *y, RPGint *width, RPGint *height)
{
    RPG_RETURN_IF_NULL(plane);
    if (x != NULL)
    {
        *x = plane->base.x;
    }
    if (y != NULL)
    {
        *y = plane->base.y;
    }
    if (width != NULL)
    {
        *width = plane->width;
    }
    if (height != NULL)
    {
        *height = plane->height;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_SetBounds(RPGplane *plane, RPGint x, RPGint y, RPGint width, RPGint height)
{
    RPG_RETURN_IF_NULL(plane);
    plane->base.x       = x;
    plane->base.y       = y;
    plane->width        = width;
    plane->height       = height;
    plane->base.updated = RPG_TRUE;

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_GetOrigin(RPGplane *plane, RPGint *x, RPGint *y)
{
    RPG_RETURN_IF_NULL(plane);
    if (x != NULL)
    {
        *x = plane->base.ox;
    }
    if (y != NULL)
    {
        *y = plane->base.oy;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_SetOrigin(RPGplane *plane, RPGint x, RPGint y)
{
    RPG_RETURN_IF_NULL(plane);
    if (x != plane->base.ox || y != plane->base.oy)
    {
        plane->base.ox   = x;
        plane->base.oy   = y;
        plane->updateVAO = RPG_TRUE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_GetZoom(RPGplane *plane, RPGfloat *x, RPGfloat *y)
{
    RPG_RETURN_IF_NULL(plane);
    if (x != NULL)
    {
        *x = plane->zoom.x;
    }
    if (y != NULL)
    {
        *y = plane->zoom.y;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_SetZoom(RPGplane *plane, RPGfloat x, RPGfloat y)
{
    RPG_RETURN_IF_NULL(plane);
    plane->zoom.x    = x;
    plane->zoom.y    = y;
    plane->updateVAO = RPG_TRUE;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_GetImage(RPGplane *plane, RPGimage **image)
{
    RPG_RETURN_IF_NULL(plane);
    if (image != NULL)
    {
        *image = plane->image;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Plane_SetImage(RPGplane *plane, RPGimage *image)
{
    RPG_RETURN_IF_NULL(plane);
    plane->image        = image;
    plane->base.updated = RPG_TRUE;
    plane->updateVAO    = RPG_TRUE;
    return RPG_NO_ERROR;
}