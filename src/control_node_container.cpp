#include <expected>
#include <string>

#include "control_node_container.hpp"

ControlNodeContainer::~ControlNodeContainer() {
    for (auto& [key, val] : nodeMap) {
        delete val;
    }

    nodeMap.clear();
}

std::expected<void, Error> ControlNodeContainer::AddNode(int x, int y) {
    auto key = ControlNode::GetKey(x, y);
    if (nodeMap.contains(key)) {
        return std::unexpected(Error(ErrorType::DuplicateValue, "Key already exists"));
    }
    
    ControlNode* newNode = new ControlNode(this, x, y);
    nodeMap[key] = newNode;

    return {};
}

ControlNode* ControlNodeContainer::GetNode(int x, int y) {
    return this->GetNodeFromKey(ControlNode::GetKey(x, y));
}

ControlNode* ControlNodeContainer::GetNodeFromKey(std::string key) {
    if (!nodeMap.contains(key)) {
        return nullptr;
    }

    return nodeMap[key];
}