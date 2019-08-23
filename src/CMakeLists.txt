

add_library(rpg SHARED
        ${PROJECT_SOURCE_DIR}/include/RPG/rpg.h
        ${PROJECT_SOURCE_DIR}/include/RPG/rpgaudio.h
        include/platform.h
        
        internal.h

        glad.c
        audio.c
        game.c
        stb_image.h 
        stb_image_write.h 
        image.c)

set_target_properties(rpg PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(rpg PROPERTIES SOVERSION 1)
set_target_properties(rpg PROPERTIES PUBLIC_HEADER include/RPG/rpg.h)

target_link_libraries(rpg glfw openal sndfile)