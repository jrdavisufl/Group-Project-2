#include <cmath>
#include <vector>

#include "control_node_container.hpp"
#include "control_node.hpp"
#include "pathable.hpp"

ControlNode::ControlNode(ControlNodeContainer* container, int x, int y, int cost) {
    this->x = x;
    this->y = y;

    this->cost = cost;

    this->container = container;
}

bool ControlNode::operator>(const Pathable& other) const {
    return this->GetFCost() > other.GetFCost();
}

std::vector<Pathable*> ControlNode::GetNeighbors() {
    std::vector<Pathable*> outgoing = {};

    auto tryAdd = [&](int x, int y) {
        auto node = container->GetNode(x, y);
        if (node == nullptr) return;

        outgoing.push_back(node);
    };

    tryAdd(x - 1, y);
    tryAdd(x + 1, y);
    tryAdd(x, y - 1);
    tryAdd(x, y + 1);

    return outgoing;
}

void ControlNode::SetParent(Pathable* parent) {
    this->parent = parent;
    this->g = parent->GetGCost() + this->cost;
}

Pathable* ControlNode::GetParent() {
    return this->parent;
}

std::string ControlNode::GetKey(int x, int y) {
    return std::to_string(x) + ":" + std::to_string(y);
}

std::string ControlNode::GetKey() const {
    return ControlNode::GetKey(this->x, this->y);
}

int ControlNode::GetCost() const {
    return this->cost;
}

char ControlNode::GetCharacterRepresentation(bool isPath) const {
    if (this == container->start) {
        return PATHABLE_START_CHARACTER;
    }

    if (this == container->target) {
        return PATHABLE_TARGET_CHARACTER;
    }

    if (isPath) {
        if (this->parent == nullptr) {
            return '-';
        }

        auto parent = static_cast<ControlNode*>(this->parent);

        return (this->y != parent->y) ? '|' : '-';
    }

    switch (this->cost) {
        case 1:
            return '0';
        case 2:
            return '#';
        case 3:
            return '%';
    }

    return '0';
}

int ControlNode::GetGCost() const {
    return this->g;
}

int ControlNode::GetHCost() const {
    auto target = this->container->target;
    if (target == nullptr) {
        return 0;
    }

    int a = target->x - this->x;
    int b = target->y - this->y;

    a *= a;
    b *= b;

    return static_cast<int>(std::ceil(std::sqrt(a + b)));
}

int ControlNode::GetFCost() const {
    return this->GetGCost() + this->GetHCost();
}

void ControlNode::Reset() {
    this->g = 0;
    this->parent = nullptr;
}
