
#include "renderable.h"

static void RPG_Viewport_Render(void *viewport) {
    RPGviewport *v = viewport;
    // Sort the render batch if any child sprites have been added or moved on the z-axis
    if (v->batch.updated) {
        RPG_Batch_Sort(&v->batch, 0, v->batch.total - 1);
    }

    // Bind the viewport's FBO as the current render target
    glBindFramebuffer(GL_FRAMEBUFFER, v->fbo);
    glUniformMatrix4fv(RPG_GAME->shader.projection, 1, GL_FALSE, (GLfloat *)&v->projection);
    RPG_VIEWPORT(0, 0, v->width, v->height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render each of the viewport's child sprites
    RPGrenderable *r;
    for (int i = 0; i < v->batch.total; i++) {
        r = v->batch.items[i];
        r->render(r);
    }

    // Unbind the viewport's FBO, changing the render target back to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    RPG_RESET_VIEWPORT();
    RPG_RESET_BACK_COLOR();

    // Update Model (if required)
    if (v->base.updated) {
        GLfloat sx = v->base.scale.x * v->width;
        GLfloat sy = v->base.scale.y * v->height;

        GLfloat cos = cosf(v->base.rotation.radians);
        GLfloat sin = sinf(v->base.rotation.radians);
        RPG_MAT4_SET(v->base.model, sx * cos, sx * sin, 0.0f, 0.0f, sy * -sin, sy * cos, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                    (v->base.rotation.ox * (1.0f - cos) + v->base.rotation.oy * sin) + v->base.x,
                    (v->base.rotation.oy * (1.0f - cos) - v->base.rotation.ox * sin) + v->base.y, 0.0f, 1.0f);
        v->base.updated = RPG_FALSE;
    }

    // Set shader uniforms, restore projection, and render the viewport to the screen
    RPG_BASE_UNIFORMS(v->base);
    RPG_RESET_PROJECTION();
    RPG_RENDER_TEXTURE(v->texture, v->vao);
}

RPG_RESULT RPG_Viewport_Create(RPGint x, RPGint y, RPGint width, RPGint height, RPGviewport **viewport) {
    // Obligatory argument checking
    RPG_RETURN_IF_NULL(*viewport);
    RPG_CHECK_DIMENSIONS(width, height);

    // Allocate a new viewport object, and initialize fields
    RPG_ALLOC_ZERO(v, RPGviewport);
    RPG_Renderable_Init(&v->base, RPG_Viewport_Render, &RPG_GAME->batch);
    RPG_Batch_Init(&v->batch);

    // Set dimensions
    v->base.x = x;
    v->base.y = y;
    v->width = width;
    v->height = height;

    // Create a static VBO/VAO
    glGenVertexArrays(1, &v->vao);
    glGenBuffers(1, &v->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, v->vbo);
    RPGfloat vertices[VERTICES_COUNT] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};
    glBufferData(GL_ARRAY_BUFFER, VERTICES_SIZE, vertices, GL_STATIC_DRAW);
    glBindVertexArray(v->vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VERTICES_STRIDE, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); 

    // Framebuffer
    glGenFramebuffers(1, &v->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, v->fbo);

    // Texture
    glGenTextures(1, &v->texture);
    glBindTexture(GL_TEXTURE_2D, v->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Bind texture to FBO and creat a projection matrix for this viewport
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, v->texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    RPG_MAT4_ORTHO(v->projection, 0.0f, width, height, 0.0f, -1.0f, 1.0f);

    *viewport = v;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Viewport_CreateFromRect(RPGrect *rect, RPGviewport **viewport) {
    RPG_RETURN_IF_NULL(rect);
    return RPG_Viewport_Create(rect->x, rect->y, rect->w, rect->h, viewport);
}

RPG_RESULT RPG_Viewport_CreateDefault(RPGviewport **viewport) {
    return RPG_Viewport_Create(0, 0, RPG_GAME->bounds.w, RPG_GAME->bounds.h, viewport);
}

RPG_RESULT RPG_Viewport_GetRect(RPGviewport *viewport, RPGrect *rect) {
    RPG_RETURN_IF_NULL(viewport);
    if (rect != NULL) {
        rect->x = viewport->base.x;
        rect->y = viewport->base.y;
        rect->w = viewport->width;
        rect->h = viewport->height;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Viewport_GetBounds(RPGviewport *viewport, RPGint *x, RPGint *y, RPGint *width, RPGint *height) {
    RPG_RETURN_IF_NULL(viewport);
    if (x != NULL) {
        *x = viewport->base.x;
    }
    if (y != NULL) {
        *y = viewport->base.y;
    }
    if (width != NULL) {
        *width = viewport->width;
    }
    if (height != NULL) {
        *height = viewport->height;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Viewport_GetOrigin(RPGviewport *viewport, RPGint *x, RPGint *y) {
    RPG_RETURN_IF_NULL(viewport);
    if (x != NULL) {
        *x = viewport->base.ox;
    }
    if (y != NULL) {
        *y = viewport->base.oy;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Viewport_SetOrigin(RPGviewport *viewport, RPGint x, RPGint y) {
    RPG_RETURN_IF_NULL(viewport);
    if (viewport->base.ox != x || viewport->base.oy != y) {
        viewport->base.ox = x;
        viewport->base.oy = y;
        RPGrenderable *r;
        for (int i = 0; i < viewport->batch.total; i++) {
            r = viewport->batch.items[0];
            r->updated = RPG_TRUE;
        }
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Viewport_Free(RPGviewport *viewport) {
    RPG_RETURN_IF_NULL(viewport);
    // Remove from parent
    RPG_Batch_DeleteItem(viewport->base.parent, &viewport->base);
    // Free batch
    RPG_Batch_Free(&viewport->batch);
    // Delete VAO/VBO
    glDeleteVertexArrays(1, &viewport->vao);
    glDeleteBuffers(1, &viewport->vbo);
    // Delete texture and FBO
    glDeleteFramebuffers(1, &viewport->fbo);
    glDeleteTextures(1, &viewport->texture);
    // Free pointer
    RPG_FREE(viewport);
    return RPG_NO_ERROR;
}