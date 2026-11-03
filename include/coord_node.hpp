#pragma once

#include <cstdint>
#include <vector>

#include "pathable.hpp"

class DataBucket;

class CoordNode : public Pathable {
    public:

        CoordNode(DataBucket* bucket, uint64_t id, double latitude, double longitude);

        double GetLatitude() const;
        double GetLongitude() const;

        // pathable implementations
        bool operator>(const Pathable& other) const;

        std::vector<Pathable*> GetNeighbors();
        Pathable* GetParent();

        uint64_t GetId() const;

        void SetParent(Pathable* parent);
        
        double GetCost(Pathable* other = nullptr) const;
        double GetGCost() const;
        double GetHCost() const;
        double GetFCost() const;

        void Reset();

    private:
        DataBucket* bucket = nullptr;
        std::vector<uint64_t> neighborIds = {};

        uint64_t id = 0;

        double latitude = 0.0;
        double longitude = 0.0;

        Pathable* parent = nullptr;
        double g = 0.0;
};
