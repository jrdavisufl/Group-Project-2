#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
using namespace std;

#ifndef GRAPH_HPP
#define GRAPH_HPP

//This file creates an adjaceny list that will represent the weighted directed graph for our algorithms

struct Node {
    double latitude, longitude;
    long long id;

    Node(double lat, double lon, long long ID) : latitude(lat), longitude(lon), id(ID) {}
};

struct Edge{
    long long destination; //id of destination node
    double distance;
    string streetName;

    Edge(long long id, double d, string n) : destination(id), distance(d), streetName(n) {}
};

class Graph{
public:
    unordered_map<long long, Node> nodes;
    unordered_map<long long, vector<Edge>> adjacencyList;

    Graph(){
        nodes = {};
        adjacencyList = {};
    }

    void addNode(double latitude, double longitude, long long id);

    void addEdge(long long sourceID, long long destination, double distance, string name);

    //Load nodes from CSV into graph
    void loadNodes();

    //Load edges from CSV into graph
    void loadEdges();
};

#endif