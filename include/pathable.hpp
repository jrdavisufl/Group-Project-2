#pragma once

#include <vector>

class Pathable {
    public:
        virtual bool IsGreater(Pathable* other, Pathable* target) const = 0;
        virtual std::vector<Pathable*> GetNeighbors() = 0;

        virtual int GetGCost() const = 0;
        virtual int GetHCost(Pathable* target) const = 0;
        virtual int GetFCost(Pathable* target) const = 0;
};
