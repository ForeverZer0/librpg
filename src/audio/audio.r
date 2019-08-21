#ifndef OPEN_RPG_AUDIO_INTERNAL_H
#define OPEN_RPG_AUDIO_INTERNAL_H 1

#include <math.h>

#include "AL/al.h"
#include "AL/alext.h"

#include "audio.h"
#include "internal.h"
#include "sndfile.h"
#include "pthread.h"

#define BUFFER_COUNT 4
#define BUFFER_SIZE 16384

RPG_CHANNEL *channels[RPG_MAX_AUDIO_CHANNELS];
void *pcmBuffers[RPG_MAX_AUDIO_CHANNELS];

typedef struct RPG_SOUND {
    SNDFILE *file;
    SF_INFO info;
    pthread_mutex_t mutex;
} RPG_SOUND;

typedef struct RPG_AUDIOFX {
    ALint type;
    ALuint effect;
} RPG_AUDIOFX;

typedef struct RPG_AUXSLOT {
    ALuint effect;
    ALuint slot;
} RPG_AUXSLOT;

typedef struct RPG_CHANNEL {
    ALubyte index;
    ALuint source;
    ALuint buffers[BUFFER_COUNT];
    RPG_SOUND *sound;
    struct {
        ALubyte count;
        RPG_AUXSLOT *slots;
    } aux;
    RPGint loop;
} RPG_CHANNEL;

/* Auxiliary Effect Slot object functions */
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;
LPALGENEFFECTS alGenEffects;
LPALDELETEEFFECTS alDeleteEffects;
LPALISEFFECT alIsEffect;
LPALEFFECTI alEffecti;
LPALEFFECTIV alEffectiv;
LPALEFFECTF alEffectf;
LPALEFFECTFV alEffectfv;
LPALGETEFFECTI alGetEffecti;
LPALGETEFFECTIV alGetEffectiv;
LPALGETEFFECTF alGetEffectf;
LPALGETEFFECTFV alGetEffectfv;

#endif /* OPEN_RPG_AUDIO_INTERNAL_H */