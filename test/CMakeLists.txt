add_subdirectory( gtest-1.7.0 )

include_directories( ${CHEFFE_ROOT_DIR}/src )
include_directories( gtest-1.7.0/include gtest-1.7.0 )

add_definitions( -DTEST_ROOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}" )

add_executable( cheffe_test
  CheffeParserTest.cpp
  CheffeJITExecutionTest.cpp
  CheffeDiagnosticsTest.cpp
)

target_link_libraries( cheffe_test cheffe_test_lib gtest gtest_main )
