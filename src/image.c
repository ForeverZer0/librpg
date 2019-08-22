
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define STBI_MALLOC RPG_MALLOC
#define STBI_REALLOC RPG_REALLOC
#define STBI_FREE RPG_FREE
#define STBIW_MALLOC STBI_MALLOC
#define STBIW_REALLOC STBI_REALLOC
#define STBIW_FREE STBI_FREE

#include "rpg.h"
#include "stb_image.h"
#include "stb_image_write.h"


