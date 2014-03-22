set(
  cheffe-src-files
  CheffeParser.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )

add_library( CheffeParser ${cheffe-src-files} )
