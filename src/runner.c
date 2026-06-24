#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "print_macros.h"
#include "runner.h"

struct Runner
{
    int thread_count;
};

Runner *runner_create(void)
{
    Runner *runner = malloc(sizeof(Runner));
    if (!runner)
    {
        PRINT_SYSERR("malloc", ENOMEM);
        return NULL;
    }
    runner->thread_count = 1;
    return runner;
}

int runner_get_status(Runner *runner)
{
    return runner->thread_count;
}

void runner_destroy(Runner *runner)
{
    if (!runner)
        return;
    free(runner);
}

int runner_execute(Runner *runner, Graph *schedule,
                   int (*job)(void *, void *), void *opts)
{
    if (!schedule || !job)
        return -1;

    if (graph_start_topological_scan(schedule) != 0)
        return -1;

    struct graph_vtx vtx = graph_read_topological_scan(schedule);
    int status = 0;
    while (graph_vtx_ok(&vtx))
    {
        if (job(vtx.value, opts) != 0)
        {
            graph_taint_vtx(schedule, vtx.id);
            status = -1;
        }
        vtx = graph_read_topological_scan(schedule);
    }

    return status;
}
