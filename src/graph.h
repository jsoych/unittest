#ifndef GRAPH_H
#define GRAPH_H

#include "unittest.h"

struct graph_vtx
{
    int id;
    void *value;
};

#define GRAPH_VTX_EMPTY ((struct graph_vtx){0})

static inline int graph_vtx_check(int id, void *value)
{
    return id > 0 && value != NULL;
}

static inline int graph_vtx_ok(const struct graph_vtx *vtx)
{
    return vtx && vtx->id > 0 && vtx->value;
}

typedef struct Graph Graph;

/*
 * Creates a new graph and returns it.
 */
Graph *graph_create(void);

/*
 * Destroys the graphs and all of its resources.
 */
void graph_destroy(Graph *graph);

/*
 * Adds a new vertex to the graph with the given id and value.
 */
int graph_add_vtx(Graph *graph, int id, void *value);

/*
 * Adds a new edge to the graph.
 */
int graph_add_edge(Graph *graph, int from, int to);

/*
 * Starts the scanner at first vertex in the graph.
 */
void graph_start_scan(Graph *graph);

/*
 * Reads the current vertex value and returns it, and advances the scanner
 * to the next vertex.
 */
struct graph_vtx graph_read_scan(Graph *graph);

/*
 * Initializes the graph scanner in topological order and returns 0, if
 * sucessful. Otherwise, returns -1 indicating a cycle was detected and
 * the iterator traverses along the cycle.
 */
int graph_start_topological_scan(Graph *graph);

/*
 * Reads the current vertex value and returns it, and advances the scanner
 * to next vertex in topological order.
 */
struct graph_vtx graph_read_topological_scan(Graph *graph);

/*
 * Taint vertex.
 */
void graph_taint_vtx(Graph *graph, int id);

#endif
