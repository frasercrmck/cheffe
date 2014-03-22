cmake_minimum_required ( VERSION 2.6 )

project( cheffe )

add_subdirectory( src )

set_target_properties( cheffe PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/bin 
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)
