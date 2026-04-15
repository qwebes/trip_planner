#ifndef Graph_hpp
#define Graph_hpp

#include <string>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <limits>

using namespace std;

struct Edge {
    string target;
    double weight;
    Edge(string t, double w) : target(t), weight(w) {}
};

class Graph {
private:
    map<string, list<Edge>> adjacencyList;

public:
    void addEdge(string source, string target, double weight);
    vector<string> findShortestPath(string start, string end);
};

#endif // !Graph_hpp
