/**
 * @file test.h
 * @brief Public interface for the individual test wrapper.
 *
 * This header declares a thin wrapper around the `unittest` framework
 * that provides a richer `test_result_t` structure containing the test
 * name, status, output streams, timings, and error information.
 */

#ifndef TEST_H
#define TEST_H

#include "unittest.h"

/**
 * @brief Test status codes.
 *
 * The enum represents the high‑level outcome of a test.  The
 * corresponding string can be obtained via @ref test_status_to_str().
 */
typedef enum {
	TEST_OK = 0, /**< Test executed successfully. */
	TEST_FAIL, /**< Test ran but failed. */
	TEST_ERROR, /**< An error occurred during test execution. */
	TEST_SYSERR /**< A system error (e.g., errno) was set. */
} test_status_t;

/**
 * @brief Container for all output from a running test.
 *
 * The @c test_result_t structure holds the status, user message,
 * redirected output streams, timing information, and any system
 * error or signal information produced during the test.
 */
typedef struct {
	test_status_t status; /**< Final status of the test. */
	char *msg; /**< User supplied message. */
	char *out; /**< Captured stdout. */
	char *err; /**< Captured stderr. */
	long long time_ms; /**< Total execution time in milliseconds. */
	int sys_errno; /**< Saved system errno value. */
	int exit_code; /**< Exit code returned by the test process. */
	int signo; /**< Signal number that terminated the test. */
} test_result_t;

/**
 * @brief Opaque wrapper around a test case.
 *
 * The @c Test structure is opaque to callers; it is used to store the
 * test function and its name.
 */
typedef struct Test Test;

/**
 * @brief Convert a @c test_status_t value to a human‑readable string.
 *
 * @param status The status value to convert.
 * @return A null‑terminated string describing @p status.
 */
const char *test_status_to_str(test_status_t status);

/**
 * @brief Initialize a @c test_result_t structure.
 *
 * Allocates and clears all fields in @p result and sets its @c status to
 * @c TEST_ERROR.  This function is typically called before executing a test
 * to ensure the structure is in a known state.
 *
 * @param result Pointer to the result structure to initialize.
 * @return 0 on success, non‑zero on allocation failure.
 */
int test_result_init(test_result_t *result);

/**
 * @brief Free all dynamically allocated fields of @p result.
 *
 * This function does not free the @c test_result_t pointer itself;
 * it simply releases any memory that has been allocated for its members
 * and resets the structure to an empty state.
 *
 * @param result Pointer to the result structure to free.
 */
void test_result_free(test_result_t *result);

/**
 * @brief Create a new @c Test instance.
 *
 * @param name Human‑readable name for the test. Must not be @c NULL.
 * @param fn   Test function to execute. Must not be @c NULL.
 * @return Pointer to the newly allocated @c Test object, or @c NULL on
 * allocation failure.
 *
 * @pre @p name and @p fn must not be @c NULL.
 */
Test *test_create(const char *name, unittest_fn fn);

/**
 * @brief Destroy a @c Test object and free all associated resources.
 *
 * If @p test is @c NULL, this function performs no action.
 *
 * @param test Pointer to the @c Test object to destroy.
 */
void test_destroy(Test *test);

/**
 * @brief Retrieve the name of a @c Test.
 *
 * @param test Pointer to the @c Test object.
 * @return The name string associated with @p test, or @c NULL if @p test is
 * @c NULL.
 */
const char *test_get_name(const Test *test);

/**
 * @brief Execute a test and capture its results.
 *
 * The @p result structure is filled with information about the test
 * execution, including status, output streams, timing, exit code,
 * errno, and signal information.  The function returns @c 0 if the
 * test framework ran successfully, otherwise @c -1.
 *
 * @param test     Pointer to the @c Test object to run. Must not be @c NULL.
 * @param result   Pointer to a pre‑allocated @c test_result_t structure to
 *                 be populated with execution data. Must not be @c NULL.
 * @param run_opts Optional runtime options passed to the underlying
 *                 @c unittest framework. May be @c NULL.
 *
 * @pre @p test and @p result must not be @c NULL.
 *
 * @post The @p result structure must be freed by the caller using
 * @ref test_result_free().
 *
 * @return 0 on success, -1 on failure to execute the test.
 */
int test_run(const Test *test, test_result_t *result,
	     const unittest_opts_t *run_opts);

/**
 * @brief Print a test and its result with the specified verbosity.
 *
 * This helper prints the test name, status, captured output, error
 * messages, and timing information to the console.  The output is
 * formatted according to the @c unittest_verbosity_t level.
 *
 * @param test   Pointer to the @c Test object to print. Must not be @c NULL.
 * @param result Pointer to the @c test_result_t structure returned by
 *               @ref test_run(). Must not be @c NULL.
 * @param level  Verbosity level controlling the amount of detail printed.
 */
void test_print_result(const Test *test, const test_result_t *result,
		       const unittest_verbosity_t level);

#endif /* TEST_H */
