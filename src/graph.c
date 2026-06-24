#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include "graph.h"
#include "print.h"
#include "print_macros.h"
#include "table.h"

#define ARRAY_SIZE (8)
#define INFINITY (-1)

enum
{
    WHITE = 0,
    GREY,
    BLACK
};

enum
{
    MODE_TABLE,
    MODE_GRAPH
};

struct graph_array
{
    int size;
    int capacity;
    int *id;
};

struct graph_metadata
{
    int colour;
    int is_tainted;
};

struct graph_node
{
    struct graph_metadata metadata;

    struct graph_vtx vtx;
    struct graph_array parent;
    struct graph_array children;

    struct graph_node *scanner_next;
};

struct graph_scanner
{
    int cycle_id;
    struct graph_node *curr;
    struct graph_node *head;
    struct graph_node **tail;
};

struct Graph
{
    int size;
    Table *node;
    struct graph_scanner scanner;
};

static int array_init(struct graph_array *arr, int cap)
{
    arr->id = malloc(cap * sizeof(int));
    if (cap != 0 && !arr->id)
        return ENOMEM;
    arr->size = 0;
    arr->capacity = cap;
    return 0;
}

static void array_free(struct graph_array *arr)
{
    if (!arr)
        return;
    if (arr->id)
        free(arr->id);
    arr->size = 0;
    arr->capacity = 0;
    arr->id = NULL;
}

static int array_add(struct graph_array *arr, int id)
{
    if (arr->capacity == 0)
    {
        array_free(arr);
        array_init(arr, ARRAY_SIZE);
    }
    else if (arr->size == arr->capacity)
    {
        int c = 2 * arr->capacity;
        int *id = realloc(arr->id, c * sizeof(int));
        if (!id)
            return ENOMEM;
        arr->capacity = c;
        arr->id = id;
    }
    arr->id[arr->size++] = id;
    return 0;
}

/*
 * Pops the last element from the array and returns it. Otherwise, returns 0
 * indiciting the array is empty.
 */
static int array_pop(struct graph_array *arr)
{
    if (!arr->size)
        return 0;
    return arr->id[arr->size--];
}

static void metadata_init(struct graph_metadata *meta)
{
    meta->colour = WHITE;
    meta->is_tainted = 0;
}

static void node_free(struct graph_node *node)
{
    if (!node)
        return;
    array_free(&node->parent);
    array_free(&node->children);
}

static void scanner_init(struct graph_scanner *scanner)
{
    scanner->cycle_id = INFINITY;
    scanner->curr = NULL;
    scanner->head = NULL;
    scanner->tail = &scanner->head;
}

/* Removes the head of the iter and returns it. */
static struct graph_node *scanner_dequeue(struct graph_scanner *scanner)
{
    if (!scanner->head)
        return NULL;

    struct graph_node *node = scanner->head;
    scanner->head = node->scanner_next;

    if (scanner->tail == &node->scanner_next)
        scanner->tail = &scanner->head;

    node->scanner_next = NULL;
    return node;
}

/* Adds the node to the tail of the iter. */
static void scanner_enqueue(struct graph_scanner *scanner, struct graph_node *node)
{
    *(scanner->tail) = node;
    scanner->tail = &node->scanner_next;
}

/*
 * Checks the id and returns 0, if the id is greater than true. Otherwise,
 * returns false.
 */
static inline int check_id(int id)
{
    return id > 0;
}

/*
 * Gets the node from the graph with the given id.
 */
static inline struct graph_node *get_node(const Graph *graph, int id)
{
    return (struct graph_node *)table_get(graph->node, id);
}

/*
 * Gets the next node and advances the table iterator.
 */
static inline struct graph_node *next_node(Graph *graph)
{
    struct table_entry entry = table_next_iter(graph->node);
    if (!table_entry_ok(&entry))
        return NULL;
    return (struct graph_node *)entry.value;
}

/*
 * Gets the colour of the node and returns it.
 */
static inline int get_colour(const struct graph_node *node)
{
    return node->metadata.colour;
}

/*
 * Sets the colour of the node.
 */
static inline void set_colour(struct graph_node *node, int colour)
{
    if (!node)
        return;
    node->metadata.colour = colour;
}

Graph *graph_create(void)
{
    Graph *graph = malloc(sizeof(Graph));
    if (!graph)
    {
        PRINT_SYSERR("malloc", ENOMEM);
        return NULL;
    }
    graph->size = 0;
    graph->node = table_create();
    if (!graph->node)
    {
        PRINT_ERROR("unable to create table");
        free(graph);
        return NULL;
    }
    scanner_init(&graph->scanner);
    return graph;
}

void graph_destroy(Graph *graph)
{
    if (!graph)
        return;
    table_init_iter(graph->node);
    struct graph_node *curr = next_node(graph);
    while (curr)
    {
        node_free(curr);
        curr = next_node(graph);
    }
    table_destroy(graph->node);
    free(graph);
}

static int graph_add_node(Graph *graph, int id, void *value)
{
    struct graph_node *node = malloc(sizeof(struct graph_node));
    if (!node)
    {
        PRINT_SYSERR("malloc", ENOMEM);
        return ENOMEM;
    }

    node->vtx.id = id;
    node->vtx.value = value;
    metadata_init(&node->metadata);
    array_init(&node->parent, 0);
    array_init(&node->children, 0);
    node->scanner_next = NULL;

    if (table_insert(graph->node, id, node) != 0)
    {
        PRINT_ERROR("unable to insert vertex");
        node_free(node);
        free(node);
        return -1;
    }

    graph->size++;
    return 0;
}

int graph_add_vtx(Graph *graph, int id, void *value)
{
    if (!graph_vtx_check(id, value))
    {
        PRINT_WARNING("invalid arguments");
        return -1;
    }

    struct graph_node *node = get_node(graph, id);
    if (node)
    {
        PRINT_INFO("vertex already exists (%d)", id);
        return -1;
    }

    return graph_add_node(graph, id, value);
}

int graph_add_edge(Graph *graph, int from, int to)
{
    if (!check_id(from) || !check_id(to) || from == to)
    {
        PRINT_DEBUG("invalid id (%d, %d)", from, to);
        return -1;
    }

    struct graph_node *from_node = get_node(graph, from);
    if (!from_node)
    {
        PRINT_DEBUG("unable to find (%d)", from);
        return -1;
    }

    struct graph_node *to_node = get_node(graph, to);
    if (!to_node)
    {
        PRINT_DEBUG("unable to find (%d)", to);
        return -1;
    }

    if (array_add(&from_node->children, to) != 0)
    {
        PRINT_ERROR("unable to add child (%d)", to);
        return -1;
    }

    if (array_add(&to_node->parent, from) != 0)
    {
        PRINT_ERROR("unable to add parent (%d)", from);
        array_pop(&from_node->children);
        return -1;
    }

    return 0;
}

static void isolate_cycle(Graph *graph)
{
    // Empty scanner queue
    while (scanner_dequeue(&graph->scanner))
        ;

    // Check for cycle
    if (graph->scanner.cycle_id == INFINITY)
        return;

    struct graph_node *curr = get_node(graph, graph->scanner.cycle_id);
    while (curr)
    {
        // Enqueue current node
        set_colour(curr, BLACK);
        scanner_enqueue(&graph->scanner, curr);

        // Find grey child
        struct graph_node *next = NULL;
        for (int i = 0; i < curr->children.size; i++)
        {
            struct graph_node *child = get_node(graph, curr->children.id[i]);
            if (get_colour(child) == GREY)
            {
                next = child;
                break;
            }
        }
        curr = next;
    }
}

static int topological_search(Graph *graph, struct graph_node *node)
{
    set_colour(node, GREY);

    // Search for white children
    for (int i = 0; i < node->children.size; i++)
    {
        struct graph_node *child = get_node(graph, node->children.id[i]);
        switch (get_colour(child))
        {
        case WHITE:
            if (topological_search(graph, child) == -1)
                return -1;
            break;
        case GREY:
            graph->scanner.cycle_id = child->vtx.id;
            return -1;
        }
    }

    set_colour(node, BLACK);
    scanner_enqueue(&graph->scanner, node);
    return 0;
}

static int topological_sort(Graph *graph)
{
    table_init_iter(graph->node);
    struct graph_node *curr = next_node(graph);
    while (curr)
    {
        if (get_colour(curr) == WHITE)
        {
            graph->scanner.cycle_id = INFINITY;

            if (topological_search(graph, curr) != 0)
            {
                isolate_cycle(graph);
                return -1;
            }
        }
        curr = next_node(graph);
    }
    return 0;
}

void graph_start_scan(Graph *graph)
{
    table_init_iter(graph->node);
}

struct graph_vtx graph_read_scan(Graph *graph)
{
    struct table_entry entry = table_next_iter(graph->node);
    if (!table_entry_ok(&entry))
        return GRAPH_VTX_EMPTY;
    return ((struct graph_node *)entry.value)->vtx;
}

int graph_start_topological_scan(Graph *graph)
{
    // Empty scanner queue
    while (scanner_dequeue(&graph->scanner))
        ;

    // Initialize vertices
    graph_start_scan(graph);
    struct graph_node *curr = next_node(graph);
    while (curr)
    {
        metadata_init(&curr->metadata);
        curr = next_node(graph);
    }

    // Sort graph in topological order
    return topological_sort(graph);
}

static inline int is_tainted(const struct graph_node *node)
{
    return node->metadata.is_tainted == 1;
}

struct graph_vtx graph_read_topological_scan(Graph *graph)
{
    struct graph_node *node = scanner_dequeue(&graph->scanner);
    while (node)
    {
        if (!is_tainted(node))
            return node->vtx;
        node = scanner_dequeue(&graph->scanner);
    }
    return GRAPH_VTX_EMPTY;
}

void graph_taint_vtx(Graph *graph, int id)
{
    struct graph_node *tainted_node = get_node(graph, id);
    if (!tainted_node || is_tainted(tainted_node))
        return;

    tainted_node->metadata.is_tainted = 1;
    for (int i = 0; i < tainted_node->parent.size; i++)
        graph_taint_vtx(graph, tainted_node->parent.id[i]);
}
