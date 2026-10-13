cmake_minimum_required(VERSION 3.20)
project(MyProject CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# 1. Fetch Catch2 for Unit Testing
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# 2. Fetch BppTree as a header-only library (bypassing its benchmarks)
FetchContent_Declare(
  BppTree
  GIT_REPOSITORY https://github.com/jeffplaisance/BppTree.git
  GIT_TAG        main
)
FetchContent_GetProperties(BppTree)
if(NOT bpptree_POPULATED)
  FetchContent_Populate(BppTree)
endif()

# Define your main application target
add_executable(app
    src/main.cpp
    src/core.cpp
    src/error.cpp
)
target_include_directories(app PRIVATE include ${bpptree_SOURCE_DIR})
set_target_properties(app PROPERTIES OUTPUT_NAME "app")

# Optional: Unit tests target
enable_testing()
add_executable(unit_tests
    tests/test_core.cpp
)
target_include_directories(unit_tests PRIVATE include ${bpptree_SOURCE_DIR})
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
add_test(NAME UnitTests COMMAND unit_tests)