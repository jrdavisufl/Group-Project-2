#include <ankerl/unordered_dense.h>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

#include "coord_edge.hpp"
#include "coord_node.hpp"
#include "data_parser.hpp"
#include "error.hpp"

DataBucket::DataBucket(DataParser* dataParser, uint32_t id, std::string filePath, bool autoInitialize) {
    this->dataParser = dataParser;
    this->id = id;
    this->filePath = filePath;

    if (autoInitialize) {
        this->InitializeDataFromPath(filePath);
    }
}

DataBucket::~DataBucket() {
    this->clearData();
}

std::expected<void, Error> DataBucket::InitializeDataFromPath(std::string filePath) {
    // todo

    this->initialized = true;

    return {};
}

std::expected<CoordNode*, Error> DataBucket::GetNode(uint64_t nodeId) const {
    auto result = nodeMap.find(nodeId);
    if (result == nodeMap.end()) {
        return std::unexpected(Error(ErrorType::Empty, "NodeMap is Empty"));
    }

    return result->second;
}

std::expected<CoordEdge*, Error> DataBucket::GetEdge(uint64_t startId, uint64_t endId) const {
    uint64_t idx = CoordEdge::GetEdgeIndex(startId, endId);
    return this->GetEdge(idx);
}

std::expected<CoordEdge*, Error> DataBucket::GetEdge(uint64_t edgeId) const {
    auto result = edgeMap.find(edgeId);
    if (result == edgeMap.end()) {
        return std::unexpected(Error(ErrorType::Empty, "EdgeMap is Empty"));
    }

    return result->second;
}

size_t DataBucket::GetTotalNodes() const {
    return this->totalNodes;
}

// assumes the bucket id is in the back 16 bits of the 64 bit id
size_t DataBucket::GetBucketIdFromId(uint64_t id) {
    return static_cast<size_t>(id & 0xFFFF);
}

void DataBucket::clearData() {
    for (const auto& [key, value] : nodeMap) {
        delete value;
    }

    for (const auto& [key, value] : edgeMap) {
        delete value;
    }

    nodeMap.clear();
    edgeMap.clear();

    totalNodes = 0;
}