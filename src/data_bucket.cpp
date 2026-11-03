#include <ankerl/unordered_dense.h>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "coord_edge.hpp"
#include "coord_node.hpp"
#include "data_parser.hpp"
#include "error.hpp"

DataBucket::DataBucket(DataParser* dataParser, uint32_t id, bool autoInitialize) {
    this->dataParser = dataParser;
    this->id = id;

    if (autoInitialize) {
        this->InitializeDataFromCache();
    }
}

DataBucket::~DataBucket() {
    this->clearData();
}


std::expected<void, Error> DataBucket::InitializeDataFromCache() {
    std::string path = this->dataParser->GetFilePath();
    if (path.empty()) {
        return std::unexpected(Error(ErrorType::InvalidInput, "DataParser path is not set"));
    }

    std::filesystem::path cacheDir = std::filesystem::path(path) / "cache";

    auto readFile = [](std::ifstream& file, void* data, size_t size) -> bool {
        file.read(static_cast<char*>(data), static_cast<std::streamsize>(size));
        return file.good();
    };

    std::string nodePath = (cacheDir / (std::to_string(id) + "_nodes.bin")).string();
    std::ifstream nodeFile(nodePath, std::ios::binary);

    if (!nodeFile.is_open()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to open node cache"));
    }

    uint64_t nodeCount = 0;
    if (!readFile(nodeFile, &nodeCount, sizeof(nodeCount))) {
        return std::unexpected(Error(ErrorType::FileError, "Failed reading node count"));
    }

    for (uint64_t i = 0; i < nodeCount; ++i) {
        uint64_t nodeId = 0;
        double lat = 0.0;
        double lon = 0.0;
        uint64_t edgeCount = 0;

        if (!readFile(nodeFile, &nodeId, sizeof(nodeId)) ||
            !readFile(nodeFile, &lat, sizeof(lat)) ||
            !readFile(nodeFile, &lon, sizeof(lon)) ||
            !readFile(nodeFile, &edgeCount, sizeof(edgeCount)))
        {
            return std::unexpected(Error(ErrorType::FileError, "Failed reading node data"));
        }

        if (nodeMap.contains(nodeId)) {
            return std::unexpected(Error(ErrorType::DuplicateValue, "Duplicate node ID in cache"));
        }

        CoordNode* node = new CoordNode(this, nodeId, lat, lon);
        nodeMap[nodeId] = node;

        for (uint64_t j = 0; j < edgeCount; ++j) {
            uint64_t endId = 0;
            double distance = 0.0;
            uint32_t nameId = 0;

            if (!readFile(nodeFile, &endId, sizeof(endId)) ||
                !readFile(nodeFile, &distance, sizeof(distance)) ||
                !readFile(nodeFile, &nameId, sizeof(nameId)))
            {
                return std::unexpected(Error(ErrorType::FileError, "Failed reading edge data"));
            }

            node->AddEdgeById(nameId, endId, distance);
        }
    }

    nodeFile.close();

    if (nodeFile.fail()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed closing node cache"));
    }

    this->initialized = true;

    return {};
}

std::expected<void, Error> DataBucket::SaveDataToCache() {
    if (!this->initialized) {
        return std::unexpected(Error(ErrorType::Empty, "Bucket not initialized"));
    }

    std::string path = this->dataParser->GetFilePath();
    if (path.empty()) {
        return std::unexpected(Error(ErrorType::InvalidInput, "DataParser path is not set"));
    }

    std::filesystem::path cacheDir = std::filesystem::path(path) / "cache";

    std::error_code ec;
    std::filesystem::create_directories(cacheDir, ec);

    if (ec) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to create cache directory"));
    }

    auto writeFile = [](std::ofstream& file, const void* data, size_t size) -> bool {
        file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
        return file.good();
    };

    std::string nodePath = (cacheDir / (std::to_string(id) + "_nodes.bin")).string();
    std::ofstream nodeFile(nodePath, std::ios::binary | std::ios::trunc);

    if (!nodeFile.is_open()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to open node cache"));
    }

    // count only non-null nodes so the header always matches what is written
    uint64_t nodeCount = 0;
    for (const auto& [nodeId, nodePtr] : nodeMap) {
        if (nodePtr) nodeCount++;
    }

    if (!writeFile(nodeFile, &nodeCount, sizeof(nodeCount))) {
        return std::unexpected(Error(ErrorType::FileError, "Failed writing node count"));
    }

    for (const auto& [nodeId, nodePtr] : nodeMap) {
        if (!nodePtr) continue;

        double lat = nodePtr->GetLatitude();
        double lon = nodePtr->GetLongitude();
        const std::vector<CoordEdge>& edges = nodePtr->GetEdges();
        uint64_t edgeCount = edges.size();

        if (!writeFile(nodeFile, &nodeId, sizeof(nodeId)) ||
            !writeFile(nodeFile, &lat, sizeof(lat)) ||
            !writeFile(nodeFile, &lon, sizeof(lon)) ||
            !writeFile(nodeFile, &edgeCount, sizeof(edgeCount)))
        {
            return std::unexpected(Error(ErrorType::FileError, "Failed writing node data"));
        }

        for (const CoordEdge& edge : edges) {
            uint64_t endId = edge.endId;
            double distance = edge.distance;
            uint32_t nameId = edge.nameId;

            if (!writeFile(nodeFile, &endId, sizeof(endId)) ||
                !writeFile(nodeFile, &distance, sizeof(distance)) ||
                !writeFile(nodeFile, &nameId, sizeof(nameId)))
            {
                return std::unexpected(Error(ErrorType::FileError, "Failed writing edge data"));
            }
        }
    }

    nodeFile.close();
    if (nodeFile.fail()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed closing node cache"));
    }

    return {};
}

std::expected<uint64_t, Error> DataBucket::AddNode(double latitude, double longitude) {
    uint64_t localNodeIndex = nodeMap.size();
    uint64_t nodeId = (localNodeIndex << 16) | (static_cast<uint64_t>(id) & 0xFFFF);

    if (nodeMap.contains(nodeId)) {
        return std::unexpected(Error(ErrorType::DuplicateValue, "Node already exists"));
    }

    nodeMap[nodeId] = new CoordNode(this, nodeId, latitude, longitude);
    return nodeId;
}

std::expected<CoordNode*, Error> DataBucket::GetNode(uint64_t nodeId) const {
    auto result = nodeMap.find(nodeId);
    if (result == nodeMap.end()) {
        return std::unexpected(Error(ErrorType::Empty, "NodeMap is Empty"));
    }

    return result->second;
}

size_t DataBucket::GetTotalNodes() const {
    return this->nodeMap.size();
}

// assumes the bucket id is in the back 16 bits of the 64 bit id
size_t DataBucket::GetBucketIdFromId(uint64_t id) {
    return static_cast<size_t>(id & 0xFFFF);
}

void DataBucket::clearData() {
    for (const auto& [key, value] : nodeMap) {
        delete value;
    }

    nodeMap.clear();
}