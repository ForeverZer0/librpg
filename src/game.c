#include "rpg.h"
#include "glad.h"
#include "GLFW/glfw3.h"
#include <string.h>

#include "graphics.h" // TODO:

int errorCode;

typedef struct RPG_GRAPHICS {
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
} RPG_GRAPHICS;

// GLFWwindow *gameWindow;

static void RPG_Graphics_ErrorCallback(int error, const char *msg) {
    errorCode = error;
}

static RPG_RESULT RPG_Graphics_GetError(void) {
    switch (errorCode) {
        case GLFW_NO_ERROR: return RPG_NO_ERROR;
        default: return RPG_ERR_UNKNOWN; // Change to generic "graphics failed" type error
    }
}

RPG_RESULT RPG_Graphics_Initialize(void) {
    glfwSetErrorCallback(RPG_Graphics_ErrorCallback);
    if (glfwInit()) {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);   
    }
    return RPG_Graphics_GetError();
}


RPG_RESULT RPG_Graphics_Terminate(void) {
    glfwTerminate();
    // TODO:
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Graphics_Create(const char *title, RPGint width, RPGint height, RPG_INIT_FLAGS flags, RPG_GRAPHICS **graphics) {
    
    if (width < 1 || height < 1) {
        return RPG_ERR_OUT_OF_RANGE;
    }

    RPG_ALLOC_ZERO(gfx, RPG_GRAPHICS);
    gfx->resolution.width = width;
    gfx->resolution.height = height;
    gfx->flags = flags;
    if (title) {
        size_t sz = strlen(title) + 1;
        gfx->title = RPG_MALLOC(sz);
        memcpy(gfx->title, title, sz);
    }

    glfwWindowHint(GLFW_RESIZABLE, (flags & RPG_INIT_RESIZABLE) != 0);
    glfwWindowHint(GLFW_DECORATED, (flags & RPG_INIT_DECORATED) != 0);
    glfwWindowHint(GLFW_VISIBLE, (flags & RPG_INIT_HIDDEN) == 0);
    GLFWmonitor *monitor = ((flags & RPG_INIT_FULLSCREEN) != 0) ? glfwGetPrimaryMonitor() : NULL;

    // Create graphics context
    gfx->window = glfwCreateWindow(width, height, title, monitor, NULL);
    if (gfx->window == NULL) {
        RPG_FREE(gfx);
        return RPG_Graphics_GetError();
    }
    // Position window if necessary
    if ((flags & RPG_INIT_CENTERED) != 0 && monitor == NULL) {
        monitor = glfwGetPrimaryMonitor();
        int x, y, w, h;
        glfwGetMonitorWorkarea(monitor, &x, &y, &w, &h);
        glfwSetWindowPos(gfx->window, x + ((w - width) / 2), y + ((h - height) / 2));
    }
    // Lock aspect ratio if necessary
    if ((flags & RPG_INIT_LOCK_ASPECT) != 0) {
        glfwSetWindowAspectRatio(gfx->window, width, height);
    } else {
        gfx->autoAspect = (flags & RPG_INIT_AUTO_ASPECT) != 0;
    }

    // Make context current, import OpenGL functions
    glfwMakeContextCurrent(gfx->window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Set user pointer and return
    glfwSetWindowUserPointer(gfx->window, gfx);
    *graphics = gfx;

    // TODO: Bind callbacks

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Graphics_Main(RPG_GRAPHICS *gfx) {

    glClearColor(0.4f, 0.5f, 0.55f, 1.0f);
    while (!glfwWindowShouldClose(gfx->window)) {
        glfwSwapBuffers(gfx->window);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwPollEvents();
    }

}