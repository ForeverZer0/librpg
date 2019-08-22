
#include <math.h>
#include <string.h>

#include "AL/al.h"
#include "AL/alext.h"

#include "internal.h"
#include "sndfile.h"

ALCcontext *context;
ALCdevice *device;

#define RPG_IS_VALID_SLOT(i) (i >= 0 && i < RPG_AUDIO_MAX_SLOTS && audioSlots[i] != NULL)
#define AL_LOAD_PROC(x, y) ((x) = (y)alGetProcAddress(#x))
#define BUFFER_COUNT 3
#define BUFFER_SIZE 32768

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

typedef struct RPG_SOUND {
    SNDFILE *file;
    SF_INFO info;
    char * filename;
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
} RPG_SOUND;

typedef struct RPG_AUDIO_SLOT {
    ALuint source;
    ALuint buffers[BUFFER_COUNT];
    void *pcm;
    RPGint loopCount;
    RPG_SOUND *sound;
    pthread_t thread;
} RPG_AUDIO_SLOT;

#define RPG_AUDIO_EACH_SLOT(var,i) \
RPG_AUDIO_SLOT *var = audioSlots[0]; \
for (int i = 0; i < RPG_AUDIO_MAX_SLOTS; i++, var = audioSlots[i])

RPG_AUDIO_SLOT *audioSlots[RPG_AUDIO_MAX_SLOTS];


static inline void RPG_Audio_SoundSeek(RPG_SOUND *sound, sf_count_t position, int whence) {
    pthread_mutex_unlock(&sound->mutex);
    sf_seek(sound->file, position, whence);
    pthread_mutex_unlock(&sound->mutex);
}

static void RPG_Audio_FreeSound(RPG_SOUND *sound) {
    if (sound) {
        pthread_mutex_lock(&sound->mutex);
        sf_close(sound->file);
        if (sound->filename) {
            RPG_FREE(sound->filename);
        }
        pthread_mutex_destroy(&sound->mutex);
        RPG_FREE(sound);
    }
}

static void RPG_Audio_FreeSlot(RPGint index) {
    if (index < 0 || index >= RPG_AUDIO_MAX_SLOTS) {
        return;
    }
    RPG_AUDIO_SLOT *slot = audioSlots[index];
    if (slot) {
        alSourceStop(slot->source);
        alDeleteSources(1, &slot->source);
        alDeleteBuffers(BUFFER_COUNT, slot->buffers);
        RPG_FREE(slot->pcm);
        audioSlots[index] = NULL;
    }
}

static void RPG_Audio_SetALFormat(RPG_SOUND *sound) {

    int type = sound->info.format & SF_FORMAT_SUBMASK;
    switch (type) {
        case SF_FORMAT_PCM_S8:
        case SF_FORMAT_PCM_U8:
        case SF_FORMAT_FLAC:
            sound->al.format = sound->info.channels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
            sound->al.itemsize = sizeof(ALbyte) * sound->info.channels;
            sound->func.readframes = sf_read_raw;
            break;
        case SF_FORMAT_PCM_16:
            sound->al.format = sound->info.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
            sound->al.itemsize = sizeof(ALshort) * sound->info.channels;
            sound->func.sf.readshort = sf_readf_short;
            break;
        case SF_FORMAT_DOUBLE:
            sound->al.format = sound->info.channels == 1 ? AL_FORMAT_MONO_DOUBLE_EXT : AL_FORMAT_STEREO_DOUBLE_EXT;
            sound->al.itemsize = sizeof(ALdouble) * sound->info.channels;
            sound->func.sf.readdouble = sf_readf_double;
            break;
        case SF_FORMAT_PCM_32:
        case SF_FORMAT_VORBIS:
        case SF_FORMAT_FLOAT:
            sound->al.format = sound->info.channels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
            sound->al.itemsize = sizeof(ALfloat) * sound->info.channels;
            sound->func.sf.readfloat = sf_readf_float;
            break;
        default:
            // TODO: Warn we are just winging it here?
            sound->al.format = sound->info.channels == 1 ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32;
            sound->al.itemsize = sizeof(ALfloat) * sound->info.channels;
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

static RPG_RESULT RPG_Audio_CreateSound(const char *filename, RPG_SOUND **sound) {
    *sound = NULL;
    RPG_ENSURE_FILE(filename);
    RPG_ALLOC_ZERO(snd, RPG_SOUND);
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

static RPGbool RPG_Audio_TryGetSlot(RPGint index, RPG_AUDIO_SLOT **slot) {
    if (index < 0 || index >= RPG_AUDIO_MAX_SLOTS) {
        *slot = NULL;
        return RPG_FALSE;
    }
    if (audioSlots[index] == NULL) {
        RPG_ALLOC_ZERO(s, RPG_AUDIO_SLOT);
        alGenSources(1, &s->source);
        alGenBuffers(BUFFER_COUNT, s->buffers);
        s->pcm = RPG_MALLOC(BUFFER_SIZE);
        audioSlots[index] = s;
    }
    *slot = audioSlots[index];
    return RPG_TRUE;
}

static RPGbool RPG_Channel_FillBuffer(RPG_AUDIO_SLOT *slot, ALuint buffer) {
    if (!alIsSource(slot->source) || slot->sound == NULL) {
        return RPG_TRUE;
    }
    pthread_mutex_lock(&slot->sound->mutex);
    RPG_SOUND *snd = slot->sound;
    sf_count_t n = BUFFER_SIZE / snd->al.itemsize;
    sf_count_t size = snd->func.readframes(snd->file, slot->pcm, n) * snd->al.itemsize;
    alBufferData(buffer, AL_FORMAT_STEREO_FLOAT32, slot->pcm, (ALsizei)size, snd->info.samplerate);
    if (size == 0) {
        sf_seek(snd->file, 0, SF_SEEK_SET);
        pthread_mutex_unlock(&slot->sound->mutex);
        if (slot->loopCount > 0) {
            slot->loopCount--;
            return AL_FALSE;
        } else if (slot->loopCount < 0) {
            return AL_FALSE;
        }
    } else {
        pthread_mutex_unlock(&slot->sound->mutex);
    }
    return size == 0;
}

static void *RPG_Audio_Stream(void *slot) {
    RPG_AUDIO_SLOT *s = slot;

    for (int i = 0; i < BUFFER_COUNT; i++) {
        RPG_Channel_FillBuffer(s, s->buffers[i]);
    }
    alSourceQueueBuffers(s->source, BUFFER_COUNT, s->buffers);
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
    return NULL;
}

RPG_RESULT RPG_Audio_Initialize(void) {

    device = alcOpenDevice(NULL);
    RPG_ASSERT(device);
    if (device == NULL) {
        return RPG_ERR_AUDIO_DEVICE;
    }
    context = alcCreateContext(device, NULL);
    RPG_ASSERT(context);
    if (context == NULL) {
        return RPG_ERR_AUDIO_CONTEXT;
    }
    if (!alcMakeContextCurrent(context)) {
        return RPG_ERR_AUDIO_CONTEXT;
    }

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

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_Terminate(void) {
    for (int i = 0; i < RPG_AUDIO_MAX_SLOTS; i++) {
        RPG_Audio_FreeSlot(i);
    }
    alcDestroyContext(context);
    return alcCloseDevice(device) ? RPG_NO_ERROR : RPG_ERR_AUDIO_DEVICE;
}

RPG_RESULT RPG_Audio_Play(RPGint index, const char *filename, RPGfloat volume, RPGfloat pitch, RPGint loopCount) {
    if (index < 0 || index >= RPG_AUDIO_MAX_SLOTS) {
        return RPG_ERR_OUT_OF_RANGE;
    }
    RPG_RETURN_IF_NULL(filename);
    RPG_AUDIO_SLOT *slot;
    if (!RPG_Audio_TryGetSlot(index, &slot)) {
        return RPG_ERR_MEMORY;
    }

    if (slot->sound) {
        if (strcasecmp(filename, slot->sound->filename) == 0) {
            // Same sound is already loaded in slot
            ALint state;
            alGetSourcei(slot->source, AL_SOURCE_STATE, &state);
            if (state == AL_PLAYING) {
                // Do nothing if already playing same sound
                return RPG_NO_ERROR;
            } else if (state == AL_PAUSED) {
                // Resume play and return if paused
                alSourcePlay(slot->source);
                return RPG_NO_ERROR;
            } 
        } else {
            // Different sound, remove it
            alSourceStop(slot->source);
            RPG_Audio_FreeSound(slot->sound);
            slot->sound = NULL;
        }
    }

    slot->loopCount = loopCount;
    alSourcef(slot->source, AL_GAIN, RPG_CLAMPF(volume, 0.0f, 1.0f));
    alSourcef(slot->source, AL_PITCH, fmaxf(pitch, 0.0f)); // TODO: Greater than 0.0f ?
    if (slot->sound == NULL) {
        RPG_RESULT result = RPG_Audio_CreateSound(filename, &slot->sound);
        if (result != RPG_NO_ERROR) {
            return result;
        }
        // Begin streaming on separate thread
        if (pthread_create(&slot->thread, NULL, RPG_Audio_Stream, slot)) {
            return RPG_ERR_THREAD_FAILURE;
        }
    }

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_GetVolume(RPGint slot, RPGfloat *volume) {
    if (RPG_IS_VALID_SLOT(slot)) {
        alGetSourcef(audioSlots[slot]->source, AL_GAIN, volume);
        return RPG_NO_ERROR;
    }
    *volume = 0.0f;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_SetVolume(RPGint slot, RPGfloat volume) {
    if (RPG_IS_VALID_SLOT(slot)) {
        alSourcef(audioSlots[slot]->source, AL_GAIN, RPG_CLAMPF(volume, 0.0f, 1.0f));
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetPitch(RPGint slot, RPGfloat *pitch) {
    if (RPG_IS_VALID_SLOT(slot)) {
        alGetSourcef(audioSlots[slot]->source, AL_PITCH, pitch);
        return RPG_NO_ERROR;
    }
    *pitch = 0.0f;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_SetPitch(RPGint slot, RPGfloat pitch) {
    if (RPG_IS_VALID_SLOT(slot)) {
        alSourcef(audioSlots[slot]->source, AL_PITCH, fmaxf(0.0f, pitch));
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetSampleRate(RPGint slot, RPGint *rate) {
    if (RPG_IS_VALID_SLOT(slot) && audioSlots[slot]->sound != NULL) {
        *rate = audioSlots[slot]->sound->info.samplerate;
        return RPG_NO_ERROR;
    }
    *rate = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetChannelCount(RPGint slot, RPGint *count) {
    if (RPG_IS_VALID_SLOT(slot) && audioSlots[slot]->sound != NULL) {
        *count = audioSlots[slot]->sound->info.channels;
        return RPG_NO_ERROR;
    }
    *count = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetSampleCount(RPGint slot, RPGint *count) {
    if (RPG_IS_VALID_SLOT(slot) && audioSlots[slot]->sound != NULL) {
        *count = audioSlots[slot]->sound->info.frames;
        return RPG_NO_ERROR;
    }
    *count = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetSectionCount(RPGint slot, RPGint *count) {
    if (RPG_IS_VALID_SLOT(slot) && audioSlots[slot]->sound != NULL) {
        *count = audioSlots[slot]->sound->info.sections;
        return RPG_NO_ERROR;
    }
    *count = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetFormat(RPGint slot, RPG_SOUND_FORMAT *format) {
    if (RPG_IS_VALID_SLOT(slot) && audioSlots[slot]->sound != NULL) {
        *format = (RPG_SOUND_FORMAT)(audioSlots[slot]->sound->info.format & SF_FORMAT_TYPEMASK);
        return RPG_NO_ERROR;
    }
    *format = RPG_SOUND_FORMAT_NONE;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetType(RPGint slot, RPG_SOUND_TYPE *type) {
    if (RPG_IS_VALID_SLOT(slot) && audioSlots[slot]->sound != NULL) {
        *type = (RPG_SOUND_TYPE)(audioSlots[slot]->sound->info.format & SF_FORMAT_SUBMASK);
        return RPG_NO_ERROR;
    }
    *type = RPG_SOUND_TYPE_NONE;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetDuration(RPGint slot, RPGint64 *milliseconds) {
    if (RPG_IS_VALID_SLOT(slot) && audioSlots[slot]->sound != NULL) {
        RPG_SOUND *snd = audioSlots[slot]->sound;
        *milliseconds = (RPGint64)round((1000.0 / snd->info.samplerate) * snd->info.frames);
        return RPG_NO_ERROR;
    }
    *milliseconds = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetInfo(RPGint slot, RPG_SOUND_INFO type, char *buffer, size_t bufferSize, size_t *written) {
    if (bufferSize == 0) {
        return RPG_NO_ERROR;
    }
    if (buffer == NULL) {
        return RPG_ERR_NULL_POINTER;
    }
    memset(buffer, 0, bufferSize);
    if (RPG_IS_VALID_SLOT(slot) && audioSlots[slot]->sound != NULL) {
        const char *str = sf_get_string(audioSlots[slot]->sound->file, type);
        if (str != NULL) {
            size_t sz = strlen(str) + 1;
            *written = bufferSize > sz ? sz : bufferSize;
            memcpy(buffer, str, *written);
        }
        return RPG_NO_ERROR;
    }
    *written = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetPlaybackState(RPGint slot, RPG_PLAYBACK_STATE *state) {
    if (RPG_IS_VALID_SLOT(slot)) {
        ALint s;
        alGetSourcei(audioSlots[slot]->source, AL_SOURCE_STATE, &s);
        switch (s) {
            case AL_PLAYING:
                *state = RPG_PLAYBACK_STATE_PLAYING;
                break;
            case AL_PAUSED:
                *state = RPG_PLAYBACK_STATE_PAUSED;
                break;
            default:
                *state = RPG_PLAYBACK_STATE_STOPPED;
                break;
        }
        return RPG_NO_ERROR;
    } 
    *state = RPG_PLAYBACK_STATE_STOPPED;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetLoopCount(RPGint slot, RPGint *count) {
    *count = RPG_IS_VALID_SLOT(slot) ? audioSlots[slot]->loopCount : 0;
     return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_SetLoopCount(RPGint slot, RPGint count) {
    RPG_AUDIO_SLOT *s;
    if (RPG_Audio_TryGetSlot(slot, &s)) {
        s->loopCount = count;
        return RPG_NO_ERROR;
    }
    return RPG_ERR_MEMORY;
}

RPG_RESULT RPG_Audio_Resume(RPGint slot) {
    if (RPG_IS_VALID_SLOT(slot)) {
        alSourcePlay(audioSlots[slot]->source);
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_Stop(RPGint slot) {
    if (RPG_IS_VALID_SLOT(slot)) {
        alSourceStop(audioSlots[slot]->source);
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_Pause(RPGint slot) {
    if (RPG_IS_VALID_SLOT(slot)) {
        alSourcePause(audioSlots[slot]->source);
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_Seek(RPGint slot, RPGint64 position, RPG_AUDIO_SEEK seek) {
    if (RPG_IS_VALID_SLOT(slot) && audioSlots[slot]->sound) {
        RPG_AUDIO_SLOT *s = audioSlots[slot];
        sf_count_t pos;
        switch (seek) {
            case RPG_AUDIO_SEEK_MS:
                pos = (sf_count_t) round(position * (s->sound->info.samplerate / 1000.0));
                break;
            case RPG_AUDIO_SEEK_SAMPLES:
                pos = position;
                break;
            default:
                return RPG_ERR_INVALID_VALUE;
        }
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