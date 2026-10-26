#pragma once

#include <unordered_map>
#include <string>

#include "pathable.hpp"

class ControlNodeContainer;

class ControlNode : public Pathable {
    public:
        ControlNode(ControlNodeContainer* container, int x, int y, int cost = 1);
        bool operator>(const Pathable& other) const;

        std::vector<Pathable*> GetNeighbors();
        Pathable* GetParent();
        void SetParent(Pathable* parent);

        std::string GetKey() const;
        char GetCharacterRepresentation(bool isPath = false) const;
        
        int GetCost() const;
        int GetGCost() const;
        int GetHCost() const;
        int GetFCost() const;

        void Reset();
        
        static std::string GetKey(int x, int y);

        friend class ControlNodeContainer;

    private:
        ControlNodeContainer* container = nullptr;

        int x = 0;
        int y = 0;

        int cost = 0;

        // A* pathfinding variables
        int g = 0;
        Pathable* parent = nullptr;
};
