#ifndef TEST_H
#define TEST_H

#include "unittest.h"

/*
 * The struct test_result records all the outputs from the running test.
 */
struct test_result
{
	int status;
	char *msg; /* user message */
	char *out; /* redirected stdout */
	char *err; /* redirected stderr */

	long long time_ms; /* total test time */

	int sys_errno; /* saved system error number */
	int exit_code; /* test exit code */
	int signo;	   /* saved signal number */
};

/*
 * An opaque object used to wrap the test case with its name.
 */
typedef struct Test Test;

/*
 * Returns the status code as a string.
 */
const char *into_str(int status);

/*
 * Initailizes the result attributes and sets its status to TEST_ERROR.
 */
int test_result_init(struct test_result *result);

/*
 * Frees all the result attributes and reinitializes it.
 */
void test_result_free(struct test_result *result);

/*
 * Creates a new Test instance and returns it. Otherwise, returns NULL.
 *
 * Preconditions:
 * - name must not be NULL
 * - fn must not be NULL
 */
Test *test_create(const char *name, unittest_fn fn);

/*
 * Destroys the Test and all of its resources. If NULL, test_destroy does
 * nothing.
 */
void test_destroy(Test *test);

/*
 * Returns the name of the Test.
 */
const char *test_get_name(const Test *test);

/*
 * Runs the Test and records its results to result. Returns a 0, if the
 * testing framework ran successfully. Otherwise, return -1, indicating it did
 * not run successfully.
 *
 * Note: The testing framework will attempt to set the exit code, errno and
 * signal number to the result.
 *
 * Preconditions:
 * - test must not be NULL
 * - result must not be NULL
 *
 * Postconditions:
 * - result must be freed
 */
int test_run(const Test *test, struct test_result *result,
			 const struct unittest_opts *run_opts);

/*
 * Prints the Test and its result at the given verbosity level.
 *
 * Preconditions:
 * - test must not be NULL
 * - result must not be NULL
 * - result must be the output from test_run() for the given test
 */
void test_print_result(const Test *test, const struct test_result *result, int level);

#endif
