
#include "rpg.h"
#include "rpgaudio.h"
#include "rpgext.h"
#include "stdio.h"
#include <time.h>

// static void test_reverb_presets(void) {
//     RPGaudiofx *reverb;
//     char buffer[256];
//     RPGsize written;
//     char c;
//     for (int i = 0; i <= RPG_REVERB_TYPE_LAST; i++) {
//         RPG_Reverb_CreateFromType((RPG_REVERB_TYPE) i , &reverb);
//         RPG_Audio_AttachEffect(0, reverb);
//         RPG_Reverb_GetPresetDescription(i, buffer, 256, &written);
//         printf("%s\n", buffer);
//         c = getc(stdin);
//         if (c == 'q') {
//             break;
//         }
//         RPG_Audio_ReleaseEffect(reverb);
//     }
// }

int sec;
RPGplane *plane;
RPGtilemap *tilemap;

static void update(RPGint64 tick) {

    if ((tick % 40) == 0) {
        sec++;
        // RPGint64 ms;
        // printf("%d\n", sec);
    }
    RPGint x, y;
    RPG_Plane_GetOrigin(plane, &x, &y);
    RPG_Plane_SetOrigin(plane, x + 1, y + 1);

    RPGbool trigger;
    RPG_Input_KeyTrigger(RPG_KEY_A, &trigger);

    if (trigger) {
        printf("TRIGGER\n");
    }

    RPG_Tilemap_Update(tilemap);
}

#include <math.h>

int main(int argc, char **argv) {
    srand(time(NULL));

    // Let's create a game object, and make a window for it
    RPGgame *game;
    RPG_Game_Create("OpenRPG", 800, 600, RPG_INIT_DEFAULT | RPG_INIT_RESIZABLE, &game);
    RPG_Game_SetIconFromFile(game, "/home/eric/Pictures/books-512.png");

    RPGviewport *viewport;
    RPG_Viewport_CreateDefault(&viewport);


    // RPGimage *fog;
    // RPG_Image_CreateFromFile("/home/eric/Pictures/RTP/XP/Graphics/Fogs/001-Fog01.png", &fog);
    // RPG_Plane_Create(NULL, &plane);
    // RPG_Plane_SetImage(plane, fog);
    // RPG_Renderable_SetAlpha((RPGrenderable*) plane, 0.3);

    // RPGsprite *sprite;
    // RPG_Sprite_Create(NULL, &sprite);
    // RPG_Sprite_SetImage(sprite, fog);
    // RPG_Renderable_SetLocation((RPGrenderable*) sprite, 128, 128);
    // RPG_Renderable_SetAlpha((RPGrenderable*) viewport, 0.5f);

    // const char *mapPath = "/home/eric/Desktop/sample/island.tmx";
    const char *mapPath = "/home/eric/Desktop/sample/xpbigmap.tmx";

    RPG_Tilemap_CreateFromFile(mapPath, NULL, &tilemap);
    // RPG_Tilemap_SetOrigin(tilemap, 96, 128);
    
    // Play some music
    // const char *path = "/home/eric/Desktop/The Blackest Day.ogg";
    const char *path = "/home/eric/Soulseek Downloads/complete/sposker/Press/03 I Wasted You.flac";
    RPG_Audio_Play(0, path, 1.0f, 1.0f, 2);
    RPGaudiofx *e;
    RPG_Reverb_CreateFromType(RPG_REVERB_TYPE_BATHROOM, &e);
    RPG_Audio_AttachEffect(0, e);

    // Cleanup
    RPG_Game_Main(game, 40.0, update);
    RPG_Game_Destroy(game);
}
