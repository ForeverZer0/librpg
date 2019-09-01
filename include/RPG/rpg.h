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
#ifdef RPG_DEBUG
#ifndef RPG_ASSERT
#include <assert.h>
#define RPG_ASSERT assert

#include <stdio.h> // FIXME: internal.h
#define RPG_CHECK_GL_ERROR()                                                                                                               \
    do {                                                                                                                                   \
        GLenum error = glGetError();                                                                                                       \
        if (error) {                                                                                                                       \
            printf("%x\n", error);                                                                                                           \
            RPG_ASSERT(!error);                                                                                                             \
        }                                                                                                                                  \
    } while (0)
#endif
#else
#define RPG_ASSERT(expr)
#endif

#define RPG_NONE 0
#define RPG_TRUE 1
#define RPG_FALSE 0

// Primitive types // TODO: Group for types, sub-group for primitives, function protypes, enums, incomplete, complete, etc

typedef uint8_t RPGbool;    /** A boolean type (32-bits), zero is false, otherwise non-zero is true */
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
typedef uint32_t RPGsize;   /** Unsigned 32-bit integer (0 to 4,294,967,295) */

// Incomplete types

typedef struct RPGrenderable RPGrenderable;
typedef struct RPGsprite RPGsprite;
typedef struct RPGviewport RPGviewport;
typedef struct RPGplane RPGplane;
typedef struct RPGgame RPGgame;
typedef struct RPGimage RPGimage;
typedef struct RPGfont RPGfont;
typedef struct RPGshader RPGshader;
typedef struct RPGtilemap RPGtilemap;

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
 * @brief Represents a color in the RGBA colorspace, where each component is in the range of 0.0 to 1.0
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
    RPG_ERR_CONTEXT,
    RPG_ERR_UNSUPPORTED,
    RPG_ERR_FILE_NOT_FOUND,
    RPG_ERR_FILE_READ_ERROR,
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
    RPG_ERR_IMAGE_SAVE,
    RPG_ERR_MEMORY,
    RPG_ERR_SHADER_COMPILE,
    RPG_ERR_SHADER_LINK,
    RPG_ERR_SHADER_NOT_ACTIVE,
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
    RPG_INIT_RESIZABLE   = 0x0004, /* Window will be resizable by user, graphics will fill client area of the window */
    RPG_INIT_DECORATED   = 0x0008, /* Window will have border, titlebar, widgets for close, minimize, etc */
    RPG_INIT_FULLSCREEN  = 0x0010, /* Start window in fullscreen mode */
    RPG_INIT_HIDDEN      = 0x0020, /* Do not display newly created window initially */
    RPG_INIT_CENTERED    = 0x0040, /* Center window on the screen (ignored for fullscreen windows) */

    /* Auto-aspect, decorated, and centered */
    RPG_INIT_DEFAULT = RPG_INIT_AUTO_ASPECT | RPG_INIT_DECORATED | RPG_INIT_CENTERED
} RPG_INIT_FLAGS;

typedef enum {
    RPG_PIXEL_FORMAT_RGB  = 0x1907,
    RPG_PIXEL_FORMAT_RGBA = 0x1908,
    RPG_PIXEL_FORMAT_BGR  = 0x80E0,
    RPG_PIXEL_FORMAT_BGRA = 0x80E1
} RPG_PIXEL_FORMAT;

typedef enum {
    RPG_ALIGN_NONE          = 0x00,
    RPG_ALIGN_TOP           = 0x01,
    RPG_ALIGN_CENTER_V      = 0x02,
    RPG_ALIGN_BOTTOM        = 0x04,
    RPG_ALIGN_LEFT          = 0x08,
    RPG_ALIGN_CENTER_H      = 0x10,
    RPG_ALIGN_RIGHT         = 0x20,
    RPG_ALIGN_TOP_LEFT      = RPG_ALIGN_TOP | RPG_ALIGN_LEFT,
    RPG_ALIGN_TOP_RIGHT     = RPG_ALIGN_TOP | RPG_ALIGN_RIGHT,
    RPG_ALIGN_TOP_CENTER    = RPG_ALIGN_TOP | RPG_ALIGN_CENTER_H,
    RPG_ALIGN_BOTTOM_LEFT   = RPG_ALIGN_BOTTOM | RPG_ALIGN_LEFT,
    RPG_ALIGN_BOTTOM_RIGHT  = RPG_ALIGN_BOTTOM | RPG_ALIGN_RIGHT,
    RPG_ALIGN_BOTTOM_CENTER = RPG_ALIGN_BOTTOM | RPG_ALIGN_CENTER_H,
    RPG_ALIGN_CENTER_LEFT   = RPG_ALIGN_CENTER_V | RPG_ALIGN_LEFT,
    RPG_ALIGN_CENTER_RIGHT  = RPG_ALIGN_CENTER_V | RPG_ALIGN_RIGHT,
    RPG_ALIGN_CENTER        = RPG_ALIGN_CENTER_V | RPG_ALIGN_CENTER_H,
    RPG_ALIGN_DEFAULT       = RPG_ALIGN_CENTER_LEFT
} RPG_ALIGN;

typedef enum { RPG_IMAGE_FORMAT_PNG, RPG_IMAGE_FORMAT_JPG, RPG_IMAGE_FORMAT_BMP } RPG_IMAGE_FORMAT;

typedef enum {
    RPG_KEY_UNKNOWN       = -1,
    RPG_KEY_SPACE         = 32,
    RPG_KEY_APOSTROPHE    = 39,
    RPG_KEY_COMMA         = 44,
    RPG_KEY_MINUS         = 45,
    RPG_KEY_PERIOD        = 46,
    RPG_KEY_SLASH         = 47,
    RPG_KEY_0             = 48,
    RPG_KEY_1             = 49,
    RPG_KEY_2             = 50,
    RPG_KEY_3             = 51,
    RPG_KEY_4             = 52,
    RPG_KEY_5             = 53,
    RPG_KEY_6             = 54,
    RPG_KEY_7             = 55,
    RPG_KEY_8             = 56,
    RPG_KEY_9             = 57,
    RPG_KEY_SEMICOLON     = 59,
    RPG_KEY_EQUAL         = 61,
    RPG_KEY_A             = 65,
    RPG_KEY_B             = 66,
    RPG_KEY_C             = 67,
    RPG_KEY_D             = 68,
    RPG_KEY_E             = 69,
    RPG_KEY_F             = 70,
    RPG_KEY_G             = 71,
    RPG_KEY_H             = 72,
    RPG_KEY_I             = 73,
    RPG_KEY_J             = 74,
    RPG_KEY_K             = 75,
    RPG_KEY_L             = 76,
    RPG_KEY_M             = 77,
    RPG_KEY_N             = 78,
    RPG_KEY_O             = 79,
    RPG_KEY_P             = 80,
    RPG_KEY_Q             = 81,
    RPG_KEY_R             = 82,
    RPG_KEY_S             = 83,
    RPG_KEY_T             = 84,
    RPG_KEY_U             = 85,
    RPG_KEY_V             = 86,
    RPG_KEY_W             = 87,
    RPG_KEY_X             = 88,
    RPG_KEY_Y             = 89,
    RPG_KEY_Z             = 90,
    RPG_KEY_LEFT_BRACKET  = 91,
    RPG_KEY_BACKSLASH     = 92,
    RPG_KEY_RIGHT_BRACKET = 93,
    RPG_KEY_GRAVE_ACCENT  = 96,
    RPG_KEY_WORLD_1       = 161,
    RPG_KEY_WORLD_2       = 162,
    RPG_KEY_ESCAPE        = 256,
    RPG_KEY_ENTER         = 257,
    RPG_KEY_TAB           = 258,
    RPG_KEY_BACKSPACE     = 259,
    RPG_KEY_INSERT        = 260,
    RPG_KEY_DELETE        = 261,
    RPG_KEY_RIGHT         = 262,
    RPG_KEY_LEFT          = 263,
    RPG_KEY_DOWN          = 264,
    RPG_KEY_UP            = 265,
    RPG_KEY_PAGE_UP       = 266,
    RPG_KEY_PAGE_DOWN     = 267,
    RPG_KEY_HOME          = 268,
    RPG_KEY_END           = 269,
    RPG_KEY_CAPS_LOCK     = 280,
    RPG_KEY_SCROLL_LOCK   = 281,
    RPG_KEY_NUM_LOCK      = 282,
    RPG_KEY_PRINT_SCREEN  = 283,
    RPG_KEY_PAUSE         = 284,
    RPG_KEY_F1            = 290,
    RPG_KEY_F2            = 291,
    RPG_KEY_F3            = 292,
    RPG_KEY_F4            = 293,
    RPG_KEY_F5            = 294,
    RPG_KEY_F6            = 295,
    RPG_KEY_F7            = 296,
    RPG_KEY_F8            = 297,
    RPG_KEY_F9            = 298,
    RPG_KEY_F10           = 299,
    RPG_KEY_F11           = 300,
    RPG_KEY_F12           = 301,
    RPG_KEY_F13           = 302,
    RPG_KEY_F14           = 303,
    RPG_KEY_F15           = 304,
    RPG_KEY_F16           = 305,
    RPG_KEY_F17           = 306,
    RPG_KEY_F18           = 307,
    RPG_KEY_F19           = 308,
    RPG_KEY_F20           = 309,
    RPG_KEY_F21           = 310,
    RPG_KEY_F22           = 311,
    RPG_KEY_F23           = 312,
    RPG_KEY_F24           = 313,
    RPG_KEY_F25           = 314,
    RPG_KEY_KP_0          = 320,
    RPG_KEY_KP_1          = 321,
    RPG_KEY_KP_2          = 322,
    RPG_KEY_KP_3          = 323,
    RPG_KEY_KP_4          = 324,
    RPG_KEY_KP_5          = 325,
    RPG_KEY_KP_6          = 326,
    RPG_KEY_KP_7          = 327,
    RPG_KEY_KP_8          = 328,
    RPG_KEY_KP_9          = 329,
    RPG_KEY_KP_DECIMAL    = 330,
    RPG_KEY_KP_DIVIDE     = 331,
    RPG_KEY_KP_MULTIPLY   = 332,
    RPG_KEY_KP_SUBTRACT   = 333,
    RPG_KEY_KP_ADD        = 334,
    RPG_KEY_KP_ENTER      = 335,
    RPG_KEY_KP_EQUAL      = 336,
    RPG_KEY_LEFT_SHIFT    = 340,
    RPG_KEY_LEFT_CONTROL  = 341,
    RPG_KEY_LEFT_ALT      = 342,
    RPG_KEY_LEFT_SUPER    = 343,
    RPG_KEY_RIGHT_SHIFT   = 344,
    RPG_KEY_RIGHT_CONTROL = 345,
    RPG_KEY_RIGHT_ALT     = 346,
    RPG_KEY_RIGHT_SUPER   = 347,
    RPG_KEY_MENU          = 348,
    RPG_KEY_LAST          = RPG_KEY_MENU
} RPG_KEY;

typedef enum {
    RPG_MODKEY_NONE      = 0x0000,
    RPG_MODKEY_SHIFT     = 0x0001,
    RPG_MODKEY_CONTROL   = 0x0002,
    RPG_MODKEY_ALT       = 0x0004,
    RPG_MODKEY_SUPER     = 0x0008,
    RPG_MODKEY_CAPS_LOCK = 0x0010,
    RPG_MODKEY_NUM_LOCK  = 0x0020
} RPG_MODKEY;

typedef enum {
    RPG_MBUTTON_1      = 0,
    RPG_MBUTTON_2      = 1,
    RPG_MBUTTON_3      = 2,
    RPG_MBUTTON_4      = 3,
    RPG_MBUTTON_5      = 4,
    RPG_MBUTTON_6      = 5,
    RPG_MBUTTON_7      = 6,
    RPG_MBUTTON_8      = 7,
    RPG_MBUTTON_LAST   = RPG_MBUTTON_8,
    RPG_MBUTTON_LEFT   = RPG_MBUTTON_1,
    RPG_MBUTTON_RIGHT  = RPG_MBUTTON_2,
    RPG_MBUTTON_MIDDLE = RPG_MBUTTON_3
} RPG_MBUTTON;

typedef enum {
    RPG_CURSOR_ARROW,
    RPG_CURSOR_IBEAM,
    RPG_CURSOR_CROSSHAIR,
    RPG_CURSOR_HAND,
    RPG_CURSOR_HRESIZE,
    RPG_CURSOR_VRESIZE,
} RPG_CURSOR;

typedef enum {
    RPG_TRANSITION_TYPE_GLITCH_DISPLACE,
    RPG_TRANSITION_TYPE_DIRECTIONAL_WARP,
    RPG_TRANSITION_TYPE_LUMINANCE_MELT,
    RPG_TRANSITION_TYPE_PINWHEEL,
    RPG_TRANSITION_TYPE_CIRCLE,
    RPG_TRANSITION_TYPE_LUMA,
    RPG_TRANSITION_TYPE_RIPPLE,
    RPG_TRANSITION_TYPE_CURTAIN,
    RPG_TRANSITION_TYPE_DIRECTIONAL_WIPE,
    RPG_TRANSITION_TYPE_WATERDROP,
    RPG_TRANSITION_TYPE_GRID_FLIP,
    RPG_TRANSITION_TYPE_BURN,
    RPG_TRANSITION_TYPE_COLOR_PHASE,
    RPG_TRANSITION_TYPE_SWIRL,
    RPG_TRANSITION_TYPE_HEART,
    RPG_TRANSITION_TYPE_HEXAGONALIZE,
    RPG_TRANSITION_TYPE_SQUARES_WIRE,
    RPG_TRANSITION_TYPE_WINDOWBLINDS,
    RPG_TRANSITION_TYPE_STATIC,
    RPG_TRANSITION_TYPE_DISPLACEMENT,
    RPG_TRANSITION_TYPE_BOW_TIE_V,
    RPG_TRANSITION_TYPE_PARAMETRICS,
    RPG_TRANSITION_TYPE_WIPE_LEFT,
    RPG_TRANSITION_TYPE_DIRECTIONAL,
    RPG_TRANSITION_TYPE_WIPE_DOWN,
    RPG_TRANSITION_TYPE_MORPH,
    RPG_TRANSITION_TYPE_FADE_GRAYSCALE,
    RPG_TRANSITION_TYPE_ROTATE_SCALE_FADE,
    RPG_TRANSITION_TYPE_WIPE_UP,
    RPG_TRANSITION_TYPE_CROSS_ZOOM,
    RPG_TRANSITION_TYPE_MOSAIC,
    RPG_TRANSITION_TYPE_PIXELIZE,
    RPG_TRANSITION_TYPE_SQUEEZE,
    RPG_TRANSITION_TYPE_WINDOW_SLICE,
    RPG_TRANSITION_TYPE_RADIAL,
    RPG_TRANSITION_TYPE_FADE_COLOR,
    RPG_TRANSITION_TYPE_SWAP,
    RPG_TRANSITION_TYPE_KALEIDOSCOPE,
    RPG_TRANSITION_TYPE_ANGULAR,
    RPG_TRANSITION_TYPE_DOORWAY,
    RPG_TRANSITION_TYPE_MULTIPLY_BLEND,
    RPG_TRANSITION_TYPE_LINEAR_BLUR,
    RPG_TRANSITION_TYPE_WIPE_RIGHT,
    RPG_TRANSITION_TYPE_CROSS_WARP,
    RPG_TRANSITION_TYPE_GLITCH_MEMORIES,
    RPG_TRANSITION_TYPE_SQUARES,
    RPG_TRANSITION_TYPE_SIMPLE_ZOOM,
    RPG_TRANSITION_TYPE_PERLIN,
    RPG_TRANSITION_TYPE_FLYEYE,
    RPG_TRANSITION_TYPE_CUBE,
    RPG_TRANSITION_TYPE_BURNOUT,
    RPG_TRANSITION_TYPE_PAGE_CURL,
    RPG_TRANSITION_TYPE_FADE,
    RPG_TRANSITION_TYPE_FILM_BURN,
    RPG_TRANSITION_TYPE_DOOM,
    RPG_TRANSITION_TYPE_CROSSHATCH,
    RPG_TRANSITION_TYPE_BOUNCE,
    RPG_TRANSITION_TYPE_CIRCLE_OPEN,
    RPG_TRANSITION_TYPE_BOW_TIE_H,
    RPG_TRANSITION_TYPE_DREAMY,
    RPG_TRANSITION_TYPE_POLAR,
    RPG_TRANSITION_TYPE_COLOR_DISTANCE,
    RPG_TRANSITION_TYPE_WIND,
    RPG_TRANSITION_TYPE_LAST = RPG_TRANSITION_TYPE_WIND
} RPG_TRANSITION_TYPE;

typedef enum { RPG_INPUT_STATE_RELEASE, RPG_INPUT_STATE_PRESS, RPG_INPUT_STATE_REPEAT } RPG_INPUT_STATE;

typedef enum { RPG_CURSOR_MODE_NORMAL, RPG_CURSOR_MODE_HIDDEN, RPG_CURSOR_MODE_DISABLED } RPG_CURSOR_MODE;

// Function protoypes
typedef void (*RPGupdatefunc)(RPGint64 time);
typedef void (*RPGrenderfunc)(void *renderable);
typedef void (*RPGkeyfunc)(RPGgame *game, RPG_KEY key, int scancode, RPG_INPUT_STATE state, RPG_MODKEY mods);
typedef void (*RPGmbuttonfunc)(RPGgame *game, RPG_MBUTTON button, RPG_INPUT_STATE state, RPG_MODKEY mods);
typedef void (*RPGcursorfunc)(RPGgame *game, RPGdouble x, RPGdouble y);
typedef void (*RPGgamestate)(RPGgame *game, RPGbool state);
typedef void (*RPGgameaction)(RPGgame *game);
typedef void (*RPGfiledropfunc)(RPGgame *game, RPGint count, const char **files);
typedef void (*RPGmovefunc)(RPGgame *game, RPGint x, RPGint y);
typedef void (*RPGsizefunc)(RPGgame *game, RPGint width, RPGint height);
typedef void (*RPGtransitionfunc)(RPGgame *game, RPGshader *shader);

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
RPG_RESULT RPG_Game_Snapshot(RPGgame *game, RPGimage **image);
RPG_RESULT RPG_Game_Close(RPGgame *game, RPGbool close);
RPG_RESULT RPG_Game_Show(RPGgame *game);
RPG_RESULT RPG_Game_Hide(RPGgame *game);
RPG_RESULT RPG_Game_Focus(RPGgame *game);
RPG_RESULT RPG_Game_RequestAttention(RPGgame *game);
RPG_RESULT RPG_Game_Minimize(RPGgame *game);
RPG_RESULT RPG_Game_Maximize(RPGgame *game);
RPG_RESULT RPG_Game_Restore(RPGgame *game);
RPG_RESULT RPG_Game_GetWindowSize(RPGgame *game, RPGint *width, RPGint *height);
RPG_RESULT RPG_Game_GetWindowLocation(RPGgame *game, RPGint *x, RPGint *y);
RPG_RESULT RPG_Game_SetWindowSize(RPGgame *game, RPGint width, RPGint height);
RPG_RESULT RPG_Game_SetWindowLocation(RPGgame *game, RPGint x, RPGint y);
RPG_RESULT RPG_Game_SetWindowOpacity(RPGgame *game, RPGfloat opacity);
RPG_RESULT RPG_Game_GetWindowOpacity(RPGgame *game, RPGfloat *opacity);
RPG_RESULT RPG_Game_GetClipboardString(RPGgame *game, void *buffer, RPGsize sizeBuffer, RPGsize *size);
RPG_RESULT RPG_Game_SetClipboardString(RPGgame *game, const char *str);
RPG_RESULT RPG_Game_GetWindowTitle(RPGgame *game, void *buffer, RPGsize sizeBuffer, RPGsize *size);
RPG_RESULT RPG_Game_SetWindowTitle(RPGgame *game, const char *str);
RPG_RESULT RPG_Game_GetIsMaximized(RPGgame *game, RPGbool *state);
RPG_RESULT RPG_Game_GetIsMinimized(RPGgame *game, RPGbool *state);
RPG_RESULT RPG_Game_GetIsFocused(RPGgame *game, RPGbool *state);
RPG_RESULT RPG_Game_GetTopMost(RPGgame *game, RPGbool *state);
RPG_RESULT RPG_Game_SetTopMost(RPGgame *game, RPGbool state);
RPG_RESULT RPG_Game_GetWindowFrameSize(RPGgame *game, RPGint *left, RPGint *top, RPGint *right, RPGint *bottom);
RPG_RESULT RPG_Game_SetCloseCallback(RPGgame *game, RPGgameaction func);
RPG_RESULT RPG_Game_SetMinimizeCallback(RPGgame *game, RPGgamestate func);
RPG_RESULT RPG_Game_SetMaximizeCallback(RPGgame *game, RPGgamestate func);
RPG_RESULT RPG_Game_SetFocusCallback(RPGgame *game, RPGgamestate func);
RPG_RESULT RPG_Game_SetRestoredCallback(RPGgame *game, RPGgameaction func);
RPG_RESULT RPG_Game_SetFileDropCallback(RPGgame *game, RPGfiledropfunc func);
RPG_RESULT RPG_Game_SetMoveCallback(RPGgame *game, RPGmovefunc func);
RPG_RESULT RPG_Game_SetResizeCallback(RPGgame *game, RPGsizefunc func);
RPG_RESULT RPG_Game_Transition(RPGgame *game, RPGshader *shader, RPGint duration, RPGtransitionfunc func);

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
RPG_RESULT RPG_Image_Clear(RPGimage *image);
RPG_RESULT RPG_Image_Fill(RPGimage *image, RPGcolor *color, RPGint x, RPGint y, RPGint w, RPGint h);
RPG_RESULT RPG_Image_FillRect(RPGimage *image, RPGcolor *color, RPGrect *rect);
RPG_RESULT RPG_Image_GetTexture(RPGimage *image, RPGuint *texture);
RPG_RESULT RPG_Image_GetFramebuffer(RPGimage *image, RPGuint *fbo);
RPG_RESULT RPG_Image_SubImage(RPGimage *image, RPGint x, RPGint y, RPGint width, RPGint height, RPGimage **subImage);
RPG_RESULT RPG_Image_Blit(RPGimage *dst, RPGrect *dstRect, RPGimage *src, RPGrect *srcRect, RPGfloat alpha);
RPG_RESULT RPG_Image_GetPixel(RPGimage *image, RPGint x, RPGint y, RPGcolor *color);
RPG_RESULT RPG_Image_SetPixel(RPGimage *image, RPGint x, RPGint y, RPGcolor *color);
RPG_RESULT RPG_Image_GetPixels(RPGimage *image, void *buffer, RPGsize sizeBuffer);
RPG_RESULT RPG_Image_Save(RPGimage *image, const char *filename, RPG_IMAGE_FORMAT format, RPGfloat quality);

// Renderable (base for Sprite, Viewport, Plane)
RPG_RESULT RPG_Renderable_Update(RPGrenderable *renderable);
RPG_RESULT RPG_Renderable_GetZ(RPGrenderable *renderable, RPGint *z);
RPG_RESULT RPG_Renderable_SetZ(RPGrenderable *renderable, RPGint z);
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
RPG_RESULT RPG_Renderable_GetLocation(RPGrenderable *renderable, RPGint *x, RPGint *y);
RPG_RESULT RPG_Renderable_SetLocation(RPGrenderable *renderable, RPGint x, RPGint y);
RPG_RESULT RPG_Renderable_SetRenderFunc(RPGrenderable *renderable, RPGrenderfunc func);

// Sprite
RPG_RESULT RPG_Sprite_Create(RPGviewport *viewport, RPGsprite **sprite);
RPG_RESULT RPG_Sprite_Free(RPGsprite *sprite);
RPG_RESULT RPG_Sprite_GetViewport(RPGsprite *sprite, RPGviewport **viewport);
RPG_RESULT RPG_Sprite_GetImage(RPGsprite *sprite, RPGimage **image);
RPG_RESULT RPG_Sprite_SetImage(RPGsprite *sprite, RPGimage *image);
RPG_RESULT RPG_Sprite_GetSourceRect(RPGsprite *sprite, RPGrect *rect);
RPG_RESULT RPG_Sprite_SetSourceRect(RPGsprite *sprite, RPGrect *rect);
RPG_RESULT RPG_Sprite_GetSourceBounds(RPGsprite *sprite, RPGint *x, RPGint *y, RPGint *width, RPGint *height);
RPG_RESULT RPG_Sprite_SetSourceBounds(RPGsprite *sprite, RPGint x, RPGint y, RPGint width, RPGint height);
RPG_RESULT RPG_Sprite_GetOrigin(RPGsprite *sprite, RPGint *x, RPGint *y);
RPG_RESULT RPG_Sprite_SetOrigin(RPGsprite *sprite, RPGint x, RPGint y);

// Plane
RPG_RESULT RPG_Plane_Create(RPGviewport *viewport, RPGplane **plane);
RPG_RESULT RPG_Plane_Free(RPGplane *plane);
RPG_RESULT RPG_Plane_GetViewport(RPGplane *plane, RPGviewport **viewport);
RPG_RESULT RPG_Plane_GetRect(RPGplane *plane, RPGrect *rect);
RPG_RESULT RPG_Plane_SetRect(RPGplane *plane, RPGrect *rect);
RPG_RESULT RPG_Plane_GetBounds(RPGplane *plane, RPGint *x, RPGint *y, RPGint *width, RPGint *height);
RPG_RESULT RPG_Plane_SetBounds(RPGplane *plane, RPGint x, RPGint y, RPGint width, RPGint height);
RPG_RESULT RPG_Plane_GetOrigin(RPGplane *plane, RPGint *x, RPGint *y);
RPG_RESULT RPG_Plane_SetOrigin(RPGplane *plane, RPGint x, RPGint y);
RPG_RESULT RPG_Plane_GetZoom(RPGplane *plane, RPGfloat *x, RPGfloat *y);
RPG_RESULT RPG_Plane_SetZoom(RPGplane *plane, RPGfloat x, RPGfloat y);
RPG_RESULT RPG_Plane_GetImage(RPGplane *plane, RPGimage **image);
RPG_RESULT RPG_Plane_SetImage(RPGplane *plane, RPGimage *image);

// Viewport
RPG_RESULT RPG_Viewport_Create(RPGint x, RPGint y, RPGint width, RPGint height, RPGviewport **viewport);
RPG_RESULT RPG_Viewport_CreateFromRect(RPGrect *rect, RPGviewport **viewport);
RPG_RESULT RPG_Viewport_CreateDefault(RPGviewport **viewport);
RPG_RESULT RPG_Viewport_Free(RPGviewport *viewport);
RPG_RESULT RPG_Viewport_GetRect(RPGviewport *viewport, RPGrect *rect);
RPG_RESULT RPG_Viewport_GetBounds(RPGviewport *viewport, RPGint *x, RPGint *y, RPGint *width, RPGint *height);
RPG_RESULT RPG_Viewport_GetOrigin(RPGviewport *viewport, RPGint *x, RPGint *y);
RPG_RESULT RPG_Viewport_SetOrigin(RPGviewport *viewport, RPGint x, RPGint y);

// Font
RPG_RESULT RPG_Font_Create(void *buffer, RPGsize sizeBuffer, RPGfont **font);
RPG_RESULT RPG_Font_CreateFromFile(const char *filename, RPGfont **font);
RPG_RESULT RPG_Font_Free(RPGfont *font);
RPG_RESULT RPG_Font_GetUserPointer(RPGfont *font, void **user);
RPG_RESULT RPG_Font_SetUserPointer(RPGfont *font, void *user);
RPG_RESULT RPG_Font_DrawText(RPGfont *font, RPGimage *img, const char *text, RPGrect *dstRect, RPG_ALIGN align);
RPG_RESULT RPG_Font_GetSize(RPGfont *font, RPGint *size);
RPG_RESULT RPG_Font_SetSize(RPGfont *font, RPGint size);
RPG_RESULT RPG_Font_GetColor(RPGfont *font, RPGcolor *color);
RPG_RESULT RPG_Font_SetColor(RPGfont *font, RPGcolor *color);
RPG_RESULT RPG_Font_GetDefaultSize(RPGint *size);
RPG_RESULT RPG_Font_GetDefaultColor(RPGcolor *color);
RPG_RESULT RPG_Font_SetDefaultColor(RPGcolor *color);
RPG_RESULT RPG_Font_SetDefaultSize(RPGint size);
RPG_RESULT RPG_Font_MeasureText(RPGfont *font, const char *text, RPGint *width, RPGint *height);

// Shader
RPG_RESULT RPG_Shader_Create(const char *vertSrc, const char *fragSrc, const char *geoSrc, RPGshader **shader);
RPG_RESULT RPG_Shader_CreateTransition(RPG_TRANSITION_TYPE type, RPGshader **shader);
RPG_RESULT RPG_Shader_Begin(RPGshader *shader);
RPG_RESULT RPG_Shader_Finish(RPGshader *shader);
RPG_RESULT RPG_Shader_GetIsActive(RPGshader *shader, RPGbool *active);
RPG_RESULT RPG_Shader_GetUniformLocation(RPGshader *shader, const char *name, RPGint *location);
RPG_RESULT RPG_Shader_Free(RPGshader *shader);
RPG_RESULT RPG_Shader_GetUserPointer(RPGshader *shader, void **user);
RPG_RESULT RPG_Shader_SetUserPointer(RPGshader *shader, void *user);
RPG_RESULT RPG_Shader_GetUniformf(RPGshader *shader, RPGint location, RPGfloat *buffer);
RPG_RESULT RPG_Shader_GetUniformi(RPGshader *shader, RPGint location, RPGint *buffer);
RPG_RESULT RPG_Shader_SetUniform1f(RPGshader *shader, RPGint location, RPGfloat v1);
RPG_RESULT RPG_Shader_SetUniform2f(RPGshader *shader, RPGint location, RPGfloat v1, RPGfloat v2);
RPG_RESULT RPG_Shader_SetUniform3f(RPGshader *shader, RPGint location, RPGfloat v1, RPGfloat v2, RPGfloat v3);
RPG_RESULT RPG_Shader_SetUniform4f(RPGshader *shader, RPGint location, RPGfloat v1, RPGfloat v2, RPGfloat v3, RPGfloat v4);
RPG_RESULT RPG_Shader_SetUniform1i(RPGshader *shader, RPGint location, RPGint v1);
RPG_RESULT RPG_Shader_SetUniform2i(RPGshader *shader, RPGint location, RPGint v1, RPGint v2);
RPG_RESULT RPG_Shader_SetUniform3i(RPGshader *shader, RPGint location, RPGint v1, RPGint v2, RPGint v3);
RPG_RESULT RPG_Shader_SetUniform4i(RPGshader *shader, RPGint location, RPGint v1, RPGint v2, RPGint v3, RPGint v4);
RPG_RESULT RPG_Shader_SetUniformVec2(RPGshader *shader, RPGint location, RPGvec2 *vec);
RPG_RESULT RPG_Shader_SetUniformVec3(RPGshader *shader, RPGint location, RPGvec3 *vec);
RPG_RESULT RPG_Shader_SetUniformVec4(RPGshader *shader, RPGint location, RPGvec4 *vec);
RPG_RESULT RPG_Shader_SetUniformMat3(RPGshader *shader, RPGint location, RPGmat3 *mat);
RPG_RESULT RPG_Shader_SetUniformMat4(RPGshader *shader, RPGint location, RPGmat4 *mat);
RPG_RESULT RPG_Shader_BindImage(RPGshader *shader, RPGint location, RPGint unit, RPGimage *image);
RPG_RESULT RPG_Shader_UnbindImage(RPGshader *shader, RPGint unit);

// Input
RPG_RESULT RPG_Input_Initialize(RPGgame *game);
RPG_RESULT RPG_Input_Update(void);
RPG_RESULT RPG_Input_KeyTrigger(RPG_KEY key, RPGbool *state);
RPG_RESULT RPG_Input_KeyPress(RPG_KEY key, RPGbool *state);
RPG_RESULT RPG_Input_KeyRepeat(RPG_KEY key, RPGbool *state);
RPG_RESULT RPG_Input_KeyRelease(RPG_KEY key, RPGbool *state);
RPG_RESULT RPG_Input_ButtonTrigger(RPG_MBUTTON button, RPGbool *state);
RPG_RESULT RPG_Input_ButtonPress(RPG_MBUTTON button, RPGbool *state);
RPG_RESULT RPG_Input_ButtonRepeat(RPG_MBUTTON button, RPGbool *state);
RPG_RESULT RPG_Input_ButtonRelease(RPG_MBUTTON button, RPGbool *state);
RPG_RESULT RPG_Input_GetCursorLocation(RPGint *x, RPGint *y);
RPG_RESULT RPG_Input_GetMouseScroll(RPGdouble *x, RPGdouble *y);
RPG_RESULT RPG_Input_GetScancode(RPG_KEY key, RPGint *scancode);
RPG_RESULT RPG_Input_SetCursorMode(RPG_CURSOR_MODE mode);
RPG_RESULT RPG_Input_SetCursorType(RPG_CURSOR shape);
RPG_RESULT RPG_Input_SetCursorImage(RPGimage *image, RPGint x, RPGint y);
RPG_RESULT RPG_Input_BeginTextCapture(void *buffer, RPGsize sizeBuffer);
RPG_RESULT RPG_Input_EndTextCapture(RPGsize *written);

// Tilemap
RPG_RESULT RPG_Tilemap_Create(const void *buffer, RPGsize size, RPGviewport *viewport, RPGtilemap **tilemap);
RPG_RESULT RPG_Tilemap_CreateFromFile(const char *path, RPGviewport *viewport, RPGtilemap **tilemap);
RPG_RESULT RPG_Tilemap_Free(RPGtilemap *tilemap);
RPG_RESULT RPG_Tilemap_GetSize(RPGtilemap *tilemap, RPGint *width, RPGint *height);
RPG_RESULT RPG_Tilemap_GetTileSize(RPGtilemap *tilemap, RPGint *width, RPGint *height);
RPG_RESULT RPG_Tilemap_Update(RPGtilemap *tilemap);
RPG_RESULT RPG_Tilemap_GetOrigin(RPGtilemap *tilemap, RPGint *x, RPGint *y);
RPG_RESULT RPG_Tilemap_SetOrigin(RPGtilemap *tilemap, RPGint x, RPGint y);

/**
 * @brief Pointer to the game whose context is current.
 */
extern RPGgame *RPG_GAME;

#ifndef RPG_WITHOUT_OPENAL
/** The maximum number of slots that can contain an active sound */
#define RPG_MAX_CHANNELS 32
#else
#define RPG_MAX_CHANNELS 0
#endif

#ifdef __cplusplus
}
#endif

#endif /* OPEN_RPG_H */