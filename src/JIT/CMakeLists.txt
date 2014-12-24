set(
  cheffe-src-files
  CheffeJIT.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )

add_library( CheffeJIT ${cheffe-src-files} )
