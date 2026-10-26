cmake_minimum_required(VERSION 3.16)

project(DijkstraTesting)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Fetch ankerl::unordered_dense header-only library (from main)
include(FetchContent)
FetchContent_Declare(
    unordered_dense
    GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
    GIT_TAG        v5.3.0
)
FetchContent_MakeAvailable(unordered_dense)

# 2. Dijkstra sample graph and tests (from HEAD)
add_executable(dijkstra_example
        example_main.cpp
        dijkstra.cpp
        list.cpp
)

add_executable(dijkstra_tests
        tests.cpp
        dijkstra.cpp
        list.cpp
)

# 3. Main application target (from main)
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

# 4. Main project unit tests (from main)
add_executable(unit_tests
    tests/test_core.cpp
)
target_link_libraries(unit_tests PRIVATE 
    Catch2::Catch2WithMain 
    unordered_dense::unordered_dense
)

# 5. Testing configuration (combined from both)
enable_testing()
include(CTest)

add_test(NAME DijkstraTests COMMAND dijkstra_tests)
add_test(NAME UnitTests COMMAND unit_tests)