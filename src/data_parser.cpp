#include <ankerl/unordered_dense.h>
#include <cmath>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

#include "coord_edge.hpp"
#include "coord_node.hpp"
#include "data_bucket.hpp"
#include "data_parser.hpp"
#include "error.hpp"

DataParser::DataParser(double resolution) {
    this->resolution = resolution;
}

DataParser::~DataParser() {
    for (int i = 0; i < buckets.size(); ++i) {
        delete buckets[i];
    }

    buckets.clear();
}

uint64_t DataParser::CreateCompositeID(double latitude, double longitude) {
    size_t bucketId = GetBucketIdFromCoords(latitude, longitude);
    buckets.resize(bucketId + 1, nullptr);

    DataBucket* bucket = buckets[bucketId];
    if (bucket == nullptr) {
        // potentially construct here, but not decided yet
        return 0;
    }

    uint64_t localNodeIndex = bucket->GetTotalNodes();
    bucket->totalNodes++;

    // back 16 bits: bucket id
    // front 48 bits: unique id
    return (localNodeIndex << 16) | (static_cast<uint64_t>(bucketId) & 0xFFFF);
}

size_t DataParser::GetBucketIdFromCoords(double latitude, double longitude) const {
    int64_t gridX = static_cast<int64_t>(std::floor(longitude / resolution));
    int64_t gridY = static_cast<int64_t>(std::floor(latitude / resolution));

    uint32_t ux = static_cast<uint32_t>(gridX) & 0xFF;
    uint32_t uy = static_cast<uint32_t>(gridY) & 0xFF;
    
    return static_cast<size_t>((ux << 8) | uy);
}

std::expected<CoordNode*, Error> DataParser::GetNode(uint64_t nodeId) const {
    size_t bucketId = DataBucket::GetBucketIdFromId(nodeId);
    if (buckets.size() <= bucketId || !buckets[bucketId]->initialized) {
        return std::unexpected(Error(ErrorType::OutOfBounds, "Requested bucket is not initialized"));
    }

    return buckets[bucketId]->GetNode(nodeId);
}

std::expected<CoordEdge*, Error> DataParser::GetEdge(uint64_t edgeId) const {
    size_t bucketId = DataBucket::GetBucketIdFromId(edgeId);
    if (buckets.size() <= bucketId || !buckets[bucketId]->initialized) {
        return std::unexpected(Error(ErrorType::OutOfBounds, "Requested bucket is not initialized"));
    }

    return buckets[bucketId]->GetEdge(edgeId);
}

std::expected<CoordEdge*, Error> DataParser::GetEdge(uint64_t startId, uint64_t endId) const {
    if (startId < endId) {
        std::swap(startId, endId);
    }

    size_t bucketId = DataBucket::GetBucketIdFromId(startId);
    if (buckets.size() <= bucketId || !buckets[bucketId]->initialized) {
        return std::unexpected(Error(ErrorType::OutOfBounds, "Requested bucket is not initialized"));
    }

    return buckets[bucketId]->GetEdge(startId, endId);
}

// runs beforehand to initialize the data
// sets it in the .build/cache directory
std::expected<void, Error> DataParser::ContructDataFromPath(std::string path) {
    // todo
    return {};
}