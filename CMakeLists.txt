cmake_minimum_required(VERSION 3.16)

project(DijkstraTesting)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# 1. Fetch ankerl::unordered_dense header-only library
FetchContent_Declare(
    unordered_dense
    GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
    GIT_TAG        v5.3.0
)
FetchContent_MakeAvailable(unordered_dense)

# 2. Fetch Catch2 for unit testing
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.0
)
FetchContent_MakeAvailable(Catch2)

# 3. Dijkstra tests target (dijkstra_example removed)
add_executable(dijkstra_tests
    tests/tests.cpp
    src/dijkstra.cpp
    src/list.cpp
)
target_include_directories(dijkstra_tests PRIVATE include)

# 4. Main application target (with Dijkstra and List sources added)
add_executable(app
    src/main.cpp
    src/core.cpp
    src/control_node.cpp
    src/control_node_container.cpp
    src/coord_edge.cpp
    src/coord_node.cpp
    src/error.cpp
    src/data_bucket.cpp
    src/data_parser.cpp
    src/dijkstra.cpp
    src/list.cpp
)
target_include_directories(app PRIVATE include)
target_link_libraries(app PRIVATE unordered_dense::unordered_dense)
set_target_properties(app PROPERTIES OUTPUT_NAME "app")

# 5. Main project unit tests
add_executable(unit_tests
    tests/test_core.cpp
)
target_include_directories(unit_tests PRIVATE include)
target_link_libraries(unit_tests PRIVATE 
    Catch2::Catch2WithMain 
    unordered_dense::unordered_dense
)

# 6. Testing configuration
enable_testing()
include(CTest)

add_test(NAME DijkstraTests COMMAND dijkstra_tests)
add_test(NAME UnitTests COMMAND unit_tests)