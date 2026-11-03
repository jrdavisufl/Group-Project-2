#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "coord_edge.hpp"

class DataBucket;

class CoordNode {
    public:

        CoordNode(DataBucket* bucket, uint64_t id, double latitude, double longitude);

        double GetLatitude() const;
        double GetLongitude() const;

        void AddEdge(std::string& streetName, uint64_t endId, double distance);
        void AddEdgeById(uint32_t nameId, uint64_t endId, double distance);

        const std::vector<CoordEdge>& GetEdges() const;
        const CoordEdge* GetEdge(uint64_t endId) const;

        // pathable implementations
        bool operator>(const CoordNode& other) const;

        std::vector<CoordNode*> GetNeighbors();
        CoordNode* GetParent();

        uint64_t GetId() const;

        void SetParent(CoordNode* parent, double edgeCost);
        
        double GetCost(CoordNode* other = nullptr) const;
        double GetGCost() const;
        double GetHCost() const;
        double GetFCost() const;

        void Reset();

    private:
        DataBucket* bucket = nullptr;
        std::vector<CoordEdge> edges = {};

        uint64_t id = 0;

        double latitude = 0.0;
        double longitude = 0.0;

        CoordNode* parent = nullptr;
        double g = 0.0;
};
