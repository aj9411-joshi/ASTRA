/* ASTRA safe-route demo using Dijkstra on risk/fuel-weighted sectors. */
#include <iostream>
#include "routing.hpp"

int main() {
    astra::RouteGraph g = astra::buildSampleSectorGraph();
    astra::RouteResult r = astra::runDijkstra(g, 0, 4);

    if (!r.reachable) {
        std::cout << "Target is unreachable.\n";
        return 0;
    }
    std::cout << "Total cost to target: " << r.totalCost << "\n";
    std::cout << "Recommended safe path: ";
    for (size_t i = 0; i < r.path.size(); ++i)
        std::cout << r.path[i] << (i + 1 < r.path.size() ? "->" : "");
    std::cout << "\n";
    return 0;
}
