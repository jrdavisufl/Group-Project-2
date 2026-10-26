#include "dijkstra.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    // offline sanity test
    Graph graph;
    graph.addNode(29.0, -82.0, 1001);
    graph.addNode(29.0, -82.0, 1002);
    graph.addNode(29.0, -82.0, 1003);
    graph.addNode(29.0, -82.0, 1004);
    graph.addEdge(1001, 1002, 100, "First");
    graph.addEdge(1001, 1003, 500, "Second");
    graph.addEdge(1002, 1003, 50, "Third");
    graph.addEdge(1002, 1004, 300, "Fourth");
    graph.addEdge(1003, 1004, 80, "Fifth");

    try {
        DijkstraResult result = Dijkstra(graph, 1001, 1004);
        if (!result.found) {
            std::cout << "No route found\n";
            return 0;
        }
        std::cout << "Distance: " << result.totalDistance << " meters\nPath: ";
        for (std::size_t i = 0; i < result.path.size(); ++i) {
            if (i) std::cout << " -> ";
            std::cout << result.path[i];
        }
        std::cout << "\nVertices explored: " << result.nodesExplored
                  << "\nPeak heap entries: " << result.peakHeapSize << '\n';
    } catch (const std::exception& ex) {
        std::cerr << "Routing error: " << ex.what() << '\n';
        return 1;
    }
}
