#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "print_macros.h"
#include "runner.h"
#include "suite.h"
#include "table.h"
#include "test.h"
#include "unittest.h"

/* -------------------------------------------------------------------------- */
/* Types & Enums                                                              */
/* -------------------------------------------------------------------------- */

/* Unittest kinds */
enum
{
	UNITTEST_TEST = 0,
	UNITTEST_SUITE
};

/* -------------------------------------------------------------------------- */
/* Data Structures                                                            */
/* -------------------------------------------------------------------------- */

struct unittest_metadata
{
	int id;
	int status;
};

struct Unittest
{
	union
	{
		Test *test;
		Suite *suite;
	} as;
	int kind;

	struct unittest_metadata metadata;
};

struct UnittestResult
{
	union
	{
		struct test_result test;
		struct suite_result suite;
	} as;
	int kind;
};

/* -------------------------------------------------------------------------- */
/* Globals                                                                    */
/* -------------------------------------------------------------------------- */

static int counter = 0;
static Runner *runner;
static Table *results;
static Graph *schedule;

/* -------------------------------------------------------------------------- */
/* Internal Helpers                                                           */
/* -------------------------------------------------------------------------- */

/* Initializes the metadata. */
static void metadata_init(struct unittest_metadata *data)
{
	data->id = -1;
	data->status = -1;
}

/* Checks the kind. */
static int check_kind(int kind)
{
	return (kind == UNITTEST_TEST || kind == UNITTEST_SUITE);
}

/* Returns the global id counter, then increments by 1. */
static int increment_counter(void)
{
	return counter++;
}

/* Deletes all the graph vertices. */
static void delete_tests(Graph *graph)
{
	graph_start_scan(graph);
	struct graph_vtx curr = graph_read_scan(graph);
	while (curr.value)
	{
		unittest_destroy(curr.value);
		curr = graph_read_scan(graph);
	}
}

/* Deletes all the table result entries. */
static void delete_results(Table *table)
{
	table_init_iter(table);
	struct table_entry curr = table_next_iter(table);
	while (curr.value)
	{
		unittest_result_destroy(curr.value);
		curr = table_next_iter(table);
	}
}

/* Creates a Unittest. */
static Unittest *unittest_create(int kind, void *arg)
{
	if (!check_kind(kind))
	{
		PRINT_ERROR("unexpected kind (%d)", kind);
		return NULL;
	}

	Unittest *ut = malloc(sizeof(Unittest));
	if (!ut)
	{
		PRINT_SYSERR("malloc", ENOMEM);
		return NULL;
	}

	switch (kind)
	{
	case UNITTEST_TEST:
		ut->as.test = (Test *)arg;
		break;
	case UNITTEST_SUITE:
		ut->as.suite = (Suite *)arg;
		break;
	}
	ut->kind = kind;
	metadata_init(&ut->metadata);
	return ut;
}

/* Creates a UnittestResult. */
static UnittestResult *unittest_result_create(int kind)
{
	if (!check_kind(kind))
	{
		PRINT_DEBUG("unexpected kind (%d)\n", kind);
		return NULL;
	}

	UnittestResult *result = malloc(sizeof(UnittestResult));
	if (!result)
	{
		PRINT_DEBUG("failed to allocate space\n");
		return NULL;
	}

	result->kind = kind;
	return result;
}

/* -------------------------------------------------------------------------- */
/* API Functions and Methods                                                  */
/* -------------------------------------------------------------------------- */

int unittest_setup(void)
{
	unittest_tear_down();

	if (!(runner = runner_create()))
	{
		PRINT_ERROR("failed to create runner");
		goto tear_down;
	}

	if (!(results = table_create()))
	{
		PRINT_ERROR("failed to create results table");
		goto tear_down;
	}

	if (!(schedule = graph_create()))
	{
		PRINT_ERROR("failed to create graph");
		goto tear_down;
	}

	return 0;

tear_down:
	unittest_tear_down();
	return -1;
}

void unittest_tear_down(void)
{
	if (runner)
	{
		runner_destroy(runner);
		runner = NULL;
	}

	if (results)
	{
		delete_results(results);
		table_destroy(results);
		results = NULL;
	}

	if (schedule)
	{
		delete_tests(schedule);
		graph_destroy(schedule);
		schedule = NULL;
	}

	counter = 1;
}

struct unittest_opts unittest_opts_default(void)
{
	return (struct unittest_opts){
		.timeout_ms = -1,
		.level = UNITTEST_DEFAULT,
	};
}

void unittest_result_destroy(UnittestResult *result)
{
	if (!result)
		return;
	switch (result->kind)
	{
	case UNITTEST_TEST:
		test_result_free(&result->as.test);
		break;
	case UNITTEST_SUITE:
		suite_result_free(&result->as.suite);
		break;
	}
	free(result);
}

void unittest_result_ok(UnittestResult *result)
{
	if (!result)
		return;
	result->as.test.status = UNITTEST_OK;
}

static char *vformat(const char *fmt, va_list ap)
{
	if (!fmt)
		return NULL;
	va_list ap_copy;
	va_copy(ap_copy, ap);
	int len = vsnprintf(NULL, 0, fmt, ap_copy);
	va_end(ap_copy);
	if (len < 0)
		return NULL;
	char *buf = malloc((len + 1) * sizeof(char));
	if (!buf)
		return NULL;
	vsnprintf(buf, len + 1, fmt, ap);
	return buf;
}

void unittest_result_fail(UnittestResult *result, const char *fmt, ...)
{
	if (!result)
		return;
	result->as.test.status = UNITTEST_FAIL;
	if (result->as.test.msg)
	{
		free(result->as.test.msg);
		result->as.test.msg = NULL;
	}
	if (!fmt)
		return;
	va_list ap;
	va_start(ap, fmt);
	result->as.test.msg = vformat(fmt, ap);
	va_end(ap);
}

void unittest_result_error(UnittestResult *result, const char *fmt, ...)
{
	if (!result)
		return;
	result->as.test.status = UNITTEST_ERROR;
	if (result->as.test.msg)
	{
		free(result->as.test.msg);
		result->as.test.msg = NULL;
	}
	if (!fmt)
		return;
	va_list ap;
	va_start(ap, fmt);
	result->as.test.msg = vformat(fmt, ap);
	va_end(ap);
}

Unittest *unittest_create_test(const char *name, unittest_fn fn)
{
	Test *test = test_create(name, fn);
	if (!test)
	{
		PRINT_ERROR("failed to create Test");
		return NULL;
	}

	Unittest *ut = unittest_create(UNITTEST_TEST, test);
	if (!ut)
	{
		PRINT_ERROR("failed to create Unittest");
		test_destroy(test);
		return NULL;
	}

	if (schedule)
	{
		ut->metadata.id = increment_counter();
		if (graph_add_vtx(schedule, ut->metadata.id, ut) != 0)
		{
			PRINT_ERROR("unable to add test to schedule");
			unittest_destroy(ut);
			return NULL;
		}
	}
	return ut;
}

Unittest *unittest_create_suite(const char *name)
{
	if (!name)
		return NULL;
	Suite *suite = suite_create(name);
	if (!suite)
	{
		PRINT_ERROR("failed to create Suite");
		return NULL;
	}
	Unittest *ut = unittest_create(UNITTEST_SUITE, suite);
	if (!ut)
	{
		PRINT_ERROR("faile to create Unittest");
		suite_destroy(suite);
		return NULL;
	}

	if (schedule)
	{
		ut->metadata.id = increment_counter();
		if (graph_add_vtx(schedule, ut->metadata.id, ut) != 0)
		{
			PRINT_ERROR("unable to add test to schedule");
			unittest_destroy(ut);
			return NULL;
		}
	}
	return ut;
}

void unittest_destroy(Unittest *ut)
{
	if (!ut)
		return;
	switch (ut->kind)
	{
	case UNITTEST_TEST:
		test_destroy(ut->as.test);
		break;
	case UNITTEST_SUITE:
		suite_destroy(ut->as.suite);
		break;
	}
	free(ut);
}

const char *unittest_get_name(const Unittest *ut)
{
	if (!ut)
		return "";
	switch (ut->kind)
	{
	case UNITTEST_TEST:
		return test_get_name(ut->as.test);
	case UNITTEST_SUITE:
		return suite_get_name(ut->as.suite);
	}
	return "";
}

int unittest_add_test(Unittest *suite, const char *name, unittest_fn fn)
{
	if (!suite)
		return -1;
	Test *test = test_create(name, fn);
	if (!test)
	{
		PRINT_ERROR("unable to create test");
		return -1;
	}
	if (suite_add(suite->as.suite, test) != 0)
	{
		PRINT_ERROR("failed to add test to suite");
		test_destroy(test);
		return -1;
	}
	return 0;
}

int unittest_add_dependency(Unittest *from, Unittest *to)
{
	if (!schedule)
	{
		PRINT_INFO("unittest is not setup");
		return -1;
	}

	if (graph_add_edge(schedule, from->metadata.id, to->metadata.id) != 0)
	{
		PRINT_ERROR("unable to add edge");
		return -1;
	}

	return 0;
}

UnittestResult *unittest_run(Unittest *ut, const struct unittest_opts *opts)
{
	UnittestResult *res = unittest_result_create(ut->kind);
	if (!res)
	{
		PRINT_ERROR("unable to create result");
		return NULL;
	}

	switch (ut->kind)
	{
	case UNITTEST_TEST:
		ut->metadata.status = test_run(ut->as.test, &res->as.test, opts);
		break;
	case UNITTEST_SUITE:
		ut->metadata.status = suite_run(ut->as.suite, &res->as.suite, opts);
		break;
	}

	return res;
}

static int run(void *arg, void *opts)
{
	Unittest *ut = (Unittest *)arg;
	UnittestResult *res = unittest_result_create(ut->kind);
	if (!res)
	{
		PRINT_ERROR("unable to create result");
		return -1;
	}

	if (table_insert(results, ut->metadata.id, res) != 0)
	{
		PRINT_ERROR("unable to insert result");
		unittest_result_destroy(res);
		return -1;
	}

	int rv;
	switch (ut->kind)
	{
	case UNITTEST_TEST:
		rv = test_run(ut->as.test, &res->as.test, opts);
		break;
	case UNITTEST_SUITE:
		rv = suite_run(ut->as.suite, &res->as.suite, opts);
		break;
	}
	if (rv == -1)
	{
		PRINT_ERROR("failed to run");
		return -1;
	}

	switch (ut->kind)
	{
	case UNITTEST_TEST:
		return res->as.test.status;
	case UNITTEST_SUITE:
		return suite_result_status(&res->as.suite);
	}

	return -1;
}

int unittest_run_all(struct unittest_opts *opts)
{
	struct unittest_opts run_opts =
		(opts) ? *opts : unittest_opts_default();

	return runner_execute(runner, schedule, run, &run_opts);
}

void unittest_print_result(const Unittest *ut,
						   const UnittestResult *result, int level)
{
	if (!result)
		return;

	if (ut->kind != result->kind)
	{
		PRINT_INFO("test and result kind mismatch");
		return;
	}

	switch (ut->kind)
	{
	case UNITTEST_TEST:
		test_print_result(ut->as.test, &result->as.test, level);
		break;
	case UNITTEST_SUITE:
		suite_print_result(ut->as.suite, &result->as.suite, level);
		break;
	}
}

void unittest_print_all(int level)
{
	graph_start_scan(schedule);
	struct graph_vtx test = graph_read_scan(schedule);
	while (test.value)
	{
		UnittestResult *result =
			(UnittestResult *)table_get(results, test.id);
		if (result)
			unittest_print_result(test.value, result, level);

		test = graph_read_scan(schedule);
	}
}

#undef CAPACITY
#undef TABLE_SIZE
