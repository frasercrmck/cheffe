set(
  cheffe-src-files
  CheffeDebugUtils.cpp
  CheffeFileHandler.cpp
  CheffeDiagnosticHandler.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )


add_library( CheffeUtils ${cheffe-src-files} )
