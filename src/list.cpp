#include "list.hpp"

void Graph::addNode(double latitude, double longitude, long long id){
    nodes[id] = Node(latitude, longitude, id);
    adjacencyList[id] = {};
}

void Graph::addEdge(long long sourceID, long long destination, double distance, string name){
    adjacencyList[sourceID].push_back(Edge(destination, distance, name));
}

void Graph::loadNodes(){
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

void Graph::loadEdges(){
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