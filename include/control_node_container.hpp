#pragma once

#include <ankerl/unordered_dense.h>
#include <expected>
#include <string>

#include "control_node.hpp"
#include "error.hpp"

class ControlNodeContainer {
    public:
        ~ControlNodeContainer();

        ControlNode* GetNode(int x, int y);
        ControlNode* GetNodeFromKey(std::string key);
        
        void PrintGrid(ankerl::unordered_dense::map<ControlNode*, bool> path = {});
        void PrintAstarGrid();

        std::expected<void, Error> AddNode(int x, int y, char character = '0');
        std::expected<void, Error> AddLine(std::string& line);

        friend class ControlNode;

    private:
        ankerl::unordered_dense::map<std::string, ControlNode*> nodeMap;

        std::unordered_map<char, int> costMap = {
            {'A', 1},
            {'B', 1},
            {'0', 1},
            {'#', 2},
            {'%', 3},
        };

        int xBounds = 0;
        int yBounds = 0;

        ControlNode* start = nullptr;
        ControlNode* target = nullptr;
};
