cmake_minimum_required( VERSION 3.0 )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set(
  cheffe-src-files
  cheffe.cpp
)
add_executable( cheffe ${cheffe-src-files} )

add_subdirectory( IR )
add_subdirectory( Lexer )
add_subdirectory( Parser )
add_subdirectory( JIT )
add_subdirectory( Driver )
add_subdirectory( Utils )

target_link_libraries ( cheffe
  CheffeParser CheffeLexer CheffeDriver CheffeUtils CheffeJIT
)

add_library( cheffe_test_lib INTERFACE )
target_link_libraries( cheffe_test_lib INTERFACE
  CheffeParser CheffeLexer CheffeDriver CheffeUtils CheffeJIT
)
