/**
 * @file unit_test.h
 * @brief Lightweight unit‑testing framework for C.
 *
 * This header defines the public API for creating tests or suites, running
 * them, and printing results.
 *
 * @note The framework is intentionally small – it does not depend on
 *       any external libraries, and its name and designed is modelled by
 * 		 `Python` `unittest`.
 */

#ifndef UNITTEST_H
#define UNITTEST_H

/*--------------------------------------------------------------------------*/
/*  Macros – printed “lines” used by the console UI                         */
/*--------------------------------------------------------------------------*/
/**
 * @def DLINE
 * @brief A double line, decorative separator that is printed between major
 *        UI sections.
 */
#define DLINE \
	"\n======================================================================"

/**
 * @def HLINE
 * @brief A single line, decorative separator that is printed between minor
 *        UI sections.
 */
#define HLINE \
	"\n----------------------------------------------------------------------"

/*=========================================================================*/
/*  Verbosity levels – configure the amount of output printed              */
/*=========================================================================*/
/**
 * @enum unittest_verbosity_t
 * @brief Verbosity levels for the console UI.
 *
 * The order of the enum values is important: `UNITTEST_DEFAULT` is the
 * smallest value (used when no explicit level is supplied) and
 * `UNITTEST_DEBUG` is the most verbose.
 */
typedef enum {
	UNITTEST_DEFAULT = 0, ///< Default (human‑readable) output
	UNITTEST_QUIET, ///< No run‑time output, only final summary
	UNITTEST_VERBOSE, ///< Show each test’s status as it executes
	UNITTEST_DEBUG ///< Show diagnostic information for each step
} unittest_verbosity_t;

/*=========================================================================*/
/*  Test run options – tweak timeouts, verbosity, etc.                     */
/*=========================================================================*/
/**
 * @struct unittest_opts_t
 * @brief Runtime options for a `Unittest` run.
 *
 * These options are passed *once* to `unittest_run()`.  The framework
 * will use the timeout value in milliseconds for the test or each test in
 * the suite, and use the verbosity level for the console output.
 *
 * @note A negative `timeout_ms` disables the timeout feature entirely.
 */
typedef struct {
	int timeout_ms; ///< Timeout in milliseconds, -1 = no timeout
	unittest_verbosity_t level; ///< Verbosity level for console output
} unittest_opts_t;

/*=========================================================================*/
/*  Abstract test type – defined in the unittest.c file                    */
/*=========================================================================*/
/**
 * @typedef Unittest
 * @brief Abstract test object – represents either a single test case
 *        or a collection (suite) of test cases.
 *
 * A `Unittest` is created via either `unittest_create_test()` or
 * `unittest_create_suite()`. You may then add tests to a suite with
 * `unittest_add_test()`, run it with `unittest_run()`, and finally
 * destroy it with `unittest_destroy()`.
 */
typedef struct Unittest Unittest;

/**
 * @typedef UnittestResult
 * @brief Encapsulates the outcome of a single test or suite execution.
 *
 * The result object stores the status (OK / FAIL / ERROR) as well as
 * any diagnostic messages produced by the test itself.  It also keeps
 * internal diagnostics for framework‑level failures.
 */
typedef struct UnittestResult UnittestResult;

/*=========================================================================*/
/*  Test case function type – signature for all test callbacks             */
/*=========================================================================*/
/**
 * @typedef unittest_fn
 * @brief Signature for individual test case callbacks.
 *
 * Each test function receives a pointer to a `UnittestResult` that it
 * should update via the `unittest_result_*()` helpers.  The test may
 * freely call any helper in the framework – failures in the framework
 * are recorded separately from test failures.
 */
typedef void (*unittest_fn)(UnittestResult *);

/*=========================================================================*/
/*  Helper API – defaults, diagnostics, and result helpers                 */
/*=========================================================================*/
/**
 * @brief Retrieve the framework’s default options.
 *
 * @return A `unittest_opts_t` structure with `timeout_ms == -1` (no
 *         timeout) and `level == UNITTEST_DEFAULT`.
 */
unittest_opts_t unittest_opts_default(void);

/**
 * @brief Destroys a `UnittestResult` and frees all associated resources.
 *
 * @param result Pointer to the result object to destroy.  It may be
 *        `NULL`, in which case the function is a no‑op.
 */
void unittest_result_destroy(UnittestResult *result);

/**
 * @brief Mark a test as passed.
 *
 * @param result Pointer to the result object to update.
 *
 * @pre `result` is a valid, non‑NULL pointer.
 */
void unittest_result_ok(UnittestResult *result);

/**
 * @brief Mark a test as failed and optionally record a diagnostic.
 *
 * @param result Pointer to the result object to update.
 * @param fmt    Optional printf‑style format string.  If `fmt` is
 *               non‑NULL, a diagnostic message is recorded.
 * @param ...    Additional arguments for the format string.
 *
 * @pre `result` is a valid, non‑NULL pointer.
 */
void unittest_result_fail(UnittestResult *result, const char *fmt, ...);

/**
 * @brief Mark a test as an error and optionally record a diagnostic.
 *
 * @param result Pointer to the result object to update.
 * @param fmt    Optional printf‑style format string.  If `fmt` is
 *               non‑NULL, a diagnostic message is recorded.
 * @param ...    Additional arguments for the format string.
 *
 * @pre `result` is a valid, non‑NULL pointer.
 */
void unittest_result_err(UnittestResult *result, const char *fmt, ...);

/*=========================================================================*/
/*  Test & Suite creation / destruction                                    */
/*=========================================================================*/
/**
 * @brief Create a single test case.
 *
 * The framework internally builds a small `Unittest` object that stores
 * the supplied name and function pointer.  The test case itself is
 * expected to call one of the `unittest_result_*()` helpers.
 *
 * @param name Name of the test case.  Must be a string literal or a
 *             static string; the framework will duplicate the string.
 * @param fn   Pointer to the test function.
 *
 * @return Pointer to a new `Unittest` on success, or `NULL` on failure.
 */
Unittest *unittest_create_test(const char *name, unittest_fn fn);

/**
 * @brief Create a test suite (a collection of tests).
 *
 * @param name Name of the suite.  Must be a string literal or a static
 *             string; the framework will duplicate the string.
 *
 * @return Pointer to a new `Unittest` representing the suite, or `NULL`
 *         on failure.
 */
Unittest *unittest_create_suite(const char *name);

/**
 * @brief Destroy a `Unittest` and free all associated resources.
 *
 * @param ut Pointer to the object to destroy.  The function is a no‑op
 *           if `ut` is `NULL`.
 */
void unittest_destroy(Unittest *ut);

/**
 * @brief Retrieve the name of a test or suite.
 *
 * @param ut Pointer to the object whose name is requested.
 *
 * @return The name string, or `NULL` if `ut` is `NULL`.
 */
const char *unittest_get_name(Unittest *ut);

/**
 * @brief Add a new test case to a suite.
 *
 * @param suite The suite to which the test should be added.
 * @param name  Name of the new test case.
 * @param fn    Pointer to the test function.
 *
 * @return `0` on success, `-1` on failure (e.g., memory allocation
 *         error or `suite` is actually a test case).
 */
int unittest_add_test(Unittest *suite, const char *name, unittest_fn fn);

/**
 * @brief Run a `Unittest` (test or suite) with the supplied options.
 *
 * The function allocates a fresh `UnittestResult` object that records
 * the overall status.  It returns a pointer to that object on success,
 * or `NULL` if a fatal framework error occurs.
 *
 * @param ut        Pointer to the test or suite to run.  Must be
 *                  non‑NULL.
 * @param run_opts  Pointer to a `unittest_opts_t` structure that
 *                  configures the run (timeout, verbosity).  If
 *                  `run_opts` is `NULL`, defaults are used.
 * @param status    Output pointer that receives `0` on successful
 *                  execution or `-1` if a framework‑level error
 *                  prevented the test from running.  The returned
 *                  `UnittestResult` is set accordingly.
 *
 * @return Pointer to a new `UnittestResult` on success, or `NULL` on
 *         fatal error.  `*status` is updated to match the outcome.
 *
 * @pre `ut` is not `NULL`.
 * @post The returned `UnittestResult` matches the type of `ut`
 *       (test ↔ test, suite ↔ suite).
 */
UnittestResult *unittest_run(const Unittest *ut,
			     const unittest_opts_t *run_opts, int *status);

/**
 * @brief Print a test or suite together with its result.
 *
 * @param ut      Pointer to the test or suite to print.
 * @param result  Pointer to the result object returned by
 *                `unittest_run()`.  Must not be `NULL`.
 * @param level   Verbosity level controlling how much information is
 *                displayed.  The same enum values used in
 *                `unittest_print_result()` are accepted.
 *
 * @pre `ut` and `result` are valid, non‑NULL pointers.
 * @pre `level` is a member of `unittest_verbosity_t`.
 *
 * @post The console output includes the decorative lines
 *       (`DLINE`/`HLINE`) and any diagnostics stored in the
 *       `UnittestResult`.  No data is modified by this function.
 */
void unittest_print_result(const Unittest *ut, const UnittestResult *result,
			   unittest_verbosity_t level);

#endif /* UNITTEST_H */

/*=========================================================================*/
/*  End of header                                                          */
/*=========================================================================*/
