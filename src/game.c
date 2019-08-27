#include "game.h"
#include "internal.h"
#include "renderable.h"
#include "rpg.h"
#include "rpgaudio.h"

RPGgame *RPG_GAME;

volatile int errorCode;

static void RPG_Game_FramebufferResize(GLFWwindow *window, int width, int height) {
    RPGgame *g = glfwGetWindowUserPointer(window);
    RPG_ASSERT(g);

    if ((g->flags & RPG_INIT_AUTO_ASPECT) != 0) {

        // Calculate ratios
        g->bounds.ratio.x = (GLfloat) width / g->resolution.width;
        g->bounds.ratio.y = (GLfloat) height / g->resolution.height;
        GLfloat ratio     = fminf(g->bounds.ratio.x, g->bounds.ratio.y);

        // Calculate letterbox/pillar rendering coordinates as required
        g->bounds.w = (GLint) roundf(g->resolution.width * ratio);
        g->bounds.h = (GLint) roundf(g->resolution.height * ratio);
        g->bounds.x = (GLint) roundf((width - g->resolution.width * ratio) / 2);
        g->bounds.y = (GLint) roundf((height - g->resolution.height * ratio) / 2);

        glViewport(g->bounds.x, g->bounds.y, g->bounds.w, g->bounds.h);
        glScissor(g->bounds.x, g->bounds.y, g->bounds.w, g->bounds.h);

        // Ensure the clipping area is also cleared
        glDisable(GL_SCISSOR_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_SCISSOR_TEST);
        RPG_RESET_BACK_COLOR();

    } else {
        g->bounds.x = 0;
        g->bounds.y = 0;
        g->bounds.w = width;
        g->bounds.h = height;
        glViewport(0, 0, width, height);
        glScissor(0, 0, width, height);
    }
}

static void RPG_Game_ErrorCallback(int error, const char *msg) { errorCode = error; }

static RPG_RESULT RPG_Game_GetError(void) {
    switch (errorCode) {
        case GLFW_NO_ERROR: return RPG_NO_ERROR;
        // TODO: Translate GLFW errors
        default: return RPG_ERR_UNKNOWN;  // Change to generic "graphics failed" type error
    }
}

static RPG_RESULT RPG_Game_CreateShaderProgram(RPGgame *game) {
    RPG_RETURN_IF_NULL(game);
    if (game->shader.program) {
        return RPG_NO_ERROR;
    }

    RPGshader *shader;
    RPG_RESULT result = RPG_Shader_Create(RPG_VERTEX_SHADER, RPG_FRAGMENT_SHADER, NULL, &shader);
    if (result) {
        return result;
    }

    // Only need the shader name for storing within the game structure
    game->shader.program = *((GLuint *) shader);
    RPG_FREE(shader);
    glUseProgram(game->shader.program);

    // Pre-cache the uniform locations
    game->shader.projection = glGetUniformLocation(game->shader.program, UNIFORM_PROJECTION);
    game->shader.model      = glGetUniformLocation(game->shader.program, UNIFORM_MODEL);
    game->shader.color      = glGetUniformLocation(game->shader.program, UNIFORM_COLOR);
    game->shader.tone       = glGetUniformLocation(game->shader.program, UNIFORM_TONE);
    game->shader.alpha      = glGetUniformLocation(game->shader.program, UNIFORM_ALPHA);
    game->shader.hue        = glGetUniformLocation(game->shader.program, UNIFORM_HUE);

    return RPG_NO_ERROR;
}

static void RPG_Game_BindCallbacks(RPGgame *game) {
    RPG_ASSERT(game);
    RPG_Input_Initialize(game);

    // FIXME:
    glfwSetFramebufferSizeCallback(game->window, RPG_Game_FramebufferResize);
}

RPG_RESULT RPG_Game_GetResolution(RPGgame *game, RPGint *width, RPGint *height) {
    RPG_RETURN_IF_NULL(game);
    if (width != NULL) {
        *width = game->resolution.width;
    }
    if (height != NULL) {
        *height = game->resolution.height;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetResolution(RPGgame *game, RPGint width, RPGint height) {
    RPG_CHECK_DIMENSIONS(width, height);
    RPG_RETURN_IF_NULL(game);
    game->resolution.width  = width;
    game->resolution.height = height;

    // Update projection in the shader
    RPG_MAT4_ORTHO(game->projection, 0.0f, game->resolution.width, 0.0f, game->resolution.height, -1.0f, 1.0f);
    glUniformMatrix4fv(game->shader.projection, 1, GL_FALSE, (float *) &game->projection);

    // Fake a framebuffer resize event to recalulate rendering area
    int w, h;
    glfwGetFramebufferSize(game->window, &w, &h);
    RPG_Game_FramebufferResize(game->window, w, h);
    return RPG_NO_ERROR;
}

const char *RPG_GetErrorString(RPG_RESULT result) {
    switch (result) {  // TODO:
        case RPG_NO_ERROR: return "No error.";
        case RPG_ERR_AUDIO_DEVICE: return "Failed to open audio device.";
        case RPG_ERR_AUDIO_CONTEXT: return "An error occurred creating or setting the audio context.";
        default: return "Unknown error.";
    }
}

RPG_RESULT RPG_Game_Destroy(RPGgame *game) {
    glfwTerminate();
#ifndef RPG_WITHOUT_OPENAL
    RPG_Audio_Terminate();
    alcDestroyContext(game->audio.context);
    alcCloseDevice(game->audio.device);
#endif
    RPG_Batch_Free(&game->batch);  // TODO: Check if initialized first
    RPG_FREE(game);
    return RPG_NO_ERROR;
}  // TODO:

RPG_RESULT RPG_Game_Create(const char *title, RPGint width, RPGint height, RPG_INIT_FLAGS flags, RPGgame **game) {
    RPG_ALLOC_ZERO(g, RPGgame);
    RPG_RESULT result;
#ifndef RPG_WITHOUT_OPENAL
    result = RPG_Audio_Initialize(g);
    if (result) {
        RPG_FREE(g);
        return result;
    }
#endif

    glfwSetErrorCallback(RPG_Game_ErrorCallback);
    if (glfwInit()) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    } else {
        *game = NULL;
        RPG_FREE(g);
        return RPG_Game_GetError();
    }
    RPG_Batch_Init(&g->batch);
    RPG_CHECK_DIMENSIONS(width, height);

    g->resolution.width  = width;
    g->resolution.height = height;
    g->flags             = flags;
    g->font.defaultColor = RPG_FONT_DEFAULT_COLOR;
    g->font.defaultSize = RPG_FONT_DEFAULT_SIZE;
    if (title) {
        g->title = RPG_MALLOC(strlen(title) + 1);
        strcpy(g->title, title);
    }

    glfwWindowHint(GLFW_RESIZABLE, (flags & RPG_INIT_RESIZABLE) != 0);
    glfwWindowHint(GLFW_DECORATED, (flags & RPG_INIT_DECORATED) != 0);
    glfwWindowHint(GLFW_VISIBLE, (flags & RPG_INIT_HIDDEN) == 0);
    GLFWmonitor *monitor = ((flags & RPG_INIT_FULLSCREEN) != 0) ? glfwGetPrimaryMonitor() : NULL;

    // Create graphics context
    g->window = glfwCreateWindow(width, height, title, monitor, NULL);
    if (g->window == NULL) {
        RPG_FREE(g);
        return RPG_Game_GetError();
    }
    // Position window if necessary
    if ((flags & RPG_INIT_CENTERED) != 0 && monitor == NULL) {
        monitor = glfwGetPrimaryMonitor();
        int x, y, w, h;
        glfwGetMonitorWorkarea(monitor, &x, &y, &w, &h);
        glfwSetWindowPos(g->window, x + ((w - width) / 2), y + ((h - height) / 2));
    }
    // Lock aspect ratio if necessary
    if ((flags & RPG_INIT_LOCK_ASPECT) != 0) {
        glfwSetWindowAspectRatio(g->window, width, height);
    }

    // Make context current, import OpenGL functions
    glfwMakeContextCurrent(g->window);
    RPG_GAME = g;
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSetWindowUserPointer(g->window, g);

    // Enable required OpenGL capabilities
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    

    result = RPG_Game_CreateShaderProgram(g);
    if (result) {
        glfwDestroyWindow(g->window);
        RPG_FREE(g);
        return result;
    }

    RPG_Game_BindCallbacks(g);
    RPG_Game_SetResolution(g, width, height);
    *game = g;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Main(RPGgame *game, RPGdouble tps, RPGupdatefunc updateCallback) {
    RPG_RETURN_IF_NULL(game);
    RPG_RETURN_IF_NULL(updateCallback);

    RPG_RESULT result = RPG_Game_SetFrameRate(game, tps);
    if (result != RPG_NO_ERROR) {
        return result;
    }

    double delta = glfwGetTime();
    while (!glfwWindowShouldClose(game->window)) {
        while (delta < glfwGetTime()) {
            game->update.count++;
            // TODO: Update input
            updateCallback(game->update.count);
            delta += game->update.tick;
        }
        RPG_Game_Render(game);
        glfwPollEvents();
        glfwSwapBuffers(game->window);
    }
}

RPG_RESULT RPG_Game_GetFrameRate(RPGgame *game, RPGdouble *rate) {
    RPG_RETURN_IF_NULL(game);
    RPG_RETURN_IF_NULL(rate);
    *rate = game->update.rate;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetFrameRate(RPGgame *game, RPGdouble rate) {
    RPG_RETURN_IF_NULL(game);
    if (rate < 1.0) {
        return RPG_ERR_OUT_OF_RANGE;
    }
    game->update.rate = rate;
    game->update.tick = 1.0 / rate;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetBackColor(RPGgame *game, RPGcolor *color) {
    RPG_RETURN_IF_NULL(game);
    RPG_RETURN_IF_NULL(color);
    *color = game->color;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetBackColor(RPGgame *game, RPGcolor *color) {
    RPG_RETURN_IF_NULL(game);
    if (color == NULL) {
        memset(&game->color, 0, sizeof(RPGcolor));
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    } else {
        memcpy(&game->color, color, sizeof(RPGcolor));
        glClearColor(color->x, color->y, color->z, color->w);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Render(RPGgame *game) {

    glClear(GL_COLOR_BUFFER_BIT);
    if (game->batch.updated) {
        RPG_Batch_Sort(&game->batch, 0, game->batch.total - 1);
    }
    RPGrenderable *r;
    int count = game->batch.total;
    for (int i = 0; i < count; i++) {
        r = game->batch.items[i];
        r->render(r);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetIcon(RPGgame *game, RPGrawimage *image) {
    RPG_RETURN_IF_NULL(game);
    if (image) {
        glfwSetWindowIcon(game->window, 1, (GLFWimage *) image);
    } else {
        glfwSetWindowIcon(game->window, 0, NULL);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetIconFromFile(RPGgame *game, const char *filename) {
    RPG_RETURN_IF_NULL(game);
    if (filename) {
        GLFWimage *img;
        RPG_RESULT result = RPG_Image_LoadRaw(filename, (RPGrawimage **) &img);
        if (result == RPG_NO_ERROR) {
            glfwSetWindowIcon(game->window, 1, img);
        }
        return result;
    } else {
        glfwSetWindowIcon(game->window, 0, NULL);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_MakeCurrent(RPGgame *game) {
    RPG_RETURN_IF_NULL(game);
    glfwMakeContextCurrent(game->window);
    alcMakeContextCurrent(game->audio.context);
    RPG_GAME = game;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Snapshot(RPGimage **image) {
    if (*image == NULL) {
        return RPG_ERR_INVALID_POINTER;
    }

    RPGimage *img = RPG_ALLOC(RPGimage);
    img->width = RPG_GAME->resolution.width;
    img->height = RPG_GAME->resolution.height;
    img->user = NULL;
    glDisable(GL_SCISSOR_TEST);

    // Create texture the same size as the internal resolution
    glGenTextures(1, &img->texture);
    glBindTexture(GL_TEXTURE_2D, img->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RPG_GAME->resolution.width, RPG_GAME->resolution.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create FBO and bind as the draw buffer
    glGenFramebuffers(1, &img->fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, img->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img->texture, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    RPGint l, r, t, b;
    l = RPG_GAME->bounds.x;
    t = RPG_GAME->bounds.y;
    r = l + RPG_GAME->bounds.w;
    b = t + RPG_GAME->bounds.h;

    // Bind the primary buffer as the read buffer, and blit
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBlitFramebuffer(l, t, r, b, 0, 0, RPG_GAME->resolution.width, RPG_GAME->resolution.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Rebind primary FBO and return the created image
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glEnable(GL_SCISSOR_TEST);

    *image = img;
    return RPG_NO_ERROR;
}