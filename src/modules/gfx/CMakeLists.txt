cmake_minimum_required( VERSION 3.10 )

Project( Chik_GFX )

file( GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.c )

get_property( CHIK_GAMES GLOBAL PROPERTY CHIK_GAMES )
get_property( LIBCHIK GLOBAL PROPERTY LIBCHIK )

link_libraries( LibChik SDL2 )

add_library( Chik_GFX SHARED ${SOURCES} )

add_definitions( -DUSE_SDL )

include_directories( ${LIBCHIK} )

set_target_properties(
    Chik_GFX PROPERTIES

    RUNTIME_OUTPUT_NAME chikgfx
    LIBRARY_OUTPUT_NAME chikgfx

    RUNTIME_OUTPUT_DIRECTORY ${CHIK_GAMES}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CHIK_GAMES}/bin
)