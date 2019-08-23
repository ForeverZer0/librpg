#ifndef OPEN_RPG_H
#define OPEN_RPG_H 1

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

#define RPG_ALLOC(type) ((type *)RPG_MALLOC(sizeof(type)))
#define RPG_ALLOC_N(type, n) ((type *)RPG_MALLOC(sizeof(type) * n))
#define RPG_ALLOC_ZERO(var, type)                                                                                                          \
    type *var = RPG_ALLOC(type);                                                                                                           \
    memset(var, 0, sizeof(type))

#define RPG_NONE 0
#define RPG_TRUE 1
#define RPG_FALSE 0

typedef int32_t RPGbool;
typedef int8_t RPGbyte;
typedef int16_t RPGshort;
typedef int32_t RPGint;
typedef int64_t RPGint64;
typedef uint8_t RPGubyte;
typedef uint16_t RPGushort;
typedef uint32_t RPGuint;
typedef uint64_t RPGuint64;
typedef float RPGfloat;
typedef double RPGdouble;
typedef size_t RPGsize;

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
    RPG_ERR_NULL_POINTER,
    RPG_ERR_INVALID_POINTER,
    RPG_ERR_OUT_OF_RANGE,
    RPG_ERR_INVALID_VALUE,
    RPG_ERR_THREAD_FAILURE,
    RPG_ERR_MEMORY,
    RPG_ERR_UNKNOWN
} RPG_RESULT;

typedef struct {
    RPGfloat x, y;  // FIXME:
} RPGvec2;

typedef struct {
    RPGfloat x, y, z;
} RPGvec3;

typedef struct {
    RPGfloat x, y, z, w;
} RPGvec4;

typedef struct RPGgame RPGgame;
typedef struct RPGimage RPGimage;

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

RPG_RESULT RPG_Game_Main(RPGgame *game);
RPG_RESULT RPG_Game_Create(RPGgame **game);
RPG_RESULT RPG_Game_CreateWindow(RPGgame *game, const char *title, RPGint width, RPGint height, RPG_INIT_FLAGS flags);
RPG_RESULT RPG_Game_Destroy(RPGgame *game);
const char *RPG_GetErrorString(RPG_RESULT result);

#endif /* OPEN_RPG_H */