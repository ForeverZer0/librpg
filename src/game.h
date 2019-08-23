#ifndef OPEN_RPG_GAME_H
#define OPEN_RPG_GAME_H 1

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glad.h"
#include "rpg.h"

#define UNIFORM_PROJECTION "projection"
#define UNIFORM_MODEL "model"
#define UNIFORM_COLOR "color"
#define UNIFORM_TONE "tone"
#define UNIFORM_ALPHA "alpha"
#define UNIFORM_HUE "hue"

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
    RPGcolor color;
    struct {
        RPGdouble rate;
        RPGint64 count;
        RPGdouble tick;
        RPGupdatefunc cb;
    } update;
    struct {
        GLuint program;
        GLint projection;
        GLint model;
        GLint color;
        GLint tone;
        GLint flash;
        GLint alpha;
        GLint hue;
    } shader;
    void *user;
} RPGgame;

extern const char *RPG_VERTEX_SHADER;
extern const char *RPG_FRAGMENT_SHADER;

#endif /* OPEN_RPG_GAME_H */