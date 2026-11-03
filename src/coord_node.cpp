#include <cstdint>
#include <vector>

#include "coord_node.hpp"
#include "data_bucket.hpp"
#include "data_parser.hpp"
#include "pathable.hpp"

CoordNode::CoordNode(double latitude, double longitude, DataBucket* bucket) {
    this->latitude = latitude;
    this->longitude = longitude;
    this->bucket = bucket;
}

// pathable implementations
bool CoordNode::operator>(const Pathable& other) const {
    return this->GetFCost() > other.GetFCost();
}

std::vector<Pathable*> CoordNode::GetNeighbors() {
    std::vector<Pathable*> outgoing = {};
    for (int i = 0; i < neighborIds.size(); ++i) {
        auto result = this->bucket->GetNode(neighborIds[i]);
        if (!result) {
            continue;
        }

        outgoing.push_back(*result);
    }

    return outgoing;
}

Pathable* CoordNode::GetParent() {
    return this->parent;
}

uint64_t CoordNode::GetId() const {
    return this->id;
}

void CoordNode::SetParent(Pathable* parent) {
    auto result = this->bucket->dataParser->GetEdge(this->id, parent->GetId());

    // some kind of error - most likely not a valid edge
    if (!result) {
        return;
    }

    this->parent = parent;
    this->g = parent->GetGCost() + (*result)->GetDistance();
}

double CoordNode::GetCost(Pathable* other) const {
    auto result = this->bucket->dataParser->GetEdge(this->id, parent->GetId());

    // some kind of error - most likely not a valid edge
    if (!result) {
        return 0.0;
    }

    return (*result)->GetDistance();
}

double CoordNode::GetGCost() const {
    return this->g;
}

double CoordNode::GetHCost() const {
    // todo
    return 0.0;
}

double CoordNode::GetFCost() const {
    return this->GetGCost() + this->GetHCost();
}

void CoordNode::Reset() {
    this->parent = nullptr;
    this->g = 0.0;
}