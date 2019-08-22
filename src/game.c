#include "rpg.h"
#include "glad.h"
#include "GLFW/glfw3.h"
#include "internal.h"
#include <string.h>

int errorCode;

const char *RPG_GetErrorString(RPG_RESULT result) {
    switch (result) { // TODO:
        case RPG_NO_ERROR: return "No error.";
        case RPG_ERR_AUDIO_DEVICE: return "Failed to open audio device.";
        case RPG_ERR_AUDIO_CONTEXT: return "An error occurred creating or setting the audio context.";
        default: return "Unknown error.";
    }
}

typedef struct RPGgame {
    GLFWwindow *window;
    struct {
        RPGint width;
        RPGint height;
    } resolution;
    RPGbool autoAspect;
    struct {
        RPGint x, y, w, h;
    } bounds;
    RPG_INIT_FLAGS flags;
    char *title;
    void *user;
    struct {
        RPGfloat r, g, b, a;
    } color;
    struct {
        ALCcontext *context;
        ALCdevice *device;
    } al;
} RPGgame;

// GLFWwindow *gameWindow;

static void RPG_Game_ErrorCallback(int error, const char *msg) {
    errorCode = error;
}

static RPG_RESULT RPG_Game_GetError(void) {
    switch (errorCode) {
        case GLFW_NO_ERROR: return RPG_NO_ERROR;
        default: return RPG_ERR_UNKNOWN; // Change to generic "graphics failed" type error
    }
}

RPG_RESULT RPG_Game_Destroy(RPGgame *game) {
    glfwTerminate();
    for (int i = 0; i < RPG_AUDIO_MAX_SLOTS; i++) {
        RPG_Audio_FreeChannel(i);
    }
    alcDestroyContext(game->al.context);
    return alcCloseDevice(game->al.device) ? RPG_NO_ERROR : RPG_ERR_AUDIO_DEVICE;
}

RPG_RESULT RPG_Game_Create(RPGgame **game) {
    RPG_ALLOC_ZERO(g, RPGgame); // TODO: free if failure
    glfwSetErrorCallback(RPG_Game_ErrorCallback);
    if (glfwInit()) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);   
    } else {
        return RPG_Game_GetError();
    }

    g->al.device = alcOpenDevice(NULL);
    RPG_ASSERT(g->al.device);
    if (g->al.device == NULL) {
        return RPG_ERR_AUDIO_DEVICE;
    }
    g->al.context = alcCreateContext(g->al.device, NULL);
    RPG_ASSERT(g->al.context);
    if (g->al.context == NULL) {
        return RPG_ERR_AUDIO_CONTEXT;
    }
    if (!alcMakeContextCurrent(g->al.context)) {
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

    *game = g;
    return RPG_NO_ERROR;
}


RPG_RESULT RPG_Game_CreateWindow(RPGgame *game, const char *title, RPGint width, RPGint height, RPG_INIT_FLAGS flags) {
    
    if (width < 1 || height < 1) {
        return RPG_ERR_OUT_OF_RANGE;
    }


    game->resolution.width = width;
    game->resolution.height = height;
    game->flags = flags;
    if (title) {
        size_t sz = strlen(title) + 1;
        game->title = RPG_MALLOC(sz);
        memcpy(game->title, title, sz);
    }

    glfwWindowHint(GLFW_RESIZABLE, (flags & RPG_INIT_RESIZABLE) != 0);
    glfwWindowHint(GLFW_DECORATED, (flags & RPG_INIT_DECORATED) != 0);
    glfwWindowHint(GLFW_VISIBLE, (flags & RPG_INIT_HIDDEN) == 0);
    GLFWmonitor *monitor = ((flags & RPG_INIT_FULLSCREEN) != 0) ? glfwGetPrimaryMonitor() : NULL;

    // Create graphics context
    game->window = glfwCreateWindow(width, height, title, monitor, NULL);
    if (game->window == NULL) {
        RPG_FREE(game);
        return RPG_Game_GetError();
    }
    // Position window if necessary
    if ((flags & RPG_INIT_CENTERED) != 0 && monitor == NULL) {
        monitor = glfwGetPrimaryMonitor();
        int x, y, w, h;
        glfwGetMonitorWorkarea(monitor, &x, &y, &w, &h);
        glfwSetWindowPos(game->window, x + ((w - width) / 2), y + ((h - height) / 2));
    }
    // Lock aspect ratio if necessary
    if ((flags & RPG_INIT_LOCK_ASPECT) != 0) {
        glfwSetWindowAspectRatio(game->window, width, height);
    } else {
        game->autoAspect = (flags & RPG_INIT_AUTO_ASPECT) != 0;
    }

    // Make context current, import OpenGL functions
    glfwMakeContextCurrent(game->window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Set user pointer and return
    glfwSetWindowUserPointer(game->window, game);
    // TODO: Bind callbacks

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Main(RPGgame *gfx) {

    glClearColor(0.4f, 0.5f, 0.55f, 1.0f);
    while (!glfwWindowShouldClose(gfx->window)) {
        glfwSwapBuffers(gfx->window);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwPollEvents();
    }

}