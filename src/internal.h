#ifndef OPEN_RPG_INTERNAL_H
#define OPEN_RPG_INTERNAL_H 1

#include <math.h>
#include <string.h>

#ifdef __linux__
#define RPG_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define RPG_APPLE
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define RPG_WINDOWS
#endif

#ifdef RPG_WINDOWS

#include <io.h>
#include <windows.h>
#define strcasecmp _stricmp
#define RPG_FILE_EXISTS(filename) (_access((filename), 0) != -1)
#define RPG_SLEEP(ms) Sleep(ms)

#else /* *nix */

#include <unistd.h>
#define RPG_FILE_EXISTS(filename) (access((filename), 0) != -1)
#define RPG_SLEEP(ms) usleep(ms * 1000)
#endif

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glad.h"
#include "rpg.h"

#ifndef RPG_WITHOUT_OPENAL
#include <AL/alc.h>
typedef struct RPGchannel RPGchannel;
#endif

#define BATCH_INIT_CAPACITY 4

#define BYTES_PER_PIXEL 4

#define UNIFORM_PROJECTION "projection"
#define UNIFORM_MODEL "model"
#define UNIFORM_COLOR "color"
#define UNIFORM_TONE "tone"
#define UNIFORM_ALPHA "alpha"
#define UNIFORM_HUE "hue"


#define VERTICES_COUNT 24
#define VERTICES_SIZE (sizeof(RPGfloat) * VERTICES_COUNT)
#define VERTICES_STRIDE (sizeof(RPGfloat) * 4)
#define RPG_PI 3.14159274f

#define RPG_ENSURE_FILE(filename)                                                                                                          \
    if (!RPG_FILE_EXISTS(filename))                                                                                                        \
    return RPG_ERR_FILE_NOT_FOUND

#define RPG_RETURN_IF_NULL(ptr)                                                                                                            \
    if (ptr == NULL)                                                                                                                       \
    return RPG_ERR_INVALID_POINTER

#define RPG_BASE_UNIFORMS(r)                                                                                                               \
    glUniform4f(RPG_GAME->shader.color, r.color.x, r.color.y, r.color.z, r.color.w);                                                       \
    glUniform4f(RPG_GAME->shader.tone, r.tone.x, r.tone.y, r.tone.z, r.tone.w);                                                            \
    glUniform1f(RPG_GAME->shader.alpha, r.alpha);                                                                                          \
    glUniform1f(RPG_GAME->shader.hue, r.hue);                                                                                              \
    glUniform4f(RPG_GAME->shader.flash, r.flash.color.x, r.flash.color.y, r.flash.color.z, r.flash.color.w);                               \
    glUniformMatrix4fv(RPG_GAME->shader.model, 1, GL_FALSE, (GLfloat *) &r.model);                                                         \
    glBlendEquation(r.blend.op);                                                                                                           \
    glBlendFunc(r.blend.src, r.blend.dst)

#define RPG_CLAMPF(v, min, max) (fmaxf(min, fminf(max, v)))

#define RPG_CLAMPI(v, min, max) (imax(min, imin(v, max)))

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

#define DEF_FX_PARAM_B(type, name, param, min, max)                                                                                        \
    RPG_RESULT RPG_##type##_Get##name(RPGaudiofx *fx, RPGbool *value) {                                                                    \
        RPG_RETURN_IF_NULL(fx);                                                                                                            \
        RPG_RETURN_IF_NULL(value);                                                                                                         \
        int i;                                                                                                                             \
        alGetEffecti(fx->effect, param, &i);                                                                                               \
        *value = i ? RPG_TRUE : RPG_FALSE;                                                                                                 \
        return RPG_NO_ERROR;                                                                                                               \
    }                                                                                                                                      \
    RPG_RESULT RPG_##type##_Set##name(RPGaudiofx *fx, RPGbool value) {                                                                     \
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
        alGetEffectfv(fx->effect, param, (RPGfloat *) value);                                                                              \
        return RPG_NO_ERROR;                                                                                                               \
    }                                                                                                                                      \
    RPG_RESULT RPG_##type##_Set##name(RPGaudiofx *fx, RPGvec3 *value) {                                                                    \
        RPG_RETURN_IF_NULL(fx);                                                                                                            \
        alEffectfv(fx->effect, param, (RPGfloat *) value);                                                                                 \
        if (alGetError())                                                                                                                  \
            return RPG_ERR_INVALID_POINTER;                                                                                                \
        RPG_Audio_UpdateEffectChange(fx);                                                                                                  \
        return RPG_NO_ERROR;                                                                                                               \
    }

#define DEF_FX_CREATE(name, type)                                                                                                          \
    RPG_RESULT RPG_##name##_Create(RPGaudiofx **fx) { return RPG_Audio_CreateEffect(type, fx); }

// Dynamically allocates a type
#define RPG_ALLOC(type) ((type *) RPG_MALLOC(sizeof(type)))

// Dynamically allocates an array of a type
#define RPG_ALLOC_N(type, n) ((type *) RPG_MALLOC(sizeof(type) * n))

// Dynamically allocates a type and clears it to 0
#define RPG_ALLOC_ZERO(var, type)                                                                                                          \
    type *var = RPG_ALLOC(type);                                                                                                           \
    memset(var, 0, sizeof(type))

// Sets the values of a 4x4 matrix to an orthogonal
#define RPG_MAT4_ORTHO(mat4, left, right, top, bottom, near, far)                                                                          \
    mat4.m11 = 2.0f / (right - left);                                                                                                      \
    mat4.m12 = mat4.m13 = mat4.m14 = 0.0f;                                                                                                 \
    mat4.m22                       = 2.0f / (top - bottom);                                                                                \
    mat4.m21 = mat4.m23 = mat4.m24 = 0.0f;                                                                                                 \
    mat4.m33                       = 1.0f / (near - far);                                                                                  \
    mat4.m31 = mat4.m32 = mat4.m34 = 0.0f;                                                                                                 \
    mat4.m41                       = (left + right) / (RPGfloat)(left - right);                                                            \
    mat4.m42                       = (top + bottom) / (RPGfloat)(bottom - top);                                                            \
    mat4.m43                       = near / (RPGfloat)(near - far);                                                                        \
    mat4.m44                       = 1.0f

// Sets the values of a 4x4 matrix
#define RPG_MAT4_SET(mat4, _m11, _m12, _m13, _m14, _m21, _m22, _m23, _m24, _m31, _m32, _m33, _m34, _m41, _m42, _m43, _m44)                 \
    mat4.m11 = _m11;                                                                                                                       \
    mat4.m12 = _m12;                                                                                                                       \
    mat4.m13 = _m13;                                                                                                                       \
    mat4.m14 = _m14;                                                                                                                       \
    mat4.m21 = _m21;                                                                                                                       \
    mat4.m22 = _m22;                                                                                                                       \
    mat4.m23 = _m23;                                                                                                                       \
    mat4.m24 = _m24;                                                                                                                       \
    mat4.m31 = _m31;                                                                                                                       \
    mat4.m32 = _m32;                                                                                                                       \
    mat4.m33 = _m33;                                                                                                                       \
    mat4.m34 = _m34;                                                                                                                       \
    mat4.m41 = _m41;                                                                                                                       \
    mat4.m42 = _m42;                                                                                                                       \
    mat4.m43 = _m43;                                                                                                                       \
    mat4.m44 = _m44

// Renders a texture to the currently bound framebuffer
#define RPG_RENDER_TEXTURE(TEXTURE, VAO)                                                                                                   \
    glActiveTexture(GL_TEXTURE0);                                                                                                          \
    glBindTexture(GL_TEXTURE_2D, TEXTURE);                                                                                                 \
    glBindVertexArray(VAO);                                                                                                                \
    glDrawArrays(GL_TRIANGLES, 0, 6);                                                                                                      \
    glBindVertexArray(0)

// Defines a basic get function
#define DEF_GETTER(name, param, objtype, paramtype, field)                                                                                 \
    RPG_RESULT RPG_##name##_Get##param(objtype *obj, paramtype *value) {                                                                   \
        RPG_RETURN_IF_NULL(obj);                                                                                                           \
        if (value != NULL) {                                                                                                               \
            *value = obj->field;                                                                                                           \
        }                                                                                                                                  \
        return RPG_NO_ERROR;                                                                                                               \
    }

// Defines a basic set function
#define DEF_SETTER(name, param, objtype, paramtype, field)                                                                                 \
    RPG_RESULT RPG_##name_Set##param(objtype *obj, paramtype value) {                                                                      \
        RPG_RETURN_IF_NULL(obj);                                                                                                           \
        obj->field = value;                                                                                                                \
        return RPG_NO_ERROR;                                                                                                               \
    }

// Defines a pair of basic get/set functions 
#define DEF_PARAM(name, param, objtype, paramtype, field)                                                                                  \
    DEF_GETTER(name, param, objtype, paramtype, field)                                                                                     \
    DEF_SETTER(name, param, objtype, paramtype, field)

// Checks if both width and height are greater than 0, returning RPG_ERR_OUT_OF_RANGE if not
#define RPG_CHECK_DIMENSIONS(w, h)                                                                                                         \
    if ((w) < 1 || (h) < 1)                                                                                                                \
    return RPG_ERR_OUT_OF_RANGE

// Resets the clear color back to the user-defined value.
#define RPG_RESET_BACK_COLOR() glClearColor(RPG_GAME->color.x, RPG_GAME->color.y, RPG_GAME->color.z, RPG_GAME->color.w)

// Resets the primary viewport to fit the window correctly.
#define RPG_RESET_VIEWPORT() RPG_VIEWPORT(RPG_GAME->bounds.x, RPG_GAME->bounds.y, RPG_GAME->bounds.w, RPG_GAME->bounds.h)

// Resets the primary projection matrix.
#define RPG_RESET_PROJECTION() glUniformMatrix4fv(RPG_GAME->shader.projection, 1, GL_FALSE, (GLfloat *) &RPG_GAME->projection)

// Sets the viewport and scissor rectangle of the primary viewport.
#define RPG_VIEWPORT(x, y, w, h)                                                                                                           \
    glViewport(x, y, w, h);                                                                                                                \
    glScissor(x, y, w, h)

// Binds a an image's framebuffer, creating it if it does not exist
#define RPG_ENSURE_FBO(img)                                                                                                                \
    if (img->fbo == 0) {                                                                                                                   \
        glGenFramebuffers(1, &img->fbo);                                                                                                   \
        glBindFramebuffer(GL_FRAMEBUFFER, img->fbo);                                                                                       \
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img->texture, 0);                                      \
    } else                                                                                                                                 \
        glBindFramebuffer(GL_FRAMEBUFFER, img->fbo)

// Binds an image's framebuffer, using the specified coordinates for its ortho
#define RPG_BIND_FBO(img, x, y, w, h)                                                                                                      \
    RPG_ENSURE_FBO(img);                                                                                                                   \
    RPGmat4 m;                                                                                                                             \
    RPG_MAT4_ORTHO(m, 0.0f, w, 0.0f, h, -1.0f, 1.0f);                                                                                      \
    glUniformMatrix4fv(RPG_GAME->shader.projection, 1, GL_FALSE, (RPGfloat *) &m);                                                         \
    RPG_VIEWPORT(x, y, w, h)

// Unbinds an image's framebuffer, and resets the projection and viewport
#define RPG_UNBIND_FBO(img)                                                                                                                \
    glBindFramebuffer(GL_FRAMEBUFFER, 0);                                                                                                  \
    RPG_RESET_PROJECTION();                                                                                                                \
    RPG_RESET_VIEWPORT()

// The initial pixel size used for font unless set by the user.
#define RPG_FONT_DEFAULT_SIZE 32

// The initial color used for fonts unless set by the user.
#define RPG_FONT_DEFAULT_COLOR                                                                                                             \
    (RPGcolor) { 1.0f, 1.0f, 1.0f, 1.0f }

/**
 * @brief Returns the maximum of two integer values.
 * 
 * @param i1 First value
 * @param i2 Second value
 * @return int The greater of the two values.
 */
static inline int imax(int i1, int i2) { return i1 > i2 ? i1 : i2; }

/**
 * @brief Returns the minimum of two integer values.
 * 
 * @param i1 First value
 * @param i2 Second value
 * @return int The lesser of the two values.
 */
static inline int imin(int i1, int i2) { return i1 < i2 ? i1 : i2; }


typedef struct RPGimage {
    RPGint width;
    RPGint height;
    GLuint texture;
    GLuint fbo;
    void *user;
} RPGimage;

/**
 * @brief Container for a rendering batch, with a quick-sort based on sprite's position on the z-axis.
 */
typedef struct {
    RPGrenderable **items; /** An array of pointers to the sprites within this batch. */
    int capacity;          /** The total capacity the batch can hold before reallocation. */
    int total;             /** The total number of sprites within the batch. */
    RPGbool updated;       /** Flag indicating the items may need reordered due to added entry or change of z-axis. */
} RPGbatch;

typedef struct RPGgame {
    GLFWwindow *window;
    RPGmat4 projection;
    RPGbatch batch;
    struct {
        RPGint width;
        RPGint height;
    } resolution;
    struct {
        RPGint x, y, w, h;
        RPGvec2 ratio;
    } bounds;
    RPG_INIT_FLAGS flags;
    char *title;
    RPGcolor color;
    struct {
        RPGdouble rate;
        RPGint64 count;
        RPGdouble tick;
    } update;
    struct {
        GLuint program;
        GLint projection;
        GLint model;
        GLint color;
        GLint tone;
        GLint flash;
        GLint alpha;
        GLint hue;
    } shader;
#ifndef RPG_WITHOUT_OPENAL
    struct {
        ALCcontext *context;
        ALCdevice *device;
        RPGchannel *channels[RPG_MAX_CHANNELS];
        void *cb;
    } audio;
#endif
    struct {
        RPGubyte keys[RPG_KEY_LAST + 1];
        RPGubyte buttons[RPG_MBUTTON_LAST + 1];
        double scrollX, scrollY;
        GLFWcursor *cursor;
        struct {
            RPGkeyfunc key;
            RPGmbuttonfunc mbtn;
            RPGcursorfunc cursor;
            RPGcursorfunc scroll;
        } cb;
        struct {
            void *buffer;
            RPGsize size;
            RPGint pos;
        } capture;
    } input;
    struct {
        GLuint program;
        GLint projection;
        GLint color;
        GLuint vbo;
        GLuint vao;
        RPGcolor defaultColor;
        RPGint defaultSize;
    } font;
    struct {
        RPGgamestate focused;
        RPGgamestate minimized;
        RPGgamestate maximized;
        RPGgameaction restored;
        RPGgameaction closing;
        RPGfiledropfunc filedrop;
        RPGmovefunc moved;
        RPGsizefunc resized;
    } cb;
    void *user;
} RPGgame;

typedef struct RPGshader {
    GLuint program;
    void *user;
} RPGshader;

/**
 * @brief Base structure for objects that can be rendered.
 */
typedef struct RPGrenderable {
    RPGint x;        /** The location of the sprite on the x-axis. */
    RPGint y;        /** The location of the sprite on the y-axis. */
    RPGint z;        /** The position of the sprite on the z-axis. */
    RPGint ox;       /** The origin point on the x-axis, context-dependent definition. */
    RPGint oy;       /** The origin point on the y-axis, context-dependent definition. */
    RPGbool updated; /** Flag indicating if the model matrix needs updated to reflect changes. */
    RPGbool visible; /** Flag indicating if object should be rendered. */
    RPGfloat alpha;  /** The opacity level to be rendered at in the range of 0.0 to 1.0. */
    RPGcolor color;  /** The color to blended when rendered. */
    RPGtone tone;    /** The tone to apply when rendered. */
    RPGfloat hue;    /** The amount of hue to apply, in degrees. */
    RPGvec2 scale;   /** The amount of scale to apply when rendered. */
    struct {
        RPGcolor color;    /** The color to use for the flash effect. */
        RPGubyte duration; /** The number of remaining frames to apply the flash effect. */
    } flash;               /** The flash effect to apply when rendered. */
    struct {
        RPGfloat radians; /** The amount of rotation, in radians. */
        RPGint ox;        /** The anchor point on the x-axis to rotate around, relative to the sprite. */
        RPGint oy;        /** The anchor point on the y-axis to rotate around, relative to the sprite. */
    } rotation;
    struct {
        RPG_BLEND_OP op;  /** The equation used for combining the source and destination factors. */
        RPG_BLEND src;    /** The factor to be used for the source pixel color. */
        RPG_BLEND dst;    /** The factor ot used for the destination pixel color. */
    } blend;              /** The blending factors to apply during rendering. */
    RPGrenderfunc render; /** The function to call when the object needs rendered. */
    RPGmat4 model;        /** The model matrix for the object. */
    RPGbatch *parent;     /** Pointer to the rendering batch the object is contained within */
    void *user;           /** Arbitrary user-defined pointer to store with this instance */
} RPGrenderable;

/**
 * @brief Contains the information required to render an arbitrary image on-screen.
 */
typedef struct RPGsprite {
    RPGrenderable base;    /** The base renderable object, MUST BE FIRST FIELD IN THE STRUCTURE! */
    RPGimage *image;         /** A pointer ot the sprite's image, or NULL. */
    RPGviewport *viewport; /** A pointer to the sprite's viewport, or NULL. */
    RPGrect rect;          /** The source rectangle of the sprite's image. */
    GLuint vbo;            /** The Vertex Buffer Object bound to this sprite. */
    GLuint vao;            /** The Vertex Array Object bound to this sprite. */
} RPGsprite;

/**
 * @brief A container for sprites that is drawn in its own independent batch with its own projection.
 */
typedef struct RPGviewport {
    RPGrenderable base; /** The base renderable object, MUST BE FIRST FIELD IN THE STRUCTURE! */
    RPGint width;       /** The dimension of the viewport, in pixels, on the x-axis. */
    RPGint height;      /** The dimension of the viewport, in pixels, on the y-axis. */
    RPGbatch batch;     /** A collection containing pointers to the sprites within this viewport. */
    GLuint fbo;         /** The Framebuffer Object for rendering to the viewport. */
    GLuint texture;     /** Texture with the viewport batch rendered onto it. */
    RPGmat4 projection; /** The projection matrix for the viewport. */
    GLuint vbo;         /** The Vertex Buffer Object bound to this viewport. */
    GLuint vao;         /** The Vertex Array Object bound to this viewport. */
} RPGviewport;

/**
 * @brief Specialized sprite that automatically tiles its source image across its bounds.
 */
typedef struct RPGplane {
    RPGrenderable base;    /** The base renderable object, MUST BE FIRST FIELD IN THE STRUCTURE! */
    RPGimage *image;       /** A pointer ot the sprite's image, or NULL. */
    RPGviewport *viewport; /** A pointer to the sprite's viewport, or NULL. */
    RPGint width;          /** The dimension of the plane, in pixels, on the x-axis. */
    RPGint height;         /** The dimension of the plane, in pixels, on the y-axis. */
    GLuint vbo;            /** The Vertex Buffer Object bound to this sprite. */
    GLuint vao;            /** The Vertex Array Object bound to this sprite. */
    GLuint sampler;        /** Sampler object for tiling the image across the bounds of the plane. */
    RPGbool updateVAO;     /** Flag indicating the plane's VAO needs updated to reflect a change. */
    RPGvec2 zoom;          /** The amount of scaling to apply to the source image. */
} RPGplane;

extern const char *RPG_VERTEX_SHADER;
extern const char *RPG_FRAGMENT_SHADER;
extern const char *RPG_FONT_VERTEX;
extern const char *RPG_FONT_FRAGMENT;
extern const char *RPG_TILEMAP_VERTEX;
extern const char *RPG_TILEMAP_FRAGMENT;

extern const char *RPG_TRANSITION_BASE_VERTEX;
extern const char *RPG_TRANSITION_BASE_FRAGMENT;
extern const char *RPG_TRANSITION_GLITCH_DISPLACE;
extern const char *RPG_TRANSITION_DIRECTIONAL_WARP;
extern const char *RPG_TRANSITION_LUMINANCE_MELT;
extern const char *RPG_TRANSITION_PINWHEEL;
extern const char *RPG_TRANSITION_CIRCLE;
extern const char *RPG_TRANSITION_LUMA;
extern const char *RPG_TRANSITION_RIPPLE;
extern const char *RPG_TRANSITION_CURTAIN;
extern const char *RPG_TRANSITION_DIRECTIONAL_WIPE;
extern const char *RPG_TRANSITION_WATERDROP;
extern const char *RPG_TRANSITION_GRID_FLIP;
extern const char *RPG_TRANSITION_BURN;
extern const char *RPG_TRANSITION_COLOR_PHASE;
extern const char *RPG_TRANSITION_SWIRL;
extern const char *RPG_TRANSITION_HEART;
extern const char *RPG_TRANSITION_HEXAGONALIZE;
extern const char *RPG_TRANSITION_SQUARES_WIRE;
extern const char *RPG_TRANSITION_WINDOWBLINDS;
extern const char *RPG_TRANSITION_STATIC;
extern const char *RPG_TRANSITION_DISPLACEMENT;
extern const char *RPG_TRANSITION_BOW_TIE_V;
extern const char *RPG_TRANSITION_PARAMETRICS;
extern const char *RPG_TRANSITION_WIPE_LEFT;
extern const char *RPG_TRANSITION_DIRECTIONAL;
extern const char *RPG_TRANSITION_WIPE_DOWN;
extern const char *RPG_TRANSITION_MORPH;
extern const char *RPG_TRANSITION_FADE_GRAYSCALE;
extern const char *RPG_TRANSITION_ROTATE_SCALE_FADE;
extern const char *RPG_TRANSITION_WIPE_UP;
extern const char *RPG_TRANSITION_CROSS_ZOOM;
extern const char *RPG_TRANSITION_MOSAIC;
extern const char *RPG_TRANSITION_PIXELIZE;
extern const char *RPG_TRANSITION_SQUEEZE;
extern const char *RPG_TRANSITION_WINDOW_SLICE;
extern const char *RPG_TRANSITION_RADIAL;
extern const char *RPG_TRANSITION_FADE_COLOR;
extern const char *RPG_TRANSITION_SWAP;
extern const char *RPG_TRANSITION_KALEIDOSCOPE;
extern const char *RPG_TRANSITION_ANGULAR;
extern const char *RPG_TRANSITION_DOORWAY;
extern const char *RPG_TRANSITION_MULTIPLY_BLEND;
extern const char *RPG_TRANSITION_LINEAR_BLUR;
extern const char *RPG_TRANSITION_WIPE_RIGHT;
extern const char *RPG_TRANSITION_CROSS_WARP;
extern const char *RPG_TRANSITION_GLITCH_MEMORIES;
extern const char *RPG_TRANSITION_SQUARES;
extern const char *RPG_TRANSITION_SIMPLE_ZOOM;
extern const char *RPG_TRANSITION_PERLIN;
extern const char *RPG_TRANSITION_FLYEYE;
extern const char *RPG_TRANSITION_CUBE;
extern const char *RPG_TRANSITION_BURNOUT;
extern const char *RPG_TRANSITION_PAGE_CURL;
extern const char *RPG_TRANSITION_FADE;
extern const char *RPG_TRANSITION_FILM_BURN;
extern const char *RPG_TRANSITION_DOOM;
extern const char *RPG_TRANSITION_CROSSHATCH;
extern const char *RPG_TRANSITION_BOUNCE;
extern const char *RPG_TRANSITION_CIRCLE_OPEN;
extern const char *RPG_TRANSITION_BOW_TIE_H;
extern const char *RPG_TRANSITION_DREAMY;
extern const char *RPG_TRANSITION_POLAR;
extern const char *RPG_TRANSITION_COLOR_DISTANCE;
extern const char *RPG_TRANSITION_WIND;

void RPG_Batch_Init(RPGbatch *batch);
void RPG_Batch_Free(RPGbatch *v);
int RPG_Batch_Total(RPGbatch *batch);
void RPG_Batch_Add(RPGbatch *batch, RPGrenderable *item);
void RPG_Batch_Set(RPGbatch *batch, int index, RPGrenderable *item);
void RPG_Batch_Delete(RPGbatch *batch, int index);
void RPG_Batch_DeleteItem(RPGbatch *batch, RPGrenderable *item);
void RPG_Batch_Sort(RPGbatch *batch, int first, int last);
void RPG_Renderable_Init(RPGrenderable *renderable, RPGrenderfunc renderfunc, RPGbatch *batch);
RPG_RESULT RPG_ReadFile(const char *filename, void **buffer, size_t *size);

#endif /* OPEN_RPG_INTERNAL_H */