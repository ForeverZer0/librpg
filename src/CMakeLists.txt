set(libs glfw openal sndfile) 

# zlib
include(FindZLIB)
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})
list(APPEND libs ${ZLIB_LIBRARIES})

# libxml2
include(FindLibXml2)
find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
list(APPEND libs ${LIBXML2_LIBRARIES})

set(RPG_HEADERS
        ${PROJECT_SOURCE_DIR}/include/RPG/rpg.h
        ${PROJECT_SOURCE_DIR}/include/RPG/rpgaudio.h
        ${PROJECT_SOURCE_DIR}/include/RPG/rpgext.h)

set(RPG_SOURCES
        tmx/tmx.h
        tmx/tmx.c
        tmx/tmx_utils.c
        tmx/tmx_err.c
        tmx/tmx_xml.c
        tmx/tmx_mem.c
        tmx/tmx_hash.c
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
        assets.c
        utf8.h
        image.c
        tilemap.c
        uthash.h 
        basic.c 
        renderable.c
        sprite.c 
        viewport.c)

add_library(rpg SHARED ${RPG_HEADERS} ${RPG_SOURCES})

set_target_properties(rpg PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(rpg PROPERTIES SOVERSION 1)
# TODO: Audd rpgaudio.h to public headers ?
# GNUInstall, etc


set_target_properties(rpg PROPERTIES PUBLIC_HEADER include/RPG/rpg.h)

target_link_libraries(rpg PRIVATE ${libs})