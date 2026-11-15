#include "routing.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>

namespace astra {

bool RouteGraph::addPathway(int a, int b, double distance, double risk, double fuel) {
    const int n = nodeCount();
    if (a < 0 || b < 0 || a >= n || b >= n) return false;
    adj_[a].push_back({b, distance, risk, fuel});
    adj_[b].push_back({a, distance, risk, fuel});   // reverse edge belongs to node b
    return true;
}

RouteResult runDijkstra(const RouteGraph &g, int start, int target) {
    RouteResult result;
    const int n = g.nodeCount();
    if (start < 0 || target < 0 || start >= n || target >= n) return result;

    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> cost(n, INF);
    std::vector<int> parent(n, -1);

    using Item = std::pair<double, int>;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> pq;
    cost[start] = 0.0;
    pq.push({0.0, start});

    while (!pq.empty()) {
        auto [c, u] = pq.top();
        pq.pop();
        if (c > cost[u]) continue;
        for (const RouteEdge &e : g.adjacency()[u]) {
            double next = c + e.effectiveCost();
            if (next < cost[e.target]) {
                cost[e.target] = next;
                parent[e.target] = u;
                pq.push({next, e.target});
            }
        }
    }

    if (cost[target] == INF) return result;
    result.reachable = true;
    result.totalCost = cost[target];
    for (int cur = target; cur != -1; cur = parent[cur]) result.path.push_back(cur);
    std::reverse(result.path.begin(), result.path.end());
    return result;
}

RouteGraph buildSampleSectorGraph() {
    RouteGraph g(5);
    g.addPathway(0, 1, 10.0, 0.1, 2.0);
    g.addPathway(0, 2,  5.0, 1.5, 1.0);
    g.addPathway(1, 3,  8.0, 0.2, 1.5);
    g.addPathway(2, 3,  2.0, 1.0, 0.5);
    g.addPathway(3, 4,  7.0, 0.0, 1.0);
    return g;
}

} // namespace astra
