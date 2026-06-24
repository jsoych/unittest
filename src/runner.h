#ifndef RUNNER_H
#define RUNNER_H

#include "unittest.h"

#include "graph.h"
#include "table.h"

typedef struct Runner Runner;

/*
 * Creates a new runner with the given schedule and returns it.
 */
Runner *runner_create(void);

/*
 * Destroys the runner.
 */
void runner_destroy(Runner *runner);

/*
 * Executes the job.
 */
int runner_execute(Runner *runner, Graph *schedule,
                   int (*job)(void *, void *), void *opts);

#endif
