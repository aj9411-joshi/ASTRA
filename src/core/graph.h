#ifndef ASTRA_GRAPH_H
#define ASTRA_GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NODES 100

typedef struct {
    int nodeCount;
    int adjacencyMatrix[MAX_NODES][MAX_NODES];
    double weights[MAX_NODES][MAX_NODES];
} Graph;

/* ---- construction ---- */
void initializeGraph(Graph *graph, int nodes);          /* clears graph, sets node count */
void addNode(Graph *graph);
void addEdge(Graph *graph, int source, int destination, double weight);          /* directed   */
void addUndirectedEdge(Graph *graph, int a, int b, double weight);               /* both ways  */
int  hasEdge(const Graph *graph, int source, int destination);

/* ---- file loading ----
   Format (see data/space_network.txt):
     NODES <n>
     EDGE <from> <to> <weight>      (undirected)
   Lines starting with # are comments. Returns 1 on success, 0 on failure. */
int graphLoadFromFile(Graph *graph, const char *path);

/* ---- data-returning algorithms (used by the GUI) ----
   order/path buffers must hold at least MAX_NODES ints. */
int    graphBFSOrder(const Graph *graph, int start, int *order);   /* returns count, -1 invalid */
int    graphDFSOrder(const Graph *graph, int start, int *order);   /* returns count, -1 invalid */
int    graphFindPath(const Graph *graph, int src, int dst, int *path); /* fewest hops; length or 0 */
double graphPathWeight(const Graph *graph, const int *path, int length);

/* ---- printing helpers (used by the CLI demo) ---- */
void displayGraph(const Graph *graph);
void BFS(const Graph *graph, int startNode);
void DFS(const Graph *graph, int startNode);
int  findRoute(const Graph *graph, int source, int destination);

#ifdef __cplusplus
}
#endif

#endif
