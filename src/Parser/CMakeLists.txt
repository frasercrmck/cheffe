set(
  cheffe-src-files
  CheffeParser.cpp
  CheffeScopeInfo.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )

add_library( CheffeParser ${cheffe-src-files} )

target_link_libraries( CheffeParser CheffeLexer CheffeIR )
