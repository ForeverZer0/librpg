#ifndef OPEN_RPG_GAME_H
#define OPEN_RPG_GAME_H 1

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glad.h"
#include "rpg.h"

#define BATCH_INIT_CAPACITY 4

#define UNIFORM_PROJECTION "projection"
#define UNIFORM_MODEL "model"
#define UNIFORM_COLOR "color"
#define UNIFORM_TONE "tone"
#define UNIFORM_ALPHA "alpha"
#define UNIFORM_HUE "hue"

#define RPG_RESET_BACK_COLOR(game) glClearColor(game->color.x, game->color.y, game->color.z, game->color.w)

/**
 * @brief Container for a rendering batch, with a quick-sort based on sprite's position on the z-axis.
 */
typedef struct {
    RPGrenderable **items; /** An array of pointers to the sprites within this batch. */
    int capacity;          /** The total capacity the batch can hold before reallocation. */
    int total;             /** The total number of sprites within the batch. */
    GLboolean updated;     /** Flag indicating the items may need reordered due to added entry or change of z-axis. */
} RPGbatch;

typedef struct RPGgame {
    GLFWwindow *window;
    RPGmat4 projection;
    RPGbatch batch;
    struct {
        RPGint width;
        RPGint height;
    } resolution;
    RPGbool autoAspect;
    struct {
        RPGint x, y, w, h;
        RPGvec2 ratio;
    } bounds;
    RPG_INIT_FLAGS flags;
    char *title;
    RPGcolor color;
    struct {
        RPGdouble rate;
        RPGint64 count;
        RPGdouble tick;
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


void RPG_Batch_Init(RPGbatch *batch);
void RPG_Batch_Free(RPGbatch *v);
int RPG_Batch_Total(RPGbatch *batch);
void RPG_Batch_Add(RPGbatch *batch, RPGrenderable *item);
void RPG_Batch_Set(RPGbatch *batch, int index, RPGrenderable *item);
RPGrenderable *RPG_Batch_Get(RPGbatch *batch, int index);
void RPG_Batch_Delete(RPGbatch *batch, int index);
void RPG_Batch_DeleteItem(RPGbatch *batch, RPGrenderable *item);
void RPG_Batch_Sort(RPGbatch *batch, int first, int last);

#endif /* OPEN_RPG_GAME_H */