#pragma once

#include <unordered_map>
#include <string>

#include "pathable.hpp"

class ControlNodeContainer;

class ControlNode : public Pathable {
    public:
        ControlNode(ControlNodeContainer* container, int x, int y, int cost = 1);

        bool IsGreater(Pathable* other, Pathable* target) const;
        std::vector<Pathable*> GetNeighbors();

        std::string GetKey() const;
        int GetCost() const;
        char GetCharacterRepresentation() const;
        
        int GetGCost() const;
        int GetHCost(Pathable* target) const;
        int GetFCost(Pathable* target) const;
        
        static std::string GetKey(int x, int y);

        friend class ControlNodeContainer;

    private:
        ControlNodeContainer* container = nullptr;

        int x = 0;
        int y = 0;

        int cost = 0;

        // reset A* values
        void reset();
        void setParent(ControlNode* parent);

        // A* pathfinding variables
        int g = 0;
        Pathable* parent = nullptr;
};
