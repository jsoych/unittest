#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "print_macros.h"
#include "suite.h"

#define CAPACITY 8

struct Suite
{
	char *name;
	int size;
	int capacity;
	Test **tests;
};

int suite_result_init(struct suite_result *result, int size)
{
	result->nok = 0;
	result->nfail = 0;
	result->nerror = 0;
	result->nsyserr = 0;
	result->time_ms = 0;

	if ((result->size = size) == 0)
	{
		result->test_results = NULL;
		return 0;
	}

	result->test_results = malloc(size * sizeof(struct test_result));
	if (!result->test_results)
	{
		PRINT_SYSERR("malloc", errno);
		return -1;
	}

	return 0;
}

void suite_result_free(struct suite_result *result)
{
	if (result->test_results)
		free(result->test_results);
	suite_result_init(result, 0);
}

int suite_result_status(const struct suite_result *result)
{
	if (result->nsyserr > 0)
		return UNITTEST_SYSERR;
	else if (result->nerror > 0)
		return UNITTEST_ERROR;
	else if (result->nfail > 0)
		return UNITTEST_FAIL;
	else
		return UNITTEST_OK;
	return -1;
}

Suite *suite_create(const char *name)
{
	Suite *suite = malloc(sizeof(Suite));
	if (!suite)
	{
		PRINT_SYSERR("malloc", errno);
		return NULL;
	}
	suite->name = strdup(name);
	if (!suite->name)
	{
		PRINT_SYSERR("strdup", errno);
		free(suite);
		return NULL;
	}
	suite->size = 0;
	suite->capacity = CAPACITY;
	suite->tests = calloc(CAPACITY, sizeof(Test *));
	if (!suite->tests)
	{
		PRINT_SYSERR("calloc", errno);
		free(suite->name);
		free(suite);
		return NULL;
	}
	for (int i = 0; i < CAPACITY; i++)
		suite->tests[i] = NULL;
	return suite;
}

void suite_destroy(Suite *suite)
{
	if (!suite)
		return;
	free(suite->name);
	for (int i = 0; i < suite->size; i++)
	{
		test_destroy(suite->tests[i]);
	}
	free(suite);
}

const char *suite_get_name(const Suite *suite)
{
	return suite->name;
}

int suite_add(Suite *suite, Test *test)
{
	if (suite->size == suite->capacity)
	{
		int capacity = 2 * suite->capacity;
		Test **tests = realloc(suite->tests, capacity * sizeof(Test *));
		if (!tests)
		{
			PRINT_SYSERR("malloc", errno);
			return -1;
		}
		for (int i = suite->size; i < capacity; i++)
		{
			tests[i] = NULL;
		}
		suite->capacity = capacity;
		suite->tests = tests;
	}
	suite->tests[suite->size++] = test;
	return 0;
}

int suite_run(const Suite *suite, struct suite_result *result,
			  const struct unittest_opts *run_opts)
{
	if (!suite || !result)
		return -1;
	if (suite_result_init(result, suite->size) == -1)
		return -1;

	int rv = 0;
	for (int i = 0; i < suite->size; i++)
	{
		if (test_run(suite->tests[i], &result->test_results[i],
					 run_opts) == -1)
		{
			result->nsyserr++;
			rv = -1;
			continue;
		}
		switch (result->test_results[i].status)
		{
		case UNITTEST_OK:
			result->nok++;
			break;
		case UNITTEST_FAIL:
			result->nfail++;
			break;
		case UNITTEST_ERROR:
			result->nerror++;
			break;
		default:
			result->nsyserr++;
			break;
		}
		result->time_ms += result->test_results[i].time_ms;
	}
	return rv;
}

static const char *result_to_str(const struct suite_result *result)
{
	if (result->nsyserr > 0)
	{
		return "SYSERR";
	}
	else if (result->nerror > 0)
	{
		return "ERROR";
	}
	else if (result->nfail > 0)
	{
		return "FAILED";
	}
	else
	{
		return "OK";
	}
}

void print_banner(const Suite *suite)
{
	const char *name = suite_get_name(suite);

	int width = 70;
	int len = (int)strlen(name);

	printf(DLINE);
	putchar('\n');

	if (len >= width)
	{
		printf("%s\n", name);
	}
	else
	{
		int padding = (width - len) / 2;
		for (int i = 0; i < padding; i++)
		{
			putchar(' ');
		}
		printf("%s", name);
	}
}

static void print_summary(const struct suite_result *result)
{
	printf(DLINE);
	printf("\nRan %d tests in %.3fs", result->size,
		   (double)(result->time_ms / 1000));

	printf("\n\n%s (", result_to_str(result));
	int comma = 0;
	if (result->nok)
	{
		printf("ok=%d", result->nok);
		comma = 1;
	}
	if (result->nfail)
	{
		printf("%sfailures=%d", comma ? ", " : "", result->nfail);
		comma = 1;
	}
	if (result->nerror)
	{
		printf("%serrors=%d", comma ? ", " : "", result->nerror);
		comma = 1;
	}
	if (result->nsyserr)
	{
		printf("%ssyserrs=%d", comma ? ", " : "", result->nsyserr);
		comma = 1;
	}
	printf(")");
}

void suite_print_result(const Suite *suite, const struct suite_result *result, int level)
{
	if (level == UNITTEST_QUIET)
		return;

	/* Print suite banner */
	print_banner(suite);

	/* Print test results */
	for (int i = 0; i < result->size; i++)
	{
		test_print_result(suite->tests[i], &result->test_results[i],
						  level);
	}

	/* Print result summary */
	print_summary(result);
}
