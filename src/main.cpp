#include <expected>
#include <iostream>
#include <string>
#include <vector>

#include "core.hpp"
#include "error.hpp"

int main(int argc, char* argv[]) {

    if (argc == 1) {
        std::cerr << "No arguments passed" << std::endl;
        return 1;
    }

    // parse arguments into nicer vector format
    std::vector<std::string> args = {};
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    auto result = ParseMainInputs(args);
    if (!result) {
        std::cerr << result.error() << std::endl;
        return 1;
    }
    
    return 0;
}