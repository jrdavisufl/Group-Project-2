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

# 2. Fetch ankerl::unordered_dense header-only library
FetchContent_Declare(
  unordered_dense
  GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
  GIT_TAG        v5.3.0
)
FetchContent_MakeAvailable(unordered_dense)

# Define your main application target
add_executable(app
    src/main.cpp
    src/core.cpp
    src/control_node.cpp
    src/control_node_container.cpp
    src/error.cpp
)
target_include_directories(app PRIVATE include)
target_link_libraries(app PRIVATE unordered_dense::unordered_dense)
set_target_properties(app PROPERTIES OUTPUT_NAME "app")

# Optional: Unit tests target
enable_testing()
add_executable(unit_tests
    tests/test_core.cpp
)
target_link_libraries(unit_tests PRIVATE 
    Catch2::Catch2WithMain 
    unordered_dense::unordered_dense
)

include(CTest)
add_test(NAME UnitTests COMMAND unit_tests)