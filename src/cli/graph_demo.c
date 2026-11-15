/* ASTRA space-network demo (BFS / DFS / route finding).
   Usage: astra_graph_demo [path/to/space_network.txt] */
#include <stdio.h>
#include "graph.h"

int main(int argc, char **argv) {
    static Graph astra;   /* ~120 KB, keep off the stack */
    const char *file = argc > 1 ? argv[1] : "data/space_network.txt";

    if (graphLoadFromFile(&astra, file)) {
        printf("Loaded network from %s\n", file);
    } else {
        printf("Could not read %s - using built-in sample network.\n", file);
        initializeGraph(&astra, 4);
        addUndirectedEdge(&astra, 0, 1, 10.5);
        addUndirectedEdge(&astra, 0, 2, 15.0);
        addUndirectedEdge(&astra, 1, 2, 8.5);
        addUndirectedEdge(&astra, 2, 3, 12.0);
    }

    displayGraph(&astra);
    printf(hasEdge(&astra, 0, 1) ? "\nConnection exists between Node 0 and Node 1.\n"
                                  : "\nNo connection between Node 0 and Node 1.\n");
    BFS(&astra, 0);
    DFS(&astra, 0);
    printf("\nRoute from Node 0 to Node 3:\n");
    findRoute(&astra, 0, 3);
    printf("\nRoute from Node 3 to Node 0:\n");
    findRoute(&astra, 3, 0);
    return 0;
}
