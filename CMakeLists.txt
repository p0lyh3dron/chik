cmake_minimum_required( VERSION 3.10 )

project( Chik VERSION 1.0 )

include( src/scripts/gen.cmake )

set( CMAKE_CXX_STANDARD 20 )

set_property( GLOBAL PROPERTY CHIK       ${CMAKE_CURRENT_LIST_DIR} )
set_property( GLOBAL PROPERTY LIBCHIK    ${CMAKE_CURRENT_LIST_DIR}/libchik )
set_property( GLOBAL PROPERTY CHIK_GAMES ${CMAKE_CURRENT_LIST_DIR}/../games )

add_subdirectory( libchik )

add_subdirectory( src/modules/engine )
add_subdirectory( src/modules/gfx )
add_subdirectory( src/modules/platform )