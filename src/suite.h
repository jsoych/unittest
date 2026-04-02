#ifndef SUITE_H
#define SUITE_H

#include "test.h"
#include "unittest.h"

/*
 * The suite_result_t records and summarizes of the test results.
 */
typedef struct {
	int nok; /* number of ok test results */
	int nfail; /* number of failed test results */
	int nerror; /* number of test errors */
	int nsyserr; /* number of testing framework errors */

	long long time_ms; /* total suite time */

	int size; /* number of test results */
	test_result_t *test_results; /* test results */
} suite_result_t;

/*
 * Suite is an opaque object that groups Test objects together.
 */
typedef struct Suite Suite;

/*
 * Initializes the result and allocates space for exactly size test results.
 */
int suite_result_init(suite_result_t *result, int size);

/*
 * Frees the result and all of its resources. If NULL, suite_result_free does
 * nothing.
 */
void suite_result_free(suite_result_t *result);

/*
 * Creates a new Suite instance and returns it. Otherwise, returns NULL.
 *
 * Preconditions:
 * - name must not be NULL
 */
Suite *suite_create(const char *name);

/*
 * Destroys the suite and all of its resources. If NULL, suite_destroy does
 * nothing.
 */
void suite_destroy(Suite *suite);

/*
 * Returns the name of the Suite.
 */
const char *suite_get_name(Suite *suite);

/*
 * Adds the test to the suite.
 *
 * Preconditions:
 * - test must not be added to the suite already
 */
int suite_add(Suite *suite, Test *test);

/*
 * Runs the suite and records the results to result. Returns 0, if the suite
 * and all of its tests ran successfully. Othewise, returns -1.
 *
 * Preconditions:
 * - suite must not be NULL
 * - result must not be NULL
 * 
 * Postconditions:
 * - result must be freed
 */
int suite_run(const Suite *suite, suite_result_t *result,
	      const unittest_opts_t *run_opts);

/*
 * Prints the suite and its result.
 * 
 * Preconditions:
 * - suite must not be NULL
 * - result must not be NULL
 * - result must be the output from suite_run() for the give suite
 */
void suite_print_result(const Suite *suite, const suite_result_t *result,
			const unittest_verbosity_t level);

#endif
