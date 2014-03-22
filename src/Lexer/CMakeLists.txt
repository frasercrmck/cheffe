set(
  cheffe-src-files
  CheffeLexer.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )

add_library( CheffeLexer ${cheffe-src-files} )
