#pragma once

#include <cstdint>
#include <string>

class DataBucket;

class CoordEdge {
    public:

        CoordEdge(std::string streetName, uint64_t startId, uint64_t endId);

        double GetDistance();

        static uint64_t GetEdgeIndex(uint64_t startNodeId, uint64_t endNodeId);

    private:
        DataBucket* bucket = nullptr;
        std::string streetName = "";

        double distance = 0.0;
        uint64_t startId = 0;
        uint64_t endId = 0;
};
