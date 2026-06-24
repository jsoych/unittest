#include <stdio.h>

#include "registry.h"
#include "runner.h"

static int create(void)
{
    Runner *runner = runner_create();
    if (!runner)
        return TEST_FAILURE;
    runner_destroy(runner);
    return TEST_SUCCESS;
}
REGISTER_TEST("runner_create", create);

static int destroy(void)
{
    runner_destroy(NULL);
    Runner *runner = runner_create();
    runner_destroy(runner);
    return TEST_SUCCESS;
}
REGISTER_TEST("runner_destroy", destroy);

static int job(void *arg, void *opts)
{
    int *ip = (int *)arg;
    const char *colour = (char *)opts;
    printf("%s%d\033[0m\n", colour, *ip);
    return 0;
}

static int execute(void)
{
    const char *msg = "";
    Runner *runner = runner_create();

    // Create job schedule (1 -> 2 -> 3)
    Graph *schedule = graph_create();
    int value[3] = {0};
    for (int i = 1; i <= 3; i++)
    {
        value[i - 1] = i;
        graph_add_vtx(schedule, i, &value[i - 1]);
    }

    graph_add_edge(schedule, 3, 2);
    graph_add_edge(schedule, 2, 1);

    char *orange = "\033[38;5;208m";
    if (runner_execute(runner, schedule, job, orange) != 0)
    {
        msg = "failed to execute";
        goto failure;
    }

    // Add cycle
    graph_add_edge(schedule, 1, 3);
    if (runner_execute(runner, schedule, job, orange) == 0)
    {
        msg = "executed schedule with cycle";
        goto failure;
    }

    graph_destroy(schedule);
    runner_destroy(runner);
    return TEST_SUCCESS;

failure:
    printf("%s\n", msg);
    graph_destroy(schedule);
    runner_destroy(runner);
    return TEST_FAILURE;
}
REGISTER_TEST("runner_execute", execute);
