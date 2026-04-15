#include "Graph.hpp"
#include <cmath>
#include <limits>

void Graph::addEdge(string source, string target, double weight) {
    adjacencyList[source].push_back(Edge(target, weight));
    
    if (adjacencyList.find(target) == adjacencyList.end()) {
        adjacencyList[target] = list<Edge>();
    }
}

vector<string> Graph::findShortestPath(string start, string end) {
    map<string, double> distances;
    map<string, string> previous;
    
    for (auto const& [name, edges] : adjacencyList) {
        distances[name] = INFINITY;
    }
    distances[start] = 0;

    priority_queue<pair<double, string>, vector<pair<double, string>>, greater<pair<double, string>>> pq;
    pq.push({0, start});

    while (!pq.empty()) {
        string u = pq.top().second;
        double d = pq.top().first;
        pq.pop();

        if (d > distances[u]){
            continue;
        }
        if (u == end){
            break;
        }

        for (const auto& edge : adjacencyList[u]) {
            double newDist = distances[u] + edge.weight;
            
            if (newDist < distances[edge.target]) {
                distances[edge.target] = newDist;
                previous[edge.target] = u;
                pq.push({newDist, edge.target});
            }
        }
    }

    vector<string> path;
    string at = end;
    
    while (at != "") {
        path.push_back(at);
        if (at == start) {
            break;
        }
        
    
        if (previous.find(at) == previous.end()){
            break;
        }
        at = previous[at];
    }


    if (path.empty() || path.back() != start) return {};

    size_t n = path.size();
    for (int i = 0; i < n / 2; ++i) {
        string temp = path[i];
        path[i] = path[n - 1 - i];
        path[n - 1 - i] = temp;
    }

    return path;
}
