#include <cstdint>
#include <string>

#include "coord_edge.hpp"

CoordEdge::CoordEdge(std::string streetName, uint64_t startId, uint64_t endId, double distance) {
    this->streetName = streetName;
    this->startId = startId;
    this->endId = endId;
    this->distance = distance;
}

std::string CoordEdge::GetStreetName() const {
    return this->streetName;
}

uint64_t CoordEdge::GetStartId() const {
    return this->startId;
}

uint64_t CoordEdge::GetEndId() const {
    return this->endId;
}

double CoordEdge::GetDistance() {
    return distance;
}

uint64_t CoordEdge::GetEdgeIndex(uint64_t startNodeId, uint64_t endNodeId) {
    uint64_t startLocal = (startNodeId >> 16) & 0xFFFFFF;
    uint64_t endLocal = (endNodeId >> 16) & 0xFFFFFF;
    uint64_t bucketId16 = startNodeId & 0xFFFF;

    // Bit-pack into a single 64-bit integer:
    // Bits 48-63: Bucket ID
    // Bits 24-47: Start Node Local Index
    // Bits 0-23:  End Node Local Index
    return (bucketId16 << 48) | (startLocal << 24) | endLocal;
}