

add_library(rpg SHARED
        ${PROJECT_SOURCE_DIR}/include/RPG/rpg.h
        include/platform.h

        include/internal.h
        include/graphics.h

        graphics/glad.c
        audio.c
        rpg.c
        graphics/graphics.c )

set_target_properties(rpg PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(rpg PROPERTIES SOVERSION 1)
set_target_properties(rpg PROPERTIES PUBLIC_HEADER include/RPG/rpg.h)

target_link_libraries(rpg glfw openal sndfile)