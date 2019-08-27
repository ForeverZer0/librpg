
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
    }
    RPGint x, y;
    RPG_Input_GetCursorLocation(&x, &y);
    // printf("%d, %d\n", x, y);
    
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

    // Get some random background color, and set an app icon
    // RPGcolor backColor = { .x=comp(), .y=comp(), .z=comp(), .w=1.0f };
    RPGcolor backColor = { .x=1, .y=1, .z=1, .w=1.0f };
    RPG_Game_SetBackColor(game, &backColor);
    RPG_Game_SetIconFromFile(game, "/home/eric/Pictures/books-512.png");

    // Create an image
    RPGimage *image; 
    RPGcolor blockColor = { 0.2f, 0.2f, 0.2f, 1.0f };
    RPGcolor fontColor = { 0.3f, 0.3f, 0.3f, 1.1f };
    RPG_Image_CreateFilled(600, 400, &blockColor, &image);


    RPGimage *character;
    RPG_Image_CreateFromFile("/home/eric/Pictures/character.png", &character);




    RPGfont *font;
    // RPG_Font_SetDefaultColor(&fontColor);
    RPG_Font_CreateFromFile("/code/c/open_rpg/assets/fonts/NotoSans-Black.ttf", &font);
    
RPG_Font_DrawText(font, image, "Heljo", NULL, RPG_ALIGN_DEFAULT);


    RPG_Image_Blit(image, NULL, character, NULL, 1.0f);

    // RPG_ALIGN aligns[9] = 
    // { 
    //     RPG_ALIGN_TOP_LEFT, RPG_ALIGN_TOP_CENTER, RPG_ALIGN_TOP_RIGHT, 
    //     RPG_ALIGN_CENTER_LEFT, RPG_ALIGN_CENTER, RPG_ALIGN_CENTER_RIGHT, 
    //     RPG_ALIGN_BOTTOM_LEFT, RPG_ALIGN_BOTTOM_CENTER, RPG_ALIGN_BOTTOM_RIGHT,
    // };
    // for (int i = 0; i < 9; i++) {
    //     RPG_Font_DrawText(font, image, "OpenRPG", NULL, aligns[i], 1.0f);
    // }


    // Create a sprite to display the image
    RPGsprite *sprite1, *sprite2;
    RPG_Sprite_Create(NULL, &sprite1);
    RPG_Sprite_Create(NULL, &sprite2);

    RPG_Sprite_SetImage(sprite1, image);
    RPG_Sprite_SetImage(sprite2, character);

        RPGimage *fog;
    RPG_Image_CreateFromFile("/home/eric/Pictures/RTP/XP/Graphics/Fogs/001-Fog01.png", &fog);

    RPGplane *plane;
    RPG_Plane_Create(NULL, &plane);
    RPG_Plane_SetImage(plane, fog);

    // RPG_Sprite_SetSourceRectValues(sprite, 0, 0, w / 3, h / 4);
    RPG_Renderable_SetLocation((RPGrenderable*) sprite1, 128, 128);
    
    // Play some music
    const char *path = "/home/eric/Desktop/The Blackest Day.ogg";
    RPG_Audio_SetPlaybackCompleteCallback(audio_done, NULL);
    RPG_Audio_Play(0, path, 1.0f, 1.0f, 2);
    RPGaudiofx *e;
    RPG_Reverb_CreateFromType(RPG_REVERB_TYPE_BATHROOM, &e);
    RPG_Audio_AttachEffect(0, e);

    // Cleanup
    RPG_Game_Main(game, 40.0, update);
    RPG_Game_Destroy(game);
}
