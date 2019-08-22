
#include <math.h>
#include <string.h>

//#include "AL/al.h"
//#include "AL/alext.h"

#include "internal.h"
#include "sndfile.h"

#define RPG_MAX_CHANNELS 32
#define RPG_VALID_CHANNEL(i) (i >= 0 && i < RPG_AUDIO_MAX_SLOTS && audioSlots[i] != NULL)
#define BUFFER_COUNT 3
#define BUFFER_SIZE 32768

typedef struct RPGsound {
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
} RPGsound;

typedef struct RPGchannel {
    ALuint source;
    ALuint buffers[BUFFER_COUNT];
    void *pcm;
    RPGint loopCount;
    RPGsound *sound;
    pthread_t thread;
    struct {
        RPGint num;
        ALuint *effects;
        ALuint *slots;
    } aux;
} RPGchannel;

#define RPG_AUDIO_EACH_SLOT(var,i) \
RPGchannel *var = audioSlots[0]; \
for (int i = 0; i < RPG_AUDIO_MAX_SLOTS; i++, var = audioSlots[i])

RPGchannel *audioSlots[RPG_AUDIO_MAX_SLOTS];


static inline void RPG_Audio_SoundSeek(RPGsound *sound, sf_count_t position, int whence) {
    pthread_mutex_unlock(&sound->mutex);
    sf_seek(sound->file, position, whence);
    pthread_mutex_unlock(&sound->mutex);
}

static void RPG_Audio_FreeSound(RPGsound *sound) {
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

RPG_RESULT RPG_Audio_FreeChannel(RPGint index) {
    if (index < 0 || index >= RPG_AUDIO_MAX_SLOTS) {
        return RPG_ERR_OUT_OF_RANGE;
    }
    RPGchannel *channel = audioSlots[index];
    if (channel) {
        alSourceStop(channel->source);
        alDeleteSources(1, &channel->source);
        alDeleteBuffers(BUFFER_COUNT, channel->buffers);
        RPG_FREE(channel->pcm);
        audioSlots[index] = NULL;
    }
    return RPG_NO_ERROR;
}

static void RPG_Audio_SetALFormat(RPGsound *sound) {

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
    if (index < 0 || index >= RPG_AUDIO_MAX_SLOTS) {
        *channel = NULL;
        return RPG_FALSE;
    }
    if (audioSlots[index] == NULL) {
        RPG_ALLOC_ZERO(s, RPGchannel);
        alGenSources(1, &s->source);
        alGenBuffers(BUFFER_COUNT, s->buffers);
        s->pcm = RPG_MALLOC(BUFFER_SIZE);
        audioSlots[index] = s;
    }
    *channel = audioSlots[index];
    return RPG_TRUE;
}

static RPGbool RPG_Channel_FillBuffer(RPGchannel *channel, ALuint buffer) {
    if (!alIsSource(channel->source) || channel->sound == NULL) {
        return RPG_TRUE;
    }
    pthread_mutex_lock(&channel->sound->mutex);
    RPGsound *snd = channel->sound;
    sf_count_t n = BUFFER_SIZE / snd->al.itemsize;
    sf_count_t size = snd->func.readframes(snd->file, channel->pcm, n) * snd->al.itemsize;
    alBufferData(buffer, AL_FORMAT_STEREO_FLOAT32, channel->pcm, (ALsizei)size, snd->info.samplerate);
    if (size == 0) {
        sf_seek(snd->file, 0, SF_SEEK_SET);
        pthread_mutex_unlock(&channel->sound->mutex);
        if (channel->loopCount > 0) {
            channel->loopCount--;
            return AL_FALSE;
        } else if (channel->loopCount < 0) {
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


RPG_RESULT RPG_Audio_Play(RPGint index, const char *filename, RPGfloat volume, RPGfloat pitch, RPGint loopCount) {
    if (index < 0 || index >= RPG_AUDIO_MAX_SLOTS) {
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
        alGetSourcef(audioSlots[channel]->source, AL_GAIN, volume);
        return RPG_NO_ERROR;
    }
    *volume = 0.0f;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_SetVolume(RPGint channel, RPGfloat volume) {
    if (RPG_VALID_CHANNEL(channel)) {
        alSourcef(audioSlots[channel]->source, AL_GAIN, RPG_CLAMPF(volume, 0.0f, 1.0f));
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetPitch(RPGint channel, RPGfloat *pitch) {
    if (RPG_VALID_CHANNEL(channel)) {
        alGetSourcef(audioSlots[channel]->source, AL_PITCH, pitch);
        return RPG_NO_ERROR;
    }
    *pitch = 0.0f;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_SetPitch(RPGint channel, RPGfloat pitch) {
    if (RPG_VALID_CHANNEL(channel)) {
        alSourcef(audioSlots[channel]->source, AL_PITCH, fmaxf(0.0f, pitch));
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetSampleRate(RPGint channel, RPGint *rate) {
    if (RPG_VALID_CHANNEL(channel) && audioSlots[channel]->sound != NULL) {
        *rate = audioSlots[channel]->sound->info.samplerate;
        return RPG_NO_ERROR;
    }
    *rate = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetChannelCount(RPGint channel, RPGint *count) {
    if (RPG_VALID_CHANNEL(channel) && audioSlots[channel]->sound != NULL) {
        *count = audioSlots[channel]->sound->info.channels;
        return RPG_NO_ERROR;
    }
    *count = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetSampleCount(RPGint channel, RPGint *count) {
    if (RPG_VALID_CHANNEL(channel) && audioSlots[channel]->sound != NULL) {
        *count = audioSlots[channel]->sound->info.frames;
        return RPG_NO_ERROR;
    }
    *count = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetSectionCount(RPGint channel, RPGint *count) {
    if (RPG_VALID_CHANNEL(channel) && audioSlots[channel]->sound != NULL) {
        *count = audioSlots[channel]->sound->info.sections;
        return RPG_NO_ERROR;
    }
    *count = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetFormat(RPGint channel, RPG_SOUND_FORMAT *format) {
    if (RPG_VALID_CHANNEL(channel) && audioSlots[channel]->sound != NULL) {
        *format = (RPG_SOUND_FORMAT)(audioSlots[channel]->sound->info.format & SF_FORMAT_TYPEMASK);
        return RPG_NO_ERROR;
    }
    *format = RPG_SOUND_FORMAT_NONE;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetType(RPGint channel, RPG_SOUND_TYPE *type) {
    if (RPG_VALID_CHANNEL(channel) && audioSlots[channel]->sound != NULL) {
        *type = (RPG_SOUND_TYPE)(audioSlots[channel]->sound->info.format & SF_FORMAT_SUBMASK);
        return RPG_NO_ERROR;
    }
    *type = RPG_SOUND_TYPE_NONE;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetDuration(RPGint channel, RPGint64 *milliseconds) {
    if (RPG_VALID_CHANNEL(channel) && audioSlots[channel]->sound != NULL) {
        RPGsound *snd = audioSlots[channel]->sound;
        *milliseconds = (RPGint64)round((1000.0 / snd->info.samplerate) * snd->info.frames);
        return RPG_NO_ERROR;
    }
    *milliseconds = 0;
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_GetInfo(RPGint channel, RPG_SOUND_INFO type, char *buffer, size_t bufferSize, size_t *written) {
    if (bufferSize == 0) {
        return RPG_NO_ERROR;
    }
    if (buffer == NULL) {
        return RPG_ERR_NULL_POINTER;
    }
    memset(buffer, 0, bufferSize);
    if (RPG_VALID_CHANNEL(channel) && audioSlots[channel]->sound != NULL) {
        const char *str = sf_get_string(audioSlots[channel]->sound->file, type);
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

RPG_RESULT RPG_Audio_GetPlaybackState(RPGint channel, RPG_PLAYBACK_STATE *state) {
    if (RPG_VALID_CHANNEL(channel)) {
        ALint s;
        alGetSourcei(audioSlots[channel]->source, AL_SOURCE_STATE, &s);
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

RPG_RESULT RPG_Audio_GetLoopCount(RPGint channel, RPGint *count) {
    *count = RPG_VALID_CHANNEL(channel) ? audioSlots[channel]->loopCount : 0;
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
        alSourcePlay(audioSlots[channel]->source);
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_Stop(RPGint channel) {
    if (RPG_VALID_CHANNEL(channel)) {
        alSourceStop(audioSlots[channel]->source);
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_Pause(RPGint channel) {
    if (RPG_VALID_CHANNEL(channel)) {
        alSourcePause(audioSlots[channel]->source);
        return RPG_NO_ERROR;
    }
    return RPG_ERR_AUDIO_NO_SOUND;
}

RPG_RESULT RPG_Audio_Seek(RPGint channel, RPGint64 position, RPG_AUDIO_SEEK seek) {
    if (RPG_VALID_CHANNEL(channel) && audioSlots[channel]->sound) {
        RPGchannel *s = audioSlots[channel];
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

RPGbool RPG_Audio_GetIsEffectAttached(RPGchannel *c, RPGaudiofx *fx) {
    for (int i = 0; i < c->aux.num; i++) {
        if (c->aux.effects[i] == fx->effect) {
            return RPG_TRUE;
        }
    }
    return RPG_FALSE;
}

RPG_RESULT RPG_Audio_AttachEffect(RPGint channel, RPGaudiofx *fx) {
    RPGchannel *c;
    if (RPG_Audio_TryGetSlot(channel, &c)) {
        if (RPG_Audio_GetIsEffectAttached(c, fx)) {
            return RPG_NO_ERROR;
        }
        ALuint slot;
        alGenAuxiliaryEffectSlots(1, &slot);
        alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, fx->effect);
        if (c->aux.num == 0) {
            c->aux.effects = RPG_ALLOC(ALuint);
            c->aux.slots = RPG_ALLOC(ALuint);
        } else {
            size_t sz = sizeof(ALuint) * (c->aux.num + 1);
            c->aux.effects = RPG_REALLOC(c->aux.effects, sz);
            c->aux.slots = RPG_REALLOC(c->aux.slots, sz);
        }
        
        c->aux.slots[c->aux.num] = slot;
        c->aux.effects[c->aux.num] = fx->effect;
        alSource3i(c->source, AL_AUXILIARY_SEND_FILTER, slot, c->aux.num, AL_FILTER_NULL);
        printf("8: %d\n", alGetError());
        c->aux.num++;
        return RPG_NO_ERROR;
    }
    return RPG_ERR_MEMORY;
}

RPG_RESULT RPG_Audio_DetachEffect(RPGint channel, RPGaudiofx *fx) {

}
