#include <ankerl/unordered_dense.h>
#include <cmath>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

#include "astar_algorithm.hpp"
#include "coord_edge.hpp"
#include "coord_node.hpp"
#include "data_bucket.hpp"
#include "data_parser.hpp"
#include "error.hpp"

DataParser::DataParser(std::string filePath, bool autoInitialize) {
    this->filePath = filePath;
    this->resolution = DEFAULT_RESOLUTION;

    if (autoInitialize) {
        InitializeDataFromCache();
    }
}

DataParser::~DataParser() {
    ClearData();
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

std::expected<DataBucket*, Error> DataParser::GetOrLoadBucket(size_t bucketId) {
    if (bucketId >= buckets.size()) {
        return std::unexpected(Error(ErrorType::OutOfBounds, "Bucket id out of range"));
    }

    if (buckets[bucketId] == nullptr) {
        DataBucket* bucket = new DataBucket(this, bucketId, false);

        auto result = bucket->InitializeDataFromCache();
        if (!result) {
            delete bucket;
            return std::unexpected(result.error());
        }

        buckets[bucketId] = bucket;
    }

    return buckets[bucketId];
}

std::expected<CoordNode*, Error> DataParser::GetNode(uint64_t nodeId) {
    auto bucket = GetOrLoadBucket(DataBucket::GetBucketIdFromId(nodeId));
    if (!bucket) {
        return std::unexpected(bucket.error());
    }

    return (*bucket)->GetNode(nodeId);
}

std::expected<CoordNode*, Error> DataParser::GetClosestNodeFromCoords(double latitude, double longitude) {
    size_t bucketId = GetBucketIdFromCoords(latitude, longitude);
    if (bucketId >= buckets.size()) {
        return std::unexpected(Error(ErrorType::OutOfBounds, "Bucket id out of range"));
    }

    if (buckets[bucketId] == nullptr) {
        DataBucket* bucket = new DataBucket(this, bucketId, false);

        auto result = bucket->InitializeDataFromCache();
        if (!result) {
            delete bucket;
            return std::unexpected(result.error());
        }

        buckets[bucketId] = bucket;
    }

    CoordNode* closest = nullptr;
    double closestDist = std::numeric_limits<double>::max();

    const double cosLat = std::cos(latitude * M_PI / 180.0);
    auto distanceTo = [&](double lat, double lon) -> double {
        double dLat = lat - latitude;
        double dLon = (lon - longitude) * cosLat;
        return dLat * dLat + dLon * dLon;
    };

    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            size_t bucketId = GetBucketIdFromCoords(latitude + dy * resolution, longitude + dx * resolution);

            auto bucket = GetOrLoadBucket(bucketId);
            if (!bucket) continue;

            for (const auto& [key, value] : (*bucket)->nodeMap) {
                if (!value) continue;

                double dist = distanceTo(value->GetLatitude(), value->GetLongitude());
                if (dist < closestDist) {
                    closestDist = dist;
                    closest = value;
                }
            }
        }
    }

    if (closest == nullptr) {
        return std::unexpected(Error(ErrorType::NotFound, "No nodes found near coordinates"));
    }

    return closest;
}

std::expected<std::string, Error> DataParser::GetNameFromId(uint32_t id) {
    if (nameList.size() <= id) {
        return std::unexpected(Error(ErrorType::NotFound, "Name ID Does not exist"));
    }

    return nameList[id];
}

std::expected<uint32_t, Error> DataParser::FindOrCreateName(std::string& name) {
    if (name == "") {
        return std::unexpected(Error(ErrorType::InvalidInput, "Invalid Input in FindOrCreateName"));
    }

    if (nameMap.contains(name)) {
        return nameMap[name];
    }

    nameList.push_back(name);
    nameMap[name] = nameList.size() - 1;
    return nameList.size() - 1;
}

std::string DataParser::GetFilePath() const {
    return this->filePath;
}

std::expected<void, Error> DataParser::SaveNamesToCache() {
    std::filesystem::path cacheDir = std::filesystem::path(filePath) / "cache";

    std::error_code ec;
    std::filesystem::create_directories(cacheDir, ec);

    if (ec) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to create cache directory"));
    }

    auto writeFile = [](std::ofstream& file, const void* data, size_t size) -> bool {
        file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
        return file.good();
    };

    std::string namePath = (cacheDir / "names.bin").string();
    std::ofstream nameFile(namePath, std::ios::binary | std::ios::trunc);

    if (!nameFile.is_open()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to open name cache"));
    }

    uint64_t nameCount = nameList.size();
    if (!writeFile(nameFile, &nameCount, sizeof(nameCount))) {
        return std::unexpected(Error(ErrorType::FileError, "Failed writing name count"));
    }

    for (const std::string& name : nameList) {
        uint64_t nameLength = name.size();

        if (!writeFile(nameFile, &nameLength, sizeof(nameLength)) ||
            (nameLength > 0 && !writeFile(nameFile, name.data(), nameLength)))
        {
            return std::unexpected(Error(ErrorType::FileError, "Failed writing name data"));
        }
    }

    nameFile.close();
    if (nameFile.fail()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed closing name cache"));
    }

    return {};
}

std::expected<void, Error> DataParser::InitializeNamesFromCache() {
    std::filesystem::path cacheDir = std::filesystem::path(filePath) / "cache";

    auto readFile = [](std::ifstream& file, void* data, size_t size) -> bool {
        file.read(static_cast<char*>(data), static_cast<std::streamsize>(size));
        return file.good();
    };

    std::string namePath = (cacheDir / "names.bin").string();
    std::ifstream nameFile(namePath, std::ios::binary);

    if (!nameFile.is_open()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to open name cache"));
    }

    uint64_t nameCount = 0;
    if (!readFile(nameFile, &nameCount, sizeof(nameCount))) {
        return std::unexpected(Error(ErrorType::FileError, "Failed reading name count"));
    }

    nameList.clear();
    nameMap.clear();
    nameList.reserve(nameCount);
    nameMap.reserve(nameCount);

    for (uint64_t i = 0; i < nameCount; ++i) {
        uint64_t nameLength = 0;
        if (!readFile(nameFile, &nameLength, sizeof(nameLength))) {
            nameList.clear();
            nameMap.clear();
            return std::unexpected(Error(ErrorType::FileError, "Failed reading name length"));
        }

        std::string name(nameLength, '\0');
        if (nameLength > 0 && !readFile(nameFile, name.data(), nameLength)) {
            nameList.clear();
            nameMap.clear();
            return std::unexpected(Error(ErrorType::FileError, "Failed reading name data"));
        }

        nameMap[name] = static_cast<uint32_t>(i);
        nameList.push_back(std::move(name));
    }

    nameFile.close();
    if (nameFile.fail()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed closing name cache"));
    }

    return {};
}

std::expected<void, Error> DataParser::SaveMetaToCache() {
    std::filesystem::path cacheDir = std::filesystem::path(filePath) / "cache";

    std::error_code ec;
    std::filesystem::create_directories(cacheDir, ec);

    if (ec) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to create cache directory"));
    }

    auto writeFile = [](std::ofstream& file, const void* data, size_t size) -> bool {
        file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
        return file.good();
    };

    std::string metaPath = (cacheDir / "meta.bin").string();
    std::ofstream metaFile(metaPath, std::ios::binary | std::ios::trunc);

    if (!metaFile.is_open()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to open meta cache"));
    }

    uint64_t bucketCount = buckets.size();
    double savedResolution = resolution;

    if (!writeFile(metaFile, &bucketCount, sizeof(bucketCount)) ||
        !writeFile(metaFile, &savedResolution, sizeof(savedResolution)))
    {
        return std::unexpected(Error(ErrorType::FileError, "Failed writing meta data"));
    }

    metaFile.close();
    if (metaFile.fail()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed closing meta cache"));
    }

    return {};
}

std::expected<void, Error> DataParser::InitializeMetaFromCache() {
    std::filesystem::path cacheDir = std::filesystem::path(filePath) / "cache";

    auto readFile = [](std::ifstream& file, void* data, size_t size) -> bool {
        file.read(static_cast<char*>(data), static_cast<std::streamsize>(size));
        return file.good();
    };

    std::string metaPath = (cacheDir / "meta.bin").string();
    std::ifstream metaFile(metaPath, std::ios::binary);

    if (!metaFile.is_open()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed to open meta cache"));
    }

    uint64_t bucketCount = 0;
    double savedResolution = 0.0;

    if (!readFile(metaFile, &bucketCount, sizeof(bucketCount)) ||
        !readFile(metaFile, &savedResolution, sizeof(savedResolution)))
    {
        return std::unexpected(Error(ErrorType::FileError, "Failed reading meta data"));
    }

    if (savedResolution <= 0.0) {
        return std::unexpected(Error(ErrorType::InvalidInput, "Invalid resolution in meta cache"));
    }

    metaFile.close();
    if (metaFile.fail()) {
        return std::unexpected(Error(ErrorType::FileError, "Failed closing meta cache"));
    }

    // the cache's resolution always wins, since bucket ids were built with it
    resolution = savedResolution;
    buckets.assign(bucketCount, nullptr);

    return {};
}

std::expected<void, Error> DataParser::PrintAlgorithmResponseFromCoords(double lat1, double long1, double lat2, double long2, AlgorithmType algorithmType) {
    auto snapStart = std::chrono::steady_clock::now();

    auto resultA = GetClosestNodeFromCoords(lat1, long1);
    if (!resultA) {
        return std::unexpected(resultA.error());
    }

    auto resultB = GetClosestNodeFromCoords(lat2, long2);
    if (!resultB) {
        return std::unexpected(resultB.error());
    }

    this->algorithmType = algorithmType;
    this->start = *resultA;
    this->end = *resultB;

    double snapSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - snapStart).count();

    // snapshot the load counters so only the search itself is measured
    auto response = GetAstarPath<CoordNode>(*resultA, *resultB);

    if (!response.pathFound) {
        return std::unexpected(Error(ErrorType::NotFound, "No route found between the two points"));
    }

    std::cout << std::fixed << std::setprecision(1);

    // turn-by-turn, merging consecutive segments on the same street
    std::cout << "Route:" << std::endl;

    const std::vector<CoordNode*>& path = response.path;

    uint32_t currentName = 0;
    double legDistance = 0.0;
    bool haveLeg = false;
    int step = 1;

    auto printLeg = [&]() {
        auto name = GetNameFromId(currentName);
        std::cout << "  " << step++ << ". Follow " << (name ? *name : std::string("Unknown")) << " for " << legDistance << " m" << std::endl;
    };

    for (size_t i = 0; i + 1 < path.size(); ++i) {
        const CoordEdge* edge = path[i]->GetEdge(path[i + 1]->GetId());
        if (edge == nullptr) {
            return std::unexpected(Error(ErrorType::NotFound, "Path contains a missing edge"));
        }

        if (haveLeg && edge->nameId != currentName) {
            printLeg();
            legDistance = 0.0;
        }

        currentName = edge->nameId;
        haveLeg = true;
        legDistance += edge->distance;
    }

    if (haveLeg) {
        printLeg();
    }

    std::cout << "  Arrive at destination." << std::endl;

    // metrics
    double circuity = response.straightLineDistance > 0.0 ? response.totalDistanceTraveled / response.straightLineDistance : 0.0;

    std::cout << std::endl << "Result:" << std::endl;
    std::cout << "  Path found:            " << (response.pathFound ? "yes" : "no") << std::endl;
    std::cout << "  Path nodes:            " << response.pathNodeCount << std::endl;
    std::cout << "  Distance traveled:     " << response.totalDistanceTraveled << " m" << std::endl;
    std::cout << "  Straight-line:         " << response.straightLineDistance << " m" << std::endl;
    std::cout << std::setprecision(2);
    std::cout << "  Circuity:              " << circuity << std::endl;

    std::cout << std::endl << "Search effort:" << std::endl;
    std::cout << "  Nodes processed:       " << response.nodesProcessed << std::endl;
    std::cout << "  Nodes pushed to heap:  " << response.nodesPushedToHeap << std::endl;
    std::cout << "  Stale entries skipped: " << response.staleEntriesSkipped << std::endl;
    std::cout << "  Parent changes:        " << response.nodesParentChanged << std::endl;
    std::cout << "  Edges examined:        " << response.edgesExamined << std::endl;
    std::cout << "  Peak heap size:        " << response.peakHeapSize << std::endl;

    std::cout << std::setprecision(4);
    std::cout << std::endl << "Timing:" << std::endl;
    std::cout << "  Snap to nodes:         " << snapSeconds << " s" << std::endl;
    std::cout << "  Total time:            " << response.totalTime << " s" << std::endl;

    return {};
}


// Returns machine-readable route geometry and search metrics for the browser.
// This calls the team's EXISTING A* template (with zero heuristic in Dijkstra mode).
// A standalone Dijkstra implementation can replace that call later.
std::expected<void, Error> DataParser::PrintRouteJSONFromCoords(
    double lat1, double lon1, double lat2, double lon2,
    AlgorithmType selectedAlgorithm)
{
    auto snapStart = std::chrono::steady_clock::now();
    auto startResult = GetClosestNodeFromCoords(lat1, lon1);
    if (!startResult) return std::unexpected(startResult.error());

    auto endResult = GetClosestNodeFromCoords(lat2, lon2);
    if (!endResult) return std::unexpected(endResult.error());

    this->algorithmType = selectedAlgorithm;
    this->start = *startResult;
    this->end = *endResult;
    double snapSeconds = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - snapStart).count();

    const auto response = GetAstarPath<CoordNode>(*startResult, *endResult);
    if (!response.pathFound) {
        return std::unexpected(Error(ErrorType::NotFound, "No route found between those locations"));
    }

    // Build display directions by merging consecutive roads with the same name.
    std::vector<std::pair<std::string, double>> steps;
    for (size_t i = 0; i + 1 < response.path.size(); ++i) {
        const CoordEdge* edge = response.path[i]->GetEdge(response.path[i + 1]->GetId());
        if (edge == nullptr) {
            return std::unexpected(Error(ErrorType::NotFound, "Route contains a missing road edge"));
        }
        auto foundName = GetNameFromId(edge->nameId);
        std::string street = foundName ? *foundName : "Unknown road";
        if (!steps.empty() && steps.back().first == street) {
            steps.back().second += edge->distance;
        } else {
            steps.emplace_back(street, edge->distance);
        }
    }

    std::ostringstream json;
    json << std::setprecision(15);

    auto writeJsonString = [&](const std::string& value) {
        constexpr char hex[] = "0123456789abcdef";
        json << '"';
        for (unsigned char c : value) {
            switch (c) {
                case '"': json << "\\\""; break;
                case '\\': json << "\\\\"; break;
                case '\n': json << "\\n"; break;
                case '\r': json << "\\r"; break;
                case '\t': json << "\\t"; break;
                default:
                    if (c < 0x20) {
                        json << "\\u00" << hex[c >> 4] << hex[c & 15];
                    } else {
                        json << static_cast<char>(c);
                    }
            }
        }
        json << '"';
    };

    json << "{\"found\":true";
    json << ",\"distanceMeters\":" << response.totalDistanceTraveled;
    json << ",\"straightLineMeters\":" << response.straightLineDistance;
    json << ",\"pathNodes\":" << response.pathNodeCount;
    json << ",\"nodesProcessed\":" << response.nodesProcessed;
    json << ",\"nodesPushedToHeap\":" << response.nodesPushedToHeap;
    json << ",\"staleEntriesSkipped\":" << response.staleEntriesSkipped;
    json << ",\"parentChanges\":" << response.nodesParentChanged;
    json << ",\"edgesExamined\":" << response.edgesExamined;
    json << ",\"peakHeapSize\":" << response.peakHeapSize;
    json << ",\"runtimeSeconds\":" << response.totalTime;
    json << ",\"snapSeconds\":" << snapSeconds;
    json << ",\"coordinates\":[";
    for (size_t i = 0; i < response.path.size(); ++i) {
        if (i != 0) json << ',';
        json << '[' << response.path[i]->GetLatitude()
             << ',' << response.path[i]->GetLongitude() << ']';
    }
    json << "],\"steps\":[";
    for (size_t i = 0; i < steps.size(); ++i) {
        if (i != 0) json << ',';
        json << "{\"street\":";
        writeJsonString(steps[i].first);
        json << ",\"distanceMeters\":" << steps[i].second << '}';
    }
    json << "]}";

    // stdout must contain ONLY JSON for this command.
    std::cout << json.str() << '\n';
    return {};
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

        auto result2 = GetNode(newStartId);
        if (!result2) {
            return std::unexpected(result2.error());
        }

        (*result2)->AddEdge(streetName, newEndId, distance);
    }

    return {};
}

// runs beforehand to initialize the data
// sets it in the .build/cache directory
std::expected<void, Error> DataParser::ContructDataFromPath() {
    if (filePath == "") {
        return std::unexpected(Error(ErrorType::InvalidInput, "Invalid Path Input"));
    }

    ClearData();
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

    auto resultC = SaveNamesToCache();
    if (!resultC) {
        return std::unexpected(resultC.error());
    }

    auto resultD = SaveMetaToCache();
    if (!resultD) {
        return std::unexpected(resultD.error());
    }

    return {};
}

std::expected<void, Error> DataParser::InitializeDataFromCache() {
    ClearData();

    auto resultA = InitializeMetaFromCache();
    if (!resultA) {
        return std::unexpected(resultA.error());
    }

    auto resultB = InitializeNamesFromCache();
    if (!resultB) {
        return std::unexpected(resultB.error());
    }

    return {};
}

void DataParser::ClearData() {
    for (int i = 0; i < buckets.size(); ++i) {
        delete buckets[i];
    }

    buckets.clear();
    nameList.clear();
    nameMap.clear();
}
