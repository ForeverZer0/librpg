#include "game.h"
#include "internal.h"
#include "rpg.h"
#include <string.h>

volatile int errorCode;

const char *RPG_GetErrorString(RPG_RESULT result) {
    switch (result) { // TODO:
        case RPG_NO_ERROR: return "No error.";
        case RPG_ERR_AUDIO_DEVICE: return "Failed to open audio device.";
        case RPG_ERR_AUDIO_CONTEXT: return "An error occurred creating or setting the audio context.";
        default: return "Unknown error.";
    }
}



static void RPG_Game_ErrorCallback(int error, const char *msg) { errorCode = error; }

static RPG_RESULT RPG_Game_GetError(void) {
    switch (errorCode) {
        case GLFW_NO_ERROR: return RPG_NO_ERROR;
        // TODO: Translate GLFW errors
        default: return RPG_ERR_UNKNOWN; // Change to generic "graphics failed" type error
    }
}

static void RPG_Game_CacheUniformLocations(RPGgame *game) {
    RPG_ASSERT(game->shader.program);
    game->shader.projection = glGetUniformLocation(game->shader.program, UNIFORM_PROJECTION);
    game->shader.model = glGetUniformLocation(game->shader.program, UNIFORM_MODEL);
    game->shader.color = glGetUniformLocation(game->shader.program, UNIFORM_COLOR);
    game->shader.tone = glGetUniformLocation(game->shader.program, UNIFORM_TONE);
    game->shader.alpha = glGetUniformLocation(game->shader.program, UNIFORM_ALPHA);
    game->shader.hue = glGetUniformLocation(game->shader.program, UNIFORM_HUE);
}

static inline RPGbool RPG_Game_CreateShader(const char *source, GLenum type, GLuint *result) {
    GLuint shader = glCreateShader(type);
    GLint length = (GLint) strlen(source);
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        glDeleteShader(shader);
        *result = 0;
        return RPG_TRUE;
    }
    *result = shader;
    return RPG_FALSE;
}

static RPG_RESULT RPG_Game_CreateProgram(RPGgame *game) {
    if (game->shader.program)
        return RPG_NO_ERROR;
    
    GLuint program, vertex, fragment;
    if (RPG_Game_CreateShader(RPG_VERTEX_SHADER, GL_VERTEX_SHADER, &vertex)) {
        return RPG_ERR_SHADER_COMPILE;
    }
    if (RPG_Game_CreateShader(RPG_FRAGMENT_SHADER, GL_FRAGMENT_SHADER, &fragment)) {
        glDeleteShader(vertex);
        return RPG_ERR_SHADER_COMPILE;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        glDeleteProgram(program);
        return RPG_ERR_SHADER_LINK;
    }
    game->shader.program = program;
    return RPG_NO_ERROR;
}

static void RPG_Game_BindCallbacks(RPGgame *game, GLFWwindow *window) {}

RPG_RESULT RPG_Game_Destroy(RPGgame *game) { glfwTerminate(); } // TODO:

RPG_RESULT RPG_Game_Create(RPGgame **game) {
    RPG_ALLOC_ZERO(g, RPGgame);
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
    *game = g;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_CreateWindow(RPGgame *game, const char *title, RPGint width, RPGint height, RPG_INIT_FLAGS flags) {

    if (width < 1 || height < 1) {
        return RPG_ERR_OUT_OF_RANGE;
    }

    game->resolution.width  = width;
    game->resolution.height = height;
    game->flags             = flags;
    if (title) {
        game->title = RPG_MALLOC(strlen(title) + 1);
        strcpy(game->title, title);
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

    RPG_Game_CreateProgram(game);
    RPG_Game_CacheUniformLocations(game);
    // TODO: Bind callbacks, check errors, cleanup if fail

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_Main(RPGgame *game, RPGdouble tps) {

    RPG_RESULT result = RPG_Game_SetFrameRate(game, tps);
    if (result != RPG_NO_ERROR) {
        return result;
    }
    double delta = glfwGetTime();
    while (!glfwWindowShouldClose(game->window)) {
        while (delta < glfwGetTime()) {
            game->update.count++;
            // TODO: Update input
            if (game->update.cb) {
                game->update.cb(game->update.count);
            }
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

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Game_SetUpdateCallback(RPGgame *game, RPGupdatefunc update) {
    RPG_RETURN_IF_NULL(game);
    game->update.cb = update;
    return RPG_NO_ERROR;
}