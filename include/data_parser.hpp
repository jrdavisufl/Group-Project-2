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

enum AlgorithmType {
    AstarAlgorithm,
    DijkstraAlgorithm
};

constexpr std::string ROOT_DATA_DIRECTORY = ".data/";
constexpr double DEFAULT_RESOLUTION = 0.15;

class DataParser {
    public:
        DataParser(std::string filePath, bool autoInitialize = true);
        ~DataParser();

        size_t GetBucketIdFromCoords(double latitude, double longitude) const;

        std::expected<DataBucket*, Error> GetOrLoadBucket(size_t bucketId);
        std::expected<CoordNode*, Error> GetNode(uint64_t nodeId);
        std::expected<CoordNode*, Error> GetClosestNodeFromCoords(double latitude, double longitude);

        std::expected<std::string, Error> GetNameFromId(uint32_t id);
        std::expected<uint32_t, Error> FindOrCreateName(std::string& name);

        std::string GetFilePath() const;

        std::expected<void, Error> InitializeNamesFromCache();
        std::expected<void, Error> SaveNamesToCache();

        std::expected<void, Error> SaveMetaToCache();
        std::expected<void, Error> InitializeMetaFromCache();

        std::expected<void, Error> PrintAlgorithmResponseFromCoords(double lat1, double long1, double lat2, double long2, AlgorithmType algorithmType = AlgorithmType::AstarAlgorithm);

        std::expected<void, Error> PrintRouteJSONFromCoords(double lat1, double lon1, double lat2, double lon2, AlgorithmType algorithmType);

        // runs beforehand to initialize the data
        // sets it in the .build/cache directory
        std::expected<void, Error> ContructDataFromPath();
        std::expected<void, Error> InitializeDataFromCache();

        void ClearData();

        friend class DataBucket;
        friend class CoordEdge;
        friend class CoordNode;

    private:
        std::vector<std::string> nameList;
        ankerl::unordered_dense::map<std::string, uint32_t> nameMap;

        double resolution = 0.15;
        std::vector<DataBucket*> buckets;
        std::string filePath = "";

        std::expected<void, Error> parseNodes(ankerl::unordered_dense::map<uint64_t, uint64_t>& idMap);
        std::expected<void, Error> parseEdges(ankerl::unordered_dense::map<uint64_t, uint64_t>& idMap);

        AlgorithmType algorithmType = AlgorithmType::AstarAlgorithm;
        CoordNode* start = nullptr;
        CoordNode* end = nullptr;
};
