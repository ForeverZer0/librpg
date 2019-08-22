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

typedef int8_t RPGbool;
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

typedef struct RPGgame RPGgame;

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

RPG_RESULT RPG_Game_Main(RPGgame *gfx);

RPG_RESULT RPG_Game_Create(RPGgame **game);
RPG_RESULT RPG_Game_CreateWindow(RPGgame *game, const char *title, RPGint width, RPGint height, RPG_INIT_FLAGS flags);
RPG_RESULT RPG_Game_Destroy(RPGgame *game);
const char *RPG_GetErrorString(RPG_RESULT result);

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
    RPG_SOUND_FORMAT_NONE  = 0x000000, /* Undefined/invalid */
    RPG_SOUND_FORMAT_WAV   = 0x010000, /* Microsoft WAV format (little endian). */
    RPG_SOUND_FORMAT_AIFF  = 0x020000, /* Apple/SGI AIFF format (big endian). */
    RPG_SOUND_FORMAT_AU    = 0x030000, /* Sun/NeXT AU format (big endian). */
    RPG_SOUND_FORMAT_RAW   = 0x040000, /* RAW PCM data. */
    RPG_SOUND_FORMAT_PAF   = 0x050000, /* Ensoniq PARIS file format. */
    RPG_SOUND_FORMAT_SVX   = 0x060000, /* Amiga IFF / SVX8 / SV16 format. */
    RPG_SOUND_FORMAT_NIST  = 0x070000, /* Sphere NIST format. */
    RPG_SOUND_FORMAT_VOC   = 0x080000, /* VOC files. */
    RPG_SOUND_FORMAT_IRCAM = 0x0A0000, /* Berkeley/IRCAM/CARL */
    RPG_SOUND_FORMAT_W64   = 0x0B0000, /* Sonic Foundry's 64 bit RIFF/WAV */
    RPG_SOUND_FORMAT_MAT4  = 0x0C0000, /* Matlab (tm) V4.2 / GNU Octave 2.0 */
    RPG_SOUND_FORMAT_MAT5  = 0x0D0000, /* Matlab (tm) V5.0 / GNU Octave 2.1 */
    RPG_SOUND_FORMAT_PVF   = 0x0E0000, /* Portable Voice Format */
    RPG_SOUND_FORMAT_XI    = 0x0F0000, /* Fasttracker 2 Extended Instrument */
    RPG_SOUND_FORMAT_HTK   = 0x100000, /* HMM Tool Kit format */
    RPG_SOUND_FORMAT_SDS   = 0x110000, /* Midi Sample Dump Standard */
    RPG_SOUND_FORMAT_AVR   = 0x120000, /* Audio Visual Research */
    RPG_SOUND_FORMAT_WAVEX = 0x130000, /* MS WAVE with WAVEFORMATEX */
    RPG_SOUND_FORMAT_SD2   = 0x160000, /* Sound Designer 2 */
    RPG_SOUND_FORMAT_FLAC  = 0x170000, /* FLAC lossless file format */
    RPG_SOUND_FORMAT_CAF   = 0x180000, /* Core Audio File format */
    RPG_SOUND_FORMAT_WVE   = 0x190000, /* Psion WVE format */
    RPG_SOUND_FORMAT_OGG   = 0x200000, /* Xiph OGG container */
    RPG_SOUND_FORMAT_MPC2K = 0x210000, /* Akai MPC 2000 sampler */
    RPG_SOUND_FORMAT_RF64  = 0x220000, /* RF64 WAV file */
} RPG_SOUND_FORMAT;

typedef enum {
    RPG_SOUND_TYPE_NONE      = 0x0000, /* Undefined/invalid */
    RPG_SOUND_TYPE_PCM_S8    = 0x0001, /* Signed 8 bit data */
    RPG_SOUND_TYPE_PCM_16    = 0x0002, /* Signed 16 bit data */
    RPG_SOUND_TYPE_PCM_24    = 0x0003, /* Signed 24 bit data */
    RPG_SOUND_TYPE_PCM_32    = 0x0004, /* Signed 32 bit data */
    RPG_SOUND_TYPE_PCM_U8    = 0x0005, /* Unsigned 8 bit data (WAV and RAW only) */
    RPG_SOUND_TYPE_FLOAT     = 0x0006, /* 32 bit float data */
    RPG_SOUND_TYPE_DOUBLE    = 0x0007, /* 64 bit float data */
    RPG_SOUND_TYPE_ULAW      = 0x0010, /* U-Law encoded. */
    RPG_SOUND_TYPE_ALAW      = 0x0011, /* A-Law encoded. */
    RPG_SOUND_TYPE_IMA_ADPCM = 0x0012, /* IMA ADPCM. */
    RPG_SOUND_TYPE_MS_ADPCM  = 0x0013, /* Microsoft ADPCM. */
    RPG_SOUND_TYPE_GSM610    = 0x0020, /* GSM 6.10 encoding. */
    RPG_SOUND_TYPE_VOX_ADPCM = 0x0021, /* Oki Dialogic ADPCM encoding. */
    RPG_SOUND_TYPE_G721_32   = 0x0030, /* 32kbs G721 ADPCM encoding. */
    RPG_SOUND_TYPE_G723_24   = 0x0031, /* 24kbs G723 ADPCM encoding. */
    RPG_SOUND_TYPE_G723_40   = 0x0032, /* 40kbs G723 ADPCM encoding. */
    RPG_SOUND_TYPE_DWVW_12   = 0x0040, /* 12 bit Delta Width Variable Word encoding. */
    RPG_SOUND_TYPE_DWVW_16   = 0x0041, /* 16 bit Delta Width Variable Word encoding. */
    RPG_SOUND_TYPE_DWVW_24   = 0x0042, /* 24 bit Delta Width Variable Word encoding. */
    RPG_SOUND_TYPE_DWVW_N    = 0x0043, /* N bit Delta Width Variable Word encoding. */
    RPG_SOUND_TYPE_DPCM_8    = 0x0050, /* 8 bit differential PCM (XI only) */
    RPG_SOUND_TYPE_DPCM_16   = 0x0051, /* 16 bit differential PCM (XI only) */
    RPG_SOUND_TYPE_VORBIS    = 0x0060, /* Xiph Vorbis encoding. */
} RPG_SOUND_TYPE;

typedef enum {
    RPG_SOUND_INFO_TITLE       = 0x0001, /* The title of the track */
    RPG_SOUND_INFO_COPYRIGHT   = 0x0002, /* Copyright information */
    RPG_SOUND_INFO_SOFTWARE    = 0x0003, /* Software/codec used for encoding */
    RPG_SOUND_INFO_ARTIST      = 0x0004, /* Artist or creator of the file */
    RPG_SOUND_INFO_COMMENT     = 0x0005, /* Additional comments */
    RPG_SOUND_INFO_DATE        = 0x0006, /* Date/year */
    RPG_SOUND_INFO_ALBUM       = 0x0007, /* Album for the track */
    RPG_SOUND_INFO_LICENSE     = 0x0008, /* Licensing information */
    RPG_SOUND_INFO_TRACKNUMBER = 0x0009, /* The track number in a collection/album */
    RPG_SOUND_INFO_GENRE       = 0x000A, /* The genre associated with the song */
    RPG_SOUND_INFO_FIRST       = RPG_SOUND_INFO_TITLE,
    RPG_SOUND_INFO_LAST        = RPG_SOUND_INFO_GENRE
} RPG_SOUND_INFO;

typedef enum {
    RPG_PLAYBACK_STATE_UNKNOWN = 0x0000, /* Unknown/invalid playback state */
    RPG_PLAYBACK_STATE_PLAYING = 0x0001, /* Currently playing */
    RPG_PLAYBACK_STATE_PAUSED  = 0x0002, /* Suspended state, will resume from current position */
    RPG_PLAYBACK_STATE_STOPPED = 0x0003, /* Not playing */
} RPG_PLAYBACK_STATE;

typedef enum {
    RPG_AUDIO_SEEK_MS      = 0x0000, /* Seek the stream in millisecond units */
    RPG_AUDIO_SEEK_SAMPLES = 0x0001  /* Seek the stream in sample units */
} RPG_AUDIO_SEEK;

typedef void (*RPGaudiofunc)(RPGint channel); // TODO:

RPG_RESULT RPG_Audio_Play(RPGint channel, const char *filename, RPGfloat volume, RPGfloat pitch, RPGint loopCount);
RPG_RESULT RPG_Audio_FreeChannel(RPGint index);
RPG_RESULT RPG_Audio_GetVolume(RPGint channel, RPGfloat *volume);
RPG_RESULT RPG_Audio_SetVolume(RPGint channel, RPGfloat volume);
RPG_RESULT RPG_Audio_GetPitch(RPGint channel, RPGfloat *pitch);
RPG_RESULT RPG_Audio_SetPitch(RPGint channel, RPGfloat pitch);
RPG_RESULT RPG_Audio_GetSampleRate(RPGint channel, RPGint *rate);
RPG_RESULT RPG_Audio_GetChannelCount(RPGint channel, RPGint *count);
RPG_RESULT RPG_Audio_GetSampleCount(RPGint channel, RPGint *count);
RPG_RESULT RPG_Audio_GetSectionCount(RPGint channel, RPGint *count);
RPG_RESULT RPG_Audio_GetFormat(RPGint channel, RPG_SOUND_FORMAT *format);
RPG_RESULT RPG_Audio_GetType(RPGint channel, RPG_SOUND_TYPE *format);
RPG_RESULT RPG_Audio_GetDuration(RPGint channel, RPGint64 *milliseconds);
RPG_RESULT RPG_Audio_GetInfo(RPGint channel, RPG_SOUND_INFO type, char *buffer, size_t bufferSize, size_t *written);
RPG_RESULT RPG_Audio_GetPlaybackState(RPGint channel, RPG_PLAYBACK_STATE *state);
RPG_RESULT RPG_Audio_GetLoopCount(RPGint channel, RPGint *count);
RPG_RESULT RPG_Audio_SetLoopCount(RPGint channel, RPGint count);
RPG_RESULT RPG_Audio_Resume(RPGint channel);
RPG_RESULT RPG_Audio_Stop(RPGint channel);
RPG_RESULT RPG_Audio_Pause(RPGint channel);
RPG_RESULT RPG_Audio_Seek(RPGint channel, RPGint64 position, RPG_AUDIO_SEEK seek);

typedef enum {
    RPG_AUDIOFX_TYPE_NULL              = 0x0000,
    RPG_AUDIOFX_TYPE_REVERB            = 0x0001,
    RPG_AUDIOFX_TYPE_CHORUS            = 0x0002,
    RPG_AUDIOFX_TYPE_DISTORTION        = 0x0003,
    RPG_AUDIOFX_TYPE_ECHO              = 0x0004,
    RPG_AUDIOFX_TYPE_FLANGER           = 0x0005,
    RPG_AUDIOFX_TYPE_FREQUENCY_SHIFTER = 0x0006,
    RPG_AUDIOFX_TYPE_VOCAL_MORPHER     = 0x0007,
    RPG_AUDIOFX_TYPE_PITCH_SHIFTER     = 0x0008,
    RPG_AUDIOFX_TYPE_RING_MODULATOR    = 0x0009,
    RPG_AUDIOFX_TYPE_AUTOWAH           = 0x000A,
    RPG_AUDIOFX_TYPE_COMPRESSOR        = 0x000B,
    RPG_AUDIOFX_TYPE_EQUALIZER         = 0x000C,
    RPG_AUDIOFX_TYPE_REVERBHQ          = 0x8000
} RPG_AUDIOFX_TYPE;

typedef enum {
    RPG_REVERB_PRESET_GENERIC,
    RPG_REVERB_PRESET_PADDEDCELL,
    RPG_REVERB_PRESET_ROOM,
    RPG_REVERB_PRESET_BATHROOM,
    RPG_REVERB_PRESET_LIVINGROOM,
    RPG_REVERB_PRESET_STONEROOM,
    RPG_REVERB_PRESET_AUDITORIUM,
    RPG_REVERB_PRESET_CONCERTHALL,
    RPG_REVERB_PRESET_CAVE,
    RPG_REVERB_PRESET_ARENA,
    RPG_REVERB_PRESET_HANGAR,
    RPG_REVERB_PRESET_CARPETEDHALLWAY,
    RPG_REVERB_PRESET_HALLWAY,
    RPG_REVERB_PRESET_STONECORRIDOR,
    RPG_REVERB_PRESET_ALLEY,
    RPG_REVERB_PRESET_FOREST,
    RPG_REVERB_PRESET_CITY,
    RPG_REVERB_PRESET_MOUNTAINS,
    RPG_REVERB_PRESET_QUARRY,
    RPG_REVERB_PRESET_PLAIN,
    RPG_REVERB_PRESET_PARKINGLOT,
    RPG_REVERB_PRESET_SEWERPIPE,
    RPG_REVERB_PRESET_UNDERWATER,
    RPG_REVERB_PRESET_DRUGGED,
    RPG_REVERB_PRESET_DIZZY,
    RPG_REVERB_PRESET_PSYCHOTIC,

    /* Castle Presets */
    RPG_REVERB_PRESET_CASTLE_SMALLROOM,
    RPG_REVERB_PRESET_CASTLE_SHORTPASSAGE,
    RPG_REVERB_PRESET_CASTLE_MEDIUMROOM,
    RPG_REVERB_PRESET_CASTLE_LARGEROOM,
    RPG_REVERB_PRESET_CASTLE_LONGPASSAGE,
    RPG_REVERB_PRESET_CASTLE_HALL,
    RPG_REVERB_PRESET_CASTLE_CUPBOARD,
    RPG_REVERB_PRESET_CASTLE_COURTYARD,
    RPG_REVERB_PRESET_CASTLE_ALCOVE,

    /* Factory Presets */
    RPG_REVERB_PRESET_FACTORY_SMALLROOM,
    RPG_REVERB_PRESET_FACTORY_SHORTPASSAGE,
    RPG_REVERB_PRESET_FACTORY_MEDIUMROOM,
    RPG_REVERB_PRESET_FACTORY_LARGEROOM,
    RPG_REVERB_PRESET_FACTORY_LONGPASSAGE,
    RPG_REVERB_PRESET_FACTORY_HALL,
    RPG_REVERB_PRESET_FACTORY_CUPBOARD,
    RPG_REVERB_PRESET_FACTORY_COURTYARD,
    RPG_REVERB_PRESET_FACTORY_ALCOVE,
    /* Ice Palace Presets */
    RPG_REVERB_PRESET_ICEPALACE_SMALLROOM,
    RPG_REVERB_PRESET_ICEPALACE_SHORTPASSAGE,
    RPG_REVERB_PRESET_ICEPALACE_MEDIUMROOM,
    RPG_REVERB_PRESET_ICEPALACE_LARGEROOM,
    RPG_REVERB_PRESET_ICEPALACE_LONGPASSAGE,
    RPG_REVERB_PRESET_ICEPALACE_HALL,
    RPG_REVERB_PRESET_ICEPALACE_CUPBOARD,
    RPG_REVERB_PRESET_ICEPALACE_COURTYARD,
    RPG_REVERB_PRESET_ICEPALACE_ALCOVE,
    /* Space Station Presets */
    RPG_REVERB_PRESET_SPACESTATION_SMALLROOM,
    RPG_REVERB_PRESET_SPACESTATION_SHORTPASSAGE,
    RPG_REVERB_PRESET_SPACESTATION_MEDIUMROOM,
    RPG_REVERB_PRESET_SPACESTATION_LARGEROOM,
    RPG_REVERB_PRESET_SPACESTATION_LONGPASSAGE,
    RPG_REVERB_PRESET_SPACESTATION_HALL,
    RPG_REVERB_PRESET_SPACESTATION_CUPBOARD,
    RPG_REVERB_PRESET_SPACESTATION_ALCOVE,
    /* Wooden Galleon Presets */
    RPG_REVERB_PRESET_WOODEN_SMALLROOM,
    RPG_REVERB_PRESET_WOODEN_SHORTPASSAGE,
    RPG_REVERB_PRESET_WOODEN_MEDIUMROOM,
    RPG_REVERB_PRESET_WOODEN_LARGEROOM,
    RPG_REVERB_PRESET_WOODEN_LONGPASSAGE,
    RPG_REVERB_PRESET_WOODEN_HALL,
    RPG_REVERB_PRESET_WOODEN_CUPBOARD,
    RPG_REVERB_PRESET_WOODEN_COURTYARD,
    RPG_REVERB_PRESET_WOODEN_ALCOVE,
    /* Sports Presets */
    RPG_REVERB_PRESET_SPORT_EMPTYSTADIUM,
    RPG_REVERB_PRESET_SPORT_SQUASHCOURT,
    RPG_REVERB_PRESET_SPORT_SMALLSWIMMINGPOOL,
    RPG_REVERB_PRESET_SPORT_LARGESWIMMINGPOOL,
    RPG_REVERB_PRESET_SPORT_GYMNASIUM,
    RPG_REVERB_PRESET_SPORT_FULLSTADIUM,
    RPG_REVERB_PRESET_SPORT_STADIUMTANNOY,
    /* Prefab Presets */
    RPG_REVERB_PRESET_PREFAB_WORKSHOP,
    RPG_REVERB_PRESET_PREFAB_SCHOOLROOM,
    RPG_REVERB_PRESET_PREFAB_PRACTISEROOM,
    RPG_REVERB_PRESET_PREFAB_OUTHOUSE,
    RPG_REVERB_PRESET_PREFAB_CARAVAN,
    /* Dome and Pipe Presets */
    RPG_REVERB_PRESET_DOME_TOMB,
    RPG_REVERB_PRESET_PIPE_SMALL,
    RPG_REVERB_PRESET_DOME_SAINTPAULS,
    RPG_REVERB_PRESET_PIPE_LONGTHIN,
    RPG_REVERB_PRESET_PIPE_LARGE,
    RPG_REVERB_PRESET_PIPE_RESONANT,
    /* Outdoors Presets */
    RPG_REVERB_PRESET_OUTDOORS_BACKYARD,
    RPG_REVERB_PRESET_OUTDOORS_ROLLINGPLAINS,
    RPG_REVERB_PRESET_OUTDOORS_DEEPCANYON,
    RPG_REVERB_PRESET_OUTDOORS_CREEK,
    RPG_REVERB_PRESET_OUTDOORS_VALLEY,
    /* Mood Presets */
    RPG_REVERB_PRESET_MOOD_HEAVEN,
    RPG_REVERB_PRESET_MOOD_HELL,
    RPG_REVERB_PRESET_MOOD_MEMORY,
    /* Driving Presets */
    RPG_REVERB_PRESET_DRIVING_COMMENTATOR,
    RPG_REVERB_PRESET_DRIVING_PITGARAGE,
    RPG_REVERB_PRESET_DRIVING_INCAR_RACER,
    RPG_REVERB_PRESET_DRIVING_INCAR_SPORTS,
    RPG_REVERB_PRESET_DRIVING_INCAR_LUXURY,
    RPG_REVERB_PRESET_DRIVING_FULLGRANDSTAND,
    RPG_REVERB_PRESET_DRIVING_EMPTYGRANDSTAND,
    RPG_REVERB_PRESET_DRIVING_TUNNEL,
    /* City Presets */
    RPG_REVERB_PRESET_CITY_STREETS,
    RPG_REVERB_PRESET_CITY_SUBWAY,
    RPG_REVERB_PRESET_CITY_MUSEUM,
    RPG_REVERB_PRESET_CITY_LIBRARY,
    RPG_REVERB_PRESET_CITY_UNDERPASS,
    RPG_REVERB_PRESET_CITY_ABANDONED,
    /* Misc. Presets */
    RPG_REVERB_PRESET_DUSTYROOM,
    RPG_REVERB_PRESET_CHAPEL,
    RPG_REVERB_PRESET_SMALLWATERROOM
} RPG_REVERB_PRESET;

typedef struct RPGaudiofx RPGaudiofx;
typedef struct RPGaudiofx RPGreverb;

RPG_RESULT RPG_Audio_CreateEffect(RPG_AUDIOFX_TYPE type, RPGaudiofx **fx);
RPG_RESULT RPG_Audio_CreateReverb(RPG_REVERB_PRESET preset, RPGreverb **reverb);
RPG_RESULT RPG_Audio_AttachEffect(RPGint channel, RPGaudiofx *fx);
RPG_RESULT RPG_Audio_DetachEffect(RPGint channel, RPGaudiofx *fx);

/** @} */

typedef struct RPGimage RPGimage;

#endif /* OPEN_RPG_H */