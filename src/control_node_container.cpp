#include <algorithm>
#include <expected>
#include <unordered_map>
#include <sstream>
#include <string>

#include "astar_algorithm.hpp"
#include "control_node_container.hpp"

ControlNodeContainer::~ControlNodeContainer() {
    for (auto& [key, val] : nodeMap) {
        delete val;
    }

    nodeMap.clear();
}

ControlNode* ControlNodeContainer::GetNode(int x, int y) {
    if (x < 0 || y < 0) {
        return nullptr;
    }

    return this->GetNodeFromKey(ControlNode::GetKey(x, y));
}

ControlNode* ControlNodeContainer::GetNodeFromKey(std::string key) {
    if (!nodeMap.contains(key)) {
        return nullptr;
    }

    return nodeMap[key];
}

void ControlNodeContainer::PrintGrid(ankerl::unordered_dense::map<ControlNode*, bool> path) {
    for (int i = 0; i < this->yBounds; ++i) {
        std::stringstream outgoing;

        for (int j = 0; j < this->xBounds; ++j) {
            auto node = this->GetNode(j, i);
            if (node == nullptr) {
                outgoing << ' ';
            } else {
                outgoing << node->GetCharacterRepresentation(path.contains(node));
            }
        }

        std::cout << outgoing.str() << std::endl;
    }
}

void ControlNodeContainer::PrintAstarGrid() {
    auto result = GetAstarPath<ControlNode>(this->start, this->target);
    ankerl::unordered_dense::map<ControlNode*, bool> path = {};

    for (int i = 0; i < result.path.size(); ++i) {
        path[result.path[i]] = true;
    }

    this->PrintGrid(path);
}

std::expected<void, Error> ControlNodeContainer::AddNode(int x, int y, char character) {
    auto key = ControlNode::GetKey(x, y);
    if (nodeMap.contains(key)) {
        return std::unexpected(Error(ErrorType::DuplicateValue, "Key already exists"));
    }

    int cost = costMap[character] > 0 ? costMap[character] : 1;
    
    ControlNode* newNode = new ControlNode(this, x, y, cost);
    nodeMap[key] = newNode;

    if (character == PATHABLE_START_CHARACTER) {
        if (this->start != nullptr) {
            return std::unexpected(Error(ErrorType::DuplicateValue, "Two start points defined"));
        }

        this->start = newNode;
    }

    if (character == PATHABLE_TARGET_CHARACTER) {
        if (this->target != nullptr) {
            return std::unexpected(Error(ErrorType::DuplicateValue, "Two target points defined"));
        }

        this->target = newNode;
    }

    return {};
}

std::expected<void, Error> ControlNodeContainer::AddLine(std::string& line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    if (line == "") {
        return std::unexpected(Error(ErrorType::InvalidInput, "Empty Line"));
    }

    for (int x = 0; x < line.size(); ++x) {
        if (!costMap.contains(line[x])) {
            if (line[x] != ' ') {
                return std::unexpected(Error(ErrorType::InvalidInput, "Invalid character found on line"));
            }

            continue;
        }

        this->AddNode(x, this->yBounds, line[x]);
    }

    this->xBounds = std::max(this->xBounds, static_cast<int>(line.size()));
    this->yBounds++;
    return {};
}