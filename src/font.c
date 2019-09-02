
#include "internal.h"
#include "utf8.h"
#include "uthash.h"
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x, u) RPG_MALLOC(x)
#define STBTT_free(x, u) RPG_FREE(x)
#define STBTT_assert(x) RPG_ASSERT(x)
#include "stb_truetype.h"

typedef struct RPGglyph
{
    UT_hash_handle hh;
    RPGint codepoint;
    GLuint tex;
    RPGint w, h, ox, oy;
    RPGfloat advance;
} RPGglyph;

typedef struct RPGfontsize
{
    UT_hash_handle hh;
    RPGint size;
    RPGint ascent;
    RPGint descent;
    RPGint baseline;
    RPGfloat scale;
    RPGglyph *glyphs;
} RPGfontsize;

typedef struct RPGfont
{
    RPGcolor color;
    RPGint size;
    stbtt_fontinfo font;
    RPGfontsize *sizes;
    void *user;
} RPGfont;

RPG_RESULT RPG_ReadFile(const char *filename, char **buffer, size_t *size)
{
    RPG_ENSURE_FILE(filename);
    char *source = NULL;
    FILE *fp     = fopen(filename, "rb");
    if (fp != NULL)
    {
        if (fseek(fp, 0L, SEEK_END) == 0)
        {
            long len = ftell(fp);
            if (len == -1)
            {
                fclose(fp);
                return RPG_ERR_FILE_READ_ERROR;
            }
            if (fseek(fp, 0L, SEEK_SET) != 0)
            {
                return RPG_ERR_FILE_READ_ERROR;
            }
            source    = RPG_MALLOC(len + 1);
            size_t sz = fread(source, sizeof(char), (size_t) len, fp);
            if (size != NULL)
            {
                *size = sz;
            }
            source[len] = '\0';
            if (ferror(fp) != 0)
            {
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

static void RPG_Font_GetGlyph(RPGfont *font, int codepoint, RPGglyph **glyph)
{
    RPGfontsize *fs;
    HASH_FIND(hh, font->sizes, &font->size, sizeof(RPGint), fs);
    RPGglyph *g = NULL;
    HASH_FIND(hh, fs->glyphs, &codepoint, sizeof(RPGint), g);
    if (g == NULL)
    {
        g            = RPG_ALLOC(RPGglyph);
        g->codepoint = codepoint;

        int advance;
        void *bmp = stbtt_GetCodepointBitmap(&font->font, fs->scale, fs->scale, codepoint, &g->w, &g->h, &g->ox, &g->oy);
        stbtt_GetCodepointHMetrics(&font->font, codepoint, &advance, NULL);
        g->advance = fs->scale * advance;

        glGenTextures(1, &g->tex);
        RPG_Drawing_BindTexture(g->tex, GL_TEXTURE0);
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

static void RPG_Font_Initialize(RPGgame *game)
{
    RPGshader *shader;
    RPG_RESULT s = RPG_Shader_Create(RPG_FONT_VERTEX, RPG_FONT_FRAGMENT, NULL, &shader);
    RPG_ASSERT(s == RPG_NO_ERROR);

    game->font.program    = *((GLuint *) shader);
    game->font.projection = glGetUniformLocation(game->font.program, "projection");
    game->font.color      = glGetUniformLocation(game->font.program, "color");

    glGenVertexArrays(1, &game->font.vao);
    glGenBuffers(1, &game->font.vbo);
    glBindVertexArray(game->font.vao);
    glBindBuffer(GL_ARRAY_BUFFER, game->font.vbo);
    glBufferData(GL_ARRAY_BUFFER, VERTICES_SIZE, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    RPG_FREE(shader);
}

RPG_RESULT RPG_Font_Create(void *buffer, RPGsize sizeBuffer, RPGfont **font)
{
    RPG_RETURN_IF_NULL(buffer);
    if (RPG_GAME->font.program == 0)
    {
        RPG_Font_Initialize(RPG_GAME);
    }
    if (sizeBuffer == 0)
    {
        return RPG_ERR_INVALID_VALUE;
    }
    RPG_ALLOC_ZERO(f, RPGfont);
    if (stbtt_InitFont(&f->font, buffer, 0))
    {
        RPG_Font_SetSize(f, RPG_GAME->font.defaultSize);
        RPG_Font_SetColor(f, &RPG_GAME->font.defaultColor);
        *font = f;
        return RPG_NO_ERROR;
    }
    RPG_FREE(f);
    return RPG_ERR_FORMAT;
}

RPG_RESULT RPG_Font_CreateFromFile(const char *filename, RPGfont **font)
{
    size_t size;
    char *buffer;
    RPG_RESULT result = RPG_ReadFile(filename, &buffer, &size);
    if (result == RPG_NO_ERROR)
    {
        result = RPG_Font_Create(buffer, size, font);
        RPG_FREE(buffer);
    }
    return result;
}

RPG_RESULT RPG_Font_Free(RPGfont *font)
{
    RPGfontsize *size, *tmpSize;
    HASH_ITER(hh, font->sizes, size, tmpSize)
    {
        // Enumerate through each glyph
        RPGglyph *glyph, *tmpGlyph;
        HASH_ITER(hh, size->glyphs, glyph, tmpGlyph)
        {
            glDeleteTextures(1, &glyph->tex);
            RPG_FREE(glyph);
        }
        if (size->glyphs)
            RPG_FREE(size->glyphs);
        RPG_FREE(size);
    }
    if (font->sizes)
    {
        RPG_FREE(font->sizes);
    }
    RPG_FREE(font);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_GetSize(RPGfont *font, RPGint *size)
{
    RPG_RETURN_IF_NULL(font);
    if (size != NULL)
    {
        *size = font->size;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_SetSize(RPGfont *font, RPGint size)
{
    RPG_RETURN_IF_NULL(font);
    if (size < 1)
    {
        return RPG_ERR_OUT_OF_RANGE;
    }
    font->size      = size;
    RPGfontsize *fs = NULL;
    HASH_FIND(hh, font->sizes, &size, sizeof(RPGint), fs);
    if (fs == NULL)
    {
        fs        = RPG_ALLOC(RPGfontsize);
        fs->size  = size;
        fs->scale = stbtt_ScaleForPixelHeight(&font->font, size);
        stbtt_GetFontVMetrics(&font->font, &fs->ascent, &fs->descent, NULL);
        fs->baseline = (int) (fs->ascent * fs->scale);
        fs->glyphs   = NULL;
        HASH_ADD(hh, font->sizes, size, sizeof(RPGint), fs);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_GetColor(RPGfont *font, RPGcolor *color)
{
    RPG_RETURN_IF_NULL(font);
    if (color != NULL)
    {
        *color = font->color;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_SetColor(RPGfont *font, RPGcolor *color)
{
    RPG_RETURN_IF_NULL(font);
    if (color != NULL)
    {
        font->color = *color;
    }
    else
    {
        font->color = RPG_GAME->font.defaultColor;
    }
    return RPG_NO_ERROR;
}

static void RPG_Font_AlignAdjust(RPGrect *rect, RPG_ALIGN align, RPGint dimX, RPGint dimY, RPGint descent)
{

    switch (align)
    {
        case RPG_ALIGN_TOP_LEFT:
        {
            break;
        }
        case RPG_ALIGN_TOP_RIGHT:
        {
            rect->x = rect->w - rect->x - dimX;
            break;
        }
        case RPG_ALIGN_CENTER_H:
        case RPG_ALIGN_TOP:
        case RPG_ALIGN_TOP_CENTER:
        {
            rect->x += (rect->w - dimX) / 2;
            break;
        }
        case RPG_ALIGN_BOTTOM_LEFT:
        {
            rect->y = rect->h - rect->y - dimY + descent;
            break;
        }
        case RPG_ALIGN_BOTTOM_RIGHT:
        {
            rect->x = rect->w - rect->x - dimX;
            rect->y = rect->h - rect->y - dimY + descent;
            break;
        }
        case RPG_ALIGN_BOTTOM:
        case RPG_ALIGN_BOTTOM_CENTER:
        {
            rect->x += (rect->w - dimX) / 2;
            rect->y = rect->h - rect->y - dimY + descent;
            break;
        }
        case RPG_ALIGN_CENTER_V:
        case RPG_ALIGN_LEFT:
        case RPG_ALIGN_CENTER_LEFT:
        {
            rect->y += (rect->h - dimY) / 2;
            break;
        }
        case RPG_ALIGN_RIGHT:
        case RPG_ALIGN_CENTER_RIGHT:
        {
            rect->x = rect->w - rect->x - dimX;
            rect->y += (rect->h - dimY) / 2;
            break;
        }
        case RPG_ALIGN_NONE:
        case RPG_ALIGN_CENTER:
        {
            rect->x += (rect->w - dimX) / 2;
            rect->y += (rect->h - dimY) / 2;
            break;
        }
    }
}

RPG_RESULT RPG_Font_DrawText(RPGfont *font, RPGimage *image, const char *text, RPGrect *dstRect, RPG_ALIGN align)
{
    RPG_RETURN_IF_NULL(font);
    RPG_RETURN_IF_NULL(image);
    RPG_RETURN_IF_NULL(text);

    // Use full image bounds to draw if no destination rectangle is defined
    RPGrect d;
    if (dstRect == NULL)
    {
        d.x = 0;
        d.y = 0;
        d.w = image->width;
        d.h = image->height;
    }
    else
    {
        d = *dstRect;
    }

    // Find the current size set for the font
    RPGfontsize *fs = NULL;
    HASH_FIND(hh, font->sizes, &font->size, sizeof(RPGint), fs);

    int dimX, dimY;
    RPG_Font_MeasureText(font, text, &dimX, &dimY);
    RPG_Font_AlignAdjust(&d, align, dimX, dimY, (RPGint)(fs->scale * fs->descent));

    // Enable the font shader, bind the FBO, and set the projection matrix
    glUseProgram(RPG_GAME->font.program);
    RPG_ENSURE_FBO(image);
    RPGmat4 ortho;
    RPG_MAT4_ORTHO(ortho, 0.0f, image->width, image->height, 0.0f, -1.0f, 1.0f);
    glViewport(0, 0, image->width, image->height);
    glScissor(d.x, d.y, d.w, d.h);

    glUniformMatrix4fv(RPG_GAME->font.projection, 1, GL_FALSE, (RPGfloat *) &ortho);
    glUniform4f(RPG_GAME->font.color, font->color.x, font->color.y, font->color.z, font->color.w);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(RPG_GAME->font.vao);
    glBindBuffer(GL_ARRAY_BUFFER, RPG_GAME->font.vbo);
    RPG_Drawing_SetBlending(GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // Declare variable storage
    utf8_int32_t cp, next;
    RPGglyph *glyph;
    RPGfloat ox = d.x;
    RPGfloat x, y, w, h;
    void *str = utf8codepoint(text, &cp);

    // Enumerate through each codepoint in the string, calculating metrics and rendering each glyph
    while (cp)
    {
        // Load the glyph for the current codepoint
        RPG_Font_GetGlyph(font, cp, &glyph);
        x = ox + glyph->ox;
        y = d.y + glyph->oy + fs->baseline;
        w = glyph->w;
        h = glyph->h;

        // Buffer glyph vertices
        RPG_Drawing_BindTexture(glyph->tex, GL_TEXTURE0);
        GLfloat vertices[VERTICES_COUNT] = {x, y + h, 0.0f, 1.0f, x + w, y,     1.0f, 0.0f, x,     y, 0.0f, 0.0f,
                                            x, y + h, 0.0f, 1.0f, x + w, y + h, 1.0f, 1.0f, x + w, y, 1.0f, 0.0f};
        glBufferSubData(GL_ARRAY_BUFFER, 0, VERTICES_SIZE, vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Check if there is another codepoint to render after this one, if so, apply kerning
        str = utf8codepoint(str, &next);
        if (next)
        {
            ox += fs->scale * stbtt_GetCodepointKernAdvance(&font->font, cp, next);
        }
        ox += glyph->advance;
        cp = next;
    }

    // // Unbind attachments
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);

    // Restore projection to the primary framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(RPG_GAME->shader.program);
    RPG_RESET_PROJECTION();
    RPG_RESET_VIEWPORT();

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_GetDefaultSize(RPGint *size)
{
    if (size != NULL)
    {
        *size = RPG_GAME->font.defaultSize;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_GetDefaultColor(RPGcolor *color)
{
    if (color != NULL)
    {
        *color = RPG_GAME->font.defaultColor;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_SetDefaultSize(RPGint size)
{
    if (size < 1)
    {
        return RPG_ERR_OUT_OF_RANGE;
    }
    RPG_GAME->font.defaultSize = size;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_SetDefaultColor(RPGcolor *color)
{
    if (color == NULL)
    {
        RPG_GAME->font.defaultColor = RPG_FONT_DEFAULT_COLOR;
    }
    else
    {
        RPG_GAME->font.defaultColor = *color;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_MeasureText(RPGfont *font, const char *text, RPGint *width, RPGint *height)
{
    RPG_RETURN_IF_NULL(font);
    if (text == NULL)
    {
        *width  = 0;
        *height = 0;
        return RPG_NO_ERROR;
    }

    RPGfontsize *fs;
    HASH_FIND(hh, font->sizes, &font->size, sizeof(RPGint), fs);

    RPGfloat w, h = 0.0f;
    RPGglyph *glyph = NULL;
    utf8_int32_t cp, next;
    void *str = utf8codepoint(text, &cp);
    while (cp)
    {
        RPG_Font_GetGlyph(font, cp, &glyph);
        h   = fmaxf(glyph->h + glyph->oy + fs->baseline, h);
        str = utf8codepoint(str, &next);
        if (next)
        {
            w += fs->scale * stbtt_GetCodepointKernAdvance(&font->font, cp, next);
        }
        w += glyph->advance;
        cp = next;
    }
    *width  = (RPGint) roundf(w);
    *height = (RPGint) roundf(h);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_GetUserPointer(RPGfont *font, void **user)
{
    RPG_RETURN_IF_NULL(font);
    if (user != NULL)
    {
        *user = font->user;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Font_SetUserPointer(RPGfont *font, void *user)
{
    RPG_RETURN_IF_NULL(font);
    font->user = user;
    return RPG_NO_ERROR;
}