//
// Created by eric on 8/20/19.
//

#include "rpg.h"
#include "stdio.h"

int main(int argc, char **argv) {

    const char *path = "/home/eric/Music/flora cash - I Wasted You (Audio)-0-kennkvJLE.ogg";

    RPGgame *game;
    RPG_Game_Create(&game);

    RPG_Audio_Play(0, path, 1.0f, 1.0f, 5);

    getc(stdin);

    RPGreverb *reverb;
    RPG_Audio_CreateReverb(RPG_REVERB_PRESET_SEWERPIPE, &reverb);
    RPG_Audio_AttachEffect(0, reverb);


    RPG_Game_CreateWindow(game, "librpg", 800, 600, RPG_INIT_DEFAULT);
    RPG_Game_Main(game);
    RPG_Game_Destroy(game);
}
