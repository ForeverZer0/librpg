//
// Created by eric on 8/20/19.
//

#include "rpg.h"
#include "rpgaudio.h"
#include "stdio.h"

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


int main(int argc, char **argv) {

    const char *path = "/home/eric/Music/flora cash - I Wasted You (Audio)-0-kennkvJLE.ogg";

    RPGgame *game;
    RPG_Game_Create(&game);

    // RPG_Audio_Play(0, path, 1.0f, 1.0f, 5);

    // RPGaudiofx *e;
    // RPG_Flanger_Create(&e);
    // RPG_Audio_AttachEffect(0, e);

    RPG_Game_CreateWindow(game, "librpg", 800, 600, RPG_INIT_DEFAULT);
    RPG_Game_Main(game);
    RPG_Game_Destroy(game);
}
