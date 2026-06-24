#include <stdio.h>

#include "graph.h"
#include "registry.h"

#define GRAPH_SIZE (1024)
#define MSG_LEN (256)

static int vtx_check(void)
{
    if (graph_vtx_check(0, NULL))
        return TEST_FAILURE;

    if (graph_vtx_check(-13, NULL))
        return TEST_FAILURE;

    int value = 42;
    if (graph_vtx_check(0, &value))
        return TEST_FAILURE;

    if (!graph_vtx_check(33, &value))
        return TEST_FAILURE;

    if (graph_vtx_check(33, NULL))
        return TEST_FAILURE;

    return TEST_SUCCESS;
}
REGISTER_TEST("graph_vtx_check", vtx_check);

static int vtx_ok(void)
{
    if (graph_vtx_ok(NULL))
        return TEST_FAILURE;

    struct graph_vtx vtx = GRAPH_VTX_EMPTY;
    if (graph_vtx_ok(&vtx))
        return TEST_FAILURE;

    vtx.id = -13;
    if (graph_vtx_ok(&vtx))
        return TEST_FAILURE;

    int value = 42;
    vtx.value = &value;
    if (graph_vtx_ok(&vtx))
        return TEST_FAILURE;

    vtx.id = 33;
    if (!graph_vtx_ok(&vtx))
        return TEST_FAILURE;

    vtx.value = NULL;
    if (graph_vtx_ok(&vtx))
        return TEST_FAILURE;

    return TEST_SUCCESS;
}
REGISTER_TEST("graph_vtx_ok", vtx_ok);

static int test_create(void)
{
    Graph *graph = graph_create();
    if (!graph)
        return TEST_FAILURE;
    graph_destroy(graph);
    return TEST_SUCCESS;
}
REGISTER_TEST("graph_create", test_create);

static int test_destroy(void)
{
    graph_destroy(NULL);

    Graph *graph = graph_create();
    int value = 42;
    for (int i = 1; i <= GRAPH_SIZE; i++)
        if (graph_add_vtx(graph, i, &value) != 0)
        {
            graph_destroy(graph);
            return TEST_ERROR;
        }

    graph_destroy(graph);
    return TEST_SUCCESS;
}
REGISTER_TEST("graph_destroy", test_destroy);

static int test_add_vtx(void)
{
    Graph *graph = graph_create();
    int value = 42;

    // Add vertices
    for (int i = 1; i <= GRAPH_SIZE; i++)
        if (graph_add_vtx(graph, i, &value) != 0)
        {
            graph_destroy(graph);
            return TEST_FAILURE;
        }

    // Check duplicates
    for (int i = 1; i <= GRAPH_SIZE; i++)
        if (graph_add_vtx(graph, i, &value) == 0)
        {
            graph_destroy(graph);
            return TEST_FAILURE;
        }

    graph_destroy(graph);
    return TEST_SUCCESS;
}
REGISTER_TEST("graph_add_vtx", test_add_vtx);

static int test_add_edge(void)
{
    Graph *graph = graph_create();
    int value = 33;
    for (int i = 1; i <= GRAPH_SIZE; i++)
        if (graph_add_vtx(graph, i, &value) != 0)
        {
            graph_destroy(graph);
            return TEST_ERROR;
        }

    for (int i = 1; i < GRAPH_SIZE; i++)
        if (graph_add_edge(graph, i, i + 1) != 0)
        {
            graph_destroy(graph);
            return TEST_FAILURE;
        }

    for (int i = 1; i < GRAPH_SIZE - 1; i += 2)
        if (graph_add_edge(graph, i, i + 2) != 0)
        {
            graph_destroy(graph);
            return TEST_FAILURE;
        }

    graph_destroy(graph);
    return TEST_SUCCESS;
}
REGISTER_TEST("graph_add_edge", test_add_edge);

static int test_start_scan(void)
{
    Graph *graph = graph_create();
    graph_start_scan(graph);

    int value = 33;
    for (int i = 0; i < GRAPH_SIZE; i++)
        graph_add_vtx(graph, i, &value);

    graph_start_scan(graph);
    return TEST_SUCCESS;
}
REGISTER_TEST("graph_start_scan", test_start_scan);

static int test_read_scan(void)
{
    Graph *graph = graph_create();
    graph_start_scan(graph);

    // Check empty graph
    struct graph_vtx vtx = graph_read_scan(graph);
    if (graph_vtx_ok(&vtx))
    {
        graph_destroy(graph);
        return TEST_FAILURE;
    }

    // Add square valued vertices to the graph
    int square[GRAPH_SIZE] = {0};
    for (int i = 1; i <= GRAPH_SIZE; i++)
    {
        square[i - 1] = i * i;
        graph_add_vtx(graph, i, &square[i - 1]);
    }

    // Start scan
    graph_start_scan(graph);

    // Check first vertex
    vtx = graph_read_scan(graph);
    if (!graph_vtx_ok(&vtx))
    {
        graph_destroy(graph);
        return TEST_FAILURE;
    }

    int i = 0;
    while (graph_vtx_ok(&vtx))
    {
        if (*((int *)vtx.value) != square[i++])
        {
            graph_destroy(graph);
            return TEST_FAILURE;
        }
        vtx = graph_read_scan(graph);
    }

    graph_destroy(graph);
    return TEST_SUCCESS;
}
REGISTER_TEST("graph_read_scan", test_read_scan);

static int start_topologial_scan(void)
{
    Graph *graph = graph_create();

    // Empty graph
    if (graph_start_topological_scan(graph) != 0)
    {
        graph_destroy(graph);
        return TEST_FAILURE;
    }

    // Graph without edges
    int value = 42;
    for (int i = 1; i <= GRAPH_SIZE; i++)
        graph_add_vtx(graph, i, &value);

    if (graph_start_topological_scan(graph) != 0)
    {
        graph_destroy(graph);
        return TEST_FAILURE;
    }

    // 1 -> 2 -> 3 -> ... -> GRAPH_SIZE
    for (int i = 1; i < GRAPH_SIZE; i++)
        graph_add_edge(graph, i, i + 1);

    if (graph_start_topological_scan(graph) != 0)
    {
        graph_destroy(graph);
        return TEST_FAILURE;
    }

    // Add cycle GRAPH_SIZE -> 1
    graph_add_edge(graph, GRAPH_SIZE, 1);

    if (graph_start_topological_scan(graph) == 0)
    {
        graph_destroy(graph);
        return TEST_FAILURE;
    }

    graph_destroy(graph);
    return TEST_SUCCESS;
}
REGISTER_TEST("graph_start_topological_scan", start_topologial_scan);

static int read_topological_scan(void)
{
    char msg[MSG_LEN] = {0};

    // Create graph with no edges
    Graph *graph = graph_create();
    int value = 42;
    for (int i = 1; i <= GRAPH_SIZE; i++)
        graph_add_vtx(graph, i, &value);

    // Check first vertex
    graph_start_topological_scan(graph);

    struct graph_vtx vtx = graph_read_topological_scan(graph);
    if (!graph_vtx_ok(&vtx))
    {
        snprintf(msg, MSG_LEN, "%s", "failed to read topological scan");
        goto failure;
    }

    // Count scanned vertices
    int count = 1;
    while (graph_vtx_ok(&vtx))
    {
        if (count > GRAPH_SIZE)
        {
            snprintf(msg, MSG_LEN, "%s", "count out of range");
            goto failure;
        }

        if (vtx.id != count++)
        {
            snprintf(msg, MSG_LEN, "%s (%d, %d)", "unexpected id", vtx.id, count);
            goto failure;
        }
        vtx = graph_read_topological_scan(graph);
    }

    // 1 -> ... -> GRAPH_SIZE
    for (int i = 1; i < GRAPH_SIZE; i++)
        graph_add_edge(graph, i, i + 1);

    if (graph_start_topological_scan(graph) != 0)
    {
        graph_destroy(graph);
        return TEST_FAILURE;
    }

    vtx = graph_read_topological_scan(graph);
    if (!graph_vtx_ok(&vtx))
    {
        snprintf(msg, MSG_LEN, "%s", "failed to start scan with edges");
        goto failure;
    }

    count = GRAPH_SIZE;
    while (graph_vtx_ok(&vtx))
    {
        if (count < 0)
        {
            snprintf(msg, MSG_LEN, "%s", "index out of bouds");
            goto failure;
        }

        if (vtx.id != count--)
        {
            snprintf(msg, MSG_LEN, "%s (%d, %d)", "unexpected id", vtx.id, count);
            goto failure;
        }

        vtx = graph_read_topological_scan(graph);
    }

    // Add cycle GRAPH_SIZE -> 1
    graph_add_edge(graph, GRAPH_SIZE, 1);

    // Check for cycle
    if (graph_start_topological_scan(graph) == 0)
    {
        snprintf(msg, MSG_LEN, "%s", "failed to detect cycle");
        goto failure;
    }

    // Scan cycle
    vtx = graph_read_topological_scan(graph);
    if (!graph_vtx_ok(&vtx))
    {
        snprintf(msg, MSG_LEN, "%s", "failed to start scan with cycle");
        goto failure;
    }

    count = 1;
    while (graph_vtx_ok(&vtx))
    {
        if (count > GRAPH_SIZE)
        {
            snprintf(msg, MSG_LEN, "%s", "index out of bounds");
            goto failure;
        }

        if (vtx.id != count++)
        {
            snprintf(msg, MSG_LEN, "%s", "unexpected index");
            goto failure;
        }

        vtx = graph_read_topological_scan(graph);
    }

    if (count < GRAPH_SIZE)
    {
        snprintf(msg, MSG_LEN, "%s", "failed to scan cycle");
        goto failure;
    }

    graph_destroy(graph);
    return TEST_SUCCESS;

failure:
    printf("%s\n", msg);
    graph_destroy(graph);
    return TEST_FAILURE;
}
REGISTER_TEST("graph_read_topological_scan", read_topological_scan);
