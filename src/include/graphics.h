
#ifndef OPEN_RPG_GRAPHICS_H
#define OPEN_RPG_GRAPHICS_H 1

#include "rpg.h"

// FIXME: Temp header, will be merged with rpg.h

typedef struct RPG_GRAPHICS RPG_GRAPHICS;

typedef enum {
    RPG_INIT_NONE               = 0x0000,       /* No flags */
    RPG_INIT_AUTO_ASPECT        = 0x0001,       /* A resized window resizes graphics, but applies pillars/letterbox to maintain internal resolution */
    RPG_INIT_LOCK_ASPECT        = 0x0002,       /* A resized window resizes graphics, but only to dimensions that match the graphics aspect ratio */
    RPG_INIT_RESIZABLE          = 0x0004,       /* WIndow will be resizable by user, graphics will fill client area of the window */
    RPG_INIT_DECORATED          = 0x0008,       /* Window will have border, titlebar, widgets for close, minimize, etc */
    RPG_INIT_FULLSCREEN         = 0x0010,       /* Start window in fullscreen mode */
    RPG_INIT_HIDDEN             = 0x0020,       /* Do not display newly created window initially */
    RPG_INIT_CENTERED           = 0x0040,       /* Center window on the screen (ignored for fullscreen windows) */

    /* Auto-aspect, decorated, and centered */
    RPG_INIT_DEFAULT = RPG_INIT_AUTO_ASPECT | RPG_INIT_DECORATED | RPG_INIT_CENTERED, 
    RPG_INIT_FORCE_UNSIGNED     = 0xFFFFFFFF    /* Forces enum to use unsigned values, do not use */
} RPG_INIT_FLAGS;

RPG_RESULT RPG_Graphics_Initialize(void);
RPG_RESULT RPG_Graphics_Terminate(void);
RPG_RESULT RPG_Graphics_Create(const char *title, RPGint width, RPGint height, RPG_INIT_FLAGS flags, RPG_GRAPHICS **graphics);

RPG_RESULT RPG_Graphics_Main(RPG_GRAPHICS *gfx);

#endif /*OPEN_RPG_GRAPHICS_H*/
