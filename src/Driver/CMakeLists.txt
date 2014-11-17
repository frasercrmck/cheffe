set(
  cheffe-src-files
  CheffeDriver.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )

add_library( CheffeDriver ${cheffe-src-files} )

