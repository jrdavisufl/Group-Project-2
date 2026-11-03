#include <cstdint>
#include <string>

#include "coord_edge.hpp"

CoordEdge::CoordEdge() {}

CoordEdge::CoordEdge(uint32_t nameId, uint64_t startId, uint64_t endId, double distance) {
    this->nameId = nameId;
    this->startId = startId;
    this->endId = endId;
    this->distance = distance;
}
