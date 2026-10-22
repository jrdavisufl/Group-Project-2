#include "list.hpp"

void Graph::addNode(double latitude, double longitude, long long id){
    nodes.emplace(id, Node(latitude, longitude, id));
    adjacencyList.emplace(id, vector<Edge>{});
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


        getline(ss, strID, '\t');
        getline(ss, strLat, '\t');
        getline(ss, strLon, '\t');

        long long id = stoll(strID);
        double lat = stod(strLat);
        double lon = stod(strLon);

        addNode(lat, lon, id);
    }
}

void Graph::loadEdges(){
    ifstream file("data/edges.csv");
    string line;

    getline(file, line);

    while(getline(file, line)){
        stringstream ss(line);
        string strSource, strDest, strDist, name;


        getline(ss, strSource, '\t');
        getline(ss, strDest, '\t');
        getline(ss, strDist, '\t');
        getline(ss, name);

        long long source = stoll(strSource);
        long long destination = stoll(strDest);
        double distance = stod(strDist);

        addEdge(source, destination, distance, name);
    }
}