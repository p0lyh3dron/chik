cmake_minimum_required( VERSION 3.10 )

Project( Chik_GFX )

file( GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.c *.h )

get_property( CHIK_GAMES GLOBAL PROPERTY CHIK_GAMES )
get_property( LIBCHIK GLOBAL PROPERTY LIBCHIK )

link_libraries( LibChik SDL2 )

if ( MSVC )
    link_directories( "${LIBCHIK}/../../SDL2/lib/x64" )
    include_directories( "${LIBCHIK}" "${LIBCHIK}/../../SDL2/include" )
endif()

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

# set output directories for all builds (Debug, Release, etc.)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set_target_properties(
    	Chik_GFX PROPERTIES
    	RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CHIK_GAMES}/bin
    	LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CHIK_GAMES}/bin
    )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
