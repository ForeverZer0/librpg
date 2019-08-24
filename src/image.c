
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
#include "renderable.h"
#include "internal.h"
#include "stb_image.h"
#include "stb_image_write.h"

#define BYTES_PER_PIXEL 4

#define RPG_IMG_BIND_FBO(img)                                                                                                                  \
    if (img->fbo == 0) {                                                                                                                   \
        glGenFramebuffers(1, &img->fbo);                                                                                                   \
        glBindFramebuffer(GL_FRAMEBUFFER, img->fbo);                                                                                       \
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img->texture, 0);                                      \
    } else {                                                                                                                               \
        glBindFramebuffer(GL_FRAMEBUFFER, img->fbo)                                                                                        \
    }

#define RPG_IMG_ORTHO(x,y,w,h) 

RPG_RESULT RPG_Image_Create(RPGint width, RPGint height, const void *pixels, RPG_PIXEL_FORMAT format, RPGimage **image) {
    if (width < 1 || height < 1) {
        return RPG_ERR_OUT_OF_RANGE;
    }
    RPG_ALLOC_ZERO(img, RPGimage);
    img->width  = width;
    img->height = height;

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

RPG_RESULT RPG_Image_CreateEmpty(RPGint width, RPGint height, RPGimage **image) {
    return RPG_Image_Create(width, height, NULL, GL_RGBA, image);
}

RPG_RESULT RPG_Image_CreateFilled(RPGint width, RPGint height, RPGcolor *color, RPGimage **image) {
    if (width < 1 || height < 1) {
        return RPG_ERR_OUT_OF_RANGE;
    }
    RPG_RETURN_IF_NULL(color);

    RPGsize size    = sizeof(RPGuint) * width * height;
    RPGuint *pixels = (RPGuint *)RPG_MALLOC(size);
    RPGubyte r      = (RPGubyte)(color->x * 255);
    RPGubyte g      = (RPGubyte)(color->y * 255);
    RPGubyte b      = (RPGubyte)(color->z * 255);
    RPGubyte a      = (RPGubyte)(color->w * 255);
    GLuint rgba     = (r << 0) | (g << 8) | (b << 16) | (a << 24);
    for (int i = 0; i < size; i++) {
        pixels[i] = rgba;
    }

    RPG_RESULT result = RPG_Image_Create(width, height, pixels, GL_RGBA, image);
    RPG_FREE(pixels);
    return result;
}

RPG_RESULT RPG_Image_CreateFromFile(const char *filename, RPGimage **image) {

    RPG_RETURN_IF_NULL(filename);
    RPG_ENSURE_FILE(filename);

    int width, height;
    void *pixels = stbi_load(filename, &width, &height, NULL, 4);
    if (pixels) {
        RPG_RESULT result = RPG_Image_Create(width, height, pixels, GL_RGBA, image);
        RPG_FREE(pixels);
        return result;
    }
    return RPG_ERR_IMAGE_LOAD;
}

RPG_RESULT RPG_Image_Free(RPGimage *image) {
    RPG_RETURN_IF_NULL(image);
    glDeleteFramebuffers(1, &image->fbo);
    glDeleteTextures(1, &image->texture);
    return RPG_NO_ERROR;
}

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

RPG_RESULT RPG_Image_FillRect(RPGimage *image, RPGcolor *color, RPGint x, RPGint y, RPGint w, RPGint h) {




    // TODO:
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Image_Clear(RPGimage *image) { RPG_RETURN_IF_NULL(image); }