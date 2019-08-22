//
// Created by eric on 8/20/19.
//

#include "rpg.h"
#include "graphics.h"
#include "stdio.h"

int main(int argc, char **argv) {

    const char *path = "/home/eric/Music/flora cash - I Wasted You (Audio)-0-kennkvJLE.ogg";

    RPG_Initialize();

    RPG_RESULT result = RPG_Audio_Play(0, path, 1.0f, 1.0f, 0);

    RPGint64 ms = 1000 * 76;


    RPG_GRAPHICS *graphics;
    RPG_Graphics_Create("librpg", 800, 600, RPG_INIT_DEFAULT, &graphics);

    RPG_Graphics_Main(graphics);


    // RPG_SOUND *sound;
    // printf("%s\n", RPG_Sound_Create("/home/eric/Music/flora cash - I Wasted You (Audio)-0-kennkvJLE.ogg", &sound) == RPG_ERR_FILE_NOT_FOUND ? "not found" : "found");

    // int channels, rate, seekable, format, type;
    // RPG_Sound_GetChannelCount(sound, &channels);
    // RPG_Sound_GetSampleRate(sound, &rate);
    // RPG_Sound_GetFormat(sound, (RPG_SOUND_FORMAT*) &format);
    // RPG_Sound_GetType(sound, (RPG_SOUND_TYPE*)&type);

    // RPGint64 ms;
    // RPG_Sound_GetDuration(sound, &ms);

    // RPGint64 min = ms / 60000;
    // ms -= min * 60000;

    // RPGint64 sec = ms / 1000;
    // ms -= sec * 1000;
    // printf("%d min, %d sec, %d ms\n", min, sec, ms);
    

    // RPG_CHANNEL *chan1, *chan2, *chan3, *chan4;
    // assert(!RPG_Channel_Create(sound, 0, NULL, &chan1));
    // assert(!RPG_Channel_Create(sound, 0, NULL, &chan2));
    // assert(!RPG_Channel_Create(sound, 0, NULL, &chan3));
    // assert(!RPG_Channel_Create(sound, 0, NULL, &chan4));

 

    RPG_Terminate();
}
