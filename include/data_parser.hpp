#pragma once

#include <ankerl/unordered_dense.h>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

#include "coord_edge.hpp"
#include "coord_node.hpp"
#include "data_bucket.hpp"
#include "error.hpp"

constexpr std::string ROOT_DATA_DIRECTORY = ".data/";

class DataParser {
    public:
        DataParser(std::string filePath, double resolution = 0.15);
        ~DataParser();

        size_t GetBucketIdFromCoords(double latitude, double longitude) const;

        std::expected<CoordNode*, Error> GetNode(uint64_t nodeId) const;
        std::expected<CoordEdge*, Error> GetEdge(uint64_t edgeId) const;
        std::expected<CoordEdge*, Error> GetEdge(uint64_t startId, uint64_t endId) const;

        std::string GetFilePath() const;

        // runs beforehand to initialize the data
        // sets it in the .build/cache directory
        std::expected<void, Error> ContructDataFromPath();

        friend class DataBucket;
        friend class CoordEdge;
        friend class CoordNode;

    private:
        double resolution = 0.15;
        std::vector<DataBucket*> buckets;
        std::string filePath = "";

        std::expected<void, Error> parseNodes(ankerl::unordered_dense::map<uint64_t, uint64_t>& idMap);
        std::expected<void, Error> parseEdges(ankerl::unordered_dense::map<uint64_t, uint64_t>& idMap);
};
