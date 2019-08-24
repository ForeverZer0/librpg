
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "rpg.h"

#define STBI_MALLOC RPG_MALLOC
#define STBI_REALLOC RPG_REALLOC
#define STBI_FREE RPG_FREE
#define STBIW_MALLOC STBI_MALLOC
#define STBIW_REALLOC STBI_REALLOC
#define STBIW_FREE STBI_FREE

#include "glad.h"
#include "internal.h"
#include "renderable.h"
#include "stb_image.h"
#include "stb_image_write.h"

GLuint blitVBO;
GLuint blitVAO;

#define BYTES_PER_PIXEL 4

#define RPG_ENSURE_FBO(img)                                                                                                                \
    if (img->fbo == 0) {                                                                                                                   \
        glGenFramebuffers(1, &img->fbo);                                                                                                   \
        glBindFramebuffer(GL_FRAMEBUFFER, img->fbo);                                                                                       \
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img->texture, 0);                                      \
    } else {                                                                                                                               \
        glBindFramebuffer(GL_FRAMEBUFFER, img->fbo);                                                                                       \
    }

#define RPG_BIND_FBO(img, x, y, w, h)                                                                                                      \
    RPG_ENSURE_FBO(img);                                                                                                                   \
    RPGmat4 m;                                                                                                                             \
    RPG_MAT4_ORTHO(m, 0.0f, w, 0.0f, h, -1.0f, 1.0f);                                                                                      \
    glUniformMatrix4fv(img->game->shader.projection, 1, GL_FALSE, (RPGfloat *) &m);                                                        \
    RPG_VIEWPORT(x, y, w, h)

#define RPG_UNBIND_FBO(img)                                                                                                                \
    glBindFramebuffer(GL_FRAMEBUFFER, 0);                                                                                                  \
    RPG_RESET_PROJECTION(img->game);                                                                                                       \
    RPG_RESET_VIEWPORT(img->game)

RPG_RESULT RPG_Image_Create(RPGgame *game, RPGint width, RPGint height, const void *pixels, RPG_PIXEL_FORMAT format, RPGimage **image) {
    RPG_RETURN_IF_NULL(game);
    RPG_CHECK_DIMENSIONS(width, height);
    RPG_ALLOC_ZERO(img, RPGimage);
    img->width  = width;
    img->height = height;
    img->game   = game;

    glGenTextures(1, &img->texture);
    glBindTexture(GL_TEXTURE_2D, img->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    *image = img;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Image_CreateEmpty(RPGgame *game, RPGint width, RPGint height, RPGimage **image) {
    return RPG_Image_Create(game, width, height, NULL, GL_RGBA, image);
}

RPG_RESULT RPG_Image_CreateFilled(RPGgame *game, RPGint width, RPGint height, RPGcolor *color, RPGimage **image) {
    RPG_CHECK_DIMENSIONS(width, height);
    RPG_RETURN_IF_NULL(color);

    RPGsize size    = width * height;
    RPGuint *pixels = (RPGuint *) RPG_MALLOC(size * sizeof(RPGuint));
    RPGubyte r      = (RPGubyte)(color->x * 255);
    RPGubyte g      = (RPGubyte)(color->y * 255);
    RPGubyte b      = (RPGubyte)(color->z * 255);
    RPGubyte a      = (RPGubyte)(color->w * 255);
    GLuint rgba     = (r << 0) | (g << 8) | (b << 16) | (a << 24);
    for (int i = 0; i < size; i++) {
        pixels[i] = rgba;
    }

    RPG_RESULT result = RPG_Image_Create(game, width, height, pixels, GL_RGBA, image);
    RPG_FREE(pixels);
    return result;
}

RPG_RESULT RPG_Image_CreateFromFile(RPGgame *game, const char *filename, RPGimage **image) {

    RPG_RETURN_IF_NULL(filename);
    RPG_ENSURE_FILE(filename);

    int width, height;
    void *pixels = stbi_load(filename, &width, &height, NULL, 4);
    if (pixels) {
        RPG_RESULT result = RPG_Image_Create(game, width, height, pixels, GL_RGBA, image);
        RPG_FREE(pixels);
        return result;
    }
    return RPG_ERR_IMAGE_LOAD;
}

RPG_RESULT RPG_Image_Free(RPGimage *image) {
    RPG_RETURN_IF_NULL(image);
    glDeleteFramebuffers(1, &image->fbo);
    glDeleteTextures(1, &image->texture);
    RPG_FREE(image);
    return RPG_NO_ERROR;
}

DEF_GETTER(Image, Texture, RPGimage, RPGuint, texture)
DEF_GETTER(Image, Framebuffer, RPGimage, RPGuint, fbo)

RPG_RESULT RPG_Image_LoadRaw(const char *filename, RPGrawimage **rawImage) {
    RPG_RETURN_IF_NULL(filename);
    RPG_ENSURE_FILE(filename);
    RPGrawimage *img = RPG_ALLOC(RPGrawimage);

    img->pixels = stbi_load(filename, &img->width, &img->height, NULL, 4);
    if (img->pixels == NULL) {
        RPG_FREE(img);
        return RPG_ERR_IMAGE_LOAD;
    }
    *rawImage = img;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Image_GetSize(RPGimage *image, RPGint *width, RPGint *height) {
    RPG_RETURN_IF_NULL(image);
    if (width != NULL) {
        *width = image->width;
    }
    if (height != NULL) {
        *height = image->height;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Image_GetUserPointer(RPGimage *image, void **pointer) {
    RPG_RETURN_IF_NULL(*pointer);
    RPG_RETURN_IF_NULL(image);
    *pointer = image->user;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Image_SetUserPointer(RPGimage *image, void *pointer) {
    RPG_RETURN_IF_NULL(image);
    image->user = pointer;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Image_FillRect(RPGimage *image, RPGcolor *color, RPGrect *rect) {
    RPG_RETURN_IF_NULL(rect);
    return RPG_Image_Fill(image, color, rect->x, rect->y, rect->w, rect->h);
}

RPG_RESULT RPG_Image_Fill(RPGimage *image, RPGcolor *color, RPGint x, RPGint y, RPGint w, RPGint h) {
    RPG_BIND_FBO(image, x, y, w, h);
    if (color == NULL) {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    } else {
        glClearColor(color->x, color->y, color->z, color->w);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    RPG_RESET_BACK_COLOR(image->game);
    RPG_UNBIND_FBO(image);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Image_Clear(RPGimage *image) {
    RPG_RETURN_IF_NULL(image);
    return RPG_Image_Fill(image, NULL, 0, 0, image->width, image->height);
}

RPG_RESULT RPG_Image_SubImage(RPGimage *image, RPGint x, RPGint y, RPGint width, RPGint height, RPGimage **subImage) {
    RPG_RETURN_IF_NULL(image);
    RPG_CHECK_DIMENSIONS(width, height);

    void *pixels = RPG_MALLOC(width * height * BYTES_PER_PIXEL);
    RPG_BIND_FBO(image, 0, 0, image->width, image->height);
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    RPG_UNBIND_FBO(image);

    RPG_RESULT result = RPG_Image_Create(image->game, width, height, pixels, RPG_PIXEL_FORMAT_RGBA, subImage);
    RPG_FREE(pixels);
    return result;
}

RPG_RESULT RPG_Image_Blit(RPGimage *dst, RPGrect *dstRect, RPGimage *src, RPGrect *srcRect, RPGfloat alpha) {
    RPG_RETURN_IF_NULL(dst);
    RPG_RETURN_IF_NULL(src);

    // Get default source/destination rectangles if NULL
    RPGrect d, s;
    if (dstRect != NULL) {
        d = *dstRect;
    } else {
        d.x = 0;
        d.y = 0;
        d.w = dst->width;
        d.h = dst->height;
    }
    if (srcRect != NULL) {
        s = *srcRect;
    } else {
        s.x = 0;
        s.y = 0;
        s.w = src->width;
        s.h = src->height;
    }
    // Generate VBO/VAO if not defined yet
    if (blitVAO == 0) {
        glGenVertexArrays(1, &blitVAO);
        glGenBuffers(1, &blitVBO);
        glBindVertexArray(blitVAO);
        glBindBuffer(GL_ARRAY_BUFFER, blitVBO);
        glBufferData(GL_ARRAY_BUFFER, VERTICES_SIZE, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VERTICES_STRIDE, NULL);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // Calculate model matrix
    GLfloat scale_x = (d.w / (GLfloat)s.w) * s.w;
    GLfloat scale_y = (d.h / (GLfloat)s.h) * s.h;
    RPGmat4 model;
    RPG_MAT4_SET(model, 
    scale_x, 0.0f, 0.0f, 0.0f, 
    0.0f, scale_y, 0.0f, 0.0f, 
    0.0f, 0.0f, 1.0f, 0.0f, 
    d.x, d.y, 0.0f, 1.0f);

    // Set shader uniforms for opacity and ortho, default for all others
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniformMatrix4fv(dst->game->shader.model, 1, GL_FALSE, (GLfloat *)&model);
    glUniform1f(dst->game->shader.alpha, alpha);
    glUniform4f(dst->game->shader.color, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform4f(dst->game->shader.tone, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform4f(dst->game->shader.flash, 0.0f, 0.0f, 0.0f, 0.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(blitVAO);

    // Calculate vertices for source rectangle
    GLfloat l = (GLfloat)s.x / src->width;
    GLfloat t = (GLfloat)s.y / src->height;
    GLfloat r = l + ((GLfloat)s.w / src->width);
    GLfloat b = t + ((GLfloat)s.h / src->height);
    glBindBuffer(GL_ARRAY_BUFFER, blitVBO);
    float vertices[VERTICES_COUNT] = {0.0f, 1.0f, l, b, 1.0f, 0.0f, r, t, 0.0f, 0.0f, l, t,
                                      0.0f, 1.0f, l, b, 1.0f, 1.0f, r, b, 1.0f, 0.0f, r, t};
    glBufferSubData(GL_ARRAY_BUFFER, 0, VERTICES_SIZE, vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create projection matrix and viewport to reflect the destination image
    RPG_ENSURE_FBO(dst);                                                                                                                   
    RPGmat4 m;                                                                                                                            
    RPG_MAT4_ORTHO(m, 0.0f, dst->width, dst->height, 0.0f, -1.0f, 1.0f);                                                                                     
    glUniformMatrix4fv(src->game->shader.projection, 1, GL_FALSE, (RPGfloat *) &m);                                                       
    RPG_VIEWPORT(0, 0, dst->width, dst->height);

    // Render the source image to the destination's framebuffer
    glBindTexture(GL_TEXTURE_2D, src->texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    RPG_UNBIND_FBO(dst);

    return RPG_NO_ERROR;
}

// FIXME: Require user allocated memory LoadRaw
