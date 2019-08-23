#include "glad.h"
#include "internal.h"
#include "renderable.h"

typedef struct RPGsprite {
    RPGrenderable base;
    RPGimage *img;
    GLuint vbo;
    GLuint vao;
} RPGsprite;

void RPG_Sprite_Render(void *renderable) {
    RPGsprite *s = renderable;
    if (!s->base.visible || s->base.alpha < __FLT_EPSILON__ || s->img == NULL) {
        // No-op if sprite won't be visible
        return;
    }
    if (s->base.updated) {

    }
    RPG_BASE_UNIFORMS(s->base);

}

RPG_RESULT RPG_Sprite_Free(RPGsprite *sprite) {
    RPG_RETURN_IF_NULL(sprite);
    glDeleteBuffers(1, &sprite->vbo);
    glDeleteVertexArrays(1, &sprite->vao);
    RPG_FREE(sprite);
}

RPG_RESULT RPG_Sprite_Create(RPGgame *game, RPGsprite **sprite) {
    RPG_ALLOC_ZERO(s, RPGsprite);
    RPG_Renderable_Init(game, &s->base, RPG_Sprite_Render);


    *sprite = s;
    return RPG_NO_ERROR;
}