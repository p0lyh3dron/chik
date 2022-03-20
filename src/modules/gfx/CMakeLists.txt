cmake_minimum_required( VERSION 3.10 )

Project( Chik_GFX )

file( GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.c )

add_library( Chik_GFX SHARED ${SOURCES} )

get_property( CHIK_GAMES GLOBAL PROPERTY CHIK_GAMES )

set_target_properties(
    Chik_GFX PROPERTIES

    RUNTIME_OUTPUT_NAME chikgfx
    LIBRARY_OUTPUT_NAME chikgfx

    RUNTIME_OUTPUT_DIRECTORY ${CHIK_GAMES}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CHIK_GAMES}/bin
)