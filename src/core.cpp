#include <expected>
#include <cmath>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include "astar_algorithm.hpp"
#include "control_node_container.hpp"
#include "core.hpp"
#include "data_parser.hpp"
#include "dijkstra.hpp"
#include "error.hpp"

// initialize web will handle the main application with ui
std::expected<void, Error> InitializeWeb() {
    return {};
}

// constructs an environment to test the different algorithms
// NOTE - file is expected to be in the ./inputs folder
std::expected<void, Error> ConstructPathFromFile(std::string fileName) {
    if (fileName.size() == 0) {
        return std::unexpected(Error(ErrorType::InvalidInput, "File name cannot be empty."));
    }

    std::ifstream inputFile("inputs/" + fileName);
    if (!inputFile.is_open()) {
        return std::unexpected(Error(ErrorType::InvalidInput, "File does not exist"));
    }

    ControlNodeContainer container;

    // ignores lines that are either all white space or comments
    // (comments denoted with a //)
    static const std::regex commentPattern(R"(^\s*(//|$))");

    std::string line;
    while (std::getline(inputFile, line)) {
        if (std::regex_search(line, commentPattern)) {
            continue;
        }

        auto result = container.AddLine(line);
        if (!result) {
            return result;
        }
    }

    inputFile.close();

    container.PrintGrid();
    std::cout << std::endl << "================" << std::endl << std::endl;
    container.PrintAstarGrid();
    std::cout << std::endl;

    return {};
}

std::expected<void, Error> RunDijkstraTest() {
    // offline sanity test
    Graph graph;
    graph.addNode(29.0, -82.0, 1001);
    graph.addNode(29.0, -82.0, 1002);
    graph.addNode(29.0, -82.0, 1003);
    graph.addNode(29.0, -82.0, 1004);
    graph.addEdge(1001, 1002, 100, "First");
    graph.addEdge(1001, 1003, 500, "Second");
    graph.addEdge(1002, 1003, 50, "Third");
    graph.addEdge(1002, 1004, 300, "Fourth");
    graph.addEdge(1003, 1004, 80, "Fifth");

    try {
        DijkstraResult result = Dijkstra(graph, 1001, 1004);
        if (!result.found) {
            return std::unexpected(Error(ErrorType::NotFound, "No route found."));
        }
        std::cout << "Distance: " << result.totalDistance << " meters\nPath: ";
        for (std::size_t i = 0; i < result.path.size(); ++i) {
            if (i) std::cout << " -> ";
            std::cout << result.path[i];
        }
        std::cout << "\nVertices explored: " << result.nodesExplored
                  << "\nPeak heap entries: " << result.peakHeapSize << '\n';
    } catch (const std::exception& ex) {
        std::cerr << "Routing error: " << ex.what() << '\n';
        return std::unexpected(Error(ErrorType::Exception, "Exception happened while finding path."));
    }

    return {};
}

std::expected<void, Error> GetClosestNodeFromCoords(std::vector<std::string>& inputs) {
    if (inputs.size() != 3) {
        return std::unexpected(Error(ErrorType::InvalidInput, "Wrong number of arguments passed"));
    }
    
    DataParser p(".data");
    
    double lat, lon;
    try {
        lat = std::stod(inputs[1]);
        lon = std::stod(inputs[2]);
    } catch (const std::exception&) {
        return std::unexpected(Error(ErrorType::InvalidInput, "Coordinates must be numbers"));
    }

    auto result = p.GetClosestNodeFromCoords(lat, lon);
    if (!result) {
        return std::unexpected(result.error());
    }

    std::cout << "Found! Closest id: " << (*result)->GetId() << " lat: " << (*result)->GetLatitude() << " long: " << (*result)->GetLongitude() << std::endl;
    return {};
}

std::expected<void, Error> GetPath(std::vector<std::string>& inputs) {
    if (inputs.size() != 5) {
        return std::unexpected(Error(ErrorType::InvalidInput, "Wrong number of arguments passed"));
    }

    double lat1, lon1, lat2, lon2;
    try {
        lat1 = std::stod(inputs[1]);
        lon1 = std::stod(inputs[2]);
        lat2 = std::stod(inputs[3]);
        lon2 = std::stod(inputs[4]);
    } catch (const std::exception&) {
        return std::unexpected(Error(ErrorType::InvalidInput, "Coordinates must be numbers"));
    }

    {
        DataParser p(".data");

        std::cout << std::endl;
        std::cout << "===============" << std::endl;
        std::cout << "==== Astar ====" << std::endl;
        std::cout << "===============" << std::endl;
        std::cout << std::endl;
        auto response = p.PrintAlgorithmResponseFromCoords(lat1, lon1, lat2, lon2);
        if (!response) {
            return std::unexpected(response.error());
        }
    }

    {
        DataParser p(".data");

        std::cout << std::endl;
        std::cout << "==================" << std::endl;
        std::cout << "==== Dijkstra ====" << std::endl;
        std::cout << "==================" << std::endl;
        std::cout << std::endl;
        return p.PrintAlgorithmResponseFromCoords(lat1, lon1, lat2, lon2, AlgorithmType::DijkstraAlgorithm);
    }
}


// Web-only entry point: get_route_json LAT1 LON1 LAT2 LON2 astar|dijkstra
std::expected<void, Error> GetWebRouteJSON(std::vector<std::string>& inputs) {
    if (inputs.size() != 6) {
        return std::unexpected(Error(ErrorType::InvalidInput,
            "Usage: get_route_json LAT1 LON1 LAT2 LON2 astar|dijkstra"));
    }

    double coords[4];
    try {
        for (int i = 0; i < 4; ++i) {
            std::size_t parsed = 0;
            coords[i] = std::stod(inputs[i + 1], &parsed);
            if (parsed != inputs[i + 1].size() || !std::isfinite(coords[i])) {
                return std::unexpected(Error(ErrorType::InvalidInput, "Invalid coordinates"));
            }
        }
    } catch (const std::exception&) {
        return std::unexpected(Error(ErrorType::InvalidInput, "Coordinates must be numbers"));
    }

    if (coords[0] < -90 || coords[0] > 90 || coords[2] < -90 || coords[2] > 90 ||
        coords[1] < -180 || coords[1] > 180 || coords[3] < -180 || coords[3] > 180) {
        return std::unexpected(Error(ErrorType::InvalidInput, "Coordinates out of range"));
    }

    AlgorithmType algorithm;
    if (inputs[5] == "astar") algorithm = AlgorithmType::AstarAlgorithm;
    else if (inputs[5] == "dijkstra") algorithm = AlgorithmType::DijkstraAlgorithm;
    else return std::unexpected(Error(ErrorType::InvalidInput, "Unknown algorithm"));

    DataParser parser(".data");
    return parser.PrintRouteJSONFromCoords(coords[0], coords[1], coords[2], coords[3], algorithm);
}

std::expected<void, Error> ParseMainInputs(std::vector<std::string>& inputs) {
    if (inputs.size() == 0) {
        return std::unexpected(Error(ErrorType::InvalidInput, "No initializer flag passed"));
    }

    if (inputs[0] == "web") {
        return InitializeWeb();
    } else if (inputs[0] == "manual") {
        // manual testing of algorithm on real florida addresses
    } else if (inputs[0] == "file") {
        return ConstructPathFromFile(inputs.size() > 1 ? inputs[1] : "");
    } else if (inputs[0] == "dijkstra") {
        return RunDijkstraTest();
    } else if (inputs[0] == "stage_data") {
        DataParser p(".data", false);
        return p.ContructDataFromPath();
    } else if (inputs[0] == "get_closest_node") {
        return GetClosestNodeFromCoords(inputs);
    } else if (inputs[0] == "get_route_json") {
        return GetWebRouteJSON(inputs);
    } else if (inputs[0] == "get_path") {
        return GetPath(inputs);
    }

    return std::unexpected(Error(ErrorType::InvalidInput, "Invalid initializer flag"));
}

// Graph createGraph(){
//     std::string start, end;

//     std::getline(std::cin, start);
//     std::getline(std::cin, end);

//     std::string command = 
//         "\"/opt/anaconda3/envs/ox/bin/python\" "
//         "\"src/getNodes.py\" \"" + start + "\" \"" + end + "\"";

//     system(command.c_str());

//     Graph graph;
//     graph.loadNodes();
//     graph.loadEdges();

//     return graph;
// }
