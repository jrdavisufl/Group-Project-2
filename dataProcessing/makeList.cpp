#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
using namespace std;

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
        ;
    }

    void addNode(double latitude, double longitude, long long id){
        nodes[id] = Node(latitude, longitude, id);
        adjacencyList[id] = {};
    }

    void addEdge(long long sourceID, long long destination, double distance, string name){
        adjacencyList[sourceID].push_back(Edge(destination, distance, name));
    }

    //Load nodes from CSV into graph
    void loadNodes(){
        ifstream file("data/nodes.csv");
        string line;

        getline(file, line);

        while(getline(file, line)){
            stringstream ss(line);
            string strID, strLat, strLon;


            getline(ss, strID, ',');
            getline(ss, strLat, ',');
            getline(ss, strLon, ',');

            long long id = stoll(strID);
            double lat = stod(strLat);
            double lon = stod(strLon);

            addNode(lat, lon, id);
        }
    }

    //Load edges from CSV into graph
    void loadEdges(){
        ifstream file("data/nodes.csv");
        string line;

        getline(file, line);

        while(getline(file, line)){
            stringstream ss(line);
            string strSource, strDest, strDist, name;


            getline(ss, strSource, ',');
            getline(ss, strDest, ',');
            getline(ss, strDist, ',');
            getline(ss, name, ',');

            long long source = stoll(strSource);
            long long destination = stoll(strDest);
            double distance = stod(strDist);

            addEdge(source, destination, distance, name);
        }
    }
};


