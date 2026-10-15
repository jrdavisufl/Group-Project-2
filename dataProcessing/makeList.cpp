#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

//This file creates an adjaceny list that will represent the weighted directed graph for our algorithms

struct Node {
    double latitude, longitude;
    long long id;

    Node(double lat, double lon, long long ID) : latitude(lat), longitude(lon), id(ID) {}
};

struct Edge{
    int destination; //index of destination node
    double distance;
    string streetName;

    Edge(int d, double d2, string n) : destination(d), distance(d2), streetName(n) {}
};

class Graph{
public:
    unordered_map<long long, Node> nodes;
    unordered_map<long long, vector<Edge>> adjacencyList;

    void addNode(double latitude, double longitude, long long id){
        nodes[id] = Node(latitude, longitude, id);
        adjacencyList[id] = {};
    }

    void addEdge(int sourceID, int destination, double distance, string name){
        adjacencyList[sourceID].push_back(Edge(destination, distance, name));
    }
};


