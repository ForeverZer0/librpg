#include "internal.h"
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
    if (g->cb.resized) {
        g->cb.resized(g, width, height);
    }
}

static void RPG_Game_CB_Error(int error, const char *msg) { errorCode = error; }

static void RPG_Game_CB_FocusChanged(GLFWwindow *window, int focused) {
    RPGgame *game = glfwGetWindowUserPointer(window);
    if (game != NULL && game->cb.focused != NULL) {
        game->cb.focused(game, focused ? RPG_TRUE : RPG_FALSE);
    }
}

static void RPG_Game_CB_MinimizeChange(GLFWwindow *window, int minimized) {
    RPGgame *game = glfwGetWindowUserPointer(window);
    if (game != NULL) {
        if (game->cb.minimized != NULL) {
            game->cb.minimized(game, minimized ? RPG_TRUE : RPG_FALSE);
        }
        if (!minimized && game->cb.restored) {
            game->cb.restored(game);
        }
    }
}

static void RPG_Game_CB_MaximizeChange(GLFWwindow *window, int maximized) {
    RPGgame *game = glfwGetWindowUserPointer(window);
    if (game != NULL) {
        if (game->cb.maximized != NULL) {
            game->cb.maximized(game, maximized ? RPG_TRUE : RPG_FALSE);
        }
        if (!maximized && game->cb.restored) {
            game->cb.restored(game);
        }
    }
}

static void RPG_Game_CB_FileDrop(GLFWwindow *window, int count, const char **files) {
    RPGgame *game = glfwGetWindowUserPointer(window);
    if (game != NULL && game->cb.filedrop != NULL) {
        game->cb.filedrop(game, count, files);
    }
}

static void RPG_Game_CB_Moved(GLFWwindow *window, int x, int y) {
    RPGgame *game = glfwGetWindowUserPointer(window);
    if (game && game->cb.moved) {
        game->cb.moved(game, x, y);
    }
}

static RPG_RESULT RPG_Game_GetError(void) {
    switch (errorCode) {
        case GLFW_NO_ERROR: return RPG_NO_ERROR;
        case GLFW_NOT_INITIALIZED:
        case GLFW_NO_CURRENT_CONTEXT: return RPG_ERR_CONTEXT;
        case GLFW_INVALID_ENUM:
        case GLFW_INVALID_VALUE: return RPG_ERR_INVALID_VALUE;
        case GLFW_OUT_OF_MEMORY: return RPG_ERR_MEMORY;
        case GLFW_API_UNAVAILABLE:
        case GLFW_VERSION_UNAVAILABLE:
        case GLFW_PLATFORM_ERROR:
        case GLFW_FORMAT_UNAVAILABLE:
        case GLFW_NO_WINDOW_CONTEXT: return RPG_ERR_UNSUPPORTED;
        default: return RPG_ERR_UNKNOWN;
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
    game->shader.flash      = glGetUniformLocation(game->shader.program, UNIFORM_FLASH);

    return RPG_NO_ERROR;
}

static void RPG_Game_BindCallbacks(RPGgame *game) {
    RPG_ASSERT(game);
    RPG_Input_Initialize(game);

    glfwSetWindowFocusCallback(game->window, RPG_Game_CB_FocusChanged);
    glfwSetWindowIconifyCallback(game->window, RPG_Game_CB_MinimizeChange);
    glfwSetWindowMaximizeCallback(game->window, RPG_Game_CB_MaximizeChange);

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
        alcDestroyContext(g->audio.context);
        alcCloseDevice(g->audio.device);
        RPG_FREE(g);
        return result;
    }
#endif

    glfwSetErrorCallback(RPG_Game_CB_Error);
    if (glfwInit()) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
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
    g->font.defaultSize  = RPG_FONT_DEFAULT_SIZE;
    if (title) {
        g->title = RPG_MALLOC(strlen(title) + 1);
        strcpy(g->title, title);
    }

    glfwWindowHint(GLFW_RESIZABLE, (flags & RPG_INIT_RESIZABLE) != 0);
    glfwWindowHint(GLFW_DECORATED, (flags & RPG_INIT_DECORATED) != 0);
    glfwWindowHint(GLFW_VISIBLE, (flags & RPG_INIT_HIDDEN) == 0);
    GLFWmonitor *monitor = ((flags & RPG_INIT_FULLSCREEN) != 0) ? glfwGetPrimaryMonitor() : NULL;

    glfwSwapInterval(0);
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
            updateCallback(game->update.count);
            RPG_Input_Update();
            delta += game->update.tick;
        }
        RPG_Game_Render(game);
        glfwPollEvents();
        glfwSwapBuffers(game->window);
    }

    return RPG_NO_ERROR;
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

RPG_RESULT RPG_Game_Close(RPGgame *game, RPGbool close) {
    RPG_RETURN_IF_NULL(game);
    glfwSetWindowShouldClose(game->window, close);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Show(RPGgame *game) {
    RPG_RETURN_IF_NULL(game);
    glfwShowWindow(game->window);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Hide(RPGgame *game) {
    RPG_RETURN_IF_NULL(game);
    glfwHideWindow(game->window);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Focus(RPGgame *game) {
    RPG_RETURN_IF_NULL(game);
    glfwFocusWindow(game->window);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_RequestAttention(RPGgame *game) {
    RPG_RETURN_IF_NULL(game);
    glfwRequestWindowAttention(game->window);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Minimize(RPGgame *game) {
    RPG_RETURN_IF_NULL(game);
    glfwIconifyWindow(game->window);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Maximize(RPGgame *game) {
    RPG_RETURN_IF_NULL(game);
    glfwMaximizeWindow(game->window);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Restore(RPGgame *game) {
    RPG_RETURN_IF_NULL(game);
    glfwRestoreWindow(game->window);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetWindowSize(RPGgame *game, RPGint width, RPGint height) {
    RPG_RETURN_IF_NULL(game);
    glfwSetWindowSize(game->window, width, height);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetWindowSize(RPGgame *game, RPGint *width, RPGint *height) {
    RPG_RETURN_IF_NULL(game);
    glfwGetWindowSize(game->window, width, height);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetWindowLocation(RPGgame *game, RPGint x, RPGint y) {
    RPG_RETURN_IF_NULL(game);
    glfwSetWindowPos(game->window, x, y);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetWindowLocation(RPGgame *game, RPGint *x, RPGint *y) {
    RPG_RETURN_IF_NULL(game);
    glfwGetWindowPos(game->window, x, y);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetWindowOpacity(RPGgame *game, RPGfloat opacity) {
    RPG_RETURN_IF_NULL(game);
    glfwSetWindowOpacity(game->window, opacity);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetWindowOpacity(RPGgame *game, RPGfloat *opacity) {
    RPG_RETURN_IF_NULL(game);
    *opacity = (RPGfloat) glfwGetWindowOpacity(game->window);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetClipboardString(RPGgame *game, void *buffer, RPGsize sizeBuffer, RPGsize *size) {
    const char *str = glfwGetClipboardString(NULL);
    if (str == NULL) {
        *size = 0;
        return RPG_NO_ERROR;
    }
    RPGsize sz = strlen(str);
    if (size != NULL) {
        *size = sz;
    }
    if (buffer != NULL) {
        memset(buffer, 0, sizeBuffer);
        memcpy(buffer, str, sz > sizeBuffer ? sizeBuffer : sz);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetClipboardString(RPGgame *game, const char *str) {
    glfwSetClipboardString(NULL, str);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetWindowTitle(RPGgame *game, void *buffer, RPGsize sizeBuffer, RPGsize *size) {
    RPG_RETURN_IF_NULL(game);
    const char *str = game->title;
    if (str == NULL) {
        *size = 0;
        return RPG_NO_ERROR;
    }
    RPGsize sz = strlen(str);
    if (size != NULL) {
        *size = sz;
    }
    if (buffer != NULL) {
        memset(buffer, 0, sizeBuffer);
        memcpy(buffer, str, sz > sizeBuffer ? sizeBuffer : sz);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetIsMaximized(RPGgame *game, RPGbool *state) {
    RPG_RETURN_IF_NULL(game);
    if (state != NULL) {
        *state = glfwGetWindowAttrib(game->window, GLFW_MAXIMIZED) ? RPG_TRUE : RPG_FALSE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetIsMinimized(RPGgame *game, RPGbool *state) {
    RPG_RETURN_IF_NULL(game);
    if (state != NULL) {
        *state = glfwGetWindowAttrib(game->window, GLFW_ICONIFIED) ? RPG_TRUE : RPG_FALSE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetIsFocused(RPGgame *game, RPGbool *state) {
    RPG_RETURN_IF_NULL(game);
    if (state != NULL) {
        *state = glfwGetWindowAttrib(game->window, GLFW_FOCUSED) ? RPG_TRUE : RPG_FALSE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetTopMost(RPGgame *game, RPGbool *state) {
    RPG_RETURN_IF_NULL(game);
    if (state != NULL) {
        *state = glfwGetWindowAttrib(game->window, GLFW_FLOATING) ? RPG_TRUE : RPG_FALSE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetTopMost(RPGgame *game, RPGbool state) {
    RPG_RETURN_IF_NULL(game);
    glfwSetWindowAttrib(game->window, GLFW_FLOATING, state);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetWindowTitle(RPGgame *game, const char *title) {
    RPG_RETURN_IF_NULL(game);
    if (game->title != NULL) {
        RPG_FREE(game->title);
    }
    glfwSetWindowTitle(game->window, title);
    if (title != NULL) {
        game->title = RPG_MALLOC(strlen(title) + 1);
        strcpy(game->title, title);
    } else {
        game->title = NULL;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Snapshot(RPGgame *game, RPGimage **image) {
    if (game == NULL || *image == NULL) {
        return RPG_ERR_INVALID_POINTER;
    }

    RPGimage *img = RPG_ALLOC(RPGimage);
    img->width    = RPG_GAME->resolution.width;
    img->height   = RPG_GAME->resolution.height;
    img->user     = NULL;
    glDisable(GL_SCISSOR_TEST);

    // Create texture the same size as the internal resolution
    glGenTextures(1, &img->texture);
    glBindTexture(GL_TEXTURE_2D, img->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, game->resolution.width, game->resolution.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
    l = game->bounds.x;
    t = game->bounds.y;
    r = l + game->bounds.w;
    b = t + game->bounds.h;

    // Bind the primary buffer as the read buffer, and blit
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBlitFramebuffer(l, t, r, b, 0, 0, game->resolution.width, game->resolution.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Rebind primary FBO and return the created image
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glEnable(GL_SCISSOR_TEST);

    *image = img;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetMinimizeCallback(RPGgame *game, RPGgamestate func) {
    RPG_RETURN_IF_NULL(game);
    game->cb.minimized = func;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetMaximizeCallback(RPGgame *game, RPGgamestate func) {
    RPG_RETURN_IF_NULL(game);
    game->cb.maximized = func;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetFocusCallback(RPGgame *game, RPGgamestate func) {
    RPG_RETURN_IF_NULL(game);
    game->cb.focused = func;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetRestoredCallback(RPGgame *game, RPGgameaction func) {
    RPG_RETURN_IF_NULL(game);
    game->cb.restored = func;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetCloseCallback(RPGgame *game, RPGgameaction func) {
    RPG_RETURN_IF_NULL(game);
    game->cb.closing = func;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetFileDropCallback(RPGgame *game, RPGfiledropfunc func) {
    RPG_RETURN_IF_NULL(game);
    glfwSetDropCallback(game->window, RPG_Game_CB_FileDrop);
    game->cb.filedrop = func;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetMoveCallback(RPGgame *game, RPGmovefunc func) {
    RPG_RETURN_IF_NULL(game);
    glfwSetWindowPosCallback(game->window, RPG_Game_CB_Moved);
    game->cb.moved = func;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetResizeCallback(RPGgame *game, RPGsizefunc func) {
    RPG_RETURN_IF_NULL(game);
    game->cb.resized = func;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_GetWindowFrameSize(RPGgame *game, RPGint *left, RPGint *top, RPGint *right, RPGint *bottom) {
    RPG_RETURN_IF_NULL(game);
    glfwGetWindowFrameSize(game->window, left, top, right, bottom);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Transition(RPGgame *game, RPGshader *shader, RPGint duration, RPGtransitionfunc func) {
    RPG_RETURN_IF_NULL(game);
    RPG_RETURN_IF_NULL(func);

    if (shader == NULL || duration < 1) {
        // Early out
        func(game, shader);
        return RPG_NO_ERROR;
    }

    // Take copy of current screen
    RPGimage *from, *to;
    RPG_Game_Snapshot(game, &from);

    // Enable transition shader and yield control back to change scene, set uniforms, etc,
    glUseProgram(shader->program);
    func(game, shader);

    // Take copy of the target screen to transition to
    glClear(GL_COLOR_BUFFER_BIT);
    RPG_Game_Render(game);
    RPG_Game_Snapshot(game, &to);

    // Copy front buffer (current frame) to the back buffer (currently has target frame drawn on it).
    // If not done, the first buffer swap will show the final frame for a single render, causing a flicker.
    int w, h;
    glfwGetFramebufferSize(game->window, &w, &h);
    glReadBuffer(GL_FRONT);
    glDrawBuffer(GL_BACK);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glReadBuffer(GL_BACK);

    // Bind the shader and set the locations to recieve the from/to textures
    glUseProgram(shader->program);
    GLint progress = glGetUniformLocation(shader->program, "progress");
    glUniform1i(glGetUniformLocation(shader->program, "from"), 0);  // TODO:
    glUniform1i(glGetUniformLocation(shader->program, "to"), 1);

    // Bind the "to" and "from" textures to the shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, from->texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, to->texture);

    // Create a VAO and VBO to use for the transition
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float vertices[VERTICES_COUNT] = {-1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                                      -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f};
    glBufferData(GL_ARRAY_BUFFER, VERTICES_SIZE, vertices, GL_STATIC_DRAW);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VERTICES_STRIDE, NULL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Get time, and calculate length of transition
    glBindVertexArray(vao);
    double done   = duration * game->update.tick;
    GLdouble time = glfwGetTime();
    GLdouble max  = time + (duration * game->update.tick);
    float percent;

    // Loop through the defined amount of time, updating the "progress" uniform each draw
    while (time < max && !glfwWindowShouldClose(game->window)) {
        percent = RPG_CLAMPF((RPGfloat)(1.0 - ((max - time) / done)), 0.0f, 1.0f);
        glUniform1f(progress, percent);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwPollEvents();
        glfwSwapBuffers(game->window);
        time = glfwGetTime();
    }

    // Unbind the textures
    RPG_RESET_BACK_COLOR();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    game->update.count += duration;

    // Cleanup
    RPG_Image_Free(from);
    RPG_Image_Free(to);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    return RPG_NO_ERROR;
}