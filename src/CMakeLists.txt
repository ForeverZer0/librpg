

add_library(rpg SHARED
        ${PROJECT_SOURCE_DIR}/include/RPG/rpg.h
        ${PROJECT_SOURCE_DIR}/include/RPG/rpgaudio.h
        ${PROJECT_SOURCE_DIR}/include/RPG/rpgcolors.h
        include/platform.h
        internal.h
        glad.c
        audio.c
        batch.c
        input.c
        game.c
        plane.c
        font.c
        shader.c
        stb_image.h 
        stb_image_write.h 
        stb_truetype.h
        utf8.h
        image.c
        shaders.c uthash.h renderable.c sprite.c viewport.c renderable.h)

add_compile_definitions(RPG_DEBUG)

set_target_properties(rpg PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(rpg PROPERTIES SOVERSION 1)
# TODO: Audd rpgaudio.h to public headers


set_target_properties(rpg PROPERTIES PUBLIC_HEADER include/RPG/rpg.h)

target_link_libraries(rpg glfw m pthread openal sndfile)