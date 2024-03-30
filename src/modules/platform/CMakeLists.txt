cmake_minimum_required( VERSION 3.10 )

Project( Chik_Platform )

file( GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.c *.h )

get_property( CHIK_GAMES GLOBAL PROPERTY CHIK_GAMES )
get_property( LIBCHIK GLOBAL PROPERTY LIBCHIK )

link_libraries( LibChik SDL2 )

message( "LIBCHIK ${LIBCHIK}" )

if ( MSVC )
    link_directories( "${LIBCHIK}/../../SDL2/lib/x64" )
    include_directories( "${LIBCHIK}" "${LIBCHIK}/../../SDL2/include" )
else()
    link_libraries( asound )
    include_directories( "${LIBCHIK}" /usr/include/SDL2 )
endif()

add_library( Chik_Platform SHARED ${SOURCES} )

# add_definitions( -DUSE_SDL -DUSE_ALSA )
add_definitions( -DUSE_SDL )


set_target_properties(
    Chik_Platform PROPERTIES

    RUNTIME_OUTPUT_NAME chikplatform
    LIBRARY_OUTPUT_NAME chikplatform

    RUNTIME_OUTPUT_DIRECTORY ${CHIK_GAMES}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CHIK_GAMES}/bin
)

# set output directories for all builds (Debug, Release, etc.)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set_target_properties(
    	Chik_Platform PROPERTIES
    	RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CHIK_GAMES}/bin
    	LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CHIK_GAMES}/bin
    )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
