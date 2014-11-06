cmake_minimum_required ( VERSION 3.0 )

project( cheffe )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )

set( CHEFFE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( src )

set_target_properties( cheffe PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/bin 
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)
