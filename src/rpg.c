//
// Created by eric on 8/20/19.
//

#include "rpg.h"
#include "audio.h"

void RPG_Initialize(void) {
    RPG_Audio_Initialize();
    glfwInit();

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

}

void RPG_Terminate(void) {
    RPG_Audio_Terminate();
    glfwTerminate();
}

const char *RPG_ErrorString(RPG_RESULT result)
{
    switch (result) {
        case RPG_NO_ERROR: return "No error.";
        case RPG_ERR_AUDIO_DEVICE: return "Failed to open audio device.";
        case RPG_ERR_AUDIO_CONTEXT: return "An error occurred creating or setting the audio context.";
        default: return "Unknown error.";
    }
}