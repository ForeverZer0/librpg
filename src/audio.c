#include "rpgaudio.h"

#if !defined(RPG_NO_AUDIO)

#include "internal.h"
#include "platform.h"
#include "sndfile.h"
#include "game.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#define RPG_VALID_CHANNEL(i) (i >= 0 && i < RPG_MAX_CHANNELS && CHANNELS[i] != NULL) 
#define BUFFER_COUNT 3
#define BUFFER_SIZE 32768

#if !defined(RPG_AUDIO_NO_EFFECTS)

static LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
static LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
static LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
static LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
static LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
static LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
static LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
static LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
static LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
static LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
static LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;
static LPALGENEFFECTS alGenEffects;
static LPALDELETEEFFECTS alDeleteEffects;
static LPALISEFFECT alIsEffect;
static LPALEFFECTI alEffecti;
static LPALEFFECTIV alEffectiv;
static LPALEFFECTF alEffectf;
static LPALEFFECTFV alEffectfv;
static LPALGETEFFECTI alGetEffecti;
static LPALGETEFFECTIV alGetEffectiv;
static LPALGETEFFECTF alGetEffectf;
static LPALGETEFFECTFV alGetEffectfv;

#endif /* RPG_AUDIO_NO_EFFECTS */

typedef struct RPGsound {
    SNDFILE *file;
    SF_INFO info;
    char *filename;
    struct {
        ALenum format;
        sf_count_t itemsize;
    } al;
    union {
        union {
            sf_count_t (*readshort)(SNDFILE *sndfile, short *ptr, sf_count_t frames);
            sf_count_t (*readint)(SNDFILE *sndfile, int *ptr, sf_count_t frames);
            sf_count_t (*readfloat)(SNDFILE *sndfile, float *ptr, sf_count_t frames);
            sf_count_t (*readdouble)(SNDFILE *sndfile, double *ptr, sf_count_t frames);
        } sf;
        sf_count_t (*readframes)(SNDFILE *sndfile, void *ptr, sf_count_t frames);
    } func;
    pthread_mutex_t mutex;
} RPGsound;

typedef struct RPGchannel {
    RPGint index;
    ALuint source;
    ALuint buffers[BUFFER_COUNT];
    void *pcm;
    RPGint loopCount;
    RPGsound *sound;
    pthread_t thread;
    struct {
        RPGint num;
        size_t capacity;
        ALuint *effects;
        ALuint *slots;
    } aux;
} RPGchannel;

RPGchannel *CHANNELS[RPG_MAX_CHANNELS]; // FIXME: Evil global
RPGaudiofunc CALLBACKS[2];

RPG_RESULT RPG_Audio_Initialize(RPGgame *game) {
    if (game->audio.context) {
        return RPG_NO_ERROR;
    }
    ALCdevice *device = alcOpenDevice(NULL);
    RPG_ASSERT(device);
    if (device == NULL) {
        return RPG_ERR_AUDIO_DEVICE;
    }
    ALCcontext *context = alcCreateContext(device, NULL);
    RPG_ASSERT(context);
    if (context == NULL) {
        return RPG_ERR_AUDIO_CONTEXT;
    }
    if (!alcMakeContextCurrent(context)) {
        return RPG_ERR_AUDIO_CONTEXT;
    }

#if !defined(RPG_AUDIO_NO_EFFECTS)
#define AL_LOAD_PROC(x, y)                                                                                                                 \
    ((x) = (y)alGetProcAddress(#x));                                                                                                       \
    if (x == NULL)                                                                                                                         \
    return RPG_ERR_AUDIO_EXT

    AL_LOAD_PROC(alGenAuxiliaryEffectSlots, LPALGENAUXILIARYEFFECTSLOTS);
    AL_LOAD_PROC(alDeleteAuxiliaryEffectSlots, LPALDELETEAUXILIARYEFFECTSLOTS);
    AL_LOAD_PROC(alIsAuxiliaryEffectSlot, LPALISAUXILIARYEFFECTSLOT);
    AL_LOAD_PROC(alAuxiliaryEffectSloti, LPALAUXILIARYEFFECTSLOTI);
    AL_LOAD_PROC(alAuxiliaryEffectSlotiv, LPALAUXILIARYEFFECTSLOTIV);
    AL_LOAD_PROC(alAuxiliaryEffectSlotf, LPALAUXILIARYEFFECTSLOTF);
    AL_LOAD_PROC(alAuxiliaryEffectSlotfv, LPALAUXILIARYEFFECTSLOTFV);
    AL_LOAD_PROC(alGetAuxiliaryEffectSloti, LPALGETAUXILIARYEFFECTSLOTI);
    AL_LOAD_PROC(alGetAuxiliaryEffectSlotiv, LPALGETAUXILIARYEFFECTSLOTIV);
    AL_LOAD_PROC(alGetAuxiliaryEffectSlotf, LPALGETAUXILIARYEFFECTSLOTF);
    AL_LOAD_PROC(alGetAuxiliaryEffectSlotfv, LPALGETAUXILIARYEFFECTSLOTFV);
    AL_LOAD_PROC(alGenEffects, LPALGENEFFECTS);
    AL_LOAD_PROC(alDeleteEffects, LPALDELETEEFFECTS);
    AL_LOAD_PROC(alIsEffect, LPALISEFFECT);
    AL_LOAD_PROC(alEffecti, LPALEFFECTI);
    AL_LOAD_PROC(alEffectiv, LPALEFFECTIV);
    AL_LOAD_PROC(alEffectf, LPALEFFECTF);
    AL_LOAD_PROC(alEffectfv, LPALEFFECTFV);
    AL_LOAD_PROC(alGetEffecti, LPALGETEFFECTI);
    AL_LOAD_PROC(alGetEffectiv, LPALGETEFFECTIV);
    AL_LOAD_PROC(alGetEffectf, LPALGETEFFECTF);
    AL_LOAD_PROC(alGetEffectfv, LPALGETEFFECTFV);

#undef AL_LOAD_PROC
#endif /* RPG_AUDIO_NO_EFFECTS */

    game->audio.context = context;
    game->audio.device = device;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_Terminate(void) {
    for (int i = 0; i < RPG_MAX_CHANNELS; i++) {
        RPG_Audio_FreeChannel(i);
    }
    return RPG_NO_ERROR;
}

static void RPG_Audio_FreeSound(RPGsound *sound) {
    if (sound) {
        pthread_mutex_destroy(&sound->mutex);
        sf_close(sound->file);
        if (sound->filename) {
            RPG_FREE(sound->filename);
        }
        RPG_FREE(sound);
    }
}

RPG_RESULT RPG_Audio_FreeChannel(RPGint index) {
    if (index < 0 || index >= RPG_MAX_CHANNELS) {
        return RPG_ERR_OUT_OF_RANGE;
    }
    RPGchannel *channel = CHANNELS[index];
    if (channel) {
        alSourceStop(channel->source);
        pthread_join(channel->thread, NULL);
        alDeleteSources(1, &channel->source);
        alDeleteBuffers(BUFFER_COUNT, channel->buffers);
        RPG_Audio_FreeSound(channel->sound);
#if !defined(RPG_AUDIO_NO_EFFECTS)
        if (channel->aux.capacity > 0) {
            for (int i = 0; i < channel->aux.num; i++) {
                alDeleteAuxiliaryEffectSlots(1, &channel->aux.slots[i]);
            }
            RPG_FREE(channel->aux.slots);
            RPG_FREE(channel->aux.effects);
        }
#endif
        RPG_FREE(channel->pcm);
        CHANNELS[index] = NULL;
    }
    return RPG_NO_ERROR;
}

static void RPG_Audio_SetALFormat(RPGsound *sound) {

    int type = sound->info.format & SF_FORMAT_SUBMASK;
    switch (type) {
        case SF_FORMAT_PCM_S8:
        case SF_FORMAT_PCM_U8:
        case SF_FORMAT_FLAC:
            sound->al.format       = sound->info.channels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
            sound->al.itemsize     = sizeof(ALbyte) * sound->info.channels;
            sound->func.readframes = sf_read_raw;
            break;
        case SF_FORMAT_PCM_16:
            sound->al.format         = sound->info.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
            sound->al.itemsize       = sizeof(ALshort) * sound->info.channels;
            sound->func.sf.readshort = sf_readf_short;
            break;
        case SF_FORMAT_DOUBLE:
            sound->al.format          = sound->info.channels == 1 ? AL_FORMAT_MONO_DOUBLE_EXT : AL_FORMAT_STEREO_DOUBLE_EXT;
            sound->al.itemsize        = sizeof(ALdouble) * sound->info.channels;
            sound->func.sf.readdouble = sf_readf_double;
            break;
        case SF_FORMAT_PCM_32:
        case SF_FORMAT_VORBIS:
        case SF_FORMAT_FLOAT:
            sound->al.format         = sound->info.channels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
            sound->al.itemsize       = sizeof(ALfloat) * sound->info.channels;
            sound->func.sf.readfloat = sf_readf_float;
            break;
        default:
            // TODO: 
            fprintf(stderr, "Unsupported audio format.\n");
            sound->al.format         = sound->info.channels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
            sound->al.itemsize       = sizeof(ALfloat) * sound->info.channels;
            sound->func.sf.readfloat = sf_readf_float;
            break;
    };
}

static RPG_RESULT RPG_Audio_GetSoundError(int error) {
    switch (error) {
        case SF_ERR_NO_ERROR: return RPG_NO_ERROR;
        case SF_ERR_UNRECOGNISED_FORMAT: return RPG_ERR_FORMAT;
        case SF_ERR_SYSTEM: return RPG_ERR_SYSTEM;
        case SF_ERR_UNSUPPORTED_ENCODING: return RPG_ERR_ENCODING;
        case SF_ERR_MALFORMED_FILE: return RPG_ERR_MALFORMED;
        default: return RPG_ERR_UNKNOWN;
    }
}

static RPG_RESULT RPG_Audio_CreateSound(const char *filename, RPGsound **sound) {
    *sound = NULL;
    RPG_ENSURE_FILE(filename);
    RPG_ALLOC_ZERO(snd, RPGsound);
    // Createa file handle to the sound
    snd->file = sf_open(filename, SFM_READ, &snd->info);
    int error = sf_error(snd->file);
    if (error) {
        return RPG_Audio_GetSoundError(error);
    }
    pthread_mutex_init(&snd->mutex, NULL);
    // Set the filename
    snd->filename = RPG_MALLOC(strlen(filename) + 1);
    strcpy(snd->filename, filename);
    // Set the format used by OpenAL, and the read function depending on the PCM type
    RPG_Audio_SetALFormat(snd);
    *sound = snd;
    return RPG_NO_ERROR;
}

static RPGbool RPG_Audio_TryGetSlot(RPGint index, RPGchannel **channel) {
    if (index < 0 || index >= RPG_MAX_CHANNELS) {
        *channel = NULL;
        return RPG_FALSE;
    }
    if (CHANNELS[index] == NULL) {
        RPG_ALLOC_ZERO(s, RPGchannel);
        alGenSources(1, &s->source);
        alGenBuffers(BUFFER_COUNT, s->buffers);
        s->pcm          = RPG_MALLOC(BUFFER_SIZE);
        s->index = index;
        CHANNELS[index] = s;
    }
    *channel = CHANNELS[index];
    return RPG_TRUE;
}

static RPGbool RPG_Channel_FillBuffer(RPGchannel *channel, ALuint buffer) {
    if (!alIsSource(channel->source) || channel->sound == NULL) {
        return RPG_TRUE;
    }
    pthread_mutex_lock(&channel->sound->mutex);
    RPGsound *snd   = channel->sound;
    sf_count_t n    = BUFFER_SIZE / snd->al.itemsize;
    sf_count_t size = snd->func.readframes(snd->file, channel->pcm, n) * snd->al.itemsize;
    alBufferData(buffer, snd->al.format, channel->pcm, (ALsizei)size, snd->info.samplerate);
    if (size == 0) {
        sf_seek(snd->file, 0, SF_SEEK_SET);
        pthread_mutex_unlock(&channel->sound->mutex);
        if (channel->loopCount != 0) {
            ALint state;
            alGetSourcei(channel->source, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING) {
                alSourcePlay((channel->source));
            }
            alSourceUnqueueBuffers(channel->source, 0, NULL);
            channel->loopCount--;
            return AL_FALSE;
        }
    } else {
        pthread_mutex_unlock(&channel->sound->mutex);
    }
    return size == 0;
}

static void *RPG_Audio_Stream(void *channel) {
    RPGchannel *s = channel;
    for (int i = 0; i < BUFFER_COUNT; i++) {
        RPG_Channel_FillBuffer(s, s->buffers[i]);
    }
    alSourceQueueBuffers(s->source, BUFFER_COUNT, s->buffers);
    if (CALLBACKS[RPG_AUDIO_CB_PLAY]) {
        CALLBACKS[RPG_AUDIO_CB_PLAY](s->index);
    }
    alSourcePlay(s->source);
    RPGbool done;
    ALint processed, state;
    ALuint buffer;
    while (!done) {
        if (!alIsSource(s->source)) {
            break;
        } else {
            alGetSourcei(s->source, AL_SOURCE_STATE, &state);
            if (state == AL_STOPPED) {
                break;
            }
            RPG_SLEEP(100);
        }
        alGetSourcei(s->source, AL_BUFFERS_PROCESSED, &processed);
        while (processed--) {
            alSourceUnqueueBuffers(s->source, 1, &buffer);
            if (RPG_Channel_FillBuffer(s, buffer)) {
                done = RPG_TRUE;
                break;
            }
            alSourceQueueBuffers(s->source, 1, &buffer);
        }
    }
    if (CALLBACKS[RPG_AUDIO_CB_DONE]) {
        CALLBACKS[RPG_AUDIO_CB_DONE](s->index);
    }
    return NULL;
}

RPG_RESULT RPG_Audio_SetCallback(RPG_AUDIO_CB_TYPE type, RPGaudiofunc func) {
    // TODO: Return existing
    CALLBACKS[type] = func;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_Play(RPGint index, const char *filename, RPGfloat volume, RPGfloat pitch, RPGint loopCount) {
    if (index < 0 || index >= RPG_MAX_CHANNELS) {
        return RPG_ERR_OUT_OF_RANGE;
    }
    RPG_RETURN_IF_NULL(filename);
    RPGchannel *channel;
    if (!RPG_Audio_TryGetSlot(index, &channel)) {
        return RPG_ERR_MEMORY;
    }

    if (channel->sound) {
        if (strcasecmp(filename, channel->sound->filename) == 0) {
            // Same sound is already loaded in channel
            ALint state;
            alGetSourcei(channel->source, AL_SOURCE_STATE, &state);
            if (state == AL_PLAYING) {
                // Do nothing if already playing same sound
                return RPG_NO_ERROR;
            } else if (state == AL_PAUSED) {
                // Resume play and return if paused
                alSourcePlay(channel->source);
                return RPG_NO_ERROR;
            }
        } else {
            // Different sound, remove it
            alSourceStop(channel->source);
            RPG_Audio_FreeSound(channel->sound);
            channel->sound = NULL;
        }
    }

    channel->loopCount = loopCount;
    alSourcef(channel->source, AL_GAIN, RPG_CLAMPF(volume, 0.0f, 1.0f));
    alSourcef(channel->source, AL_PITCH, fmaxf(pitch, 0.0f)); // TODO: Greater than 0.0f ?
    if (channel->sound == NULL) {
        RPG_RESULT result = RPG_Audio_CreateSound(filename, &channel->sound);
        if (result != RPG_NO_ERROR) {
            return result;
        }
        // Begin streaming on separate thread
        if (pthread_create(&channel->thread, NULL, RPG_Audio_Stream, channel)) {
            return RPG_ERR_THREAD_FAILURE;
        }
    }

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_GetVolume(RPGint channel, RPGfloat *volume) {
    if (RPG_VALID_CHANNEL(channel)) {
        alGetSourcef(CHANNELS[channel]->source, AL_GAIN, volume);
        return RPG_NO_ERROR;
    }
    *volume = 0.0f;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_SetVolume(RPGint channel, RPGfloat volume) {
    if (RPG_VALID_CHANNEL(channel)) {
        alSourcef(CHANNELS[channel]->source, AL_GAIN, RPG_CLAMPF(volume, 0.0f, 1.0f));
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetPitch(RPGint channel, RPGfloat *pitch) {
    if (RPG_VALID_CHANNEL(channel)) {
        alGetSourcef(CHANNELS[channel]->source, AL_PITCH, pitch);
        return RPG_NO_ERROR;
    }
    *pitch = 0.0f;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_SetPitch(RPGint channel, RPGfloat pitch) {
    if (RPG_VALID_CHANNEL(channel)) {
        alSourcef(CHANNELS[channel]->source, AL_PITCH, fmaxf(0.0f, pitch));
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetSampleRate(RPGint channel, RPGint *rate) {
    if (RPG_VALID_CHANNEL(channel) && CHANNELS[channel]->sound != NULL) {
        *rate = CHANNELS[channel]->sound->info.samplerate;
        return RPG_NO_ERROR;
    }
    *rate = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetChannelCount(RPGint channel, RPGint *count) {
    if (RPG_VALID_CHANNEL(channel) && CHANNELS[channel]->sound != NULL) {
        *count = CHANNELS[channel]->sound->info.channels;
        return RPG_NO_ERROR;
    }
    *count = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetSampleCount(RPGint channel, RPGint *count) {
    if (RPG_VALID_CHANNEL(channel) && CHANNELS[channel]->sound != NULL) {
        *count = CHANNELS[channel]->sound->info.frames;
        return RPG_NO_ERROR;
    }
    *count = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetSectionCount(RPGint channel, RPGint *count) {
    if (RPG_VALID_CHANNEL(channel) && CHANNELS[channel]->sound != NULL) {
        *count = CHANNELS[channel]->sound->info.sections;
        return RPG_NO_ERROR;
    }
    *count = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetFormat(RPGint channel, RPG_SOUND_FORMAT *format) {
    if (RPG_VALID_CHANNEL(channel) && CHANNELS[channel]->sound != NULL) {
        *format = (RPG_SOUND_FORMAT)(CHANNELS[channel]->sound->info.format & SF_FORMAT_TYPEMASK);
        return RPG_NO_ERROR;
    }
    *format = RPG_SOUND_FORMAT_NONE;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetType(RPGint channel, RPG_SOUND_TYPE *type) {
    if (RPG_VALID_CHANNEL(channel) && CHANNELS[channel]->sound != NULL) {
        *type = (RPG_SOUND_TYPE)(CHANNELS[channel]->sound->info.format & SF_FORMAT_SUBMASK);
        return RPG_NO_ERROR;
    }
    *type = RPG_SOUND_TYPE_NONE;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetDuration(RPGint channel, RPGint64 *milliseconds) {
    if (RPG_VALID_CHANNEL(channel) && CHANNELS[channel]->sound != NULL) {
        RPGsound *snd = CHANNELS[channel]->sound;
        *milliseconds = (RPGint64)round((1000.0 / snd->info.samplerate) * snd->info.frames);
        return RPG_NO_ERROR;
    }
    *milliseconds = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetInfo(RPGint channel, RPG_SOUND_INFO type, char *buffer, size_t sizeBuffer, size_t *written) {
    if (sizeBuffer == 0) {
        return RPG_NO_ERROR;
    }
    if (buffer == NULL) {
        *written = 0;
        return RPG_ERR_INVALID_POINTER;
    }
    memset(buffer, 0, sizeBuffer);
    if (RPG_VALID_CHANNEL(channel) && CHANNELS[channel]->sound != NULL) {
        const char *str = sf_get_string(CHANNELS[channel]->sound->file, type);
        if (str != NULL) {
            size_t sz = strlen(str) + 1;
            *written  = sizeBuffer > sz ? sz : sizeBuffer;
            memcpy(buffer, str, *written);
        }
        return RPG_NO_ERROR;
    }
    *written = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetPlaybackState(RPGint channel, RPG_PLAYBACK_STATE *state) {
    if (RPG_VALID_CHANNEL(channel)) {
        ALint s;
        alGetSourcei(CHANNELS[channel]->source, AL_SOURCE_STATE, &s);
        switch (s) {
            case AL_PLAYING: *state = RPG_PLAYBACK_STATE_PLAYING; break;
            case AL_PAUSED: *state = RPG_PLAYBACK_STATE_PAUSED; break;
            default: *state = RPG_PLAYBACK_STATE_STOPPED; break;
        }
        return RPG_NO_ERROR;
    }
    *state = RPG_PLAYBACK_STATE_STOPPED;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetLoopCount(RPGint channel, RPGint *count) {
    *count = RPG_VALID_CHANNEL(channel) ? CHANNELS[channel]->loopCount : 0;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_SetLoopCount(RPGint channel, RPGint count) {
    RPGchannel *s;
    if (RPG_Audio_TryGetSlot(channel, &s)) {
        s->loopCount = count;
        return RPG_NO_ERROR;
    }
    return RPG_ERR_MEMORY;
}

RPG_RESULT RPG_Audio_Resume(RPGint channel) {
    if (RPG_VALID_CHANNEL(channel)) {
        alSourcePlay(CHANNELS[channel]->source);
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_Stop(RPGint channel) {
    if (RPG_VALID_CHANNEL(channel)) {
        alSourceStop(CHANNELS[channel]->source);
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_Pause(RPGint channel) {
    if (RPG_VALID_CHANNEL(channel)) {
        alSourcePause(CHANNELS[channel]->source);
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetPosition(RPGint channel, RPGint64 *position) {
    RPG_RETURN_IF_NULL(position);
    if (RPG_VALID_CHANNEL(channel) && CHANNELS[channel]->sound){
        RPGchannel *s = CHANNELS[channel];
        pthread_mutex_lock(&s->sound->mutex);
        sf_count_t pos = sf_seek(s->sound->file, 0, SF_SEEK_CUR);
        pthread_mutex_unlock(&s->sound->mutex); 
        *position = (RPGint64) round(pos / (s->sound->info.samplerate / 1000.0));
        return RPG_NO_ERROR;
    }
    *position = 0L;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_Seek(RPGint channel, RPGint64 ms) {
    if (RPG_VALID_CHANNEL(channel) && CHANNELS[channel]->sound) {
        RPGchannel *s = CHANNELS[channel];
        sf_count_t pos = (sf_count_t)round(ms * (s->sound->info.samplerate / 1000.0));

        if (pos < 0 || pos > s->sound->info.frames) {
            return RPG_ERR_OUT_OF_RANGE;
        }
        ALenum state;
        alGetSourcei(s->source, AL_SOURCE_STATE, &state);
        if (state != AL_STOPPED) {
            alSourcePause(s->source);
            alSourceUnqueueBuffers(s->source, BUFFER_COUNT, NULL);
        }
        pthread_mutex_lock(&s->sound->mutex);
        sf_seek(s->sound->file, pos, SF_SEEK_SET);
        pthread_mutex_unlock(&s->sound->mutex);
        if (state == AL_PLAYING) {
            alSourcePlay(s->source);
        }
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

#if !defined(RPG_AUDIO_NO_EFFECTS)

#include <AL/efx-presets.h>

typedef struct RPGaudiofx {
    ALint type;
    ALuint effect;
} RPGaudiofx;

RPG_RESULT RPG_Audio_UpdateEffectChange(RPGaudiofx *fx) {
    RPG_RETURN_IF_NULL(fx);
    RPGchannel *c = CHANNELS[0];
    for (int i = 0; i < RPG_MAX_CHANNELS; i++, c = CHANNELS[i]) {
        if (c && c->aux.num > 0) {
            for (int j = 0; j < c->aux.num; j++) {
                if (c->aux.effects[j] == fx->effect) {
                    alAuxiliaryEffectSloti(c->aux.slots[j], AL_EFFECTSLOT_EFFECT, fx->effect);
                }
            }
        }
    }
    return RPG_NO_ERROR;
}

static inline RPGbool RPG_Audio_IsAffectAttached_Inline(RPGchannel *c, RPGaudiofx *fx) {
    for (int i = 0; i < c->aux.num; i++) {
        if (c->aux.effects[i] == fx->effect) {
            return RPG_TRUE;
        }
    }
    return RPG_FALSE;
}

RPG_RESULT RPG_Audio_CreateEffect(RPG_AUDIOFX_TYPE type, RPGaudiofx **fx) {
    RPGaudiofx *f = RPG_ALLOC(RPGaudiofx);
    f->type = type;
    alGenEffects(1, &f->effect);
    alEffecti(f->effect, AL_EFFECT_TYPE, (ALenum)type);
    if (alGetError()) {
        RPG_FREE(f);
        return RPG_ERR_AUDIO_EXT;
    }
    *fx = f;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_AttachEffect(RPGint channel, RPGaudiofx *fx) {
    RPG_RETURN_IF_NULL(fx);
    RPGchannel *c;
    if (RPG_Audio_TryGetSlot(channel, &c)) {
        if (RPG_Audio_IsAffectAttached_Inline(c, fx)) {
            return RPG_NO_ERROR;
        }
        ALuint slot;
        alGenAuxiliaryEffectSlots(1, &slot);
        alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, fx->effect);
        if (alGetError()) {
            alDeleteAuxiliaryEffectSlots(1, &slot);
            return RPG_ERR_INVALID_VALUE;
        }

        if (c->aux.num == 0) {
            c->aux.effects  = RPG_ALLOC(ALuint);
            c->aux.slots    = RPG_ALLOC(ALuint);
            c->aux.capacity = sizeof(ALuint);
        } else {
            size_t sz = sizeof(ALuint) * (c->aux.num + 1);
            if (sz < c->aux.capacity) {
                void *temp1, *temp2;
                temp1 = RPG_REALLOC(c->aux.effects, sz);
                temp2 = RPG_REALLOC(c->aux.slots, sz);
                if (temp1 == NULL || temp2 == NULL) {
                    alDeleteAuxiliaryEffectSlots(1, &slot);
                    return RPG_ERR_MEMORY;
                }
                c->aux.effects  = temp1;
                c->aux.slots    = temp2;
                c->aux.capacity = sz;
            }
        }

        c->aux.slots[c->aux.num]   = slot;
        c->aux.effects[c->aux.num] = fx->effect;
        alSource3i(c->source, AL_AUXILIARY_SEND_FILTER, slot, c->aux.num, AL_FILTER_NULL);
        c->aux.num++;
        return RPG_NO_ERROR;
    }
    return RPG_ERR_MEMORY;
}

RPG_RESULT RPG_Audio_DetachEffect(RPGint channel, RPGaudiofx *fx) {
    RPG_RETURN_IF_NULL(fx);
    if (RPG_VALID_CHANNEL(channel)) {
        RPGchannel *c = CHANNELS[channel];
        int index     = -1;
        for (int i = 0; i < c->aux.num; i++) {
            if (c->aux.effects[i] == fx->effect) {
                index = i;
                break;
            }
        }
        if (index > -1) {
            alSource3i(c->source, AL_AUXILIARY_SEND_FILTER, 0, index, AL_FILTER_NULL);
            alDeleteAuxiliaryEffectSlots(1, &c->aux.slots[index]);
            for (int i = index; i < c->aux.num - 1; i++) { // TODO: Test
                c->aux.effects[i] = c->aux.effects[i + 1];
                c->aux.slots[i]   = c->aux.slots[i + 1];
            }
            c->aux.num--;
        }
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_IsEffectAttached(RPGint channel, RPGaudiofx *fx, RPGbool *attached) {
    *attached = RPG_FALSE;
    RPG_RETURN_IF_NULL(fx);
    if (RPG_VALID_CHANNEL(channel)) {
        RPGchannel *c = CHANNELS[channel];
        for (int i = 0; i < c->aux.num; i++) {
            if (c->aux.effects[i] == fx->effect) {
                *attached = RPG_TRUE;
                break;
            }
        }
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_ReleaseEffect(RPGaudiofx *fx) {
    RPG_RETURN_IF_NULL(fx);
    RPGchannel *c = CHANNELS[0];
    for (int i = 0; i < RPG_MAX_CHANNELS; i++, c = CHANNELS[i]) {
        if (c) {
            RPG_Audio_DetachEffect(i, fx);
        }
    }
    alDeleteEffects(1, &fx->effect);
    RPG_FREE(fx);
    return RPG_NO_ERROR;
}

static RPGreverbpreset RPG_Reverb_GetPreset_S(RPG_REVERB_TYPE type) {
    switch (type) {
        /* General Presets */
        case RPG_REVERB_TYPE_GENERIC: return (RPGreverbpreset)EFX_REVERB_PRESET_GENERIC;
        case RPG_REVERB_TYPE_PADDEDCELL: return (RPGreverbpreset)EFX_REVERB_PRESET_PADDEDCELL;
        case RPG_REVERB_TYPE_ROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_ROOM;
        case RPG_REVERB_TYPE_BATHROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_BATHROOM;
        case RPG_REVERB_TYPE_LIVINGROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_LIVINGROOM;
        case RPG_REVERB_TYPE_STONEROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_STONEROOM;
        case RPG_REVERB_TYPE_AUDITORIUM: return (RPGreverbpreset)EFX_REVERB_PRESET_AUDITORIUM;
        case RPG_REVERB_TYPE_CONCERTHALL: return (RPGreverbpreset)EFX_REVERB_PRESET_CONCERTHALL;
        case RPG_REVERB_TYPE_CAVE: return (RPGreverbpreset)EFX_REVERB_PRESET_CAVE;
        case RPG_REVERB_TYPE_ARENA: return (RPGreverbpreset)EFX_REVERB_PRESET_ARENA;
        case RPG_REVERB_TYPE_HANGAR: return (RPGreverbpreset)EFX_REVERB_PRESET_HANGAR;
        case RPG_REVERB_TYPE_CARPETEDHALLWAY: return (RPGreverbpreset)EFX_REVERB_PRESET_CARPETEDHALLWAY;
        case RPG_REVERB_TYPE_HALLWAY: return (RPGreverbpreset)EFX_REVERB_PRESET_HALLWAY;
        case RPG_REVERB_TYPE_STONECORRIDOR: return (RPGreverbpreset)EFX_REVERB_PRESET_STONECORRIDOR;
        case RPG_REVERB_TYPE_ALLEY: return (RPGreverbpreset)EFX_REVERB_PRESET_ALLEY;
        case RPG_REVERB_TYPE_FOREST: return (RPGreverbpreset)EFX_REVERB_PRESET_FOREST;
        case RPG_REVERB_TYPE_CITY: return (RPGreverbpreset)EFX_REVERB_PRESET_CITY;
        case RPG_REVERB_TYPE_MOUNTAINS: return (RPGreverbpreset)EFX_REVERB_PRESET_MOUNTAINS;
        case RPG_REVERB_TYPE_QUARRY: return (RPGreverbpreset)EFX_REVERB_PRESET_QUARRY;
        case RPG_REVERB_TYPE_PLAIN: return (RPGreverbpreset)EFX_REVERB_PRESET_PLAIN;
        case RPG_REVERB_TYPE_PARKINGLOT: return (RPGreverbpreset)EFX_REVERB_PRESET_PARKINGLOT;
        case RPG_REVERB_TYPE_SEWERPIPE: return (RPGreverbpreset)EFX_REVERB_PRESET_SEWERPIPE;
        case RPG_REVERB_TYPE_UNDERWATER: return (RPGreverbpreset)EFX_REVERB_PRESET_UNDERWATER;
        case RPG_REVERB_TYPE_DRUGGED: return (RPGreverbpreset)EFX_REVERB_PRESET_DRUGGED;
        case RPG_REVERB_TYPE_DIZZY: return (RPGreverbpreset)EFX_REVERB_PRESET_DIZZY;
        case RPG_REVERB_TYPE_PSYCHOTIC: return (RPGreverbpreset)EFX_REVERB_PRESET_PSYCHOTIC;
        /* Castle Presets */
        case RPG_REVERB_TYPE_CASTLE_SMALLROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_CASTLE_SMALLROOM;
        case RPG_REVERB_TYPE_CASTLE_SHORTPASSAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_CASTLE_SHORTPASSAGE;
        case RPG_REVERB_TYPE_CASTLE_MEDIUMROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_CASTLE_MEDIUMROOM;
        case RPG_REVERB_TYPE_CASTLE_LARGEROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_CASTLE_LARGEROOM;
        case RPG_REVERB_TYPE_CASTLE_LONGPASSAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_CASTLE_LONGPASSAGE;
        case RPG_REVERB_TYPE_CASTLE_HALL: return (RPGreverbpreset)EFX_REVERB_PRESET_CASTLE_HALL;
        case RPG_REVERB_TYPE_CASTLE_CUPBOARD: return (RPGreverbpreset)EFX_REVERB_PRESET_CASTLE_CUPBOARD;
        case RPG_REVERB_TYPE_CASTLE_COURTYARD: return (RPGreverbpreset)EFX_REVERB_PRESET_CASTLE_COURTYARD;
        case RPG_REVERB_TYPE_CASTLE_ALCOVE: return (RPGreverbpreset)EFX_REVERB_PRESET_CASTLE_ALCOVE;
        /* Factory Presets */
        case RPG_REVERB_TYPE_FACTORY_SMALLROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_FACTORY_SMALLROOM;
        case RPG_REVERB_TYPE_FACTORY_SHORTPASSAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_FACTORY_SHORTPASSAGE;
        case RPG_REVERB_TYPE_FACTORY_MEDIUMROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_FACTORY_MEDIUMROOM;
        case RPG_REVERB_TYPE_FACTORY_LARGEROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_FACTORY_LARGEROOM;
        case RPG_REVERB_TYPE_FACTORY_LONGPASSAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_FACTORY_LONGPASSAGE;
        case RPG_REVERB_TYPE_FACTORY_HALL: return (RPGreverbpreset)EFX_REVERB_PRESET_FACTORY_HALL;
        case RPG_REVERB_TYPE_FACTORY_CUPBOARD: return (RPGreverbpreset)EFX_REVERB_PRESET_FACTORY_CUPBOARD;
        case RPG_REVERB_TYPE_FACTORY_COURTYARD: return (RPGreverbpreset)EFX_REVERB_PRESET_FACTORY_COURTYARD;
        case RPG_REVERB_TYPE_FACTORY_ALCOVE: return (RPGreverbpreset)EFX_REVERB_PRESET_FACTORY_ALCOVE;
        /* Ice Palace Presets */
        case RPG_REVERB_TYPE_ICEPALACE_SMALLROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_ICEPALACE_SMALLROOM;
        case RPG_REVERB_TYPE_ICEPALACE_SHORTPASSAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_ICEPALACE_SHORTPASSAGE;
        case RPG_REVERB_TYPE_ICEPALACE_MEDIUMROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_ICEPALACE_MEDIUMROOM;
        case RPG_REVERB_TYPE_ICEPALACE_LARGEROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_ICEPALACE_LARGEROOM;
        case RPG_REVERB_TYPE_ICEPALACE_LONGPASSAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_ICEPALACE_LONGPASSAGE;
        case RPG_REVERB_TYPE_ICEPALACE_HALL: return (RPGreverbpreset)EFX_REVERB_PRESET_ICEPALACE_HALL;
        case RPG_REVERB_TYPE_ICEPALACE_CUPBOARD: return (RPGreverbpreset)EFX_REVERB_PRESET_ICEPALACE_CUPBOARD;
        case RPG_REVERB_TYPE_ICEPALACE_COURTYARD: return (RPGreverbpreset)EFX_REVERB_PRESET_ICEPALACE_COURTYARD;
        case RPG_REVERB_TYPE_ICEPALACE_ALCOVE: return (RPGreverbpreset)EFX_REVERB_PRESET_ICEPALACE_ALCOVE;
        /* Space Station Presets */
        case RPG_REVERB_TYPE_SPACESTATION_SMALLROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_SPACESTATION_SMALLROOM;
        case RPG_REVERB_TYPE_SPACESTATION_SHORTPASSAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_SPACESTATION_SHORTPASSAGE;
        case RPG_REVERB_TYPE_SPACESTATION_MEDIUMROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_SPACESTATION_MEDIUMROOM;
        case RPG_REVERB_TYPE_SPACESTATION_LARGEROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_SPACESTATION_LARGEROOM;
        case RPG_REVERB_TYPE_SPACESTATION_LONGPASSAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_SPACESTATION_LONGPASSAGE;
        case RPG_REVERB_TYPE_SPACESTATION_HALL: return (RPGreverbpreset)EFX_REVERB_PRESET_SPACESTATION_HALL;
        case RPG_REVERB_TYPE_SPACESTATION_CUPBOARD: return (RPGreverbpreset)EFX_REVERB_PRESET_SPACESTATION_CUPBOARD;
        case RPG_REVERB_TYPE_SPACESTATION_ALCOVE: return (RPGreverbpreset)EFX_REVERB_PRESET_SPACESTATION_ALCOVE;
        /* Wooden Galleon Presets */
        case RPG_REVERB_TYPE_WOODEN_SMALLROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_WOODEN_SMALLROOM;
        case RPG_REVERB_TYPE_WOODEN_SHORTPASSAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_WOODEN_SHORTPASSAGE;
        case RPG_REVERB_TYPE_WOODEN_MEDIUMROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_WOODEN_MEDIUMROOM;
        case RPG_REVERB_TYPE_WOODEN_LARGEROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_WOODEN_LARGEROOM;
        case RPG_REVERB_TYPE_WOODEN_LONGPASSAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_WOODEN_LONGPASSAGE;
        case RPG_REVERB_TYPE_WOODEN_HALL: return (RPGreverbpreset)EFX_REVERB_PRESET_WOODEN_HALL;
        case RPG_REVERB_TYPE_WOODEN_CUPBOARD: return (RPGreverbpreset)EFX_REVERB_PRESET_WOODEN_CUPBOARD;
        case RPG_REVERB_TYPE_WOODEN_COURTYARD: return (RPGreverbpreset)EFX_REVERB_PRESET_WOODEN_COURTYARD;
        case RPG_REVERB_TYPE_WOODEN_ALCOVE: return (RPGreverbpreset)EFX_REVERB_PRESET_WOODEN_ALCOVE;
        /* Sports Presets */
        case RPG_REVERB_TYPE_SPORT_EMPTYSTADIUM: return (RPGreverbpreset)EFX_REVERB_PRESET_SPORT_EMPTYSTADIUM;
        case RPG_REVERB_TYPE_SPORT_SQUASHCOURT: return (RPGreverbpreset)EFX_REVERB_PRESET_SPORT_SQUASHCOURT;
        case RPG_REVERB_TYPE_SPORT_SMALLSWIMMINGPOOL: return (RPGreverbpreset)EFX_REVERB_PRESET_SPORT_SMALLSWIMMINGPOOL;
        case RPG_REVERB_TYPE_SPORT_LARGESWIMMINGPOOL: return (RPGreverbpreset)EFX_REVERB_PRESET_SPORT_LARGESWIMMINGPOOL;
        case RPG_REVERB_TYPE_SPORT_GYMNASIUM: return (RPGreverbpreset)EFX_REVERB_PRESET_SPORT_GYMNASIUM;
        case RPG_REVERB_TYPE_SPORT_FULLSTADIUM: return (RPGreverbpreset)EFX_REVERB_PRESET_SPORT_FULLSTADIUM;
        case RPG_REVERB_TYPE_SPORT_STADIUMTANNOY: return (RPGreverbpreset)EFX_REVERB_PRESET_SPORT_STADIUMTANNOY;
        /* Prefab Presets */
        case RPG_REVERB_TYPE_PREFAB_WORKSHOP: return (RPGreverbpreset)EFX_REVERB_PRESET_PREFAB_WORKSHOP;
        case RPG_REVERB_TYPE_PREFAB_SCHOOLROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_PREFAB_SCHOOLROOM;
        case RPG_REVERB_TYPE_PREFAB_PRACTISEROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_PREFAB_PRACTISEROOM;
        case RPG_REVERB_TYPE_PREFAB_OUTHOUSE: return (RPGreverbpreset)EFX_REVERB_PRESET_PREFAB_OUTHOUSE;
        case RPG_REVERB_TYPE_PREFAB_CARAVAN: return (RPGreverbpreset)EFX_REVERB_PRESET_PREFAB_CARAVAN;
        /* Dome and Pipe Presets */
        case RPG_REVERB_TYPE_DOME_TOMB: return (RPGreverbpreset)EFX_REVERB_PRESET_DOME_TOMB;
        case RPG_REVERB_TYPE_PIPE_SMALL: return (RPGreverbpreset)EFX_REVERB_PRESET_PIPE_SMALL;
        case RPG_REVERB_TYPE_DOME_SAINTPAULS: return (RPGreverbpreset)EFX_REVERB_PRESET_DOME_SAINTPAULS;
        case RPG_REVERB_TYPE_PIPE_LONGTHIN: return (RPGreverbpreset)EFX_REVERB_PRESET_PIPE_LONGTHIN;
        case RPG_REVERB_TYPE_PIPE_LARGE: return (RPGreverbpreset)EFX_REVERB_PRESET_PIPE_LARGE;
        case RPG_REVERB_TYPE_PIPE_RESONANT: return (RPGreverbpreset)EFX_REVERB_PRESET_PIPE_RESONANT;
        /* Outdoors Presets */
        case RPG_REVERB_TYPE_OUTDOORS_BACKYARD: return (RPGreverbpreset)EFX_REVERB_PRESET_OUTDOORS_BACKYARD;
        case RPG_REVERB_TYPE_OUTDOORS_ROLLINGPLAINS: return (RPGreverbpreset)EFX_REVERB_PRESET_OUTDOORS_ROLLINGPLAINS;
        case RPG_REVERB_TYPE_OUTDOORS_DEEPCANYON: return (RPGreverbpreset)EFX_REVERB_PRESET_OUTDOORS_DEEPCANYON;
        case RPG_REVERB_TYPE_OUTDOORS_CREEK: return (RPGreverbpreset)EFX_REVERB_PRESET_OUTDOORS_CREEK;
        case RPG_REVERB_TYPE_OUTDOORS_VALLEY: return (RPGreverbpreset)EFX_REVERB_PRESET_OUTDOORS_VALLEY;
        /* Mood Presets */
        case RPG_REVERB_TYPE_MOOD_HEAVEN: return (RPGreverbpreset)EFX_REVERB_PRESET_MOOD_HEAVEN;
        case RPG_REVERB_TYPE_MOOD_HELL: return (RPGreverbpreset)EFX_REVERB_PRESET_MOOD_HELL;
        case RPG_REVERB_TYPE_MOOD_MEMORY: return (RPGreverbpreset)EFX_REVERB_PRESET_MOOD_MEMORY;
        /* Driving Presets */
        case RPG_REVERB_TYPE_DRIVING_COMMENTATOR: return (RPGreverbpreset)EFX_REVERB_PRESET_DRIVING_COMMENTATOR;
        case RPG_REVERB_TYPE_DRIVING_PITGARAGE: return (RPGreverbpreset)EFX_REVERB_PRESET_DRIVING_PITGARAGE;
        case RPG_REVERB_TYPE_DRIVING_INCAR_RACER: return (RPGreverbpreset)EFX_REVERB_PRESET_DRIVING_INCAR_RACER;
        case RPG_REVERB_TYPE_DRIVING_INCAR_SPORTS: return (RPGreverbpreset)EFX_REVERB_PRESET_DRIVING_INCAR_SPORTS;
        case RPG_REVERB_TYPE_DRIVING_INCAR_LUXURY: return (RPGreverbpreset)EFX_REVERB_PRESET_DRIVING_INCAR_LUXURY;
        case RPG_REVERB_TYPE_DRIVING_FULLGRANDSTAND: return (RPGreverbpreset)EFX_REVERB_PRESET_DRIVING_FULLGRANDSTAND;
        case RPG_REVERB_TYPE_DRIVING_EMPTYGRANDSTAND: return (RPGreverbpreset)EFX_REVERB_PRESET_DRIVING_EMPTYGRANDSTAND;
        case RPG_REVERB_TYPE_DRIVING_TUNNEL: return (RPGreverbpreset)EFX_REVERB_PRESET_DRIVING_TUNNEL;
        /* City Presets */
        case RPG_REVERB_TYPE_CITY_STREETS: return (RPGreverbpreset)EFX_REVERB_PRESET_CITY_STREETS;
        case RPG_REVERB_TYPE_CITY_SUBWAY: return (RPGreverbpreset)EFX_REVERB_PRESET_CITY_SUBWAY;
        case RPG_REVERB_TYPE_CITY_MUSEUM: return (RPGreverbpreset)EFX_REVERB_PRESET_CITY_MUSEUM;
        case RPG_REVERB_TYPE_CITY_LIBRARY: return (RPGreverbpreset)EFX_REVERB_PRESET_CITY_LIBRARY;
        case RPG_REVERB_TYPE_CITY_UNDERPASS: return (RPGreverbpreset)EFX_REVERB_PRESET_CITY_UNDERPASS;
        case RPG_REVERB_TYPE_CITY_ABANDONED: return (RPGreverbpreset)EFX_REVERB_PRESET_CITY_ABANDONED;
        /* Misc. Presets */
        case RPG_REVERB_TYPE_DUSTYROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_DUSTYROOM;
        case RPG_REVERB_TYPE_CHAPEL: return (RPGreverbpreset)EFX_REVERB_PRESET_CHAPEL;
        case RPG_REVERB_TYPE_SMALLWATERROOM: return (RPGreverbpreset)EFX_REVERB_PRESET_SMALLWATERROOM;
        default: return (RPGreverbpreset)EFX_REVERB_PRESET_GENERIC;
    }
}

static const char *RPG_Reverb_GetPresetDescription_S(RPG_REVERB_TYPE type) {
    // TODO: Localization
    switch (type) {
        /* General Presets */
        case RPG_REVERB_TYPE_GENERIC: return "Generic";
        case RPG_REVERB_TYPE_PADDEDCELL: return "Padded Cell";
        case RPG_REVERB_TYPE_ROOM: return "Room";
        case RPG_REVERB_TYPE_BATHROOM: return "Bathroom";
        case RPG_REVERB_TYPE_LIVINGROOM: return "Living Room";
        case RPG_REVERB_TYPE_STONEROOM: return "Stone Room";
        case RPG_REVERB_TYPE_AUDITORIUM: return "Auditorium";
        case RPG_REVERB_TYPE_CONCERTHALL: return "Concert Hall";
        case RPG_REVERB_TYPE_CAVE: return "Cave";
        case RPG_REVERB_TYPE_ARENA: return "Arena";
        case RPG_REVERB_TYPE_HANGAR: return "Hangar";
        case RPG_REVERB_TYPE_CARPETEDHALLWAY: return "Carpeted Hallway";
        case RPG_REVERB_TYPE_HALLWAY: return "Hallway";
        case RPG_REVERB_TYPE_STONECORRIDOR: return "Stone Corridor";
        case RPG_REVERB_TYPE_ALLEY: return "Alley";
        case RPG_REVERB_TYPE_FOREST: return "Forest";
        case RPG_REVERB_TYPE_CITY: return "City";
        case RPG_REVERB_TYPE_MOUNTAINS: return "Mountains";
        case RPG_REVERB_TYPE_QUARRY: return "Quarry";
        case RPG_REVERB_TYPE_PLAIN: return "Plain";
        case RPG_REVERB_TYPE_PARKINGLOT: return "Parking Lot";
        case RPG_REVERB_TYPE_SEWERPIPE: return "Sewer-Pipe";
        case RPG_REVERB_TYPE_UNDERWATER: return "Underwater";
        case RPG_REVERB_TYPE_DRUGGED: return "Drugged";
        case RPG_REVERB_TYPE_DIZZY: return "Dizzy";
        case RPG_REVERB_TYPE_PSYCHOTIC: return "Psychotic";
        /* Castle Presets */
        case RPG_REVERB_TYPE_CASTLE_SMALLROOM: return "Castle - Small Room";
        case RPG_REVERB_TYPE_CASTLE_SHORTPASSAGE: return "Castle - Short Passage";
        case RPG_REVERB_TYPE_CASTLE_MEDIUMROOM: return "Castle - Medium Room";
        case RPG_REVERB_TYPE_CASTLE_LARGEROOM: return "Castle - Large Room";
        case RPG_REVERB_TYPE_CASTLE_LONGPASSAGE: return "Castle - Long Passage";
        case RPG_REVERB_TYPE_CASTLE_HALL: return "Castle - Hall";
        case RPG_REVERB_TYPE_CASTLE_CUPBOARD: return "Castle - Cupboard";
        case RPG_REVERB_TYPE_CASTLE_COURTYARD: return "Castle - Courtyard";
        case RPG_REVERB_TYPE_CASTLE_ALCOVE: return "Castle - Alcove";
        /* Factory Presets */
        case RPG_REVERB_TYPE_FACTORY_SMALLROOM: return "Factory - Small Room";
        case RPG_REVERB_TYPE_FACTORY_SHORTPASSAGE: return "Factory - Short Passage";
        case RPG_REVERB_TYPE_FACTORY_MEDIUMROOM: return "Factory - Medium Room";
        case RPG_REVERB_TYPE_FACTORY_LARGEROOM: return "Factory - Large Room";
        case RPG_REVERB_TYPE_FACTORY_LONGPASSAGE: return "Factory - Long Passage";
        case RPG_REVERB_TYPE_FACTORY_HALL: return "Factory - Hall";
        case RPG_REVERB_TYPE_FACTORY_CUPBOARD: return "Factory - Cupboard";
        case RPG_REVERB_TYPE_FACTORY_COURTYARD: return "Factory - Courtyard";
        case RPG_REVERB_TYPE_FACTORY_ALCOVE: return "Factory - Alcove";
        /* Ice Palace Presets */
        case RPG_REVERB_TYPE_ICEPALACE_SMALLROOM: return "Ice Palace - Small Room";
        case RPG_REVERB_TYPE_ICEPALACE_SHORTPASSAGE: return "Ice Palace - Short Passage";
        case RPG_REVERB_TYPE_ICEPALACE_MEDIUMROOM: return "Ice Palace - Medium Room";
        case RPG_REVERB_TYPE_ICEPALACE_LARGEROOM: return "Ice Palace - Large Room";
        case RPG_REVERB_TYPE_ICEPALACE_LONGPASSAGE: return "Ice Palace - Long Passage";
        case RPG_REVERB_TYPE_ICEPALACE_HALL: return "Ice Palace - Hall";
        case RPG_REVERB_TYPE_ICEPALACE_CUPBOARD: return "Ice Palace - Cupboard";
        case RPG_REVERB_TYPE_ICEPALACE_COURTYARD: return "Ice Palace - Courtyard";
        case RPG_REVERB_TYPE_ICEPALACE_ALCOVE: return "Ice Palace - Alcove";
        /* Space Station Presets */
        case RPG_REVERB_TYPE_SPACESTATION_SMALLROOM: return "Spacestation - Small Room";
        case RPG_REVERB_TYPE_SPACESTATION_SHORTPASSAGE: return "Spacestation - Short Passage";
        case RPG_REVERB_TYPE_SPACESTATION_MEDIUMROOM: return "Spacestation - Medium Room";
        case RPG_REVERB_TYPE_SPACESTATION_LARGEROOM: return "Spacestation - Large Room";
        case RPG_REVERB_TYPE_SPACESTATION_LONGPASSAGE: return "Spacestation - Long Passage";
        case RPG_REVERB_TYPE_SPACESTATION_HALL: return "Spacestation - Hall";
        case RPG_REVERB_TYPE_SPACESTATION_CUPBOARD: return "Spacestation - Cupboard";
        case RPG_REVERB_TYPE_SPACESTATION_ALCOVE: return "Spacestation - Alcove";
        /* Wooden Galleon Presets */
        case RPG_REVERB_TYPE_WOODEN_SMALLROOM: return "Wooden - Small Room";
        case RPG_REVERB_TYPE_WOODEN_SHORTPASSAGE: return "Wooden - Short Passage";
        case RPG_REVERB_TYPE_WOODEN_MEDIUMROOM: return "Wooden - Medium Room";
        case RPG_REVERB_TYPE_WOODEN_LARGEROOM: return "Wooden - Large Room";
        case RPG_REVERB_TYPE_WOODEN_LONGPASSAGE: return "Wooden - Long Passage";
        case RPG_REVERB_TYPE_WOODEN_HALL: return "Wooden - Hall";
        case RPG_REVERB_TYPE_WOODEN_CUPBOARD: return "Wooden - Cupboard";
        case RPG_REVERB_TYPE_WOODEN_COURTYARD: return "Wooden - Courtyard";
        case RPG_REVERB_TYPE_WOODEN_ALCOVE: return "Wooden - Alcove";
        /* Sports Presets */
        case RPG_REVERB_TYPE_SPORT_EMPTYSTADIUM: return "Sport - Empty Stadium";
        case RPG_REVERB_TYPE_SPORT_SQUASHCOURT: return "Sport - Squash Court";
        case RPG_REVERB_TYPE_SPORT_SMALLSWIMMINGPOOL: return "Sport - Small Swimming Pool";
        case RPG_REVERB_TYPE_SPORT_LARGESWIMMINGPOOL: return "Sport - Large Swimming Pool";
        case RPG_REVERB_TYPE_SPORT_GYMNASIUM: return "Sport - Gymnasium";
        case RPG_REVERB_TYPE_SPORT_FULLSTADIUM: return "Sport - Full Stadium";
        case RPG_REVERB_TYPE_SPORT_STADIUMTANNOY: return "Sport - Stadium Tannoy";
        /* Prefab Presets */
        case RPG_REVERB_TYPE_PREFAB_WORKSHOP: return "Prefab - Workshop";
        case RPG_REVERB_TYPE_PREFAB_SCHOOLROOM: return "Prefab - Schoolroom";
        case RPG_REVERB_TYPE_PREFAB_PRACTISEROOM: return "Prefab - Practice Room";
        case RPG_REVERB_TYPE_PREFAB_OUTHOUSE: return "Prefab - Outhouse";
        case RPG_REVERB_TYPE_PREFAB_CARAVAN: return "Prefab - Caravan";
        /* Dome and Pipe Presets */
        case RPG_REVERB_TYPE_DOME_TOMB: return "Dome - Tomb";
        case RPG_REVERB_TYPE_PIPE_SMALL: return "Pipe - Small";
        case RPG_REVERB_TYPE_DOME_SAINTPAULS: return "Dome - Saint Paul;s";
        case RPG_REVERB_TYPE_PIPE_LONGTHIN: return "Pipe - Long and Thin";
        case RPG_REVERB_TYPE_PIPE_LARGE: return "Pipe - Large";
        case RPG_REVERB_TYPE_PIPE_RESONANT: return "Pipe - Resonant";
        /* Outdoors Presets */
        case RPG_REVERB_TYPE_OUTDOORS_BACKYARD: return "Outdoors - Backyard";
        case RPG_REVERB_TYPE_OUTDOORS_ROLLINGPLAINS: return "Outdoors - Rolling Plains";
        case RPG_REVERB_TYPE_OUTDOORS_DEEPCANYON: return "Outdoors - Deep Canyon";
        case RPG_REVERB_TYPE_OUTDOORS_CREEK: return "Outdoors - Creek";
        case RPG_REVERB_TYPE_OUTDOORS_VALLEY: return "Outdoors - Valley";
        /* Mood Presets */
        case RPG_REVERB_TYPE_MOOD_HEAVEN: return "Mood - Heaven";
        case RPG_REVERB_TYPE_MOOD_HELL: return "Mood - Hell";
        case RPG_REVERB_TYPE_MOOD_MEMORY: return "Mood - Memory";
        /* Driving Presets */
        case RPG_REVERB_TYPE_DRIVING_COMMENTATOR: return "Driving - Commentator";
        case RPG_REVERB_TYPE_DRIVING_PITGARAGE: return "Driving - Pit Garage";
        case RPG_REVERB_TYPE_DRIVING_INCAR_RACER: return "Driving - Interior Racer";
        case RPG_REVERB_TYPE_DRIVING_INCAR_SPORTS: return "Driving - Interior Sports";
        case RPG_REVERB_TYPE_DRIVING_INCAR_LUXURY: return "Driving - Interior Luxury";
        case RPG_REVERB_TYPE_DRIVING_FULLGRANDSTAND: return "Driving - Full Grandstand";
        case RPG_REVERB_TYPE_DRIVING_EMPTYGRANDSTAND: return "Driving - Empty Grandstand";
        case RPG_REVERB_TYPE_DRIVING_TUNNEL: return "Driving - Tunnel";
        /* City Presets */
        case RPG_REVERB_TYPE_CITY_STREETS: return "City - Streets";
        case RPG_REVERB_TYPE_CITY_SUBWAY: return "City - Subway";
        case RPG_REVERB_TYPE_CITY_MUSEUM: return "City - Museum";
        case RPG_REVERB_TYPE_CITY_LIBRARY: return "City - Library";
        case RPG_REVERB_TYPE_CITY_UNDERPASS: return "City - Underpass";
        case RPG_REVERB_TYPE_CITY_ABANDONED: return "City - Abandoned";
        /* Misc. Presets */
        case RPG_REVERB_TYPE_DUSTYROOM: return "Dusty Room";
        case RPG_REVERB_TYPE_CHAPEL: return "Chapel";
        case RPG_REVERB_TYPE_SMALLWATERROOM: return "Small Water Room";
        default: return "";
    }
}

RPG_RESULT RPG_Reverb_GetPresetDescription(RPG_REVERB_TYPE type, char *buffer, RPGsize sizeBuffer, RPGsize *written) {
    if (sizeBuffer == 0) {
        *written = 0;
        return RPG_NO_ERROR;
    }
    if (buffer == NULL) {
        *written = 0;
        return RPG_ERR_INVALID_POINTER;
    }
    if (type < 0 || type > RPG_REVERB_TYPE_LAST) {
        *written = 0;
        return RPG_ERR_OUT_OF_RANGE;
    }
    memset(buffer, 0, sizeBuffer);
    const char *desc = RPG_Reverb_GetPresetDescription_S(type);
    RPGsize sz       = strlen(desc);
    *written         = sz > sizeBuffer ? sizeBuffer : sz;
    memcpy(buffer, desc, *written);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Reverb_SetPreset(RPGaudiofx *e, RPGreverbpreset *p) {
    RPG_RETURN_IF_NULL(e);
    RPG_RETURN_IF_NULL(p);
    if (e->type == AL_EFFECT_EAXREVERB) {
        // OpenAL-Soft reverb
        alEffectf(e->effect, AL_EAXREVERB_DENSITY, p->density);
        alEffectf(e->effect, AL_EAXREVERB_DIFFUSION, p->diffusion);
        alEffectf(e->effect, AL_EAXREVERB_GAIN, p->gain);
        alEffectf(e->effect, AL_EAXREVERB_GAINHF, p->gainHF);
        alEffectf(e->effect, AL_EAXREVERB_GAINLF, p->gainLF);
        alEffectf(e->effect, AL_EAXREVERB_DECAY_TIME, p->decayTime);
        alEffectf(e->effect, AL_EAXREVERB_DECAY_HFRATIO, p->decayHFRatio);
        alEffectf(e->effect, AL_EAXREVERB_DECAY_LFRATIO, p->decayLFRatio);
        alEffectf(e->effect, AL_EAXREVERB_REFLECTIONS_GAIN, p->reflectionsGain);
        alEffectf(e->effect, AL_EAXREVERB_REFLECTIONS_DELAY, p->reflectionsDelay);
        alEffectfv(e->effect, AL_EAXREVERB_REFLECTIONS_PAN, p->reflectionsPan);
        alEffectf(e->effect, AL_EAXREVERB_LATE_REVERB_GAIN, p->lateReverbGain);
        alEffectf(e->effect, AL_EAXREVERB_LATE_REVERB_DELAY, p->lateReverbDelay);
        alEffectfv(e->effect, AL_EAXREVERB_LATE_REVERB_PAN, p->lateReverbPan);
        alEffectf(e->effect, AL_EAXREVERB_ECHO_TIME, p->echoTime);
        alEffectf(e->effect, AL_EAXREVERB_ECHO_DEPTH, p->echoDepth);
        alEffectf(e->effect, AL_EAXREVERB_MODULATION_TIME, p->modulationTime);
        alEffectf(e->effect, AL_EAXREVERB_MODULATION_DEPTH, p->modulationDepth);
        alEffectf(e->effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, p->airAbsorptionGainHF);
        alEffectf(e->effect, AL_EAXREVERB_HFREFERENCE, p->referenceHF);
        alEffectf(e->effect, AL_EAXREVERB_LFREFERENCE, p->referenceLF);
        alEffectf(e->effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, p->roomRolloffFactor);
        alEffecti(e->effect, AL_EAXREVERB_DECAY_HFLIMIT, p->decayHFLimit);
    } else {
        // Standard Reverb
        alEffectf(e->effect, AL_REVERB_DENSITY, p->density);
        alEffectf(e->effect, AL_REVERB_DIFFUSION, p->diffusion);
        alEffectf(e->effect, AL_REVERB_GAIN, p->gain);
        alEffectf(e->effect, AL_REVERB_GAINHF, p->gainHF);
        alEffectf(e->effect, AL_REVERB_DECAY_TIME, p->decayTime);
        alEffectf(e->effect, AL_REVERB_DECAY_HFRATIO, p->decayHFRatio);
        alEffectf(e->effect, AL_REVERB_REFLECTIONS_GAIN, p->reflectionsGain);
        alEffectf(e->effect, AL_REVERB_REFLECTIONS_DELAY, p->reflectionsDelay);
        alEffectf(e->effect, AL_REVERB_LATE_REVERB_GAIN, p->lateReverbGain);
        alEffectf(e->effect, AL_REVERB_LATE_REVERB_DELAY, p->lateReverbDelay);
        alEffectf(e->effect, AL_REVERB_AIR_ABSORPTION_GAINHF, p->airAbsorptionGainHF);
        alEffectf(e->effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, p->roomRolloffFactor);
        alEffecti(e->effect, AL_REVERB_DECAY_HFLIMIT, p->decayHFLimit);
    }
    RPG_Audio_UpdateEffectChange(e);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Reverb_CreateFromType(RPG_REVERB_TYPE type, RPGaudiofx **reverb) {
    RPGaudiofx *r;
    if (RPG_Audio_CreateEffect(AL_EFFECT_EAXREVERB, &r)) {
        return RPG_ERR_AUDIO_EXT;
    }
    RPGreverbpreset p = RPG_Reverb_GetPreset_S(type);
    RPG_Reverb_SetPreset(r, &p);
    *reverb = r;
    return (type < 0 || type > RPG_REVERB_TYPE_LAST) ? RPG_ERR_OUT_OF_RANGE : RPG_NO_ERROR;
}

RPG_RESULT RPG_Reverb_CreateFromPreset(RPGreverbpreset *preset, RPGaudiofx **reverb) {
    RPGaudiofx *r;
    if (RPG_Audio_CreateEffect(AL_EFFECT_EAXREVERB, &r)) {
        return RPG_ERR_AUDIO_EXT;
    }
    *reverb = r;
    return RPG_Reverb_SetPreset(r, preset);
}

RPG_RESULT RPG_Reverb_GetPresetFromType(RPG_REVERB_TYPE type, RPGreverbpreset *preset) {
    RPGreverbpreset result = RPG_Reverb_GetPreset_S(type);
    memcpy(preset, &result, sizeof(RPGreverbpreset));
    return (type < 0 || type > RPG_REVERB_TYPE_LAST) ? RPG_ERR_OUT_OF_RANGE : RPG_NO_ERROR;
}

// Reverb
DEF_FX_CREATE(Reverb, AL_EFFECT_REVERB)
DEF_FX_PARAM_F(Reverb, Density, AL_EAXREVERB_DENSITY, AL_EAXREVERB_MIN_DENSITY, AL_EAXREVERB_MAX_DENSITY)
DEF_FX_PARAM_F(Reverb, Diffusion, AL_EAXREVERB_DIFFUSION, AL_EAXREVERB_MIN_DIFFUSION, AL_EAXREVERB_MAX_DIFFUSION)
DEF_FX_PARAM_F(Reverb, Gain, AL_EAXREVERB_GAIN, AL_EAXREVERB_MIN_GAIN, AL_EAXREVERB_MAX_GAIN)
DEF_FX_PARAM_F(Reverb, GainHF, AL_EAXREVERB_GAINHF, AL_EAXREVERB_MIN_GAINHF, AL_EAXREVERB_MAX_GAINHF)
DEF_FX_PARAM_F(Reverb, GainLF, AL_EAXREVERB_GAINLF, AL_EAXREVERB_MIN_GAINLF, AL_EAXREVERB_MAX_GAINLF)
DEF_FX_PARAM_F(Reverb, DecayTime, AL_EAXREVERB_DECAY_TIME, AL_EAXREVERB_MIN_DECAY_TIME, AL_EAXREVERB_MAX_DECAY_TIME)
DEF_FX_PARAM_F(Reverb, RatioHF, AL_EAXREVERB_DECAY_HFRATIO, AL_EAXREVERB_MIN_DECAY_HFRATIO, AL_EAXREVERB_MAX_DECAY_HFRATIO)
DEF_FX_PARAM_F(Reverb, RatioLF, AL_EAXREVERB_DECAY_LFRATIO, AL_EAXREVERB_MIN_DECAY_LFRATIO, AL_EAXREVERB_MAX_DECAY_LFRATIO)
DEF_FX_PARAM_F(Reverb, ReflectionsGain, AL_EAXREVERB_REFLECTIONS_GAIN, AL_EAXREVERB_MIN_REFLECTIONS_GAIN, AL_EAXREVERB_MAX_REFLECTIONS_GAIN)
DEF_FX_PARAM_F(Reverb, ReflectionsDelay, AL_EAXREVERB_REFLECTIONS_DELAY, AL_EAXREVERB_MIN_REFLECTIONS_DELAY,
               AL_EAXREVERB_MAX_REFLECTIONS_DELAY)
DEF_FX_PARAM_F(Reverb, LateGain, AL_EAXREVERB_LATE_REVERB_GAIN, AL_EAXREVERB_MIN_LATE_REVERB_GAIN, AL_EAXREVERB_MAX_LATE_REVERB_GAIN)
DEF_FX_PARAM_F(Reverb, LateDelay, AL_EAXREVERB_LATE_REVERB_DELAY, AL_EAXREVERB_MIN_LATE_REVERB_DELAY, AL_EAXREVERB_MAX_LATE_REVERB_DELAY)
DEF_FX_PARAM_F(Reverb, EchoTime, AL_EAXREVERB_ECHO_TIME, AL_EAXREVERB_MIN_ECHO_TIME, AL_EAXREVERB_MAX_ECHO_TIME)
DEF_FX_PARAM_F(Reverb, EchoDepth, AL_EAXREVERB_ECHO_DEPTH, AL_EAXREVERB_MIN_ECHO_DEPTH, AL_EAXREVERB_MAX_ECHO_DEPTH)
DEF_FX_PARAM_F(Reverb, ModulationTime, AL_EAXREVERB_MODULATION_TIME, AL_EAXREVERB_MIN_MODULATION_TIME, AL_EAXREVERB_MAX_MODULATION_TIME)
DEF_FX_PARAM_F(Reverb, ModulationDepth, AL_EAXREVERB_MODULATION_DEPTH, AL_EAXREVERB_MIN_MODULATION_DEPTH, AL_EAXREVERB_MAX_MODULATION_DEPTH)
DEF_FX_PARAM_F(Reverb, AirAbsorption, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF,
               AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF)
DEF_FX_PARAM_F(Reverb, ReferenceHF, AL_EAXREVERB_HFREFERENCE, AL_EAXREVERB_MIN_HFREFERENCE, AL_EAXREVERB_MAX_HFREFERENCE)
DEF_FX_PARAM_F(Reverb, ReferenceLF, AL_EAXREVERB_LFREFERENCE, AL_EAXREVERB_MIN_LFREFERENCE, AL_EAXREVERB_MAX_LFREFERENCE)
DEF_FX_PARAM_F(Reverb, RoomRolloff, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, AL_EAXREVERB_MIN_ROOM_ROLLOFF_FACTOR,
               AL_EAXREVERB_MAX_ROOM_ROLLOFF_FACTOR)
DEF_FX_PARAM_I(Reverb, DecayLimit, AL_EAXREVERB_DECAY_HFLIMIT, RPG_FALSE, RPG_TRUE)
DEF_FX_PARAM_V(Reverb, ReflectionsPan, AL_EAXREVERB_REFLECTIONS_PAN)
DEF_FX_PARAM_V(Reverb, LatePan, AL_EAXREVERB_LATE_REVERB_PAN)

// CHorus
DEF_FX_CREATE(Chorus, AL_EFFECT_CHORUS)
DEF_FX_PARAM_I(Chorus, Waveform, AL_CHORUS_WAVEFORM, AL_CHORUS_MIN_WAVEFORM, AL_CHORUS_MAX_WAVEFORM)
DEF_FX_PARAM_I(Chorus, Phase, AL_CHORUS_PHASE, AL_CHORUS_MIN_PHASE, AL_CHORUS_MAX_PHASE)
DEF_FX_PARAM_F(Chorus, Rate, AL_CHORUS_RATE, AL_CHORUS_MIN_RATE, AL_CHORUS_MAX_RATE)
DEF_FX_PARAM_F(Chorus, Depth, AL_CHORUS_DEPTH, AL_CHORUS_MIN_DEPTH, AL_CHORUS_MAX_DEPTH)
DEF_FX_PARAM_F(Chorus, Feedback, AL_CHORUS_FEEDBACK, AL_CHORUS_MIN_FEEDBACK, AL_CHORUS_MAX_FEEDBACK)
DEF_FX_PARAM_F(Chorus, Delay, AL_CHORUS_DELAY, AL_CHORUS_MIN_DELAY, AL_CHORUS_MAX_DELAY)

// Distortion
DEF_FX_CREATE(Distortion, AL_EFFECT_DISTORTION)
DEF_FX_PARAM_F(Distortion, Edge, AL_DISTORTION_EDGE, AL_DISTORTION_MIN_EDGE, AL_DISTORTION_MAX_EDGE)
DEF_FX_PARAM_F(Distortion, Gain, AL_DISTORTION_GAIN, AL_DISTORTION_MIN_GAIN, AL_DISTORTION_MAX_GAIN)
DEF_FX_PARAM_F(Distortion, LowpassCutoff, AL_DISTORTION_LOWPASS_CUTOFF, AL_DISTORTION_MIN_LOWPASS_CUTOFF, AL_DISTORTION_MAX_LOWPASS_CUTOFF)
DEF_FX_PARAM_F(Distortion, CenterEQ, AL_DISTORTION_EQCENTER, AL_DISTORTION_MIN_EQCENTER, AL_DISTORTION_MAX_EQCENTER)
DEF_FX_PARAM_F(Distortion, BandwidthEQ, AL_DISTORTION_EQBANDWIDTH, AL_DISTORTION_MIN_EQBANDWIDTH, AL_DISTORTION_MAX_EQBANDWIDTH)

// Echo
DEF_FX_CREATE(Echo, AL_EFFECT_ECHO)
DEF_FX_PARAM_F(Echo, Delay, AL_ECHO_DELAY, AL_ECHO_MIN_DELAY, AL_ECHO_MAX_DELAY)
DEF_FX_PARAM_F(Echo, LRDelay, AL_ECHO_LRDELAY, AL_ECHO_MIN_LRDELAY, AL_ECHO_MAX_LRDELAY)
DEF_FX_PARAM_F(Echo, Damping, AL_ECHO_DAMPING, AL_ECHO_MIN_DAMPING, AL_ECHO_MAX_DAMPING)
DEF_FX_PARAM_F(Echo, Feedback, AL_ECHO_FEEDBACK, AL_ECHO_MIN_FEEDBACK, AL_ECHO_MAX_FEEDBACK)
DEF_FX_PARAM_F(Echo, Spread, AL_ECHO_SPREAD, AL_ECHO_MIN_SPREAD, AL_ECHO_MAX_SPREAD)

// Flanger
DEF_FX_CREATE(Flanger, AL_EFFECT_FLANGER)
DEF_FX_PARAM_I(Flanger, Waveform, AL_FLANGER_WAVEFORM, AL_FLANGER_MIN_WAVEFORM, AL_FLANGER_MAX_WAVEFORM)
DEF_FX_PARAM_I(Flanger, Phase, AL_FLANGER_PHASE, AL_FLANGER_MIN_PHASE, AL_FLANGER_MAX_PHASE)
DEF_FX_PARAM_F(Flanger, Rate, AL_FLANGER_RATE, AL_FLANGER_MIN_RATE, AL_FLANGER_MAX_RATE)
DEF_FX_PARAM_F(Flanger, Depth, AL_FLANGER_DEPTH, AL_FLANGER_MIN_DEPTH, AL_FLANGER_MAX_DEPTH)
DEF_FX_PARAM_F(Flanger, Feedback, AL_FLANGER_FEEDBACK, AL_FLANGER_MIN_FEEDBACK, AL_FLANGER_MAX_FEEDBACK)
DEF_FX_PARAM_F(Flanger, Delay, AL_FLANGER_DELAY, AL_FLANGER_MIN_DELAY, AL_FLANGER_MAX_DELAY)

// FrequencyShifter
DEF_FX_CREATE(FrequencyShifter, AL_EFFECT_FREQUENCY_SHIFTER)
DEF_FX_PARAM_F(FrequencyShifter, Frequency, AL_FREQUENCY_SHIFTER_FREQUENCY, AL_FREQUENCY_SHIFTER_MIN_FREQUENCY,
               AL_FREQUENCY_SHIFTER_MAX_FREQUENCY)
DEF_FX_PARAM_I(FrequencyShifter, Left, AL_FREQUENCY_SHIFTER_LEFT_DIRECTION, AL_FREQUENCY_SHIFTER_MIN_RIGHT_DIRECTION,
               AL_FREQUENCY_SHIFTER_MAX_LEFT_DIRECTION)
DEF_FX_PARAM_I(FrequencyShifter, Right, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION, AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION,
               AL_FREQUENCY_SHIFTER_MAX_RIGHT_DIRECTION)

// Vocal Morpher
DEF_FX_CREATE(VocalMorpher, AL_EFFECT_VOCAL_MORPHER)
DEF_FX_PARAM_I(VocalMorpher, PhonemeA, AL_VOCAL_MORPHER_PHONEMEA, AL_VOCAL_MORPHER_MIN_PHONEMEA, AL_VOCAL_MORPHER_MAX_PHONEMEA)
DEF_FX_PARAM_I(VocalMorpher, PhonemeB, AL_VOCAL_MORPHER_PHONEMEB, AL_VOCAL_MORPHER_MIN_PHONEMEB, AL_VOCAL_MORPHER_MAX_PHONEMEB)
DEF_FX_PARAM_I(VocalMorpher, CoarseA, AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING, AL_VOCAL_MORPHER_MIN_PHONEMEA_COARSE_TUNING,
               AL_VOCAL_MORPHER_MAX_PHONEMEA_COARSE_TUNING)
DEF_FX_PARAM_I(VocalMorpher, CoarseB, AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING, AL_VOCAL_MORPHER_MIN_PHONEMEB_COARSE_TUNING,
               AL_VOCAL_MORPHER_MAX_PHONEMEB_COARSE_TUNING)
DEF_FX_PARAM_I(VocalMorpher, Waveform, AL_VOCAL_MORPHER_WAVEFORM, AL_VOCAL_MORPHER_MIN_WAVEFORM, AL_VOCAL_MORPHER_MAX_WAVEFORM)
DEF_FX_PARAM_F(VocalMorpher, Rate, AL_VOCAL_MORPHER_RATE, AL_VOCAL_MORPHER_MIN_RATE, AL_VOCAL_MORPHER_MAX_RATE)

// PitchShifter
DEF_FX_CREATE(PitchShifter, AL_EFFECT_PITCH_SHIFTER)
DEF_FX_PARAM_I(PitchShifter, Coarse, AL_PITCH_SHIFTER_COARSE_TUNE, AL_PITCH_SHIFTER_MIN_COARSE_TUNE, AL_PITCH_SHIFTER_MAX_COARSE_TUNE)
DEF_FX_PARAM_I(PitchShifter, Fine, AL_PITCH_SHIFTER_FINE_TUNE, AL_PITCH_SHIFTER_MIN_FINE_TUNE, AL_PITCH_SHIFTER_MAX_FINE_TUNE)

// RingModulator
DEF_FX_CREATE(RingModulator, AL_EFFECT_RING_MODULATOR)
DEF_FX_PARAM_F(RingModulator, Frequency, AL_RING_MODULATOR_FREQUENCY, AL_RING_MODULATOR_MIN_FREQUENCY, AL_RING_MODULATOR_MAX_FREQUENCY)
DEF_FX_PARAM_F(RingModulator, Cutoff, AL_RING_MODULATOR_HIGHPASS_CUTOFF, AL_RING_MODULATOR_MIN_HIGHPASS_CUTOFF,
               AL_RING_MODULATOR_MAX_HIGHPASS_CUTOFF)
DEF_FX_PARAM_I(RingModulator, Waveform, AL_RING_MODULATOR_WAVEFORM, AL_RING_MODULATOR_MIN_WAVEFORM, AL_RING_MODULATOR_MAX_WAVEFORM)

// AutoWah
DEF_FX_CREATE(AutoWah, AL_EFFECT_AUTOWAH)
DEF_FX_PARAM_F(AutoWah, Attack, AL_AUTOWAH_ATTACK_TIME, AL_AUTOWAH_MIN_ATTACK_TIME, AL_AUTOWAH_MAX_ATTACK_TIME)
DEF_FX_PARAM_F(AutoWah, Release, AL_AUTOWAH_RELEASE_TIME, AL_AUTOWAH_MIN_RELEASE_TIME, AL_AUTOWAH_MAX_RELEASE_TIME)
DEF_FX_PARAM_F(AutoWah, Resonance, AL_AUTOWAH_RESONANCE, AL_AUTOWAH_MIN_RESONANCE, AL_AUTOWAH_MAX_RESONANCE)
DEF_FX_PARAM_F(AutoWah, PeakGain, AL_AUTOWAH_PEAK_GAIN, AL_AUTOWAH_MIN_PEAK_GAIN, AL_AUTOWAH_MAX_PEAK_GAIN)

// Compressor
DEF_FX_CREATE(Compressor, AL_EFFECT_COMPRESSOR)
DEF_FX_PARAM_I(Compressor, Enabled, AL_COMPRESSOR_ONOFF, RPG_FALSE, RPG_TRUE)

// Equalizer
DEF_FX_CREATE(Equalizer, AL_EFFECT_EQUALIZER)
DEF_FX_PARAM_F(Equalizer, LowGain, AL_EQUALIZER_LOW_GAIN, AL_EQUALIZER_MIN_LOW_GAIN, AL_EQUALIZER_MAX_LOW_GAIN)
DEF_FX_PARAM_F(Equalizer, LowCutoff, AL_EQUALIZER_LOW_CUTOFF, AL_EQUALIZER_MIN_LOW_CUTOFF, AL_EQUALIZER_MAX_LOW_CUTOFF)
DEF_FX_PARAM_F(Equalizer, Mid1Gain, AL_EQUALIZER_MID1_GAIN, AL_EQUALIZER_MIN_MID1_GAIN, AL_EQUALIZER_MAX_MID1_GAIN)
DEF_FX_PARAM_F(Equalizer, Mid1Center, AL_EQUALIZER_MID1_CENTER, AL_EQUALIZER_MIN_MID1_CENTER, AL_EQUALIZER_MAX_MID1_CENTER)
DEF_FX_PARAM_F(Equalizer, Mid1Width, AL_EQUALIZER_MID1_WIDTH, AL_EQUALIZER_MIN_MID1_WIDTH, AL_EQUALIZER_MAX_MID1_WIDTH)
DEF_FX_PARAM_F(Equalizer, Mid2Gain, AL_EQUALIZER_MID2_GAIN, AL_EQUALIZER_MIN_MID2_GAIN, AL_EQUALIZER_MAX_MID2_GAIN)
DEF_FX_PARAM_F(Equalizer, Mid2Center, AL_EQUALIZER_MID2_CENTER, AL_EQUALIZER_MIN_MID2_CENTER, AL_EQUALIZER_MAX_MID2_CENTER)
DEF_FX_PARAM_F(Equalizer, Mid2Width, AL_EQUALIZER_MID2_WIDTH, AL_EQUALIZER_MIN_MID2_WIDTH, AL_EQUALIZER_MAX_MID2_WIDTH)
DEF_FX_PARAM_F(Equalizer, HighGain, AL_EQUALIZER_HIGH_GAIN, AL_EQUALIZER_MIN_HIGH_GAIN, AL_EQUALIZER_MAX_HIGH_GAIN)
DEF_FX_PARAM_F(Equalizer, HighCutoff, AL_EQUALIZER_HIGH_CUTOFF, AL_EQUALIZER_MIN_HIGH_CUTOFF, AL_EQUALIZER_MAX_HIGH_CUTOFF)

#endif /* RPG_AUDIO_NO_EFFECTS */
#endif /* RPG_NO_AUDIO */