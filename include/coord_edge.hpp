#pragma once

#include <cstdint>
#include <string>

class CoordEdge {
    public:
        CoordEdge();
        CoordEdge(uint32_t nameId, uint64_t startId, uint64_t endId, double distance);
        
        uint32_t nameId = 0;
        double distance = 0.0;
        uint64_t startId = 0;
        uint64_t endId = 0;
};
