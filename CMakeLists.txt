cmake_minimum_required(VERSION 3.16)

project(DijkstraTesting)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# run the sample graph
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

enable_testing()
add_test(NAME DijkstraTests COMMAND dijkstra_tests)
