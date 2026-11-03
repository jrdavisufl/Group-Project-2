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

        DataBucket(DataParser* dataParser, uint32_t id, std::string filePath, bool autoInitialize = true);
        ~DataBucket();

        std::expected<void, Error> InitializeDataFromPath(std::string filePath);

        std::expected<CoordNode*, Error> GetNode(uint64_t nodeId) const;
        std::expected<CoordEdge*, Error> GetEdge(uint64_t startId, uint64_t endId) const;
        std::expected<CoordEdge*, Error> GetEdge(uint64_t edgeId) const;

        size_t GetTotalNodes() const;

        static size_t GetBucketIdFromId(uint64_t id);

        friend class DataParser;
        friend class CoordEdge;
        friend class CoordNode;

    private:
        DataParser* dataParser = nullptr;

        ankerl::unordered_dense::map<uint64_t, CoordNode*> nodeMap;
        ankerl::unordered_dense::map<uint64_t, CoordEdge*> edgeMap;

        uint32_t id = 0;
        bool initialized = false;
        std::string filePath = "";
        size_t totalNodes = 0;

        void clearData();
};
