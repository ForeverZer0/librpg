#include "renderable.h"
#include "glad.h"
#include "rpg.h"

void RPG_Renderable_Init(RPGgame *game, RPGrenderable *renderable, RPGrenderfunc renderfunc) {
    RPG_ASSERT(game);
    RPG_ASSERT(renderfunc);
    renderable->scale     = (RPGvec2) {1.0f, 1.0f};
    renderable->blend.op  = RPG_BLEND_OP_ADD;
    renderable->blend.src = RPG_BLEND_SRC_ALPHA;
    renderable->blend.dst = RPG_BLEND_ONE_MINUS_SRC_ALPHA;
    renderable->visible   = RPG_TRUE;
    renderable->alpha     = 1.0f;
    renderable->updated   = RPG_TRUE;
    renderable->render = renderfunc;
    renderable->game = game;
}