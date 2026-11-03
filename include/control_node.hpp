#pragma once

#include <unordered_map>
#include <string>

constexpr char PATHABLE_START_CHARACTER = 'A';
constexpr char PATHABLE_TARGET_CHARACTER = 'B';

class ControlNodeContainer;

class ControlNode {
    public:
        ControlNode(ControlNodeContainer* container, int x, int y, int cost = 1);
        bool operator>(const ControlNode& other) const;

        void GetNeighbors(std::vector<ControlNode*>& buffer);
        ControlNode* GetParent();
        void SetParent(ControlNode* parent, double distance = 0.0);

        std::string GetKey() const;
        char GetCharacterRepresentation(bool isPath = false) const;

        uint64_t GetId() const;
        
        double GetCost(ControlNode* other = nullptr) const;
        double GetGCost() const;
        double GetHCost() const;
        double GetFCost() const;

        double GetLatitude() const;
        double GetLongitude() const;

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
        ControlNode* parent = nullptr;
};
