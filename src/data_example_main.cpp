#include <iostream>
#include <string>
#include <cstdlib>
#include "list.hpp"

Graph createGraph(){
    std::string start, end;

    std::getline(std::cin, start);
    std::getline(std::cin, end);

    std::string command = 
        "\"/opt/anaconda3/envs/ox/bin/python\" "
        "\"src/getNodes.py\" \"" + start + "\" \"" + end + "\"";

    system(command.c_str());

    Graph graph;
    graph.loadNodes();
    graph.loadEdges();

    return graph;
}

int main() {
    std::cout << "test" << std::endl;
    return 0;
}