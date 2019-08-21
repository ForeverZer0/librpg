#ifndef OPEN_RPG_H
#define OPEN_RPG_H 1

#include <stdlib.h>
#include "glad.h"
#include "GLFW/glfw3.h"

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

#define RPG_ALLOC(type) ((type*)RPG_MALLOC(sizeof(type)))
#define RPG_ALLOC_N(type,n) ((type*)RPG_MALLOC(sizeof(type) * n))
#define RPG_ALLOC_ZERO(var,type) \
    type *var = RPG_ALLOC(type) \
    memset(n, 0, sizeof(type))

#if defined(__linux__)
#define RPG_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define RPG_APPLE
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define RPG_WINDOWS
#endif

#define RPG_TRUE 1
#define RPG_FALSE 0

typedef int8_t   RPGbool;
typedef int8_t   RPGbyte;
typedef int16_t  RPGshort;
typedef int32_t  RPGint;
typedef int64_t  RPGint64;
typedef uint8_t  RPGubyte;
typedef uint16_t RPGushort;
typedef uint32_t RPGuint;
typedef uint64_t RPGuint64;
typedef float    RPGfloat;
typedef double   RPGdouble;

typedef struct RPG_IMAGE    RPG_IMAGE;
typedef struct RPG_CHANNEL  RPG_CHANNEL;
typedef struct RPG_SOUND    RPG_SOUND;
typedef struct RPG_AUDIOFX  RPG_AUDIOFX;

typedef enum {
    RPG_NO_ERROR = 0,
    RPG_ERR_SYSTEM,
    RPG_ERR_FORMAT,
    RPG_ERR_FILE_NOT_FOUND,
    RPG_ERR_AUDIO_DEVICE,
    RPG_ERR_AUDIO_CONTEXT,
    RPG_ERR_AUDIO_CANNOT_SEEK,
    RPG_ERR_AUDIO_NOT_ENOUGH_CHANNELS,
    RPG_ERR_ENCODING,
    RPG_ERR_MALFORMED,
    RPG_ERR_NULL_POINTER,
    RPG_ERR_INVALID_POINTER,
    RPG_ERR_OUT_OF_RANGE,
    RPG_ERR_INVALID_VALUE,
    RPG_ERR_THREAD_FAILURE,
    RPG_ERR_UNKNOWN
} RPG_RESULT;



void RPG_Initialize(void);
void RPG_Terminate(void);
const char *RPG_ErrorString(RPG_RESULT result);


#endif /* OPEN_RPG_H */
