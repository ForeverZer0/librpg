#ifndef OPEN_RPG_H
#define OPEN_RPG_H 1

#include <stdlib.h>
#include <stdint.h>

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
    type *var = RPG_ALLOC(type); \
    memset(var, 0, sizeof(type))

#define RPG_NONE 0
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
typedef size_t   RPGsize;

typedef struct RPG_IMAGE    RPG_IMAGE;

typedef enum {
    RPG_NO_ERROR = 0,
    RPG_ERR_SYSTEM,
    RPG_ERR_FORMAT,
    RPG_ERR_FILE_NOT_FOUND,
    RPG_ERR_AUDIO_DEVICE,
    RPG_ERR_AUDIO_CONTEXT,
    RPG_ERR_AUDIO_CANNOT_SEEK,
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

void RPG_Initialize(void);
void RPG_Terminate(void);
const char *RPG_ErrorString(RPG_RESULT result);



/**
 * @defgroup RPG_AUDIO Audio 
 * 
 * @{
 */

/**
 * @defgroup RPG_AUDIO_MACROS Audio Macros
 * 
 * @{
 */

/** The maximum number of slots that can contain an active sound */
#define RPG_AUDIO_MAX_SLOTS 32

/** @} */

typedef enum {
    RPG_SOUND_FORMAT_NONE         = 0x000000,     /* Undefined/invalid */
    RPG_SOUND_FORMAT_WAV          = 0x010000,     /* Microsoft WAV format (little endian). */
    RPG_SOUND_FORMAT_AIFF         = 0x020000,     /* Apple/SGI AIFF format (big endian). */
    RPG_SOUND_FORMAT_AU           = 0x030000,     /* Sun/NeXT AU format (big endian). */
    RPG_SOUND_FORMAT_RAW          = 0x040000,     /* RAW PCM data. */
    RPG_SOUND_FORMAT_PAF          = 0x050000,     /* Ensoniq PARIS file format. */
    RPG_SOUND_FORMAT_SVX          = 0x060000,     /* Amiga IFF / SVX8 / SV16 format. */
    RPG_SOUND_FORMAT_NIST         = 0x070000,     /* Sphere NIST format. */
    RPG_SOUND_FORMAT_VOC          = 0x080000,     /* VOC files. */
    RPG_SOUND_FORMAT_IRCAM        = 0x0A0000,     /* Berkeley/IRCAM/CARL */
    RPG_SOUND_FORMAT_W64          = 0x0B0000,     /* Sonic Foundry's 64 bit RIFF/WAV */
    RPG_SOUND_FORMAT_MAT4         = 0x0C0000,     /* Matlab (tm) V4.2 / GNU Octave 2.0 */
    RPG_SOUND_FORMAT_MAT5         = 0x0D0000,     /* Matlab (tm) V5.0 / GNU Octave 2.1 */
    RPG_SOUND_FORMAT_PVF          = 0x0E0000,     /* Portable Voice Format */
    RPG_SOUND_FORMAT_XI           = 0x0F0000,     /* Fasttracker 2 Extended Instrument */
    RPG_SOUND_FORMAT_HTK          = 0x100000,     /* HMM Tool Kit format */
    RPG_SOUND_FORMAT_SDS          = 0x110000,     /* Midi Sample Dump Standard */
    RPG_SOUND_FORMAT_AVR          = 0x120000,     /* Audio Visual Research */
    RPG_SOUND_FORMAT_WAVEX        = 0x130000,     /* MS WAVE with WAVEFORMATEX */
    RPG_SOUND_FORMAT_SD2          = 0x160000,     /* Sound Designer 2 */
    RPG_SOUND_FORMAT_FLAC         = 0x170000,     /* FLAC lossless file format */
    RPG_SOUND_FORMAT_CAF          = 0x180000,     /* Core Audio File format */
    RPG_SOUND_FORMAT_WVE          = 0x190000,     /* Psion WVE format */
    RPG_SOUND_FORMAT_OGG          = 0x200000,     /* Xiph OGG container */
    RPG_SOUND_FORMAT_MPC2K        = 0x210000,     /* Akai MPC 2000 sampler */
    RPG_SOUND_FORMAT_RF64         = 0x220000,     /* RF64 WAV file */
} RPG_SOUND_FORMAT;

typedef enum {
    RPG_SOUND_TYPE_NONE           = 0x0000,       /* Undefined/invalid */
    RPG_SOUND_TYPE_PCM_S8         = 0x0001,       /* Signed 8 bit data */
    RPG_SOUND_TYPE_PCM_16         = 0x0002,       /* Signed 16 bit data */
    RPG_SOUND_TYPE_PCM_24         = 0x0003,       /* Signed 24 bit data */
    RPG_SOUND_TYPE_PCM_32         = 0x0004,       /* Signed 32 bit data */
    RPG_SOUND_TYPE_PCM_U8         = 0x0005,       /* Unsigned 8 bit data (WAV and RAW only) */
    RPG_SOUND_TYPE_FLOAT          = 0x0006,       /* 32 bit float data */
    RPG_SOUND_TYPE_DOUBLE         = 0x0007,       /* 64 bit float data */
    RPG_SOUND_TYPE_ULAW           = 0x0010,       /* U-Law encoded. */
    RPG_SOUND_TYPE_ALAW           = 0x0011,       /* A-Law encoded. */
    RPG_SOUND_TYPE_IMA_ADPCM      = 0x0012,       /* IMA ADPCM. */
    RPG_SOUND_TYPE_MS_ADPCM       = 0x0013,       /* Microsoft ADPCM. */
    RPG_SOUND_TYPE_GSM610         = 0x0020,       /* GSM 6.10 encoding. */
    RPG_SOUND_TYPE_VOX_ADPCM      = 0x0021,       /* Oki Dialogic ADPCM encoding. */
    RPG_SOUND_TYPE_G721_32        = 0x0030,       /* 32kbs G721 ADPCM encoding. */
    RPG_SOUND_TYPE_G723_24        = 0x0031,       /* 24kbs G723 ADPCM encoding. */
    RPG_SOUND_TYPE_G723_40        = 0x0032,       /* 40kbs G723 ADPCM encoding. */
    RPG_SOUND_TYPE_DWVW_12        = 0x0040,       /* 12 bit Delta Width Variable Word encoding. */
    RPG_SOUND_TYPE_DWVW_16        = 0x0041,       /* 16 bit Delta Width Variable Word encoding. */
    RPG_SOUND_TYPE_DWVW_24        = 0x0042,       /* 24 bit Delta Width Variable Word encoding. */
    RPG_SOUND_TYPE_DWVW_N         = 0x0043,       /* N bit Delta Width Variable Word encoding. */
    RPG_SOUND_TYPE_DPCM_8         = 0x0050,       /* 8 bit differential PCM (XI only) */
    RPG_SOUND_TYPE_DPCM_16        = 0x0051,       /* 16 bit differential PCM (XI only) */
    RPG_SOUND_TYPE_VORBIS         = 0x0060,       /* Xiph Vorbis encoding. */
} RPG_SOUND_TYPE;

typedef enum {
    RPG_SOUND_INFO_TITLE          = 0x0001,         /* The title of the track */
    RPG_SOUND_INFO_COPYRIGHT      = 0x0002,         /* Copyright information */
    RPG_SOUND_INFO_SOFTWARE       = 0x0003,         /* Software/codec used for encoding */
    RPG_SOUND_INFO_ARTIST         = 0x0004,         /* Artist or creator of the file */
    RPG_SOUND_INFO_COMMENT        = 0x0005,         /* Additional comments */
    RPG_SOUND_INFO_DATE           = 0x0006,         /* Date/year */
    RPG_SOUND_INFO_ALBUM          = 0x0007,         /* Album for the track */
    RPG_SOUND_INFO_LICENSE        = 0x0008,         /* Licensing information */
    RPG_SOUND_INFO_TRACKNUMBER    = 0x0009,         /* The track number in a collection/album */
    RPG_SOUND_INFO_GENRE          = 0x000A,         /* The genre associated with the song */
    RPG_SOUND_INFO_FIRST = RPG_SOUND_INFO_TITLE,
    RPG_SOUND_INFO_LAST = RPG_SOUND_INFO_GENRE
} RPG_SOUND_INFO;

typedef enum {
    RPG_PLAYBACK_STATE_UNKNOWN     = 0x0000,         /* Unknown/invalid playback state */
    RPG_PLAYBACK_STATE_PLAYING     = 0x0001,         /* Currently playing */
    RPG_PLAYBACK_STATE_PAUSED      = 0x0002,         /* Suspended state, will resume from current position */
    RPG_PLAYBACK_STATE_STOPPED     = 0x0003,         /* Not playing */
} RPG_PLAYBACK_STATE;

typedef enum {
    RPG_AUDIO_SEEK_MS               = 0x0000,         /* Seek the stream in millisecond units */
    RPG_AUDIO_SEEK_SAMPLES          = 0x0001          /* Seek the stream in sample units */
} RPG_AUDIO_SEEK;


typedef void(*RPG_Audio_StateChangeFunc)(RPGint slot); // TODO:


RPG_RESULT RPG_Audio_Initialize(void);
RPG_RESULT RPG_Audio_Terminate(void);
RPG_RESULT RPG_Audio_Play(RPGint slot, const char *filename, RPGfloat volume, RPGfloat pitch, RPGint loopCount);
RPG_RESULT RPG_Audio_GetVolume(RPGint slot, RPGfloat *volume);
RPG_RESULT RPG_Audio_SetVolume(RPGint slot, RPGfloat volume);
RPG_RESULT RPG_Audio_GetPitch(RPGint slot, RPGfloat *pitch);
RPG_RESULT RPG_Audio_SetPitch(RPGint slot, RPGfloat pitch);
RPG_RESULT RPG_Audio_GetSampleRate(RPGint slot, RPGint *rate);
RPG_RESULT RPG_Audio_GetChannelCount(RPGint slot, RPGint *count);
RPG_RESULT RPG_Audio_GetSampleCount(RPGint slot, RPGint *count);
RPG_RESULT RPG_Audio_GetSectionCount(RPGint slot, RPGint *count);
RPG_RESULT RPG_Audio_GetFormat(RPGint slot, RPG_SOUND_FORMAT *format);
RPG_RESULT RPG_Audio_GetType(RPGint slot, RPG_SOUND_TYPE *format);
RPG_RESULT RPG_Audio_GetDuration(RPGint slot, RPGint64 *milliseconds);
RPG_RESULT RPG_Audio_GetInfo(RPGint slot, RPG_SOUND_INFO type, char *buffer, size_t bufferSize, size_t *written);
RPG_RESULT RPG_Audio_GetPlaybackState(RPGint slot, RPG_PLAYBACK_STATE *state);
RPG_RESULT RPG_Audio_GetLoopCount(RPGint slot, RPGint *count);
RPG_RESULT RPG_Audio_SetLoopCount(RPGint slot, RPGint count);
RPG_RESULT RPG_Audio_Resume(RPGint slot);
RPG_RESULT RPG_Audio_Stop(RPGint slot);
RPG_RESULT RPG_Audio_Pause(RPGint slot);
RPG_RESULT RPG_Audio_Seek(RPGint slot, RPGint64 position, RPG_AUDIO_SEEK seek);

typedef enum {
    RPG_AUDIO_FX_TYPE_REVERB
} RPG_AUDIOFX_TYPE;

typedef struct RPG_AUDIO_FX RPG_AUDIO_FX;
typedef struct RPG_AUDIO_FX RPG_REVERB;


RPG_RESULT RPG_Audio_CreateEffect();
RPG_RESULT RPG_Audio_AttachEffect(RPGint slot);
RPG_RESULT RPG_Audio_DetachEffect(RPGint slot, RPG_AUDIO_FX *fx);


/** @} */



#endif /* OPEN_RPG_H */
