#ifndef UNITTEST_H
#define UNITTEST_H

#define DLINE \
	"\n\n======================================================================"
#define HLINE \
	"\n----------------------------------------------------------------------"

/* Verbosity levels configure the printed run and result outputs. */
enum
{
	UNITTEST_DEFAULT = 0,
	UNITTEST_QUIET,
	UNITTEST_VERBOSE,
	UNITTEST_DEBUG
};

enum
{
	UNITTEST_OK = 0,
	UNITTEST_FAIL,
	UNITTEST_ERROR,
	UNITTEST_SYSERR
};

/* Options changes the behaviour of the running test. */
struct unittest_opts
{
	int timeout_ms; /* Set to -1 for no timeout */
	int level;
};

/**
 * Unittest is the abstract object of testing. It organizes the test into a
 * single test case or a collection of test cases to be checked independently.
 * The order of intended use is: create, add test case(s), run and record
 * result, destroy.
 */
typedef struct Unittest Unittest;

/**
 * UnittestResult encapsulates the Unittest output and results into an object.
 * It also records failures within the testing framework itself so that test
 * errors are disguished from framework failures.
 */
typedef struct UnittestResult UnittestResult;

/*
 * unittest_fn is the signature of each test case.
 */
typedef void (*unittest_fn)(UnittestResult *);

/*
 * Sets up the testing framework with its default values.
 */
int unittest_setup(void);

/*
 * Tears down the testing framework.
 */
void unittest_tear_down(void);

/*
 * Runs all the tests.
 */
int unittest_run_all(struct unittest_opts *opts);

/*
 * Prints all the results.
 */
void unittest_print_all(int level);

/*
 * Returns the default testing options -1 (no timeout) and UNITTEST_DEFAULT.
 */
struct unittest_opts unittest_opts_default(void);

/*
 * Destroys the UnittestResult and all of its resources.
 */
void unittest_result_destroy(UnittestResult *result);

/*
 * Marks the UnittestResult as ok and sets its status to TEST_OK.
 *
 * Preconditions:
 * - UnittestResult is a test result
 */
void unittest_result_ok(UnittestResult *result);

/*
 * Marks the UnittestResult as fail and sets its status to TEST_FAIL. If
 * fmt must not be NULL, then a formatted message is recorded with the test
 * result.
 *
 * Preconditions:
 * - UnittestResult is a test result
 */
void unittest_result_fail(UnittestResult *result, const char *fmt, ...);

/*
 * Marks the UnittestResult as error and sets its status to TEST_ERROR. If
 * fmt must not be NULL, then a formatted message is recorded with the test
 * result.
 *
 * Precoditions:
 * - UnittestResult is a test result
 */
void unittest_result_error(UnittestResult *result, const char *fmt, ...);

/*
 * Creates a new test with the given name and unittest function. Returns
 * a Unittest on success, and NULL on failure.
 */
Unittest *unittest_create_test(const char *name, unittest_fn fn);

/*
 * Creates a new suite with the given name. Returns a Unittest on success,
 * and NULL on failure.
 */
Unittest *unittest_create_suite(const char *name);

/*
 * Destroys the Unittest and all of its resources.
 */
void unittest_destroy(Unittest *ut);

/*
 * Returns the name of the Unittest.
 */
const char *unittest_get_name(const Unittest *ut);

/*
 * Creates a new test and adds it to the suite. Returns 0 on success, and -1
 * on failure.
 */
int unittest_add_test(Unittest *suite, const char *name, unittest_fn fn);

/*
 * Adds a dependency between tests.
 */
int unittest_add_dependency(Unittest *from, Unittest *to);

/*
 * Runs the Unittest with the given run options. Returns a UnittestResult and
 * sets the status to 0 on success or -1 on failure. Otherwise, returns NULL
 * sets the status to -1.
 *
 * Preconditions:
 * - ut must not be NULL
 */
UnittestResult *unittest_run(Unittest *ut, const struct unittest_opts *opts);

/*
 * Prints the Unittest and its UnittestResult at the given verbosity level.
 *
 * Preconditions:
 * - ut must not be NULL
 * - result must not be NULL
 * - ut and result types match (both tests or both suites)
 */
void unittest_print_result(const Unittest *ut,
						   const UnittestResult *result, int level);

#endif
