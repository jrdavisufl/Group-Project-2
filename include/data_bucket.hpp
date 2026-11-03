#pragma once

#include <ankerl/unordered_dense.h>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

#include "error.hpp"

class CoordNode;
class CoordEdge;
class DataParser;

class DataBucket {
    public:

        DataBucket(DataParser* dataParser, uint32_t id, bool autoInitialize = true);
        ~DataBucket();

        std::expected<void, Error> InitializeDataFromCache();
        std::expected<void, Error> SaveDataToCache();

        std::expected<uint64_t, Error> AddNode(double latitude, double longitude);

        std::expected<CoordNode*, Error> GetNode(uint64_t nodeId) const;

        size_t GetTotalNodes() const;

        static size_t GetBucketIdFromId(uint64_t id);

        friend class DataParser;
        friend class CoordEdge;
        friend class CoordNode;

    private:
        DataParser* dataParser = nullptr;

        ankerl::unordered_dense::map<uint64_t, CoordNode*> nodeMap;

        uint32_t id = 0;
        bool initialized = false;

        void clearData();
};
