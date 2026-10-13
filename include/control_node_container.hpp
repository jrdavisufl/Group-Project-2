#pragma once

#include <ankerl/unordered_dense.h>
#include <expected>
#include <string>

#include "control_node.hpp"
#include "error.hpp"

class ControlNodeContainer {
    public:
        ~ControlNodeContainer();

        std::expected<void, Error> AddNode(int x, int y);
        ControlNode* GetNode(int x, int y);
        ControlNode* GetNodeFromKey(std::string key);
    private:
        ankerl::unordered_dense::map<std::string, ControlNode*> nodeMap;
};
