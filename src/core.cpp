#include <expected>
#include <string>
#include <vector>

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

    return {};
}

std::expected<void, Error> ParseMainInputs(std::vector<std::string>& inputs) {
    if (inputs.size() == 0) {
        return std::unexpected(Error(ErrorType::InvalidInput, "No initializer flag passed"));
    }

    if (inputs[0] == "web") {
        return InitializeWeb();
    } else if ("manual") {
        // manual testing of algorithm on real florida addresses
    } else if ("file") {
        return ConstructPathFromFile(inputs.size() > 1 ? inputs[1] : "");
    }

    return std::unexpected(Error(ErrorType::InvalidInput, "Invalid initializer flag"));
}