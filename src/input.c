#include "game.h"
#include <math.h>
#include <string.h>

#define INPUT_STATE_NONE 0
#define INPUT_STATE_RELEASE 1
#define INPUT_STATE_TRIGGER 2
#define INPUT_STATE_REPEAT 3
#define INPUT_STATE_PRESS 4

#define RPG_KEY_FIRST RPG_KEY_SPACE

static void RPG_Input_CB_MouseEnter(GLFWwindow *window, int entered) {
    
    // TODO: Fire callback for event driven
}

static void RPG_Input_CB_MouseScroll(GLFWwindow *window, double x, double y) {
    RPG_GAME->input.scrollX += x;
    RPG_GAME->input.scrollY += y;
    if (RPG_GAME->input.cb.scroll) {
        RPG_GAME->input.cb.scroll(RPG_GAME, x, y);
    }
}

static void RPG_Input_CB_MouseMove(GLFWwindow *window, double x, double y) {
    if (RPG_GAME->input.cb.cursor) {
        RPG_GAME->input.cb.cursor(RPG_GAME, x, y);
    }
}

static void RPG_Input_CB_Key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        RPG_GAME->input.keys[key] = INPUT_STATE_TRIGGER;
    } else if (action == GLFW_REPEAT) {
        RPG_GAME->input.keys[key] = INPUT_STATE_REPEAT;
    } else {
        RPG_GAME->input.keys[key] = INPUT_STATE_RELEASE;
    }
    if (RPG_GAME->input.cb.key) {
        RPG_GAME->input.cb.key(RPG_GAME, key, scancode, action, mods);
    }
}

static void RPG_Input_CB_MouseButton(GLFWwindow *window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        RPG_GAME->input.buttons[button] = INPUT_STATE_TRIGGER;
    } else if (action == GLFW_REPEAT) {
        RPG_GAME->input.buttons[button] = INPUT_STATE_REPEAT;
    } else {
        RPG_GAME->input.buttons[button] = INPUT_STATE_RELEASE;
    }
    if (RPG_GAME->input.cb.mbtn) {
        RPG_GAME->input.cb.mbtn(RPG_GAME, button, action, mods);
    }
}

RPG_RESULT RPG_Input_Initialize(RPGgame *game) {
    glfwSetKeyCallback(game->window, RPG_Input_CB_Key);
    glfwSetMouseButtonCallback(game->window, RPG_Input_CB_MouseButton);
    glfwSetCursorPosCallback(game->window, RPG_Input_CB_MouseMove);
    glfwSetScrollCallback(game->window, RPG_Input_CB_MouseScroll);
    glfwSetCursorEnterCallback(game->window, RPG_Input_CB_MouseEnter);


    // TODO: Gamepad support
}

RPG_RESULT RPG_Input_Update(void) {
    
    // Key States
    for (int i = RPG_KEY_FIRST; i <= RPG_KEY_LAST; i++) {
        if (RPG_GAME->input.keys[i] == INPUT_STATE_NONE) {
            continue;
        }
        if (RPG_GAME->input.keys[i] == INPUT_STATE_TRIGGER) {
            RPG_GAME->input.keys[i] = INPUT_STATE_PRESS;
        } else if (RPG_GAME->input.keys[i] == INPUT_STATE_REPEAT) {
            RPG_GAME->input.keys[i] = INPUT_STATE_PRESS;
        } else if (RPG_GAME->input.keys[i] == INPUT_STATE_RELEASE) {
            RPG_GAME->input.keys[i] = INPUT_STATE_NONE;
        }
    }

    // Mouse Button States
    for (int i = 0; i <= RPG_MBUTTON_LAST; i++) {
        if (RPG_GAME->input.buttons[i] == INPUT_STATE_NONE) {
            continue;
        }
        if (RPG_GAME->input.buttons[i] == INPUT_STATE_TRIGGER) {
            RPG_GAME->input.buttons[i] = INPUT_STATE_PRESS;
        } else if (RPG_GAME->input.buttons[i] == INPUT_STATE_REPEAT) {
            RPG_GAME->input.buttons[i] = INPUT_STATE_PRESS;
        } else if (RPG_GAME->input.buttons[i] == INPUT_STATE_RELEASE) {
            RPG_GAME->input.buttons[i] = INPUT_STATE_NONE;
        }
    }

    RPG_GAME->input.scrollX = 0.0;
    RPG_GAME->input.scrollY = 0.0;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_KeyTrigger(RPG_KEY key, RPGbool *state) {
    if (state != NULL) {
        if (key < RPG_KEY_FIRST || key > RPG_KEY_LAST) {
            *state = RPG_FALSE;
            return RPG_ERR_OUT_OF_RANGE;
        }
        *state = RPG_GAME->input.keys[key] == INPUT_STATE_TRIGGER;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_KeyPress(RPG_KEY key, RPGbool *state) {
    if (state != NULL) {
        if (key < RPG_KEY_FIRST || key > RPG_KEY_LAST) {
            *state = RPG_FALSE;
            return RPG_ERR_OUT_OF_RANGE;
        }
        *state = RPG_GAME->input.keys[key] > INPUT_STATE_RELEASE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_KeyRepeat(RPG_KEY key, RPGbool *state) {
    if (state != NULL) {
        if (key < RPG_KEY_FIRST || key > RPG_KEY_LAST) {
            *state = RPG_FALSE;
            return RPG_ERR_OUT_OF_RANGE;
        }
        *state = RPG_GAME->input.keys[key] == INPUT_STATE_REPEAT;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_KeyRelease(RPG_KEY key, RPGbool *state) {
    if (state != NULL) {
        if (key < RPG_KEY_FIRST || key > RPG_KEY_LAST) {
            *state = RPG_FALSE;
            return RPG_ERR_OUT_OF_RANGE;
        }
        *state = RPG_GAME->input.keys[key] == INPUT_STATE_RELEASE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_ButtonTrigger(RPG_MBUTTON button, RPGbool *state) {
    if (state != NULL) {
        if (button < 0 || button > RPG_MBUTTON_LAST) {
            *state = RPG_FALSE;
            return RPG_ERR_OUT_OF_RANGE;
        }
        *state = RPG_GAME->input.buttons[button] == INPUT_STATE_TRIGGER;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_ButtonPress(RPG_MBUTTON button, RPGbool *state) {
    if (state != NULL) {
        if (button < 0 || button > RPG_MBUTTON_LAST) {
            *state = RPG_FALSE;
            return RPG_ERR_OUT_OF_RANGE;
        }
        *state = RPG_GAME->input.buttons[button] > INPUT_STATE_RELEASE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_ButtonRepeat(RPG_MBUTTON button, RPGbool *state) {
    if (state != NULL) {
        if (button < 0 || button > RPG_MBUTTON_LAST) {
            *state = RPG_FALSE;
            return RPG_ERR_OUT_OF_RANGE;
        }
        *state = RPG_GAME->input.buttons[button] == INPUT_STATE_REPEAT;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_ButtonRelease(RPG_MBUTTON button, RPGbool *state) {
    if (state != NULL) {
        if (button < 0 || button > RPG_MBUTTON_LAST) {
            *state = RPG_FALSE;
            return RPG_ERR_OUT_OF_RANGE;
        }
        *state = RPG_GAME->input.buttons[button] == INPUT_STATE_RELEASE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_GetCursorLocation(RPGint *x, RPGint *y) {
    double _x, _y;
    // Translate window coordinates to be relative to the internal resolution
    glfwGetCursorPos(RPG_GAME->window, &_x, &_y);
    if (RPG_GAME->bounds.ratio.x < RPG_GAME->bounds.ratio.y) {
        _x = (_x / RPG_GAME->bounds.ratio.x);
        _y = (_y  - RPG_GAME->bounds.y) / RPG_GAME->bounds.ratio.x; 
    } else {
        _x = (_x / RPG_GAME->bounds.ratio.y) - RPG_GAME->bounds.x;
        _y = (_y / RPG_GAME->bounds.ratio.y);
    }

    if (x != NULL) {
        *x = (RPGint) round(_x);
    }
    if (y != NULL) {
        *y = (RPGint) round(_y);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_GetMouseScroll(RPGdouble *x, RPGdouble *y) {
    if (x != NULL) {
        *x = RPG_GAME->input.scrollX;
    }
    if (y != NULL) {
        *y = RPG_GAME->input.scrollY;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_GetScancode(RPG_KEY key, RPGint *scancode) {
    if (scancode != NULL) {
        *scancode = glfwGetKeyScancode(key);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_SetCursorMode(RPG_CURSOR_MODE mode) {
    switch (mode) {
        case RPG_CURSOR_MODE_NORMAL: 
            glfwSetInputMode(RPG_GAME->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case RPG_CURSOR_MODE_HIDDEN:
            glfwSetInputMode(RPG_GAME->window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            break;
        case RPG_CURSOR_MODE_DISABLED:
            glfwSetInputMode(RPG_GAME->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
        default: return RPG_ERR_INVALID_VALUE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_SetCursorType(RPG_CURSOR shape) {
    GLFWcursor *cursor;
    switch (shape) {
        case RPG_CURSOR_ARROW:
            cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
            break;
        case RPG_CURSOR_IBEAM:
            cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
            break;
        case RPG_CURSOR_CROSSHAIR:
            cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
            break;
        case RPG_CURSOR_HAND:
            cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
            break;
        case RPG_CURSOR_HRESIZE:
            cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
            break;
        case RPG_CURSOR_VRESIZE:
            cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
            break;
        default: return RPG_ERR_INVALID_VALUE;
    }

    // Destroy existing cursor if it exists
    if (RPG_GAME->input.cursor != NULL) {
        glfwDestroyCursor(RPG_GAME->input.cursor);
    }

    // Set new cursor
    RPG_GAME->input.cursor = cursor;
    glfwSetCursor(RPG_GAME->window, cursor);
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_SetCursorImage(RPGimage *image, RPGint x, RPGint y) {
    if (image == NULL) {
        return RPG_ERR_INVALID_POINTER;
    }

    GLFWimage img;
    img.width = image->width;
    img.height = image->height;
    RPGsize size = image->width * image->height * BYTES_PER_PIXEL;
    img.pixels = RPG_MALLOC(size);
    RPG_RESULT result = RPG_Image_GetPixels(image, img.pixels, size);

    if (result == RPG_NO_ERROR) {
        GLFWcursor *cursor = glfwCreateCursor(&img, x, y);
        RPG_FREE(img.pixels);
        // Destroy existing cursor if it exists
        if (RPG_GAME->input.cursor != NULL) {
            glfwDestroyCursor(RPG_GAME->input.cursor);
        }
        // Set new cursor
        RPG_GAME->input.cursor = cursor;
        glfwSetCursor(RPG_GAME->window, cursor);
        return RPG_NO_ERROR;
    }
    RPG_FREE(img.pixels);
    return result;
}

static void RPG_Input_CB_CharMods(GLFWwindow *window, unsigned int codepoint, int mods) {
    // TODO: Log input, convert codepoints to char*
    // FIXME: Implement this, or remove from lib?
}

// FIXME: Implement this, or remove from lib?
RPG_RESULT RPG_Input_BeginTextCapture(void *buffer, RPGsize sizeBuffer) {
    if (buffer == NULL) {
        return RPG_ERR_INVALID_POINTER;
    }
    if (sizeBuffer == 0) {
        return RPG_ERR_INVALID_VALUE;
    }
    if (RPG_GAME->input.capture.buffer != NULL) {
        return RPG_ERR_MEMORY;
    }
    
    // Fill buffer with 0's and set callback
    memset(buffer, 0, sizeBuffer);
    glfwSetCharModsCallback(RPG_GAME->window, RPG_Input_CB_CharMods);

    // Store pointer, and reset data to default
    RPG_GAME->input.capture.buffer = buffer;
    RPG_GAME->input.capture.size = sizeBuffer;
    RPG_GAME->input.capture.pos = 0;

    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Input_EndTextCapture(RPGsize *written) {
    // FIXME: Implement this, or remove from lib?
    if (written != NULL) {
        *written = 0;
    }
    glfwSetCharModsCallback(RPG_GAME->window, NULL);
    return RPG_NO_ERROR;
}