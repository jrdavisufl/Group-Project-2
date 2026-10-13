#include <vector>

#include "control_node_container.hpp"
#include "control_node.hpp"

ControlNode::ControlNode(ControlNodeContainer* container, int x, int y, int cost) {
    this->x = x;
    this->y = y;

    this->cost = cost;

    this->container = container;
}

bool ControlNode::IsGreater(Pathable* other, Pathable* target) const {
    if (other == nullptr) {
        return true;
    }

    return this->GetFCost(target) > other->GetFCost(target);
}

std::vector<Pathable*> ControlNode::GetNeighbors() {
    std::vector<Pathable*> outgoing = {};

    auto tryAdd = [&](int x, int y) {
        auto node = container->GetNode(x, y);
        if (node == nullptr) return;

        node->setParent(this);
        outgoing.push_back(node);
    };

    tryAdd(x - 1, y);
    tryAdd(x + 1, y);
    tryAdd(x, y - 1);
    tryAdd(x, y + 1);

    return outgoing;
}

std::string ControlNode::GetKey(int x, int y) {
    return std::to_string(x) + ":" + std::to_string(y);
}

std::string ControlNode::GetKey() const {
    return ControlNode::GetKey(this->x, this->y);
}

int ControlNode::GetGCost() const {
    return this->g;
}

int ControlNode::GetHCost(Pathable* target) const {
    if (target == nullptr) {
        return 0;
    }

    int a = target->x - this->x;
    int b = target->y - this->y;

    a *= a;
    b *= b;

    return static_cast<int>(std::ceil(std::sqrt(a + b)));
}

int ControlNode::GetFCost(Pathable* target) const {
    return this->GetGCost() + this->GetHCost(target);
}

void ControlNode::reset() {
    this->g = 0;
    this->parent = nullptr;
}

void ControlNode::setParent(ControlNode* parent) {
    this->parent = parent;
    this->g = parent->GetGCost() + this->cost;
}