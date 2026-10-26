#include "dijkstra.hpp"
#include <cassert>
#include <cmath>
#include <stdexcept>
int main() {
    Graph g;
    for (long long i=1; i<=5; ++i) g.addNode(0,0,i);
    g.addEdge(1,2,1,"a"); g.addEdge(1,3,2,"b");
    g.addEdge(2,4,10,"c"); g.addEdge(3,4,2,"d");
    g.addEdge(2,3,1,"e");
    auto r = Dijkstra(g,1,4);
    assert(r.found && r.totalDistance == 4.0 && r.path.front()==1 && r.path.back()==4);
    auto missing = Dijkstra(g,4,1); assert(!missing.found && missing.path.empty());
    auto same = Dijkstra(g,1,1); assert(same.found && same.totalDistance == 0 && same.path.size()==1);
    bool invalid = false;
    g.addEdge(1,5,-1,"invalid");
    try { (void)Dijkstra(g,1,5); } catch (const std::invalid_argument&) { invalid=true; }
    assert(invalid);
    return 0;
}
