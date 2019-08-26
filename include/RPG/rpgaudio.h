#ifndef OPEN_RGP_AUDIO_H
#define OPEN_RGP_AUDIO_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include "rpg.h"

#ifndef RPG_NO_AUDIO

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

typedef void (*RPGaudiofunc)(RPGint channel); // TODO:

RPG_RESULT RPG_Audio_Initialize(RPGgame *game);
RPG_RESULT RPG_Audio_Terminate(void);
RPG_RESULT RPG_Audio_Play(RPGint channel, const char *filename, RPGfloat volume, RPGfloat pitch, RPGint loopCount);
RPG_RESULT RPG_Audio_SetPlaybackCompleteCallback(RPGaudiofunc func, RPGaudiofunc *previous);
RPG_RESULT RPG_Audio_FreeChannel(RPGint index); // TODO: Make private?
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
RPG_RESULT RPG_Audio_GetInfo(RPGint channel, RPG_SOUND_INFO type, char *buffer, size_t sizeBuffer, size_t *written);
RPG_RESULT RPG_Audio_GetPlaybackState(RPGint channel, RPG_PLAYBACK_STATE *state);
RPG_RESULT RPG_Audio_GetLoopCount(RPGint channel, RPGint *count);
RPG_RESULT RPG_Audio_SetLoopCount(RPGint channel, RPGint count);
RPG_RESULT RPG_Audio_Resume(RPGint channel);
RPG_RESULT RPG_Audio_Stop(RPGint channel);
RPG_RESULT RPG_Audio_Pause(RPGint channel);
RPG_RESULT RPG_Audio_GetPosition(RPGint channel, RPGint64 *ms);
RPG_RESULT RPG_Audio_Seek(RPGint channel, RPGint64 ms);

#if !defined(RPG_AUDIO_NO_EFFECTS)

typedef struct RPGaudiofx RPGaudiofx;

typedef enum {
    RPG_AUDIOFX_TYPE_NULL              = 0x0000,
    // RPG_AUDIOFX_TYPE_REVERB            = 0x0001, // TODO: Remove?
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
    RPG_AUDIOFX_TYPE_REVERB            = 0x8000
} RPG_AUDIOFX_TYPE;

typedef enum {
    // General
    RPG_REVERB_TYPE_GENERIC,
    RPG_REVERB_TYPE_PADDEDCELL,
    RPG_REVERB_TYPE_ROOM,
    RPG_REVERB_TYPE_BATHROOM,
    RPG_REVERB_TYPE_LIVINGROOM,
    RPG_REVERB_TYPE_STONEROOM,
    RPG_REVERB_TYPE_AUDITORIUM,
    RPG_REVERB_TYPE_CONCERTHALL,
    RPG_REVERB_TYPE_CAVE,
    RPG_REVERB_TYPE_ARENA,
    RPG_REVERB_TYPE_HANGAR,
    RPG_REVERB_TYPE_CARPETEDHALLWAY,
    RPG_REVERB_TYPE_HALLWAY,
    RPG_REVERB_TYPE_STONECORRIDOR,
    RPG_REVERB_TYPE_ALLEY,
    RPG_REVERB_TYPE_FOREST,
    RPG_REVERB_TYPE_CITY,
    RPG_REVERB_TYPE_MOUNTAINS,
    RPG_REVERB_TYPE_QUARRY,
    RPG_REVERB_TYPE_PLAIN,
    RPG_REVERB_TYPE_PARKINGLOT,
    RPG_REVERB_TYPE_SEWERPIPE,
    RPG_REVERB_TYPE_UNDERWATER,
    RPG_REVERB_TYPE_DRUGGED,
    RPG_REVERB_TYPE_DIZZY,
    RPG_REVERB_TYPE_PSYCHOTIC,

    /* Castle Presets */
    RPG_REVERB_TYPE_CASTLE_SMALLROOM,
    RPG_REVERB_TYPE_CASTLE_SHORTPASSAGE,
    RPG_REVERB_TYPE_CASTLE_MEDIUMROOM,
    RPG_REVERB_TYPE_CASTLE_LARGEROOM,
    RPG_REVERB_TYPE_CASTLE_LONGPASSAGE,
    RPG_REVERB_TYPE_CASTLE_HALL,
    RPG_REVERB_TYPE_CASTLE_CUPBOARD,
    RPG_REVERB_TYPE_CASTLE_COURTYARD,
    RPG_REVERB_TYPE_CASTLE_ALCOVE,

    /* Factory Presets */
    RPG_REVERB_TYPE_FACTORY_SMALLROOM,
    RPG_REVERB_TYPE_FACTORY_SHORTPASSAGE,
    RPG_REVERB_TYPE_FACTORY_MEDIUMROOM,
    RPG_REVERB_TYPE_FACTORY_LARGEROOM,
    RPG_REVERB_TYPE_FACTORY_LONGPASSAGE,
    RPG_REVERB_TYPE_FACTORY_HALL,
    RPG_REVERB_TYPE_FACTORY_CUPBOARD,
    RPG_REVERB_TYPE_FACTORY_COURTYARD,
    RPG_REVERB_TYPE_FACTORY_ALCOVE,

    /* Ice Palace Presets */
    RPG_REVERB_TYPE_ICEPALACE_SMALLROOM,
    RPG_REVERB_TYPE_ICEPALACE_SHORTPASSAGE,
    RPG_REVERB_TYPE_ICEPALACE_MEDIUMROOM,
    RPG_REVERB_TYPE_ICEPALACE_LARGEROOM,
    RPG_REVERB_TYPE_ICEPALACE_LONGPASSAGE,
    RPG_REVERB_TYPE_ICEPALACE_HALL,
    RPG_REVERB_TYPE_ICEPALACE_CUPBOARD,
    RPG_REVERB_TYPE_ICEPALACE_COURTYARD,
    RPG_REVERB_TYPE_ICEPALACE_ALCOVE,

    /* Space Station Presets */
    RPG_REVERB_TYPE_SPACESTATION_SMALLROOM,
    RPG_REVERB_TYPE_SPACESTATION_SHORTPASSAGE,
    RPG_REVERB_TYPE_SPACESTATION_MEDIUMROOM,
    RPG_REVERB_TYPE_SPACESTATION_LARGEROOM,
    RPG_REVERB_TYPE_SPACESTATION_LONGPASSAGE,
    RPG_REVERB_TYPE_SPACESTATION_HALL,
    RPG_REVERB_TYPE_SPACESTATION_CUPBOARD,
    RPG_REVERB_TYPE_SPACESTATION_ALCOVE,

    /* Wooden Galleon Presets */
    RPG_REVERB_TYPE_WOODEN_SMALLROOM,
    RPG_REVERB_TYPE_WOODEN_SHORTPASSAGE,
    RPG_REVERB_TYPE_WOODEN_MEDIUMROOM,
    RPG_REVERB_TYPE_WOODEN_LARGEROOM,
    RPG_REVERB_TYPE_WOODEN_LONGPASSAGE,
    RPG_REVERB_TYPE_WOODEN_HALL,
    RPG_REVERB_TYPE_WOODEN_CUPBOARD,
    RPG_REVERB_TYPE_WOODEN_COURTYARD,
    RPG_REVERB_TYPE_WOODEN_ALCOVE,

    /* Sports Presets */
    RPG_REVERB_TYPE_SPORT_EMPTYSTADIUM,
    RPG_REVERB_TYPE_SPORT_SQUASHCOURT,
    RPG_REVERB_TYPE_SPORT_SMALLSWIMMINGPOOL,
    RPG_REVERB_TYPE_SPORT_LARGESWIMMINGPOOL,
    RPG_REVERB_TYPE_SPORT_GYMNASIUM,
    RPG_REVERB_TYPE_SPORT_FULLSTADIUM,
    RPG_REVERB_TYPE_SPORT_STADIUMTANNOY,

    /* Prefab Presets */
    RPG_REVERB_TYPE_PREFAB_WORKSHOP,
    RPG_REVERB_TYPE_PREFAB_SCHOOLROOM,
    RPG_REVERB_TYPE_PREFAB_PRACTISEROOM,
    RPG_REVERB_TYPE_PREFAB_OUTHOUSE,
    RPG_REVERB_TYPE_PREFAB_CARAVAN,

    /* Dome and Pipe Presets */
    RPG_REVERB_TYPE_DOME_TOMB,
    RPG_REVERB_TYPE_PIPE_SMALL,
    RPG_REVERB_TYPE_DOME_SAINTPAULS,
    RPG_REVERB_TYPE_PIPE_LONGTHIN,
    RPG_REVERB_TYPE_PIPE_LARGE,
    RPG_REVERB_TYPE_PIPE_RESONANT,

    /* Outdoors Presets */
    RPG_REVERB_TYPE_OUTDOORS_BACKYARD,
    RPG_REVERB_TYPE_OUTDOORS_ROLLINGPLAINS,
    RPG_REVERB_TYPE_OUTDOORS_DEEPCANYON,
    RPG_REVERB_TYPE_OUTDOORS_CREEK,
    RPG_REVERB_TYPE_OUTDOORS_VALLEY,

    /* Mood Presets */
    RPG_REVERB_TYPE_MOOD_HEAVEN,
    RPG_REVERB_TYPE_MOOD_HELL,
    RPG_REVERB_TYPE_MOOD_MEMORY,

    /* Driving Presets */
    RPG_REVERB_TYPE_DRIVING_COMMENTATOR,
    RPG_REVERB_TYPE_DRIVING_PITGARAGE,
    RPG_REVERB_TYPE_DRIVING_INCAR_RACER,
    RPG_REVERB_TYPE_DRIVING_INCAR_SPORTS,
    RPG_REVERB_TYPE_DRIVING_INCAR_LUXURY,
    RPG_REVERB_TYPE_DRIVING_FULLGRANDSTAND,
    RPG_REVERB_TYPE_DRIVING_EMPTYGRANDSTAND,
    RPG_REVERB_TYPE_DRIVING_TUNNEL,

    /* City Presets */
    RPG_REVERB_TYPE_CITY_STREETS,
    RPG_REVERB_TYPE_CITY_SUBWAY,
    RPG_REVERB_TYPE_CITY_MUSEUM,
    RPG_REVERB_TYPE_CITY_LIBRARY,
    RPG_REVERB_TYPE_CITY_UNDERPASS,
    RPG_REVERB_TYPE_CITY_ABANDONED,

    /* Misc. Presets */
    RPG_REVERB_TYPE_DUSTYROOM,
    RPG_REVERB_TYPE_CHAPEL,
    RPG_REVERB_TYPE_SMALLWATERROOM,

    // Maximum value in the enum
    RPG_REVERB_TYPE_LAST = RPG_REVERB_TYPE_SMALLWATERROOM
} RPG_REVERB_TYPE;

typedef struct {
    RPGfloat density;
    RPGfloat diffusion;
    RPGfloat gain;
    RPGfloat gainHF;
    RPGfloat gainLF;
    RPGfloat decayTime;
    RPGfloat decayHFRatio;
    RPGfloat decayLFRatio;
    RPGfloat reflectionsGain;
    RPGfloat reflectionsDelay;
    RPGfloat reflectionsPan[3];
    RPGfloat lateReverbGain;
    RPGfloat lateReverbDelay;
    RPGfloat lateReverbPan[3];
    RPGfloat echoTime;
    RPGfloat echoDepth;
    RPGfloat modulationTime;
    RPGfloat modulationDepth;
    RPGfloat airAbsorptionGainHF;
    RPGfloat referenceHF;
    RPGfloat referenceLF;
    RPGfloat roomRolloffFactor;
    RPGint   decayHFLimit;
} RPGreverbpreset;

// General Effects
RPG_RESULT RPG_Audio_IsEffectAttached(RPGint channel, RPGaudiofx *fx, RPGbool *attached);
RPG_RESULT RPG_Audio_CreateEffect(RPG_AUDIOFX_TYPE type, RPGaudiofx **fx);
RPG_RESULT RPG_Audio_AttachEffect(RPGint channel, RPGaudiofx *fx);
RPG_RESULT RPG_Audio_DetachEffect(RPGint channel, RPGaudiofx *fx);
RPG_RESULT RPG_Audio_ReleaseEffect(RPGaudiofx *fx);
RPG_RESULT RPG_Audio_UpdateEffectChange(RPGaudiofx *fx);

// Reverb
RPG_RESULT RPG_Reverb_Create(RPGaudiofx **fx);
RPG_RESULT RPG_Reverb_CreateFromType(RPG_REVERB_TYPE type, RPGaudiofx **reverb);
RPG_RESULT RPG_Reverb_CreateFromPreset(RPGreverbpreset *preset, RPGaudiofx **reverb);
RPG_RESULT RPG_Reverb_GetPresetFromType(RPG_REVERB_TYPE type, RPGreverbpreset *preset);
RPG_RESULT RPG_Reverb_SetPreset(RPGaudiofx *e, RPGreverbpreset *p);
RPG_RESULT RPG_Reverb_GetPresetDescription(RPG_REVERB_TYPE type, char *buffer, RPGsize sizeBuffer, RPGsize *written);
RPG_RESULT RPG_Reverb_GetDensity(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetDiffusion(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetGain(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetGainHF(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetGainLF(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetDecayTime(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetRatioHF(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetRatioLF(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetReflectionsGain(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetReflectionsDelay(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetLateGain(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetLateDelay(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetEchoTime(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetEchoDepth(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetModulationTime(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetModulationDepth(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetAirAbsorption(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetReferenceHF(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetReferenceLF(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetRoomRolloff(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Reverb_GetDecayLimit(RPGaudiofx *fx, RPGbool *value);
RPG_RESULT RPG_Reverb_GetReflectionsPan(RPGaudiofx *fx, RPGvec3 *value);
RPG_RESULT RPG_Reverb_GetLatePan(RPGaudiofx *fx, RPGvec3 *value);
RPG_RESULT RPG_Reverb_SetDensity(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetDiffusion(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetGain(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetGainHF(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetGainLF(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetDecayTime(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetRatioHF(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetRatioLF(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetReflectionsGain(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetReflectionsDelay(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetLateGain(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetLateDelay(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetEchoTime(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetEchoDepth(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetModulationTime(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetModulationDepth(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetAirAbsorption(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetReferenceHF(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetReferenceLF(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetRoomRolloff(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Reverb_SetDecayLimit(RPGaudiofx *fx, RPGbool value);
RPG_RESULT RPG_Reverb_SetReflectionsPan(RPGaudiofx *fx, RPGvec3 *value);
RPG_RESULT RPG_Reverb_SetLatePan(RPGaudiofx *fx, RPGvec3 *value);

// Chorus
RPG_RESULT RPG_Chorus_Create(RPGaudiofx **fx);
RPG_RESULT RPG_Chorus_GetWaveform(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_Chorus_GetPhase(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_Chorus_GetRate(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Chorus_GetDepth(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Chorus_GetFeedback(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Chorus_GetDelay(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Chorus_SetWaveform(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_Chorus_SetPhase(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_Chorus_SetRate(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Chorus_SetDepth(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Chorus_SetFeedback(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Chorus_SetDelay(RPGaudiofx *fx, RPGfloat value);

// Distortion
RPG_RESULT RPG_Distortion_Create(RPGaudiofx **fx);
RPG_RESULT RPG_Distortion_GetEdge(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Distortion_GetGain(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Distortion_GetLowpassCutoff(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Distortion_GetCenterEQ(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Distortion_GetBandwidthEQ(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Distortion_SetEdge(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Distortion_SetGain(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Distortion_SetLowpassCutoff(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Distortion_SetCenterEQ(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Distortion_SetBandwidthEQ(RPGaudiofx *fx, RPGfloat value);

// Echo
RPG_RESULT RPG_Echo_Create(RPGaudiofx **fx);
RPG_RESULT RPG_Echo_GetDelay(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Echo_GetLRDelay(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Echo_GetDamping(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Echo_GetFeedback(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Echo_GetSpread(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Echo_SetDelay(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Echo_SetLRDelay(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Echo_SetDamping(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Echo_SetFeedback(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Echo_SetSpread(RPGaudiofx *fx, RPGfloat value);

// Flanger
RPG_RESULT RPG_Flanger_Create(RPGaudiofx **fx);
RPG_RESULT RPG_Flanger_GetWaveform(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_Flanger_GetPhase(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_Flanger_GetRate(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Flanger_GetDepth(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Flanger_GetFeedback(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Flanger_GetDelay(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Flanger_SetWaveform(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_Flanger_SetPhase(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_Flanger_SetRate(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Flanger_SetDepth(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Flanger_SetFeedback(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Flanger_SetDelay(RPGaudiofx *fx, RPGfloat value);

// FrequencyShifter
RPG_RESULT RPG_FrequencyShifter_Create(RPGaudiofx **fx);
RPG_RESULT RPG_FrequencyShifter_GetFrequency(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_FrequencyShifter_GetLeft(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_FrequencyShifter_GetRight(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_FrequencyShifter_SetFrequency(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_FrequencyShifter_SetLeft(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_FrequencyShifter_SetRight(RPGaudiofx *fx, RPGint value);

// Vocal Morpher
RPG_RESULT RPG_VocalMorpher_Create(RPGaudiofx **fx);
RPG_RESULT RPG_VocalMorpher_GetPhonemeA(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_VocalMorpher_GetPhonemeB(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_VocalMorpher_GetCoarseA(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_VocalMorpher_GetCoarseB(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_VocalMorpher_GetWaveform(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_VocalMorpher_GetRate(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_VocalMorpher_SetPhonemeA(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_VocalMorpher_SetPhonemeB(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_VocalMorpher_SetCoarseA(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_VocalMorpher_SetCoarseB(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_VocalMorpher_SetWaveform(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_VocalMorpher_SetRate(RPGaudiofx *fx, RPGfloat value);

// PitchShifter
RPG_RESULT RPG_PitchShifter_Create(RPGaudiofx **fx);
RPG_RESULT RPG_PitchShifter_GetCoarse(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_PitchShifter_GetFine(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_PitchShifter_SetCoarse(RPGaudiofx *fx, RPGint value);
RPG_RESULT RPG_PitchShifter_SetFine(RPGaudiofx *fx, RPGint value);

// RingModulator
RPG_RESULT RPG_RingModulator_Create(RPGaudiofx **fx);
RPG_RESULT RPG_RingModulator_GetFrequency(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_RingModulator_GetCutoff(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_RingModulator_GetWaveform(RPGaudiofx *fx, RPGint *value);
RPG_RESULT RPG_RingModulator_SetFrequency(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_RingModulator_SetCutoff(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_RingModulator_SetWaveform(RPGaudiofx *fx, RPGint value);

// AutoWah
RPG_RESULT RPG_AutoWah_Create(RPGaudiofx **fx);
RPG_RESULT RPG_AutoWah_GetAttack(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_AutoWah_GetRelease(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_AutoWah_GetResonance(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_AutoWah_GetPeakGain(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_AutoWah_SetAttack(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_AutoWah_SetRelease(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_AutoWah_SetResonance(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_AutoWah_SetPeakGain(RPGaudiofx *fx, RPGfloat value);

// Compressor
RPG_RESULT RPG_Compressor_Create(RPGaudiofx **fx);
RPG_RESULT RPG_Compressor_GetEnabled(RPGaudiofx *fx, RPGbool *value);
RPG_RESULT RPG_Compressor_SetEnabled(RPGaudiofx *fx, RPGbool value);

// Equalizer
RPG_RESULT RPG_Equalizer_Create(RPGaudiofx **fx);
RPG_RESULT RPG_Equalizer_GetLowGain(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Equalizer_GetLowCutoff(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Equalizer_GetMid1Gain(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Equalizer_GetMid1Center(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Equalizer_GetMid1Width(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Equalizer_GetMid2Gain(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Equalizer_GetMid2Center(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Equalizer_GetMid2Width(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Equalizer_GetHighGain(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Equalizer_GetHighCutoff(RPGaudiofx *fx, RPGfloat *value);
RPG_RESULT RPG_Equalizer_SetLowGain(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Equalizer_SetLowCutoff(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Equalizer_SetMid1Gain(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Equalizer_SetMid1Center(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Equalizer_SetMid1Width(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Equalizer_SetMid2Gain(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Equalizer_SetMid2Center(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Equalizer_SetMid2Width(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Equalizer_SetHighGain(RPGaudiofx *fx, RPGfloat value);
RPG_RESULT RPG_Equalizer_SetHighCutoff(RPGaudiofx *fx, RPGfloat value);

#endif /* RPG_AUDIO_NO_EFFECTS */
#endif /* RPG_NO_AUDIO */

#ifdef __cplusplus
}
#endif

#endif /* OPEN_RGP_AUDIO_H */