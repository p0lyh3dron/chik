cmake_minimum_required( VERSION 3.10 )

Project( Chik_GFXVK )

file( GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.c )

get_property( CHIK_GAMES GLOBAL PROPERTY CHIK_GAMES )
get_property( LIBCHIK GLOBAL PROPERTY LIBCHIK )

link_libraries( LibChik SDL2 )

add_library( Chik_GFXVK SHARED ${SOURCES} )

add_definitions( -DUSE_SDL )

include_directories( ${LIBCHIK} )

set_target_properties(
    Chik_GFXVK PROPERTIES

    RUNTIME_OUTPUT_NAME chikgfxVK
    LIBRARY_OUTPUT_NAME chikgfxVK

    RUNTIME_OUTPUT_DIRECTORY ${CHIK_GAMES}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CHIK_GAMES}/bin
)