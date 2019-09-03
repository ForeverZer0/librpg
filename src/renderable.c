#include "internal.h"

void RPG_Renderable_Init(RPGrenderable *renderable, RPGrenderfunc renderfunc, RPGbatch *batch)
{
    RPG_ASSERT(renderfunc);
    renderable->z       = 0;
    renderable->visible = RPG_TRUE;
    renderable->render  = renderfunc;
    renderable->parent  = batch;
    RPG_Batch_Add(batch, renderable);
}

RPG_RESULT RPG_Renderable_Free(RPGrenderable *renderable) 
{
    RPG_RETURN_IF_NULL(renderable);
    if (renderable->parent != NULL)
    {
        RPG_Batch_DeleteItem(renderable->parent, renderable);
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_GetVisible(RPGrenderable *renderable, RPGbool *visible)
{
    RPG_RETURN_IF_NULL(renderable);
    if (visible != NULL)
    {
        *visible = renderable->visible;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetVisible(RPGrenderable *renderable, RPGbool visible)
{
    RPG_RETURN_IF_NULL(renderable);
    renderable->visible = visible;
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_GetZ(RPGrenderable *renderable, RPGint *z)
{
    RPG_RETURN_IF_NULL(renderable);
    if (z != NULL)
    {
        *z = renderable->z;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetZ(RPGrenderable *renderable, RPGint z)
{
    RPG_RETURN_IF_NULL(renderable);
    if (z != renderable->z)
    {
        renderable->z               = z;
        renderable->parent->updated = RPG_TRUE;
    }
    return RPG_NO_ERROR;
}

RPG_RESULT RPG_Renderable_SetRenderFunc(RPGrenderable *renderable, RPGrenderfunc func)
{
    RPG_RETURN_IF_NULL(renderable);
    RPG_RETURN_IF_NULL(func);
    renderable->render = func;
    return RPG_NO_ERROR;
}