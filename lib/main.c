
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
        RPGint64 ms;
        // printf("%d\n", sec);
        RPG_Audio_GetPosition(0, &ms);
        printf("%f ms\n", ms / 1000.0);
    }

    
}

static inline float comp(void) {
    return (float) rand() / (float) RAND_MAX ;
}

void audio_done(RPGint channel) {
    printf("Channel %d is done playing.\n");
}


int main(int argc, char **argv) {
    srand(time(NULL));

    // Let's create a game object, and make a window for it
    RPGgame *game;
    RPG_RESULT r = RPG_Game_Create("OpenRPG", 800, 600, RPG_INIT_DEFAULT | RPG_INIT_RESIZABLE, &game);
    // RPG_Audio_SetCallback(RPG_AUDIO_CB_DONE, audio_done);

    // Get some random background color, and set an app icon
    RPGcolor color = { .x=comp(), .y=comp(), .z=comp(), .w=1.0f };
    RPG_Game_SetBackColor(game, &color);
    RPG_Game_SetIconFromFile(game, "/home/eric/Pictures/books-512.png");

    RPGviewport *viewport;
    RPG_Viewport_Create(200, 96, 300, 300, &viewport);

    // Create an image
    RPGimage *image, *src;
    RPGcolor back = { 0.0f, 0.0f, 0.0f, 1.0f };
    RPGcolor fill = { 0.3f, 0.3f, 1.3f, 1.0f };
    RPGint w, h;
    RPG_Image_CreateFromFile("/home/eric/Pictures/character.png", &src);
    RPG_Image_CreateFilled(400, 128, &back, &image);
    RPG_Image_GetSize(image, &w, &h);


    RPGrect dr = { 0, 0, 64, 64 };
    RPGfont *font;
    RPG_RESULT result = RPG_Font_CreateFromFile("/code/c/open_rpg/assets/fonts/NotoSans-Black.ttf", &font);
    RPG_Font_DrawText(font, image, "Hello from OpenRPG!", &dr, RPG_ALIGN_DEFAULT, 1.0f);


    // RPGrect d = { 0, 0, 32, 64 };
    // RPGrect s = { 0, 0, 32, 32 };
    // RPG_Image_Blit(image, &d, src, &s, 1.0);

    // Create a sprite to display the image
    RPGsprite *sprite;
    RPG_Sprite_Create(viewport, &sprite);
    RPG_Sprite_SetImage(sprite, image);
    // RPG_Sprite_SetSourceRectValues(sprite, 0, 0, w / 3, h / 4);
    RPG_Renderable_SetLocation((RPGrenderable*) sprite, 32, 64);
    
    // // Play some music
    // const char *path = "/home/eric/Music/flora cash - I Wasted You (Audio)-0-kennkvJLE.ogg";
    // // const char *path = "/home/eric/Pictures/RTP/XP/Audio/BGS/012-Waterfall02.ogg";
    // RPG_Audio_Play(0, path, 1.0f, 1.0f, 5);
    // RPGaudiofx *e;
    // RPG_Reverb_CreateFromType(RPG_REVERB_TYPE_SEWERPIPE, &e);
    // RPG_Audio_AttachEffect(0, e);

    // Cleanup
    RPG_Game_Main(game, 40.0, update);
    RPG_Game_Destroy(game);
    RPG_Audio_Terminate();
}
