#pragma once

#include <vector>

constexpr char PATHABLE_START_CHARACTER = 'A';
constexpr char PATHABLE_TARGET_CHARACTER = 'B';

class Pathable {
    public:
        bool operator>(const Pathable& other) const;

        virtual std::vector<Pathable*> GetNeighbors() = 0;
        virtual Pathable* GetParent() = 0;

        virtual void SetParent(Pathable* parent) = 0;
        
        virtual int GetCost() const = 0;
        virtual int GetGCost() const = 0;
        virtual int GetHCost() const = 0;
        virtual int GetFCost() const = 0;

        virtual void Reset() = 0;
};
