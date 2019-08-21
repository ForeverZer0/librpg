
#include "audio.r"

ALCcontext *context;
ALCdevice *device;

RPG_RESULT RPG_Audio_PlaySound(RPG_SOUND *sound, RPGfloat gain, RPGfloat pitch, RPGint loop, RPG_CHANNEL **channel) {
    RPG_RETURN_IF_NULL(sound);

    RPG_CHANNEL *c;
    RPG_RESULT result = RPG_Channel_Create(sound, 0, NULL, &c);

    if (result) {
        if (*channel) {
            *channel = c;
        }
        RPG_Channel_SetLoopCount(c, loop);
        alSourcef(c->source, AL_GAIN, RPG_CLAMPF(gain, 0.0f, 1.0f));
        alSourcef(c->source, AL_PITCH, fmaxf(0.0f, pitch));
        return RPG_Channel_Play(c);
    }    
    *channel = NULL;
    return result;
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

    #define AL_LOAD_PROC(x, y) ((x) = (y)alGetProcAddress(#x))
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

    for (int i = 0; i < RPG_MAX_AUDIO_CHANNELS; i++) {
        pcmBuffers[i] = RPG_MALLOC((size_t) BUFFER_SIZE);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Audio_Terminate(void) {
    alcDestroyContext(context);
    for (int i = 0; i < RPG_MAX_AUDIO_CHANNELS; i++) {
        if (channels[i]) {
            RPG_Channel_Free(channels[i]);
        }
        RPG_FREE(pcmBuffers[i]);
    }
    return alcCloseDevice(device) ? RPG_NO_ERROR : RPG_ERR_AUDIO_DEVICE;
}