cmake_minimum_required( VERSION 3.10 )

Project( Chik_Audio )

file( GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.c *.h )

get_property( CHIK_GAMES GLOBAL PROPERTY CHIK_GAMES )
get_property( LIBCHIK GLOBAL PROPERTY LIBCHIK )

link_libraries( LibChik )

add_library( Chik_Audio SHARED ${SOURCES} )

include_directories( ${LIBCHIK} )

set_target_properties(
    Chik_Audio PROPERTIES

    RUNTIME_OUTPUT_NAME chikaudio
    LIBRARY_OUTPUT_NAME chikaudio

    RUNTIME_OUTPUT_DIRECTORY ${CHIK_GAMES}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CHIK_GAMES}/bin
)

# set output directories for all builds (Debug, Release, etc.)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set_target_properties(
    	Chik_Audio PROPERTIES
    	RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CHIK_GAMES}/bin
    	LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CHIK_GAMES}/bin
    )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
