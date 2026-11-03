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

    // load nodes
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

        if (!readFile(nodeFile, &nodeId, sizeof(nodeId)) ||
            !readFile(nodeFile, &lat, sizeof(lat)) ||
            !readFile(nodeFile, &lon, sizeof(lon))) 
        {
            return std::unexpected(Error(ErrorType::FileError, "Failed reading node data"));
        }

        if (nodeMap.contains(nodeId)) {
            return std::unexpected(Error(ErrorType::DuplicateValue, "Duplicate node ID in cache"));
        }

        nodeMap[nodeId] = new CoordNode(this, nodeId, lat, lon);
        totalNodes++;
    }

    nodeFile.close();

    if (nodeFile.fail()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed closing node cache"));
    }

    // load edges
    std::string edgePath = (cacheDir / (std::to_string(id) + "_edges.bin")).string();
    std::ifstream edgeFile(edgePath, std::ios::binary);

    if (!edgeFile.is_open()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to open edge cache"));
    }

    uint64_t edgeCount = 0;

    if (!readFile(edgeFile, &edgeCount, sizeof(edgeCount))) {
        return std::unexpected(Error(ErrorType::FileError, "Failed reading edge count"));
    }

    for (uint64_t i = 0; i < edgeCount; ++i) {
        uint64_t edgeId = 0;
        uint64_t startId = 0;
        uint64_t endId = 0;
        double distance = 0.0;
        uint64_t nameLength = 0;

        if (!readFile(edgeFile, &edgeId, sizeof(edgeId)) ||
            !readFile(edgeFile, &startId, sizeof(startId)) ||
            !readFile(edgeFile, &endId, sizeof(endId)) ||
            !readFile(edgeFile, &distance, sizeof(distance)) ||
            !readFile(edgeFile, &nameLength, sizeof(nameLength))) 
        {
            return std::unexpected(Error(ErrorType::FileError, "Failed reading edge data"));
        }

        std::string streetName(nameLength, '\0');

        if (nameLength > 0 && !readFile(edgeFile, streetName.data(), nameLength)) {
            return std::unexpected(Error(ErrorType::FileError, "Failed reading street name"));
        }

        if (edgeMap.contains(edgeId)) {
            return std::unexpected(Error(ErrorType::DuplicateValue, "Duplicate edge ID in cache"));
        }

        edgeMap[edgeId] = new CoordEdge(streetName, startId, endId, distance);
        totalEdges++;
    }

    edgeFile.close();

    if (edgeFile.fail()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed closing edge cache"));
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

    // save nodes
    std::string nodePath = (cacheDir / (std::to_string(id) + "_nodes.bin")).string();
    std::ofstream nodeFile(nodePath, std::ios::binary | std::ios::trunc);

    if (!nodeFile.is_open()) {
        return std::unexpected(
            Error(ErrorType::FileError, "Failed to open node cache")
        );
    }

    if (!writeFile(nodeFile, &nodeMap.size(), sizeof(nodeMap.size()))) {
        return std::unexpected(
            Error(ErrorType::FileError, "Failed writing node count")
        );
    }

    for (const auto& [nodeId, nodePtr] : nodeMap) {
        if (!nodePtr) continue;

        double lat = nodePtr->GetLatitude();
        double lon = nodePtr->GetLongitude();

        if (!writeFile(nodeFile, &nodeId, sizeof(nodeId)) ||
            !writeFile(nodeFile, &lat, sizeof(lat)) ||
            !writeFile(nodeFile, &lon, sizeof(lon))) 
        {

            return std::unexpected(
                Error(ErrorType::FileError, "Failed writing node data")
            );
        }
    }

    nodeFile.close();
    if (nodeFile.fail()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed closing node cache"));
    }

    // edges
    std::string edgePath = (cacheDir / (std::to_string(id) + "_edges.bin")).string();
    std::ofstream edgeFile(edgePath, std::ios::binary | std::ios::trunc);

    if (!edgeFile.is_open()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to open edge cache"));
    }

    if (!writeFile(edgeFile, &edgeMap.size(), sizeof(edgeMap.size()))) {
        return std::unexpected(Error(ErrorType::FileError, "Failed writing edge count"));
    }

    for (const auto& [edgeId, edgePtr] : edgeMap) {
        if (!edgePtr) continue;

        uint64_t startId = edgePtr->GetStartId();
        uint64_t endId = edgePtr->GetEndId();

        double distance = edgePtr->GetDistance();

        uint64_t nameLength =
            edgePtr->GetStreetName().size();

        if (!writeFile(edgeFile, &edgeId, sizeof(edgeId)) ||
            !writeFile(edgeFile, &startId, sizeof(startId)) ||
            !writeFile(edgeFile, &endId, sizeof(endId)) ||
            !writeFile(edgeFile, &distance, sizeof(distance)) ||
            !writeFile(edgeFile, &nameLength, sizeof(nameLength)) ||
            !writeFile(edgeFile, edgePtr->GetStreetName().data(), nameLength )) 
        {
            return std::unexpected(
                Error(ErrorType::FileError, "Failed writing edge data")
            );
        }
    }

    edgeFile.close();
    if (edgeFile.fail()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed closing edge cache"));
    }

    return {};
}

std::expected<uint64_t, Error> DataBucket::AddNode(double latitude, double longitude) {
    uint64_t localNodeIndex = totalNodes;
    uint64_t nodeId = (localNodeIndex << 16) | (static_cast<uint64_t>(id) & 0xFFFF);

    if (nodeMap.contains(nodeId)) {
        return std::unexpected(Error(ErrorType::DuplicateValue, "Node already exists"));
    }

    nodeMap[nodeId] = new CoordNode(this, nodeId, latitude, longitude);
    totalNodes++;

    return nodeId;
}

std::expected<void, Error> DataBucket::AddEdge(std::string streetName, uint64_t startId, uint64_t endId, double distance) {
    uint64_t edgeId = CoordEdge::GetEdgeIndex(startId, endId);

    // dedpulicate duplicates instead of throwing an error
    if (edgeMap.contains(edgeId)) {
        return {};
    }

    edgeMap[edgeId] = new CoordEdge(streetName, startId, endId, distance);
    totalEdges++;

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