
#include "renderable.h"

static void RPG_Viewport_Render(void *viewport) {
    RPGviewport *v = viewport;


}

RPG_RESULT RPG_Viewport_Create(RPGgame *game, RPGint x, RPGint y, RPGint width, RPGint height, RPGviewport **viewport) {
    RPG_RETURN_IF_NULL(game);
    RPG_RETURN_IF_NULL(*viewport);
    if (width < 1 || height < 1) {
        return RPG_ERR_OUT_OF_RANGE;
    }

    RPG_ALLOC_ZERO(v, RPGviewport);
    RPG_Renderable_Init(game, &v->base, RPG_Viewport_Render, &game->batch);
    RPG_Batch_Init(&v->batch);

    
}

RPG_RESULT RPG_Viewport_CreateFromRect(RPGgame *game, RPGrect *rect, RPGviewport **viewport);
RPG_RESULT RPG_Viewport_CreateDefault(RPGgame *game, RPGviewport **viewport);

RPG_RESULT RPG_Viewport_GetRect(RPGviewport *viewport, RPGint *x, RPGint *y, RPGint *width, RPGint *height);