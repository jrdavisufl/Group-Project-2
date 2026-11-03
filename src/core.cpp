#include <expected>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

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
    } else if (inputs[0] == "prime_data") {
        DataParser p(".data");
        return p.ContructDataFromPath();
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
