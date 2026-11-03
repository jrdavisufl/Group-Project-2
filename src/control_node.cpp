#include <cmath>
#include <vector>

#include "control_node_container.hpp"
#include "control_node.hpp"

ControlNode::ControlNode(ControlNodeContainer* container, int x, int y, int cost) {
    this->x = x;
    this->y = y;

    this->cost = cost;

    this->container = container;
}

bool ControlNode::operator>(const ControlNode& other) const {
    return this->GetFCost() > other.GetFCost();
}

std::vector<ControlNode*> ControlNode::GetNeighbors() {
    std::vector<ControlNode*> outgoing = {};

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

void ControlNode::SetParent(ControlNode* parent, double distance) {
    this->parent = parent;
    this->g = parent->GetGCost() + this->cost;
}

ControlNode* ControlNode::GetParent() {
    return this->parent;
}

std::string ControlNode::GetKey(int x, int y) {
    return std::to_string(x) + ":" + std::to_string(y);
}

std::string ControlNode::GetKey() const {
    return ControlNode::GetKey(this->x, this->y);
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

// unused
uint64_t ControlNode::GetId() const {
    return 0;
}

double ControlNode::GetCost(ControlNode* other) const {
    return static_cast<double>(this->cost);
}

double ControlNode::GetGCost() const {
    return static_cast<double>(this->g);
}

double ControlNode::GetHCost() const {
    auto target = this->container->target;
    if (target == nullptr) {
        return 0;
    }

    int a = target->x - this->x;
    int b = target->y - this->y;

    a *= a;
    b *= b;

    return static_cast<double>(std::ceil(std::sqrt(a + b)));
}

double ControlNode::GetFCost() const {
    return this->GetGCost() + this->GetHCost();
}

double ControlNode::GetLatitude() const {
    return 0.0;
}

double ControlNode::GetLongitude() const {
    return 0.0;
}

void ControlNode::Reset() {
    this->g = 0;
    this->parent = nullptr;
}
