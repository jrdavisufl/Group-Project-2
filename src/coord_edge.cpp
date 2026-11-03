#include <cstdint>
#include <string>

#include "coord_edge.hpp"

CoordEdge::CoordEdge(std::string streetName, uint64_t startId, uint64_t endId) {
    this->streetName = streetName;
    this->startId = startId;
    this->endId = endId;
}

double CoordEdge::GetDistance() {
    return distance;
}

uint64_t CoordEdge::GetEdgeIndex(uint64_t startNodeId, uint64_t endNodeId) {
    if (startNodeId < endNodeId) {
        std::swap(startNodeId, endNodeId);
    }

    uint64_t startLocal = startNodeId & 0xFFFFFF;
    uint64_t endLocal = endNodeId & 0xFFFFFF;

    uint64_t spatialBucket = startNodeId >> 24;
    uint64_t bucketId16 = spatialBucket & 0xFFFF;

    // Bit-pack into a single 64-bit integer:
    // Bits 48-63: Bucket ID
    // Bits 24-47: Start Node Local Index
    // Bits 0-23:  End Node Local Index
    return (bucketId16 << 48) | (startLocal << 24) | endLocal;
}