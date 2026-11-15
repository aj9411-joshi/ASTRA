#ifndef ASTRA_ROUTING_HPP
#define ASTRA_ROUTING_HPP

#include <vector>

namespace astra {

struct RouteEdge {
    int    target;
    double distance;
    double riskFactor;
    double fuelCost;

    double effectiveCost() const { return distance * (1.0 + riskFactor) + fuelCost; }
};

struct RouteResult {
    bool             reachable = false;
    double           totalCost = 0.0;
    std::vector<int> path;           // node ids from start to target
};

class RouteGraph {
public:
    explicit RouteGraph(int nodes) : adj_(nodes > 0 ? nodes : 0) {}

    // Undirected pathway. Returns false if a node index is invalid.
    bool addPathway(int a, int b, double distance, double risk, double fuel);

    int nodeCount() const { return static_cast<int>(adj_.size()); }
    const std::vector<std::vector<RouteEdge>> &adjacency() const { return adj_; }

private:
    std::vector<std::vector<RouteEdge>> adj_;
};

// Lowest effective-cost route (distance*(1+risk)+fuel) using Dijkstra.
RouteResult runDijkstra(const RouteGraph &g, int start, int target);

// The 5-sector sample network from the original prototype.
RouteGraph buildSampleSectorGraph();

} // namespace astra

#endif
