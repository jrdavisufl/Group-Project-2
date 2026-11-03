#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <queue>
#include <utility>
#include <vector>
#include <ankerl/unordered_dense.h>

template <typename T>
struct AstarAlgorithmRespnse {
    bool pathFound = false;
    std::vector<T*> path = {};

    double totalDistanceTraveled = 0.0;
    double straightLineDistance = 0.0;
    uint64_t pathNodeCount = 0;

    uint64_t nodesProcessed = 0;
    uint64_t nodesPushedToHeap = 0;
    uint64_t staleEntriesSkipped = 0;
    uint64_t nodesParentChanged = 0;
    uint64_t edgesExamined = 0;
    uint64_t peakHeapSize = 0;

    double totalTime = 0.0;
};

template <typename T>
void ConstructBackwardsPath(std::vector<T*>& input, T* start, T* target) {
    if (start == nullptr || target == nullptr) {
        return;
    }

    T* curr = target;
    while (curr != nullptr) {
        input.push_back(curr);
        if (curr == start) break;
        curr = curr->GetParent();
    }

    if (input.empty() || input.back() != start) {
        input.clear();
        return;
    }

    std::reverse(input.begin(), input.end());
}

// taken from https://www.movable-type.co.uk/scripts/latlong.html referencing https://en.wikipedia.org/wiki/Haversine_formula
// adapted into c++ by claude ai
template <typename T>
double HaversineMeters(const T* a, const T* b) {
    constexpr double PI = 3.14159265358979323846;
    constexpr double EARTH_RADIUS_M = 6371000.0;

    double lat1 = a->GetLatitude() * PI / 180.0;
    double lat2 = b->GetLatitude() * PI / 180.0;
    double dLat = lat2 - lat1;
    double dLon = (b->GetLongitude() - a->GetLongitude()) * PI / 180.0;

    double h = std::sin(dLat / 2) * std::sin(dLat / 2) + std::cos(lat1) * std::cos(lat2) * std::sin(dLon / 2) * std::sin(dLon / 2);
    return 2.0 * EARTH_RADIUS_M * std::asin(std::sqrt(h));
}

template <typename T>
AstarAlgorithmRespnse<T> GetAstarPath(T* start, T* target) {
    AstarAlgorithmRespnse<T> response;

    if (start == nullptr || target == nullptr) {
        return response;
    }

    auto timeStart = std::chrono::steady_clock::now();
    response.straightLineDistance = HaversineMeters(start, target);

    if (start == target) {
        response.pathFound = true;
        response.path = {start};
        response.pathNodeCount = 1;
        return response;
    }

    using Entry = std::pair<double, T*>;
    auto cmp = [](const Entry& a, const Entry& b) { return a.first > b.first; };
    std::priority_queue<Entry, std::vector<Entry>, decltype(cmp)> openMap(cmp);

    ankerl::unordered_dense::set<T*> closedSet;
    ankerl::unordered_dense::set<T*> visited;

    start->Reset();
    openMap.push({start->GetFCost(), start});
    visited.insert(start);
    response.nodesPushedToHeap++;
    response.peakHeapSize = 1;

    while (!openMap.empty()) {
        T* curr = openMap.top().second;
        openMap.pop();

        if (closedSet.contains(curr)) {
            response.staleEntriesSkipped++;
            continue;
        }

        response.nodesProcessed++;

        if (curr == target) {
            ConstructBackwardsPath(response.path, start, target);
            response.pathFound = !response.path.empty();
            response.pathNodeCount = response.path.size();
            response.totalDistanceTraveled = target->GetGCost();
            break;
        }

        closedSet.insert(curr);

        auto neighbors = curr->GetNeighbors();
        for (size_t i = 0; i < neighbors.size(); ++i) {
            T* neighbor = neighbors[i];
            response.edgesExamined++;

            if (closedSet.contains(neighbor)) {
                continue;
            }

            double edgeCost = curr->GetCost(neighbor);
            if (edgeCost <= 0.0) {
                continue;
            }

            double candidateCost = curr->GetGCost() + edgeCost;
            bool seen = visited.contains(neighbor);

            if (!seen || candidateCost < neighbor->GetGCost()) {
                if (!seen) {
                    neighbor->Reset();
                    visited.insert(neighbor);
                }

                neighbor->SetParent(curr, edgeCost);
                response.nodesParentChanged++;

                openMap.push({neighbor->GetFCost(), neighbor});
                response.nodesPushedToHeap++;

                if (openMap.size() > response.peakHeapSize) {
                    response.peakHeapSize = openMap.size();
                }
            }
        }
    }

    auto timeEnd = std::chrono::steady_clock::now();
    response.totalTime = std::chrono::duration<double>(timeEnd - timeStart).count();

    return response;
}
