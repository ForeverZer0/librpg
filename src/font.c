#include "glad.h"
#include "internal.h"
#include "renderable.h"
#include "rpg.h"
#include "utf8.h"
#include "uthash.h"
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

GLuint font_program;
GLint font_projection;
GLint font_color;
GLuint font_vao;
GLuint font_vbo;

typedef struct RPGglyph {
    UT_hash_handle hh;
    RPGint codepoint;
    GLuint tex;
    // GLuint vao;
    // GLuint vbo;
    RPGint w, h, ox, oy;
} RPGglyph;

typedef struct RPGfontsize {
    UT_hash_handle hh;
    RPGint size;
    RPGint ascent;
    RPGint baseline;
    RPGfloat scale;
    RPGglyph *glyphs;
} RPGfontsize;

typedef struct RPGfont {
    UT_hash_handle hh;
    // TODO: Key
    RPGcolor color;
    RPGint size;
    stbtt_fontinfo font;
    RPGfontsize *sizes;
} RPGfont;

static RPG_RESULT RPG_ReadFile(const char *filename, void **buffer, size_t *size) {
    RPG_ENSURE_FILE(filename);
    void *source = NULL;
    FILE *fp     = fopen(filename, "rb");
    if (fp != NULL) {
        if (fseek(fp, 0L, SEEK_END) == 0) {
            long len = ftell(fp);
            if (len == -1) {
                fclose(fp);
                return RPG_ERR_FILE_READ_ERROR;
            }
            if (fseek(fp, 0L, SEEK_SET) != 0) {
                return RPG_ERR_FILE_READ_ERROR;
            }
            source = RPG_MALLOC(len);
            *size  = fread(source, sizeof(char), (size_t) len, fp);
            if (ferror(fp) != 0) {
                RPG_FREE(source);
                fclose(fp);
                return RPG_ERR_FILE_READ_ERROR;
            }
        }
        fclose(fp);
    }
    *buffer = source;
    return RPG_NO_ERROR;
}

static void RPG_Font_GetGlyph(RPGfont *font, int codepoint, RPGglyph **glyph) {
    RPGfontsize *fs;
    HASH_FIND(hh, font->sizes, &font->size, sizeof(RPGint), fs);
    RPGglyph *g = NULL;
    HASH_FIND(hh, fs->glyphs, &codepoint, sizeof(RPGint), g);
    if (g == NULL) {
        g = RPG_ALLOC(RPGglyph);
        g->codepoint = codepoint;
        void* bmp = stbtt_GetCodepointBitmap(&font->font, fs->scale, fs->scale, codepoint, &g->w, &g->h, &g->ox, &g->oy);
        glGenTextures(1, &g->tex);
        glBindTexture(GL_TEXTURE_2D, g->tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, g->w, g->h, 0, GL_RED, GL_UNSIGNED_BYTE, bmp);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        stbtt_FreeBitmap(bmp, NULL);
        HASH_ADD(hh, fs->glyphs, codepoint, sizeof(RPGint), g);
    }
    *glyph = g;
}

static void RPG_Font_Initialize() {
    // TODO:
}

RPG_RESULT RPG_Font_Create(void *buffer, RPGsize sizeBuffer, RPGfont **font) {
    RPG_RETURN_IF_NULL(buffer);
    if (font_program == 0) {
        RPG_Font_Initialize();
    }
    if (sizeBuffer == 0) {
        return RPG_ERR_INVALID_VALUE;
    }
    RPG_ALLOC_ZERO(f, RPGfont);
    if (stbtt_InitFont(&f->font, buffer, 0)) {
        // FIXME: Set default size
        RPG_Font_SetSize(f, 24);
        *font = f;
        return RPG_NO_ERROR;
    }
    RPG_FREE(f);
    return RPG_ERR_FORMAT;
}

RPG_RESULT RPG_Font_CreateFromFile(const char *filename, RPGfont **font) {
    size_t size;
    void *buffer;
    RPG_RESULT result = RPG_ReadFile(filename, &buffer, &size);
    if (result == RPG_NO_ERROR) {
        result = RPG_Font_Create(buffer, size, font);
        RPG_FREE(buffer);
    }
    return result;
}

RPG_RESULT RPG_Font_Free(RPGfont *font) {
    // FIXME: Do all the crazy deleting
    RPG_FREE(font);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_GetName(RPGfont *font, void *buffer, RPGsize sizeBuffer, RPGsize *written) {
    RPG_RETURN_IF_NULL(font);
    // TODO: ?
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_GetSize(RPGfont *font, RPGint *size) {
    RPG_RETURN_IF_NULL(font);
    if (size != NULL) {
        *size = font->size;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_SetSize(RPGfont *font, RPGint size) {
    RPG_RETURN_IF_NULL(font);
    if (size < 1) {
        return RPG_ERR_OUT_OF_RANGE;
    }
    font->size  = size;
    RPGfontsize *fs = NULL;
    HASH_FIND(hh, font->sizes, &size, sizeof(RPGint), fs);
    if (fs == NULL) {
        fs = RPG_ALLOC(RPGfontsize);
        fs->size = size;
        fs->scale = stbtt_ScaleForPixelHeight(&font->font, size);
        stbtt_GetFontVMetrics(&font->font, &fs->ascent, NULL, NULL);
        fs->baseline = (int) (fs->ascent * fs->scale);
        fs->glyphs = NULL;
        HASH_ADD(hh, font->sizes, size, sizeof(RPGint), fs);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_FontGetColor(RPGfont *font, RPGcolor *color) {
    RPG_RETURN_IF_NULL(font);
    if (color != NULL) {
        *color = font->color;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_FontSetColor(RPGfont *font, RPGcolor *color) {
    RPG_RETURN_IF_NULL(font);
    if (color != NULL) {
        font->color = *color;
    } else {
        // TODO: Set to default
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_DrawText(RPGfont *font, RPGimage *image, const char *text, RPGrect *dstRect, RPG_ALIGN align, RPGfloat alpha) {
    RPG_RETURN_IF_NULL(font);
    RPG_RETURN_IF_NULL(image);
    RPG_RETURN_IF_NULL(text);
    RPGrect d;
    if (dstRect == NULL) {
        d.w = image->width;
        d.h = image->height;
    } else {
        d = *dstRect;
    }
    size_t len = utf8len(text);
    utf8_int32_t cp;
    RPGglyph *glyph;
    void *str = (void*) text;

    // TODO: Change shaders
    for (size_t i = 0; i < len; i++) {
        str = utf8codepoint(str, &cp);
        RPGglyph *glyph;
        RPG_Font_GetGlyph(font, cp, &glyph);

        // TODO: Render glyph, kerning
    }

    // TODO: Revert shader
}
    

