
#include "rpg.h"
#include "rpgaudio.h"
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

static void update(RPGint64 tick) {

    if ((tick % 40) == 0) {
        sec++;
        printf("%d\n", sec);
    }
}

static inline float comp(void) {
    return (float) rand() / (float) RAND_MAX ;
}


int main(int argc, char **argv) {
    srand(time(NULL));
    RPG_Audio_Initialize();


    

    const char *path = "/home/eric/Music/flora cash - I Wasted You (Audio)-0-kennkvJLE.ogg";
    // const char *path = "/home/eric/Pictures/RTP/XP/Audio/BGS/012-Waterfall02.ogg";

    RPGgame *game;
    RPG_Game_Create(&game);
    RPG_Game_CreateWindow(game, "librpg", 800, 600, RPG_INIT_DEFAULT);
    
    RPGcolor color = { .x=comp(), .y=comp(), .z=comp(), .w=1.0f };
    RPG_Game_SetBackColor(game, &color);


    RPG_Audio_Play(0, path, 1.0f, 1.0f, 5);

    RPGaudiofx *e;
    RPG_Reverb_CreateFromType(RPG_REVERB_TYPE_SEWERPIPE, &e);
    RPG_Audio_AttachEffect(0, e);

    
    RPG_Game_SetUpdateCallback(game, update);
    RPG_Game_Main(game, 40.0);
    RPG_Game_Destroy(game);

    RPG_Audio_Terminate();
}
