
#ifndef OPEN_RPG_AUDIO_H
#define OPEN_RPG_AUDIO_H 1

#include "rpg.h"

#define RPG_MAX_AUDIO_CHANNELS 32

typedef enum {
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
    RPG_SOUND_INFO_TITLE          = 0x0001,
    RPG_SOUND_INFO_COPYRIGHT      = 0x0002,
    RPG_SOUND_INFO_SOFTWARE       = 0x0003,
    RPG_SOUND_INFO_ARTIST         = 0x0004,
    RPG_SOUND_INFO_COMMENT        = 0x0005,
    RPG_SOUND_INFO_DATE           = 0x0006,
    RPG_SOUND_INFO_ALBUM          = 0x0007,
    RPG_SOUND_INFO_LICENSE        = 0x0008,
    RPG_SOUND_INFO_TRACKNUMBER    = 0x0009,
    RPG_SOUND_INFO_GENRE          = 0x000A,
    RPG_SOUND_INFO_FIRST = RPG_SOUND_INFO_TITLE,
    RPG_SOUND_INFO_LAST = RPG_SOUND_INFO_GENRE
} RPG_SOUND_INFO;

typedef enum {
    RPG_PLAYBACK_STATE_UNKNOWN,
    RPG_PLAYBACK_STATE_PLAYING,
    RPG_PLAYBACK_STATE_PAUSED,
    RPG_PLAYBACK_STATE_STOPPED
} RPG_PLAYBACK_STATE;

/** Channel */
RPG_RESULT RPG_Channel_Create(RPG_SOUND *sound, RPGint fxCount, RPG_AUDIOFX **fx, RPG_CHANNEL **channel);
RPG_RESULT RPG_Channel_Free(RPG_CHANNEL *channel);
RPG_RESULT RPG_Channel_Play(RPG_CHANNEL *channel);
RPG_RESULT RPG_Channel_AddEffect(RPG_CHANNEL *channel, RPG_AUDIOFX *fx);
RPG_RESULT RPG_Channel_AddEffects(RPG_CHANNEL *channel, RPGint count, RPG_AUDIOFX **fx);
RPG_RESULT RPG_Channel_GetVolume(RPG_CHANNEL *channel, RPGfloat *volume);
RPG_RESULT RPG_Channel_SetVolume(RPG_CHANNEL *channel, RPGfloat volume);
RPG_RESULT RPG_Channel_GetPitch(RPG_CHANNEL *channel, RPGfloat *pitch);
RPG_RESULT RPG_Channel_SetPitch(RPG_CHANNEL *channel, RPGfloat pitch);
RPG_RESULT RPG_Channel_GetLoopCount(RPG_CHANNEL *channel, RPGint *count);
RPG_RESULT RPG_Channel_SetLoopCount(RPG_CHANNEL *channel, RPGint count);

/** Sound */
RPG_RESULT RPG_Sound_Create(const char *path, RPG_SOUND **sound);
RPG_RESULT RPG_Sound_Free(RPG_SOUND *sound);
RPG_RESULT RPG_Sound_GetSampleRate(RPG_SOUND *sound, RPGint *rate);
RPG_RESULT RPG_Sound_GetChannelCount(RPG_SOUND *sound, RPGint *count);
RPG_RESULT RPG_Sound_GetFormat(RPG_SOUND *sound, RPG_SOUND_FORMAT *format);
RPG_RESULT RPG_Sound_GetType(RPG_SOUND *sound, RPG_SOUND_TYPE *type);
RPG_RESULT RPG_Sound_GetSampleCount(RPG_SOUND *sound, RPGint *count);
RPG_RESULT RPG_Sound_GetSectionCount(RPG_SOUND *sound, RPGint *count);
RPG_RESULT RPG_Sound_GetDuration(RPG_SOUND *sound, int64_t *milliseconds);
RPG_RESULT RPG_Sound_GetInfo(RPG_SOUND *sound, RPG_SOUND_INFO type, char *buffer, size_t bufferSize, size_t *size);

/** Audio */
RPG_RESULT RPG_Audio_Initialize(void);
RPG_RESULT RPG_Audio_Terminate(void);
RPG_RESULT RPG_Audio_PlaySound(RPG_SOUND *sound, RPGfloat gain, RPGfloat pitch, RPGint loop, RPG_CHANNEL **channel);

#endif /* OPEN_RPG_AUDIO_H */
