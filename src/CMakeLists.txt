cmake_minimum_required(VERSION 2.8.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

set(
  cheffe-src-files
  cheffe.cpp
)
add_executable( cheffe ${cheffe-src-files} )

add_subdirectory( Lexer )
add_subdirectory( Parser )
add_subdirectory( Utils )

target_link_libraries ( cheffe CheffeParser CheffeLexer CheffeUtils )
