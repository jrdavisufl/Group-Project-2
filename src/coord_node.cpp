#include <cstdint>
#include <vector>

#include "coord_node.hpp"
#include "data_bucket.hpp"
#include "data_parser.hpp"
#include "pathable.hpp"

CoordNode::CoordNode(DataBucket* bucket, uint64_t id, double latitude, double longitude) {
    this->bucket = bucket;
    this->id = id;
    this->latitude = latitude;
    this->longitude = longitude;
}

double CoordNode::GetLatitude() const {
    return this->latitude;
}

double CoordNode::GetLongitude() const {
    return this->longitude;
}

// pathable implementations
bool CoordNode::operator>(const Pathable& other) const {
    return this->GetFCost() > other.GetFCost();
}

std::vector<Pathable*> CoordNode::GetNeighbors() {
    std::vector<Pathable*> outgoing = {};
    for (int i = 0; i < neighborIds.size(); ++i) {
        auto result = this->bucket->dataParser->GetNode(neighborIds[i]);
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
    if (parent == nullptr) {
        return;
    }

    auto result = this->bucket->dataParser->GetEdge(parent->GetId(), this->id);

    // some kind of error - most likely not a valid edge
    if (!result) {
        return;
    }

    this->parent = parent;
    this->g = parent->GetGCost() + (*result)->GetDistance();
}

double CoordNode::GetCost(Pathable* other) const {
    if (other == nullptr) {
        return 0;
    }

    auto result = this->bucket->dataParser->GetEdge(this->id, other->GetId());

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