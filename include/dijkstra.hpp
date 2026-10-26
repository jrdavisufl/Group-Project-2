#ifndef DIJKSTRA_HPP
#define DIJKSTRA_HPP

#include "list.hpp"
#include <cstddef>
#include <vector>

struct DijkstraResult {
    bool found = false;
    double totalDistance = 0.0; // meters, when input weights are meters
    std::vector<long long> path; // includes start and destination
    std::size_t nodesExplored = 0;
    std::size_t peakHeapSize = 0;
};

// throws std::invalid_argument for unknown endpoints or invalid edge weights.
DijkstraResult Dijkstra(const Graph& graph, long long startID, long long endID);

#endif
