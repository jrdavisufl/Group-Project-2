#include <ankerl/unordered_dense.h>
#include <cmath>
#include <cstdint>
#include <expected>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "coord_edge.hpp"
#include "coord_node.hpp"
#include "data_bucket.hpp"
#include "data_parser.hpp"
#include "error.hpp"

DataParser::DataParser(std::string filePath, double resolution) {
    this->filePath = filePath;
    this->resolution = resolution;
}

DataParser::~DataParser() {
    for (int i = 0; i < buckets.size(); ++i) {
        delete buckets[i];
    }

    buckets.clear();
}

size_t DataParser::GetBucketIdFromCoords(double latitude, double longitude) const {
    // florida coordinates
    double baseLat = 24.0;
    double baseLon = -88.0;

    int64_t gridX = static_cast<int64_t>(std::floor((longitude - baseLon) / resolution));
    int64_t gridY = static_cast<int64_t>(std::floor((latitude - baseLat) / resolution));

    // Ensure they never go negative
    if (gridX < 0) gridX = 0;
    if (gridY < 0) gridY = 0;

    uint32_t ux = static_cast<uint32_t>(gridX) & 0xFF;
    uint32_t uy = static_cast<uint32_t>(gridY) & 0xFF;

    return static_cast<size_t>((ux << 8) | uy);
}

std::expected<CoordNode*, Error> DataParser::GetNode(uint64_t nodeId) const {
    size_t bucketId = DataBucket::GetBucketIdFromId(nodeId);
    if (buckets.size() <= bucketId || buckets[i] == nullptr || !buckets[bucketId]->initialized) {
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

std::expected<CoordEdge*, Error> DataParser::GetEdge(uint64_t startId, uint64_t endId) const {size_t bucketId = DataBucket::GetBucketIdFromId(startId);
    if (buckets.size() <= bucketId || !buckets[bucketId]->initialized) {
        return std::unexpected(Error(ErrorType::OutOfBounds, "Requested bucket is not initialized"));
    }

    return buckets[bucketId]->GetEdge(startId, endId);
}

std::string DataParser::GetFilePath() const {
    return this->filePath;
}

std::expected<void, Error> DataParser::parseNodes(ankerl::unordered_dense::map<uint64_t, uint64_t>& idMap) {
    std::ifstream file(filePath + "/nodes.csv");
    if (!file.is_open()) {
        return std::unexpected(Error(ErrorType::FileNotFound, "Raw Node CSV File Not Found"));
    }

    std::string line;

    // skip header
    if (std::getline(file, line)) {}

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::stringstream ss(line);
        std::string idStr, latStr, lonStr;

        // Stream extraction handles any combination of spaces/tabs automatically
        if (!(ss >> idStr >> latStr >> lonStr)) {
            continue; 
        }

        if (idStr == "id") continue;

        uint64_t id = std::stoull(idStr);
        double latitude = std::stod(latStr);
        double longitude = std::stod(lonStr);

        if (id == 0) continue;

        size_t bucketId = GetBucketIdFromCoords(latitude, longitude);
        if (bucketId >= buckets.size()) {
            buckets.resize(bucketId + 1, nullptr);
        }

        if (buckets[bucketId] == nullptr) {
            buckets[bucketId] = new DataBucket(this, bucketId, false);
        }

        auto result = buckets[bucketId]->AddNode(latitude, longitude);
        if (!result) {
            return std::unexpected(result.error());
        }

        idMap[id] = *result;
    }

    return {};
}

std::expected<void, Error> DataParser::parseEdges(ankerl::unordered_dense::map<uint64_t, uint64_t>& idMap) {
    std::ifstream file(filePath + "/edges.csv");
    if (!file.is_open()) {
        return std::unexpected(Error(ErrorType::FileNotFound, "Raw Edge CSV File Not Found"));
    }

    std::string line;

    // skip header
    if (std::getline(file, line)) {}

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::stringstream ss(line);
        std::string startStr, endStr, distStr;

        if (!(ss >> startStr >> endStr >> distStr)) {
            continue;
        }

        if (startStr == "source") continue;

        uint64_t startId = std::stoull(startStr);
        uint64_t endId = std::stoull(endStr);
        double distance = std::stod(distStr);

        // Capture the rest of the line as the street name (supports multi-word names)
        std::string streetName;
        std::getline(ss, streetName);
        if (!streetName.empty()) {
            size_t first = streetName.find_first_not_of(" \t");
            streetName = (first == std::string::npos) ? "Unknown" : streetName.substr(first);
        } else {
            streetName = "Unknown";
        }

        if (startId == 0 || endId == 0 || distance <= 0) continue;

        auto itStart = idMap.find(startId);
        auto itEnd = idMap.find(endId);
        if (itStart == idMap.end() || itEnd == idMap.end()) {
            continue;
        }

        uint64_t newStartId = itStart->second;
        uint64_t newEndId = itEnd->second;

        size_t bucketId = DataBucket::GetBucketIdFromId(newStartId);
        if (bucketId >= buckets.size()) {
            buckets.resize(bucketId + 1, nullptr);
        }

        if (buckets[bucketId] == nullptr) {
            buckets[bucketId] = new DataBucket(this, bucketId, false);
        }

        auto result = buckets[bucketId]->AddEdge(streetName, newStartId, newEndId, distance);
        if (!result) {
            // Handled duplicate gracefully
        }
    }

    return {};
}

// runs beforehand to initialize the data
// sets it in the .build/cache directory
std::expected<void, Error> DataParser::ContructDataFromPath() {
    if (filePath == "") {
        return std::unexpected(Error(ErrorType::InvalidInput, "Invalid Path Input"));
    }

    ankerl::unordered_dense::map<uint64_t, uint64_t> idMap;

    auto resultA = parseNodes(idMap);
    if (!resultA) {
        return std::unexpected(resultA.error());
    }
    
    auto resultB = parseEdges(idMap);
    if (!resultB) {
        return std::unexpected(resultB.error());
    }

    for (size_t i = 0; i < buckets.size(); ++i) {
        if (buckets[i] == nullptr) continue;

        buckets[i]->initialized = true;
        auto result = buckets[i]->SaveDataToCache();
        if (!result) {
            return std::unexpected(result.error());
        }
    }

    return {};
}
