#ifndef OPEN_RPG_GAME_H
#define OPEN_RPG_GAME_H 1

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

void RPG_Batch_Init(RPGbatch *batch);
void RPG_Batch_Free(RPGbatch *v);
int RPG_Batch_Total(RPGbatch *batch);
void RPG_Batch_Add(RPGbatch *batch, RPGrenderable *item);
void RPG_Batch_Set(RPGbatch *batch, int index, RPGrenderable *item);
void RPG_Batch_Delete(RPGbatch *batch, int index);
void RPG_Batch_DeleteItem(RPGbatch *batch, RPGrenderable *item);
void RPG_Batch_Sort(RPGbatch *batch, int first, int last);

void RPG_Renderable_Init(RPGrenderable *renderable, RPGrenderfunc renderfunc, RPGbatch *batch);

#endif /* OPEN_RPG_GAME_H */