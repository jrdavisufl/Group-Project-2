#include "dijkstra.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
struct HeapEntry {
    long long id;
    double distance;
};

// own binary min-heap; no std::priority_queue or heap algorithms.
class MinHeap {
public:
    bool empty() const { return items.empty(); }
    std::size_t size() const { return items.size(); }

    void push(HeapEntry value) {
        items.push_back(value);
        std::size_t i = items.size() - 1;
        while (i > 0) {
            std::size_t parent = (i - 1) / 2;
            if (items[parent].distance <= items[i].distance) break;
            std::swap(items[parent], items[i]);
            i = parent;
        }
    }

    HeapEntry pop() {
        if (items.empty()) throw std::out_of_range("cannot pop an empty heap");
        HeapEntry smallest = items.front();
        items.front() = items.back();
        items.pop_back();
        std::size_t i = 0;
        while (2 * i + 1 < items.size()) {
            std::size_t left = 2 * i + 1;
            std::size_t right = left + 1;
            std::size_t child = left;
            if (right < items.size() && items[right].distance < items[left].distance)
                child = right;
            if (items[i].distance <= items[child].distance) break;
            std::swap(items[i], items[child]);
            i = child;
        }
        return smallest;
    }
private:
    std::vector<HeapEntry> items;
};
}

DijkstraResult Dijkstra(const Graph& graph, long long startID, long long endID) {
    if (graph.nodes.find(startID) == graph.nodes.end() ||
        graph.nodes.find(endID) == graph.nodes.end()) {
        throw std::invalid_argument("Start or end node is absent from the graph");
    }

    DijkstraResult result;
    if (startID == endID) {
        result.found = true;
        result.path = {startID};
        return result;
    }

    std::unordered_map<long long, double> bestDistance;
    std::unordered_map<long long, long long> parent;
    std::unordered_set<long long> settled;
    MinHeap heap;

    bestDistance[startID] = 0.0;
    heap.push({startID, 0.0});
    result.peakHeapSize = 1;

    while (!heap.empty()) {
        const HeapEntry current = heap.pop();
        if (settled.find(current.id) != settled.end()) continue;
        if (current.distance > bestDistance.at(current.id)) continue;
        settled.insert(current.id);
        ++result.nodesExplored;

        if (current.id == endID) {
            result.found = true;
            result.totalDistance = current.distance;
            break;
        }

        const auto neighbors = graph.adjacencyList.find(current.id);
        if (neighbors == graph.adjacencyList.end()) continue;

        for (const Edge& edge : neighbors->second) {
            if (!std::isfinite(edge.distance) || edge.distance < 0.0)
                throw std::invalid_argument("dijkstra requires finite, nonnegative edge distances");
            if (graph.nodes.find(edge.destination) == graph.nodes.end())
                throw std::invalid_argument("an edge references a node missing from graph.nodes");
            if (settled.find(edge.destination) != settled.end()) continue;

            const double candidate = current.distance + edge.distance;
            if (!std::isfinite(candidate))
                throw std::overflow_error("route distance overflow");

            auto prior = bestDistance.find(edge.destination);
            if (prior == bestDistance.end() || candidate < prior->second) {
                bestDistance[edge.destination] = candidate;
                parent[edge.destination] = current.id;
                heap.push({edge.destination, candidate});
                result.peakHeapSize = std::max(result.peakHeapSize, heap.size());
            }
        }
    }

    if (!result.found) return result;
    long long node = endID;
    while (node != startID) {
        result.path.push_back(node);
        node = parent.at(node);
    }
    result.path.push_back(startID);
    std::reverse(result.path.begin(), result.path.end());
    return result;
}
