
#include "audio.r"

static RPGint RPG_Channel_GetAvailableChannel(void) {
    for (int i = 0; i < RPG_MAX_AUDIO_CHANNELS; i++) {
        if (channels[i] == NULL) {
            return i;
        }
    }
    return -1;
}

static RPGbool RPG_Channel_FillBuffer(RPG_CHANNEL *channel, ALuint buffer) {
    if (alIsSource(channel->source)) {
        return RPG_TRUE;
    }
    void *pcm = pcmBuffers[channel->index];

    // size_t n = BUFFER_SIZE / channel->al.itemsize;
    const size_t itemsize = channel->sound->info.channels * sizeof(float);
    const size_t n = BUFFER_SIZE / itemsize;

    pthread_mutex_lock(&channel->sound->mutex);
    ALsizei size = sf_readf_float(channel->sound->file, pcm, n) * itemsize;
    // ALsizei size = snd->func.readframes(snd->file, pcm, n) * snd->al.itemsize;
    alBufferData(buffer, AL_FORMAT_STEREO_FLOAT32, ((void*) pcm), size, channel->sound->info.samplerate);

    pthread_mutex_unlock(&channel->sound->mutex);
    if (size == 0 && channel->loop > 0) {
        sf_seek(channel->sound->file, 0, SF_SEEK_SET);
        channel->loop--;
        return AL_FALSE;
    }
    return size == 0;
}

static void *RPG_Channel_Stream(void *channel) {
    RPG_CHANNEL *c = channel;
    
    for (int i = 0; i < BUFFER_COUNT; i++) {
        RPG_Channel_FillBuffer(channel, c->buffers[i]);
    }
    alSourceQueueBuffers(c->source, BUFFER_COUNT, c->buffers);
    alSourcePlay(c->source);

    RPGbool done;
    ALint processed;
    ALuint buffer;
    while (!done) {
        if (alIsSource(c->source)) {
            RPG_SLEEP(100);
        } else {
            break;
        }
        alGetSourcei(c->source, AL_BUFFERS_PROCESSED, &processed);
        while (processed--) {
		    alSourceUnqueueBuffers(c->source, 1, &buffer);
            if (RPG_Channel_FillBuffer(c, buffer)) {
                done = RPG_TRUE;
                break;
            }
            alSourceQueueBuffers(c->source, 1, &buffer);
        }
    }
    // If sound needs freed, wait until all buffers are processed;
    // if (!snd->owned) {
    //     ALint processed;
    //     while (processed < BUFFER_COUNT && alIsSource(snd->source)) {
    //         RPG_SLEEP(BUFFER_CHECK_MS);
    //         alGetSourcei(snd->source, AL_BUFFERS_PROCESSED, &processed);
    //     }
    //     rpg_sound_free(snd);
    // }

    return NULL;
}


RPG_RESULT RPG_Channel_Play(RPG_CHANNEL *channel) {
    RPG_RETURN_IF_NULL(channel);
    if (!alIsSource(channel->source)) {
        return RPG_ERR_INVALID_POINTER;
    }

    pthread_t thread;
    if (pthread_create(&thread, NULL, RPG_Channel_Stream, channel)) {
        return RPG_ERR_THREAD_FAILURE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Channel_GetPlaybackState(RPG_CHANNEL *channel, RPG_PLAYBACK_STATE *state) {
    RPG_RETURN_IF_NULL(channel);
    ALint s;
    alGetSourcei(channel->source, AL_SOURCE_STATE, &s);
    if (AL_INVALID_NAME == alGetError()) {
        *state = RPG_PLAYBACK_STATE_UNKNOWN;
        return RPG_ERR_INVALID_POINTER;
    }
    switch (s) {
        case AL_PLAYING:
            *state = RPG_PLAYBACK_STATE_PLAYING;
            break;
        case AL_PAUSED:
            *state = RPG_PLAYBACK_STATE_PAUSED;
            break;
        case AL_STOPPED:
            *state = RPG_PLAYBACK_STATE_STOPPED;
            break;
        default:
            *state = RPG_PLAYBACK_STATE_UNKNOWN;
            break;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Channel_GetLoopCount(RPG_CHANNEL *channel, RPGint *count) {
     RPG_RETURN_IF_NULL(channel);
     *count = channel->loop;
     return RPG_NO_ERROR;
}

RPG_RESULT RPG_Channel_SetLoopCount(RPG_CHANNEL *channel, RPGint count) {
    RPG_RETURN_IF_NULL(channel);
    if (channel->sound) {
        if (!channel->sound->info.seekable) {
            return RPG_ERR_AUDIO_CANNOT_SEEK;
        }
    }
    channel->loop = count;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Channel_AddEffect(RPG_CHANNEL *channel, RPG_AUDIOFX *fx) {
    // TODO: Implement
}

RPG_RESULT RPG_Channel_AddEffects(RPG_CHANNEL *channel, RPGint count, RPG_AUDIOFX **fx) {
    // TODO: Implement
}

RPG_RESULT RPG_Channel_GetVolume(RPG_CHANNEL *channel, RPGfloat *volume) {
    RPG_RETURN_IF_NULL(channel);
    RPG_RETURN_IF_NULL(volume);
    alGetSourcef(channel->source, AL_GAIN, volume);
    return (AL_INVALID_NAME == alGetError()) ? RPG_ERR_INVALID_POINTER : AL_NO_ERROR;
}

RPG_RESULT RPG_Channel_SetVolume(RPG_CHANNEL *channel, RPGfloat volume) {
    RPG_RETURN_IF_NULL(channel);
    alSourcef(channel->source, AL_GAIN, RPG_CLAMPF(volume, 0.0f, 1.0f));
    return (AL_INVALID_NAME == alGetError()) ? RPG_ERR_INVALID_POINTER : AL_NO_ERROR;
}

RPG_RESULT RPG_Channel_GetPitch(RPG_CHANNEL *channel, RPGfloat *pitch) {
    RPG_RETURN_IF_NULL(channel);
    RPG_RETURN_IF_NULL(pitch);
    alGetSourcef(channel->source, AL_PITCH, pitch);
    return (AL_INVALID_NAME == alGetError()) ? RPG_ERR_INVALID_POINTER : AL_NO_ERROR;
}

RPG_RESULT RPG_Channel_SetPitch(RPG_CHANNEL *channel, RPGfloat pitch) {
    RPG_RETURN_IF_NULL(channel);
    alSourcef(channel->source, AL_GAIN, fmaxf(pitch, 0.0f));
    return (AL_INVALID_NAME == alGetError()) ? RPG_ERR_INVALID_POINTER : AL_NO_ERROR;
}

RPG_RESULT RPG_Channel_Create(RPG_SOUND *sound, RPGint fxCount, RPG_AUDIOFX **fx, RPG_CHANNEL **channel) {
    RPG_RETURN_IF_NULL(sound);
    RPG_RETURN_IF_NULL(*channel);

    RPGint index = RPG_Channel_GetAvailableChannel();
    if (index < 0) {
        return RPG_ERR_AUDIO_NOT_ENOUGH_CHANNELS;
    }

    RPG_CHANNEL *c = RPG_ALLOC(RPG_CHANNEL);
    memset(c, 0, sizeof(RPG_CHANNEL));
    alGenSources(1, &c->source);
    alGenBuffers(BUFFER_COUNT, c->buffers);

    if (fxCount > 0) {
        RPG_RETURN_IF_NULL(*fx);
        c->aux.count = fxCount;
        c->aux.slots = RPG_ALLOC_N(RPG_AUXSLOT, fxCount);
        for (int i = 0; i < fxCount; i++) {
            RPG_AUXSLOT *slot = &c->aux.slots[i];
            slot->effect = fx[i]->effect;
            alGenAuxiliaryEffectSlots(1, &slot->slot);
            alAuxiliaryEffectSloti(slot->slot, AL_EFFECTSLOT_EFFECT, fx[i]->effect);
            alSource3i(c->source, AL_AUXILIARY_SEND_FILTER, slot->slot, i, AL_FILTER_NULL);
        }
    }
    c->sound = sound;
    c->index = index;
    channels[index] = c;
    *channel = c;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Channel_Free(RPG_CHANNEL *channel) {
    if (channel) {
        alSourceStop(channel->source);
        alDeleteBuffers(BUFFER_COUNT, channel->buffers);
        alDeleteSources(1, &channel->source);
        if (channel->aux.count > 0 && channel->aux.slots) {
            for (int i = 0; i < channel->aux.count; i++) {
                alDeleteAuxiliaryEffectSlots(1, &channel->aux.slots[i].slot);
            }
        }
        channels[channel->index] = NULL;
        RPG_FREE(channel);
    }
    return RPG_NO_ERROR;
}