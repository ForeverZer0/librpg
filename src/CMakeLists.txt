

add_library(rpg SHARED
        ${PROJECT_SOURCE_DIR}/include/RPG/rpg.h
        glad.c
        include/audio.h audio/sound.c audio/audio.c audio/channel.c rpg.c include/internal.h)

set_target_properties(rpg PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(rpg PROPERTIES SOVERSION 1)
set_target_properties(rpg PROPERTIES PUBLIC_HEADER include/RPG/rpg.h)


target_link_libraries(rpg glfw openal sndfile)