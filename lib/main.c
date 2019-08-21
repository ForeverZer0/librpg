//
// Created by eric on 8/20/19.
//

#include "rpg.h"
#include "audio.h"
#include "stdio.h"

int main(int argc, char **argv) {

    RPG_Initialize();

    RPG_SOUND *sound;
    RPG_Sound_Create("/home/eric/Desktop/AWOLNATION - Sail.ogg", &sound);

    int channels, rate, seekable, format, type;
    RPG_Sound_GetChannelCount(sound, &channels);
    RPG_Sound_GetSampleRate(sound, &rate);
    RPG_Sound_GetFormat(sound, (RPG_SOUND_FORMAT*) &format);
    RPG_Sound_GetType(sound, (RPG_SOUND_TYPE*)&type);

    GLint64 ms;
    RPG_Sound_GetDuration(sound, &ms);

    GLint64 min = ms / 60000;
    ms -= min * 60000;

    GLint64 sec = ms / 1000;
    ms -= sec * 1000;
    printf("%d min, %d sec, %d ms\n", min, sec, ms);
    

    RPG_CHANNEL *channel;
    RPG_RESULT r = RPG_Channel_Create(sound, 0, NULL, &channel);

    RPGfloat vol;
    RPG_Channel_GetVolume(channel, &vol);

    printf("VOLUME: %f\n", vol);

    RPG_Channel_Play(channel);

    getc(stdin);

    RPG_Sound_Free(sound);
    RPG_Terminate();
}
