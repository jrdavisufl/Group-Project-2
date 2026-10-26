#pragma once

#include <vector>
#include <queue>
#include <algorithm>
#include <ankerl/unordered_dense.h>
#include "pathable.hpp"

struct CompareNodes {
    bool operator()(const Pathable* a, const Pathable* b) const {
        return a->GetFCost() > b->GetFCost(); 
    }
};

template <typename T>
void ConstructBackwardsPath(std::vector<T*>& input, Pathable* start, Pathable* target) {
    if (start == nullptr || target == nullptr) {
        return;
    }

    Pathable* curr = target->GetParent();
    while (curr != nullptr && curr != start) {
        T* out = static_cast<T*>(curr);
        input.push_back(out);
        curr = curr->GetParent();
    }

    std::reverse(input.begin(), input.end());
}

template <typename T>
std::vector<T*> GetAstarPath(Pathable* start, Pathable* target) {
    if (start == nullptr || target == nullptr || start == target) {
        return {};
    }

    std::vector<T*> outgoing = {};
    std::priority_queue<Pathable*, std::vector<Pathable*>, CompareNodes> openMap;
    ankerl::unordered_dense::map<Pathable*, bool> closedMap;
    ankerl::unordered_dense::map<Pathable*, bool> visited;

    start->Reset();
    openMap.push(start);
    visited[start] = true;

    while (!openMap.empty()) {
        Pathable* curr = openMap.top();
        openMap.pop();

        if (closedMap.contains(curr)) {
            continue;
        }

        if (curr == target) {
            ConstructBackwardsPath(outgoing, start, target);
            return outgoing;
        }

        closedMap[curr] = true;

        auto neighbors = curr->GetNeighbors();
        for (size_t i = 0; i < neighbors.size(); ++i) {
            Pathable* neighbor = neighbors[i];
            if (closedMap.contains(neighbor)) {
                continue;
            }

            int candidateCost = curr->GetGCost() + neighbor->GetCost();
            bool isUnvisited = (!visited.contains(neighbor) && neighbor != start);
            if (isUnvisited || candidateCost < neighbor->GetGCost()) {
                if (!visited.contains(neighbor)) {
                    neighbor->Reset();
                }

                neighbor->SetParent(curr);
                visited[neighbor] = true;
                openMap.push(neighbor);
            }
        }
    }

    return {};
}
