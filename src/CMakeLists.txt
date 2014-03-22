set(
  cheffe-src-files
  cheffe.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( cheffe ${cheffe-src-files} )

add_subdirectory( Lexer )
add_subdirectory( Parser )

target_link_libraries ( cheffe CheffeParser CheffeLexer )
