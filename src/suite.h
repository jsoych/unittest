#ifndef SUITE_H
#define SUITE_H

#include "test.h"
#include "unittest.h"

/*
 * The struct suite_result records and summarizes of the test results.
 */
struct suite_result
{
	int nok;	 /* number of ok test results */
	int nfail;	 /* number of failed test results */
	int nerror;	 /* number of test errors */
	int nsyserr; /* number of testing framework errors */

	long long time_ms; /* total suite time */

	int size;						  /* number of test results */
	struct test_result *test_results; /* test results */
};

/*
 * Suite is an opaque object that groups Test objects together.
 */
typedef struct Suite Suite;

/*
 * Initializes the result and allocates space for exactly size test results.
 */
int suite_result_init(struct suite_result *result, int size);

/*
 * Frees the result and all of its resources. If NULL, suite_result_free does
 * nothing.
 */
void suite_result_free(struct suite_result *result);

/*
 * Evaluates and returns the overall execution status of a suite result.
 *
 * The status is determined by the highest severity outcome present in the telemetry:
 * - Returns UNITTEST_SYSERR  if any low-level system errors occurred.
 * - Returns UNITTEST_ERROR   if any runtime exception/test errors occurred.
 * - Returns UNITTEST_FAIL    if any test assertion failures occurred.
 * - Returns UNITTEST_OK      if all test metrics passed cleanly.
 */
int suite_result_status(const struct suite_result *result);

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
const char *suite_get_name(const Suite *suite);

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
int suite_run(const Suite *suite, struct suite_result *result,
			  const struct unittest_opts *opts);

/*
 * Prints the suite and its result.
 *
 * Preconditions:
 * - suite must not be NULL
 * - result must not be NULL
 * - result must be the output from suite_run() for the give suite
 */
void suite_print_result(const Suite *suite, const struct suite_result *result,
						int level);

#endif
