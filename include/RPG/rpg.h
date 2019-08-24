#ifndef OPEN_RPG_H
#define OPEN_RPG_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#ifndef RPG_MALLOC
#define RPG_MALLOC malloc
#endif
#ifndef RPG_FREE
#define RPG_FREE free
#endif
#ifndef RPG_REALLOC
#define RPG_REALLOC realloc
#endif
#ifndef RPG_ASSERT
#include <assert.h>
#define RPG_ASSERT assert
#endif

#define RPG_NONE 0
#define RPG_TRUE 1
#define RPG_FALSE 0

// Primitive types // TODO: Group for types, sub-group for primitives, function protypes, enums, incomplete, complete, etc

typedef int32_t RPGbool;    /** A boolean type (32-bits), zero is false, otherwise non-zero is true */
typedef int8_t RPGbyte;     /** Signed 8-bit integer (-128 to 127) */
typedef int16_t RPGshort;   /** Signed 16-bit integer (-32,768 to 32,7677) */
typedef int32_t RPGint;     /** Signed 32-bit integer (-2,147,483,648 to 2,147,483,647) */
typedef int64_t RPGint64;   /** Signed 64-bit integer (-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807) */
typedef uint8_t RPGubyte;   /** Unsigned 8-bit integer (0 to 255) */
typedef uint16_t RPGushort; /** Unsigned 16-bit integer (0 to 65,535) */
typedef uint32_t RPGuint;   /** Unsigned 32-bit integer (0 to 4,294,967,295) */
typedef uint64_t RPGuint64; /** Unsigned 64-bit integer (0 to 18,446,744,073,709,551,615) */
typedef float RPGfloat;     /** Single-precision 32-bit floating point number with 6 decimal places of precision (1.2E-38 to 3.4E+38) */
typedef double RPGdouble;   /** Double-precision 64-bit floating point number with 15 decimal places of precision (2.3E-308 to 1.7E+308) */
typedef size_t RPGsize;     /** Platform specific (32/64 bit) unsigned integer */

// Incomplete types

typedef struct RPGrenderable RPGrenderable;
typedef struct RPGsprite RPGsprite;
typedef struct RPGviewport RPGviewport;
typedef struct RPGgame RPGgame;
typedef struct RPGimage RPGimage;

// Function protoypes
typedef void (*RPGupdatefunc)(RPGint64 time);

// Complete types

/**
 * @brief A structure encapsulating two single precision floating point values
 */
typedef struct {
    RPGfloat x; /** The value of the x component. */
    RPGfloat y; /** The value of the y component. */
} RPGvec2;

/**
 * @brief A structure encapsulating three single precision floating point values
 */
typedef struct {
    RPGfloat x; /** The value of the x component. */
    RPGfloat y; /** The value of the y component. */
    RPGfloat z; /** The value of the z component. */
} RPGvec3;

/**
 * @brief A structure encapsulating four single precision floating point values.
 */
typedef struct {
    RPGfloat x; /** The value of the x component. */
    RPGfloat y; /** The value of the y component. */
    RPGfloat z; /** The value of the z component. */
    RPGfloat w; /** The value of the w component. */
} RPGvec4;

/**
 * @brief A structure encapsulating a 3x3 matrix.
 */
typedef struct {
    RPGfloat m11; /** Value at row 1, column 1 of the matrix. */
    RPGfloat m12; /** Value at row 1, column 2 of the matrix. */
    RPGfloat m13; /** Value at row 1, column 3 of the matrix. */
    RPGfloat m21; /** Value at row 2, column 1 of the matrix. */
    RPGfloat m22; /** Value at row 2, column 2 of the matrix. */
    RPGfloat m23; /** Value at row 2, column 3 of the matrix. */
    RPGfloat m31; /** Value at row 3, column 1 of the matrix. */
    RPGfloat m32; /** Value at row 3, column 2 of the matrix. */
    RPGfloat m33; /** Value at row 3, column 3 of the matrix. */
} RPGmat3;

/**
 * @brief A structure encapsulating a 4x4 matrix.
 */
typedef struct {
    RPGfloat m11; /** Value at row 1, column 1 of the matrix. */
    RPGfloat m12; /** Value at row 1, column 2 of the matrix. */
    RPGfloat m13; /** Value at row 1, column 3 of the matrix. */
    RPGfloat m14; /** Value at row 1, column 4 of the matrix. */
    RPGfloat m21; /** Value at row 2, column 1 of the matrix. */
    RPGfloat m22; /** Value at row 2, column 2 of the matrix. */
    RPGfloat m23; /** Value at row 2, column 3 of the matrix. */
    RPGfloat m24; /** Value at row 2, column 4 of the matrix. */
    RPGfloat m31; /** Value at row 3, column 1 of the matrix. */
    RPGfloat m32; /** Value at row 3, column 2 of the matrix. */
    RPGfloat m33; /** Value at row 3, column 3 of the matrix. */
    RPGfloat m34; /** Value at row 3, column 4 of the matrix. */
    RPGfloat m41; /** Value at row 4, column 1 of the matrix. */
    RPGfloat m42; /** Value at row 4, column 2 of the matrix. */
    RPGfloat m43; /** Value at row 4, column 3 of the matrix. */
    RPGfloat m44; /** Value at row 4, column 4 of the matrix. */
} RPGmat4;

/**
 * @brief Represents a color, with each value 
 */
typedef RPGvec4 RPGcolor;
typedef RPGvec4 RPGtone;



typedef struct {
    RPGint width;
    RPGint height;
    void *pixels;
} RPGrawimage;

/**
 * @brief Describes a shape with four sides and four 90 degree angles.
 */
typedef struct {
    RPGint x; /** The location on the x-axis. */
    RPGint y; /** The location on the y-axis. */
    RPGint w; /** The dimension on the x-axis. */
    RPGint h; /** The dimension on the y-axis. */
} RPGrect;

// Enums

typedef enum {
    RPG_NO_ERROR,
    RPG_ERR_SYSTEM,
    RPG_ERR_FORMAT,
    RPG_ERR_FILE_NOT_FOUND,
    RPG_ERR_AUDIO_DEVICE,
    RPG_ERR_AUDIO_CONTEXT,
    RPG_ERR_AUDIO_CANNOT_SEEK,
    RPG_ERR_AUDIO_EXT,
    RPG_ERR_AUDIO_NO_SOUND,
    RPG_ERR_ENCODING,
    RPG_ERR_MALFORMED,
    RPG_ERR_INVALID_POINTER,
    RPG_ERR_OUT_OF_RANGE,
    RPG_ERR_INVALID_VALUE,
    RPG_ERR_THREAD_FAILURE,
    RPG_ERR_IMAGE_LOAD,
    RPG_ERR_MEMORY,
    RPG_ERR_SHADER_COMPILE,
    RPG_ERR_SHADER_LINK,
    RPG_ERR_UNKNOWN
} RPG_RESULT;

typedef enum {
    RPG_BLEND_ZERO                     = 0x0000,
    RPG_BLEND_ONE                      = 0x0001,
    RPG_BLEND_SRC_COLOR                = 0x0300,
    RPG_BLEND_ONE_MINUS_SRC_COLOR      = 0x0301,
    RPG_BLEND_DST_COLOR                = 0x0306,
    RPG_BLEND_ONE_MINUS_DST_COLOR      = 0x0307,
    RPG_BLEND_SRC_ALPHA                = 0x0302,
    RPG_BLEND_ONE_MINUS_SRC_ALPHA      = 0x0303,
    RPG_BLEND_DST_ALPHA                = 0x0304,
    RPG_BLEND_ONE_MINUS_DST_ALPHA      = 0x0305,
    RPG_BLEND_CONSTANT_COLOR           = 0x8001,
    RPG_BLEND_ONE_MINUS_CONSTANT_COLOR = 0x8002,
    RPG_BLEND_CONSTANT_ALPHA           = 0x8003,
    RPG_BLEND_ONE_MINUS_CONSTANT_ALPHA = 0x8004,
    RPG_BLEND_SRC_ALPHA_SATURATE       = 0x0308,
    RPG_BLEND_SRC1_COLOR               = 0x88F9,
    RPG_BLEND_ONE_MINUS_SRC1_COLOR     = 0x88FA,
    RPG_BLEND_SRC1_ALPHA               = 0x8589,
    RPG_BLEND_ONE_MINUS_SRC1_ALPHA     = 0x88FB
} RPG_BLEND;

typedef enum {
    RPG_BLEND_OP_ADD              = 0x8006,
    RPG_BLEND_OP_SUBTRACT         = 0x800A,
    RPG_BLEND_OP_REVERSE_SUBTRACT = 0x800B,
    RPG_BLEND_OP_MIN              = 0x8007,
    RPG_BLEND_OP_MAX              = 0x8008
} RPG_BLEND_OP;

typedef enum {
    RPG_INIT_NONE        = 0x0000, /* No flags */
    RPG_INIT_AUTO_ASPECT = 0x0001, /* A resized window resizes graphics, but applies pillars/letterbox to maintain internal resolution */
    RPG_INIT_LOCK_ASPECT = 0x0002, /* A resized window resizes graphics, but only to dimensions that match the graphics aspect ratio */
    RPG_INIT_RESIZABLE   = 0x0004, /* WIndow will be resizable by user, graphics will fill client area of the window */
    RPG_INIT_DECORATED   = 0x0008, /* Window will have border, titlebar, widgets for close, minimize, etc */
    RPG_INIT_FULLSCREEN  = 0x0010, /* Start window in fullscreen mode */
    RPG_INIT_HIDDEN      = 0x0020, /* Do not display newly created window initially */
    RPG_INIT_CENTERED    = 0x0040, /* Center window on the screen (ignored for fullscreen windows) */

    /* Auto-aspect, decorated, and centered */
    RPG_INIT_DEFAULT        = RPG_INIT_AUTO_ASPECT | RPG_INIT_DECORATED | RPG_INIT_CENTERED,
    RPG_INIT_FORCE_UNSIGNED = 0xFFFFFFFF /* Forces enum to use unsigned values, do not use */
} RPG_INIT_FLAGS;

typedef enum {
    RPG_PIXEL_FORMAT_RGB  = 0x1907,
    RPG_PIXEL_FORMAT_RGBA = 0x1908,
    RPG_PIXEL_FORMAT_BGR  = 0x80E0,
    RPG_PIXEL_FORMAT_BGRA = 0x80E1
} RPG_PIXEL_FORMAT;

typedef enum { RPG_IMAGE_FORMAT_PNG, RPG_IMAGE_FORMAT_JPG, RPG_IMAGE_FORMAT_BMP } RPG_IMAGE_FORMAT;

// Game
RPG_RESULT RPG_Game_Create(const char *title, RPGint width, RPGint height, RPG_INIT_FLAGS flags, RPGgame **game);
RPG_RESULT RPG_Game_Destroy(RPGgame *game);
RPG_RESULT RPG_Game_Render(RPGgame *game);
RPG_RESULT RPG_Game_Main(RPGgame *game, RPGdouble tps, RPGupdatefunc updateCallback);
RPG_RESULT RPG_Game_GetFrameRate(RPGgame *game, RPGdouble *rate);
RPG_RESULT RPG_Game_SetFrameRate(RPGgame *game, RPGdouble rate);
RPG_RESULT RPG_Game_GetBackColor(RPGgame *game, RPGcolor *color);
RPG_RESULT RPG_Game_SetBackColor(RPGgame *game, RPGcolor *color);
RPG_RESULT RPG_Game_GetResolution(RPGgame *game, RPGint *width, RPGint *height);
RPG_RESULT RPG_Game_SetResolution(RPGgame *game, RPGint width, RPGint height);
RPG_RESULT RPG_Game_SetIcon(RPGgame *game, RPGrawimage *image);
RPG_RESULT RPG_Game_SetIconFromFile(RPGgame *game, const char *filename);

const char *RPG_GetErrorString(RPG_RESULT result);

// Image
RPG_RESULT RPG_Image_Create(RPGint width, RPGint height, const void *pixels, RPG_PIXEL_FORMAT format, RPGimage **image);
RPG_RESULT RPG_Image_CreateEmpty(RPGint width, RPGint height, RPGimage **image);
RPG_RESULT RPG_Image_CreateFilled(RPGint width, RPGint height, RPGcolor *color, RPGimage **image);
RPG_RESULT RPG_Image_CreateFromFile(const char *filename, RPGimage **image);
RPG_RESULT RPG_Image_Free(RPGimage *image);
RPG_RESULT RPG_Image_LoadRaw(const char *filename, RPGrawimage **rawImage);
RPG_RESULT RPG_Image_GetSize(RPGimage *image, RPGint *width, RPGint *height);
RPG_RESULT RPG_Image_GetUserPointer(RPGimage *image, void **pointer);
RPG_RESULT RPG_Image_SetUserPointer(RPGimage *image, void *pointer);

// Renderable (base for Sprite, Viewport, Plane)
RPG_RESULT RPG_Renderable_Update(RPGrenderable *renderable);
RPG_RESULT RPG_Renderable_GetZ(RPGrenderable *renderable, RPGint *z);
RPG_RESULT RPG_Renderable_SetZ(RPGrenderable *renderable, RPGint z);
RPG_RESULT RPG_Renderable_GetOrigin(RPGrenderable *renderable, RPGint *x, RPGint *y);
RPG_RESULT RPG_Renderable_SetOrigin(RPGrenderable *renderable, RPGint x, RPGint y);
RPG_RESULT RPG_Renderable_GetVisible(RPGrenderable *renderable, RPGbool *visible);
RPG_RESULT RPG_Renderable_SetVisible(RPGrenderable *renderable, RPGbool visible);
RPG_RESULT RPG_Renderable_GetAlpha(RPGrenderable *renderable, RPGfloat *alpha);
RPG_RESULT RPG_Renderable_SetAlpha(RPGrenderable *renderable, RPGfloat alpha);
RPG_RESULT RPG_Renderable_GetHue(RPGrenderable *renderable, RPGfloat *hue);
RPG_RESULT RPG_Renderable_SetHue(RPGrenderable *renderable, RPGfloat hue);
RPG_RESULT RPG_Renderable_GetColor(RPGrenderable *renderable, RPGcolor *color);
RPG_RESULT RPG_Renderable_SetColor(RPGrenderable *renderable, RPGcolor *color);
RPG_RESULT RPG_Renderable_GetTone(RPGrenderable *renderable, RPGcolor *tone);
RPG_RESULT RPG_Renderable_SetTone(RPGrenderable *renderable, RPGcolor *tone);
RPG_RESULT RPG_Renderable_GetScale(RPGrenderable *renderable, RPGfloat *x, RPGfloat *y);
RPG_RESULT RPG_Renderable_SetScale(RPGrenderable *renderable, RPGfloat x, RPGfloat y);
RPG_RESULT RPG_Renderable_GetRotation(RPGrenderable *renderable, RPGfloat *degrees, RPGint *anchorX, RPGint *anchorY);
RPG_RESULT RPG_Renderable_SetRotation(RPGrenderable *renderable, RPGfloat degrees, RPGint anchorX, RPGint anchorY);
RPG_RESULT RPG_Renderable_GetUserPointer(RPGrenderable *renderable, void **user);
RPG_RESULT RPG_Renderable_SetUserPointer(RPGrenderable *renderable, void *user);
RPG_RESULT RPG_Renderable_GetBlendMode(RPGrenderable *renderable, RPG_BLEND *src, RPG_BLEND *dst, RPG_BLEND_OP *op);
RPG_RESULT RPG_Renderable_SetBlendMode(RPGrenderable *renderable, RPG_BLEND src, RPG_BLEND dst, RPG_BLEND_OP op);
RPG_RESULT RPG_Renderable_GetFlash(RPGrenderable *renderable, RPGcolor *color, RPGubyte *duration);
RPG_RESULT RPG_Renderable_SetFlash(RPGrenderable *renderable, RPGcolor *color, RPGubyte duration);

// Sprite
RPG_RESULT RPG_Sprite_Create(RPGgame *game, RPGviewport *viewport, RPGsprite **sprite);
RPG_RESULT RPG_Sprite_Free(RPGsprite *sprite);
RPG_RESULT RPG_Sprite_GetViewport(RPGsprite *sprite, RPGviewport **viewport);
RPG_RESULT RPG_Sprite_GetImage(RPGsprite *sprite, RPGimage **image);
RPG_RESULT RPG_Sprite_SetImage(RPGsprite *sprite, RPGimage *image);
RPG_RESULT RPG_Sprite_GetSourceRect(RPGsprite *sprite, RPGrect *rect);
RPG_RESULT RPG_Sprite_SetSourceRect(RPGsprite *sprite, RPGrect *rect);
RPG_RESULT RPG_Sprite_SetSourceRectValues(RPGsprite *sprite, RPGint x, RPGint y, RPGint w, RPGint h);
RPG_RESULT RPG_Sprite_GetVertexArray(RPGsprite *sprite, RPGuint *vao);
RPG_RESULT RPG_Sprite_GetVertexBuffer(RPGsprite *sprite, RPGuint *vbo);
RPG_RESULT RPG_Sprite_GetX(RPGsprite *sprite, RPGint *x); 
RPG_RESULT RPG_Sprite_GetY(RPGsprite *sprite, RPGint *y);
RPG_RESULT RPG_Sprite_SetX(RPGsprite *sprite, RPGint x); 
RPG_RESULT RPG_Sprite_SetY(RPGsprite *sprite, RPGint y);
RPG_RESULT RPG_Sprite_GetLocation(RPGsprite *sprite, RPGint *x, RPGint *y);
RPG_RESULT RPG_Sprite_SetLocation(RPGsprite *sprite, RPGint x, RPGint y);

// Viewport
RPG_RESULT RPG_Viewport_Create(RPGgame *game, RPGint x, RPGint y, RPGint width, RPGint height, RPGviewport **viewport);
RPG_RESULT RPG_Viewport_CreateFromRect(RPGgame *game, RPGrect *rect, RPGviewport **viewport);
RPG_RESULT RPG_Viewport_CreateDefault(RPGgame *game, RPGviewport **viewport);
RPG_RESULT RPG_Viewport_Free(RPGviewport *viewport);
RPG_RESULT RPG_Viewport_GetRect(RPGviewport *viewport, RPGint *x, RPGint *y, RPGint *width, RPGint *height);

#ifdef __cplusplus
}
#endif

#endif /* OPEN_RPG_H */