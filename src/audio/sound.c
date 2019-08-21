
#include "audio.r"
#include <string.h>

static RPG_RESULT RPG_Sound_GetError(int error) {
    switch (error) {
        case SF_ERR_NO_ERROR:               return RPG_NO_ERROR;
        case SF_ERR_UNRECOGNISED_FORMAT:    return RPG_ERR_FORMAT;
        case SF_ERR_SYSTEM:                 return RPG_ERR_SYSTEM;
        case SF_ERR_UNSUPPORTED_ENCODING:   return RPG_ERR_ENCODING;
        case SF_ERR_MALFORMED_FILE:         return RPG_ERR_MALFORMED;
        default:                            return RPG_ERR_UNKNOWN;
    }
}

RPG_RESULT RPG_Sound_Create(const char *path, RPG_SOUND **sound) {
    RPG_ENSURE_FILE(path);
    RPG_SOUND *snd = RPG_ALLOC(RPG_SOUND);
    snd->file = sf_open(path, SFM_READ, &snd->info);
    int error = sf_error(snd->file);
    if (error) {
        sf_close(snd->file); // TODO: ????
        *sound = NULL;
        return RPG_Sound_GetError(error);
    } else {
        pthread_mutex_init(&snd->mutex, NULL);
        *sound = snd;
        return RPG_NO_ERROR;
    }
}

RPG_RESULT RPG_Sound_Free(RPG_SOUND *sound) {
    if (sound) {
        for (int i = 0; i < RPG_MAX_AUDIO_CHANNELS; i++) {
            if (channels[i] && channels[i]->sound == sound) {
                alSourceStop(channels[i]->source);
                channels[i]->sound = NULL;
            }
        }
        RPG_Sound_GetError(sf_close(sound->file));
        pthread_mutex_destroy(&sound->mutex);
        RPG_FREE(sound);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sound_GetSampleRate(RPG_SOUND *sound, RPGint *rate) {
    RPG_RETURN_IF_NULL(sound);
    *rate = sound->info.samplerate;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sound_GetChannelCount(RPG_SOUND *sound, RPGint *count) {
    RPG_RETURN_IF_NULL(sound);
    *count = sound->info.channels;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sound_GetFormat(RPG_SOUND *sound, RPG_SOUND_FORMAT *format) {
    RPG_RETURN_IF_NULL(sound);
    *format = (RPG_SOUND_FORMAT)(sound->info.format & SF_FORMAT_TYPEMASK);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sound_GetType(RPG_SOUND *sound, RPG_SOUND_TYPE *type) {
    RPG_RETURN_IF_NULL(sound);
    *type = (RPG_SOUND_TYPE)(sound->info.format & SF_FORMAT_SUBMASK);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sound_GetSampleCount(RPG_SOUND *sound, RPGint *count) {
    RPG_RETURN_IF_NULL(sound);
    *count = sound->info.frames;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sound_GetSectionCount(RPG_SOUND *sound, RPGint *count) {
    RPG_RETURN_IF_NULL(sound);
    *count = sound->info.sections;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sound_GetDuration(RPG_SOUND *sound, RPGint64 *milliseconds) {
    RPG_RETURN_IF_NULL(sound);
    *milliseconds = (RPGint64) round((1000.0 / sound->info.samplerate) * sound->info.frames);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Sound_GetInfo(RPG_SOUND *sound, RPG_SOUND_INFO type, char *buffer, size_t bufferSize, size_t *size) {
    RPG_RETURN_IF_NULL(sound);
    if (bufferSize == 0) {
        return RPG_NO_ERROR;
    }
    memset(buffer, 0, bufferSize);
    const char *str = sf_get_string(sound->file, type);
    if (str) {
        size_t sz = strlen(str);
        *size = bufferSize > sz ? sz : bufferSize;
        if (buffer) {
            memcpy(buffer, str, *size);
        }
    } else {
        *size = 0;
    }
    return RPG_NO_ERROR;
}