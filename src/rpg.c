//
// Created by eric on 8/20/19.
//

#include "rpg.h"
#include "graphics.h"

void RPG_Initialize(void) {
    RPG_Audio_Initialize();
    RPG_Graphics_Initialize();
}

void RPG_Terminate(void) {
    RPG_Audio_Terminate();
    RPG_Graphics_Terminate();
}

const char *RPG_ErrorString(RPG_RESULT result) {
    switch (result) {
        case RPG_NO_ERROR: return "No error.";
        case RPG_ERR_AUDIO_DEVICE: return "Failed to open audio device.";
        case RPG_ERR_AUDIO_CONTEXT: return "An error occurred creating or setting the audio context.";
        default: return "Unknown error.";
    }
}