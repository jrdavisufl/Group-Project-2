#include <expected>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include "control_node_container.hpp"
#include "core.hpp"
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
    }

    return std::unexpected(Error(ErrorType::InvalidInput, "Invalid initializer flag"));
}