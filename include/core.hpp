#pragma once

#include <expected>
#include <string>
#include <vector>

#include "error.hpp"

std::expected<void, Error> InitializeWeb();
std::expected<void, Error> ConstructPathFromFile(std::string fileName);
std::expected<void, Error> ParseMainInputs(std::vector<std::string>& inputs);