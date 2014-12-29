set(
  cheffe-src-files
  CheffeParser.cpp
  CheffeRecipeInfo.cpp
  CheffeProgramInfo.cpp
  CheffeMethodStep.cpp
)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../ )

add_library( CheffeParser ${cheffe-src-files} )

target_link_libraries( CheffeParser CheffeLexer )
