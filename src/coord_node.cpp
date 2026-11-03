#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "coord_node.hpp"
#include "coord_edge.hpp"
#include "data_bucket.hpp"
#include "data_parser.hpp"

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

void CoordNode::AddEdge(std::string& streetName, uint64_t endId, double distance) {
    auto result = this->bucket->dataParser->FindOrCreateName(streetName);
    if (!result) {
        return;
    }

    uint32_t streetNameId = *result;
    edges.push_back(CoordEdge(streetNameId, this->id, endId, distance));
}

void CoordNode::AddEdgeById(uint32_t nameId, uint64_t endId, double distance) {
    edges.push_back(CoordEdge(nameId, this->id, endId, distance));
}

const std::vector<CoordEdge>& CoordNode::GetEdges() const {
    return this->edges;
}

const CoordEdge* CoordNode::GetEdge(uint64_t endId) const {
    for (size_t i = 0; i < edges.size(); ++i) {
        if (edges[i].endId == endId) {
            return &edges[i];
        }
    }

    return nullptr;
}

// pathable implementations
bool CoordNode::operator>(const CoordNode& other) const {
    return this->GetFCost() > other.GetFCost();
}

std::vector<CoordNode*> CoordNode::GetNeighbors() {
    std::vector<CoordNode*> outgoing = {};
    for (int i = 0; i < edges.size(); ++i) {
        auto result = this->bucket->dataParser->GetNode(edges[i].endId);
        if (!result) {
            continue;
        }

        outgoing.push_back(*result);
    }

    return outgoing;
}

CoordNode* CoordNode::GetParent() {
    return this->parent;
}

uint64_t CoordNode::GetId() const {
    return this->id;
}

void CoordNode::SetParent(CoordNode* parent, double edgeCost) {
    if (parent == nullptr || edgeCost <= 0.0) {
        return;
    }

    this->parent = parent;
    this->g = parent->GetGCost() + edgeCost;
}

double CoordNode::GetCost(CoordNode* other) const {
    if (other == nullptr) {
        return 0;
    }

    double distance = -1.0;
    for (int i = 0; i < edges.size(); ++i) {
        if (edges[i].endId == other->GetId()) {
            distance = edges[i].distance;
            break;
        }
    }

    if (distance == -1.0) {
        return 0.0;
    }

    return distance;
}

double CoordNode::GetGCost() const {
    return this->g;
}

double CoordNode::GetHCost() const {
    const double cosLat = std::cos(latitude * M_PI / 180.0);
    auto distanceTo = [&](double lat, double lon) -> double {
        double dLat = lat - latitude;
        double dLon = (lon - longitude) * cosLat;
        return dLat * dLat + dLon * dLon;
    };

    CoordNode* end = bucket->dataParser->end;
    if (end == nullptr) {
        return 0.0;
    }

    return distanceTo(end->GetLatitude(), end->GetLongitude());
}

double CoordNode::GetFCost() const {
    return this->GetGCost() + this->GetHCost();
}

void CoordNode::Reset() {
    this->parent = nullptr;
    this->g = 0.0;
}