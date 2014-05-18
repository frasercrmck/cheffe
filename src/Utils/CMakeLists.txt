set(
  cheffe-src-files
  CheffeDiagnosticHandler.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )


add_library( CheffeUtils ${cheffe-src-files} )
