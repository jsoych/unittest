/**
 * @file suite.h
 * @brief Public interface for a collection of tests (a “suite”).
 *
 * This header defines an opaque `Suite` type that aggregates one or more
 * `Test` objects.  It also provides a lightweight result container,
 * `suite_result_t`, that aggregates the per‑test results and provides
 * summary statistics such as the number of passed/failed tests and the
 * total execution time.
 */

#ifndef SUITE_H
#define SUITE_H

#include "test.h"
#include "unittest.h"

/**
 * @brief Aggregates the results of all tests in a suite.
 *
 * The structure holds summary counts, a per‑test array, and the total
 * execution time.  It is typically allocated with exactly the number of
 * tests that will be run, and its members are populated by `suite_run()`.
 */
typedef struct {
	/** @var Number of test results that passed. */
	int nok; /* number of ok test results */
	/** @var Number of test results that failed. */
	int nfail; /* number of failed test results */
	/** @var Number of tests that produced an error. */
	int nerror; /* number of test errors */
	/** @var Number of errors reported by the testing framework itself. */
	int nsyserr; /* number of testing framework errors */
	/** @var Total suite execution time in milliseconds. */
	long long time_ms; /* total suite time */
	/** @var Total number of test results stored. */
	int size; /* number of test results */
	/** @var Array of per‑test results. */
	test_result_t *test_results; /* test results */
} suite_result_t;

/**
 * @brief Opaque object that groups one or more `Test` objects.
 *
 * The actual fields of `Suite` are hidden from the user; only the pointer
 * type is exposed.  All manipulation of a `Suite` instance occurs via the
 * functions declared below.
 */
typedef struct Suite Suite;

/**
 * @brief Initialize a `suite_result_t` structure.
 *
 * Allocates storage for exactly `size` test results and clears all
 * fields.  The caller is responsible for freeing the structure with
 * `suite_result_free()` after use.
 *
 * @param result Pointer to the result structure to initialise.
 * @param size   Number of test results that will be stored.
 * @return 0 on success, non‑zero on allocation failure.
 *
 * @pre `result` must not be @c NULL.
 */
int suite_result_init(suite_result_t *result, int size);

/**
 * @brief Free all resources owned by a `suite_result_t`.
 *
 * This function deallocates the array returned by
 * `suite_result_init()` and resets the structure to an empty state.
 * It performs no action if @p result is @c NULL.
 *
 * @param result Pointer to the result structure to free.
 */
void suite_result_free(suite_result_t *result);

/**
 * @brief Create a new suite with the given name.
 *
 * @param name Human‑readable name for the suite.  Must not be @c NULL.
 * @return Pointer to the newly allocated `Suite` object, or @c NULL on
 * allocation failure.
 *
 * @pre `name` must not be @c NULL.
 */
Suite *suite_create(const char *name);

/**
 * @brief Destroy a suite and release all associated resources.
 *
 * If @p suite is @c NULL, this function performs no action.
 *
 * @param suite Pointer to the `Suite` object to destroy.
 */
void suite_destroy(Suite *suite);

/**
 * @brief Retrieve the name of a suite.
 *
 * @param suite Pointer to the `Suite` object.
 * @return The name string associated with `suite`, or @c NULL if
 * `suite` is @c NULL.
 */
const char *suite_get_name(Suite *suite);

/**
 * @brief Add a test to a suite.
 *
 * The test is appended to the internal list of tests that will be run by
 * `suite_run()`.  The same test object must not already belong to the
 * suite.
 *
 * @param suite Pointer to the `Suite` to modify. Must not be @c NULL.
 * @param test  Pointer to the `Test` object to add. Must not be @c NULL.
 * @return 0 on success, -1 if the test could not be added (e.g. memory
 * allocation failure or test already present).
 *
 * @pre `test` must not have been added to `suite` before.
 */
int suite_add(Suite *suite, Test *test);

/**
 * @brief Execute all tests in a suite and collect the results.
 *
 * `suite_run()` iterates over each test in the suite, executes it via
 * `test_run()`, and stores the per‑test results in the supplied
 * `suite_result_t`.  The function returns @c 0 if the framework executed
 * the entire suite successfully, otherwise @c -1.
 *
 * @param suite    Pointer to the `Suite` to run. Must not be @c NULL.
 * @param result   Pointer to a pre‑allocated `suite_result_t` to be filled
 *                 with aggregate data. Must not be @c NULL.
 * @param run_opts Optional runtime options passed to the underlying
 *                 unittest framework.  May be @c NULL.
 *
 * @pre `suite` and `result` must not be @c NULL.
 *
 * @post The caller must release `result` with `suite_result_free()`.
 *
 * @return 0 on success, -1 on failure to run the suite.
 */
int suite_run(const Suite *suite, suite_result_t *result,
	      const unittest_opts_t *run_opts);

/**
 * @brief Print a suite and its aggregated results.
 *
 * The output is formatted according to the requested verbosity level and
 * includes a summary of passed/failed/error counts, total time, and
 * optionally the per‑test output if the verbosity level is high enough.
 *
 * @param suite  Pointer to the `Suite` object. Must not be @c NULL.
 * @param result Pointer to the `suite_result_t` produced by `suite_run()`.
 *               Must not be @c NULL.
 * @param level  Verbosity level controlling how much detail is printed.
 */
void suite_print_result(const Suite *suite, const suite_result_t *result,
			const unittest_verbosity_t level);

#endif /* SUITE_H */
