# Unittest C Framework

This repository includes a small C testing framework intentionally modeled after Python’s `unittest` module. The goal is to keep tests **easy to write**, **easy to read**, and **fast to run**.

## Quick Start

* Report a bug?
* Install
* Examples

## Installation

Clone repo
Make
Install (Optional)

## Examples

how to include a code snippet?

### Test

```c
#include <stdlib.h>
#include <unittest/unittest.h>

void test_case(UnittestResult* result) {
    /* create array */
    int *arr = malloc(3 * sizeof(int));
    if (!arr) {
        unittest_result_err(result, "failed to create array");
        return;
    }

    /* init array */
    for (int i = 0; i < 3; i++) {
        arr[i] = i;
    }

    /* sum array values */
    int sum = 0;
    for (int i = 0; i < 5 /* index out of bounds */; i++) {
        sum += arr[i];
    }

    if (sum != 3) {
        unittest_result_fail(result, "unexpected sum (%d)", sum);
        free(arr);
        return;
    }
    unittest_result_ok(result);
    free(arr);
}

int main() {
    /* create test */
    Unittest *test = unittest_create_test("example", test_case);

    /* set timeout to 1s and output verbosity to quiet */
    unittest_opts_t opts = {
        .timeout_ms = 1000,
        .level = UNITTEST_QUIET
    };

    /* run test */
    int status;
    UnittestResult *result = unittest_run(test, &opts, &status);
    if (status != 0) {
        unittest_destroy(test);
        return 1;
    }

    /* print test result */
    unittest_print_result(test, result, UNITTEST_VERBOSE);

    /* cleanup and exit */
    unittest_result_destroy(result);
    unittest_destroy(test);
    return 0;
}

```

```text
[jsoych@machine ~]# gcc -lunittest test.c -o test 
[jsoych@machine ~]# ./test 
.
======================================================================
ok: example

Ran test in 0.000s
```

### Suite

The following example demostrates how to create and run a suite of tests with default values.

```c
#include <stdlib.h>
#include <unittest/unittest.h>

void test_case_1(UnittestResult *result)
{
    int sum = 1 + 3;
    if (sum != 3)
    {
        unittest_result_fail(result, "unexpected value (%d)", sum);
        return;
    }
    unittest_result_ok(result);
}

void test_case_2(UnittestResult *result)
{
    void *ptr;
    free(ptr); /* invalid free */
}

int main()
{
    /* create suite */
    Unittest *suite = unittest_create_suite("example");

    /* add tests to the suite */
    (void)unittest_add_test(suite, "test_1", test_case_1);
    (void)unittest_add_test(suite, "test_2", test_case_2);

    /* run suite */
    UnittestResult *result = unittest_run(
        suite,
        NULL /* use default options */,
        NULL /* ignore run status */
    );

    /* print suite result */
    unittest_print_result(suite, result, UNITTEST_DEFAULT);

    /* cleanup and exit */
    unittest_result_destroy(result);
    unittest_destroy(suite);
    return 0;
}

```

```text
[[jsoych@machine ~]# gcc -lunittest suite.c -o suite
[[jsoych@machine ~]# ./suite
FE
======================================================================
Ran 2 tests in 0.000s

FAILED (failures=1, errors=1)

======================================================================
FAIL: test_1
----------------------------------------------------------------------
msg: unexpected value (4)
======================================================================
ERROR: test_2
----------------------------------------------------------------------
msg: terminated by signal
----------------------------------------------------------------------
stderr: suite(54268,0x1dfd7e500) malloc: *** error for object 0x2735fc: pointer being freed was not allocated
suite(54268,0x1dfd7e500) malloc: *** set a breakpoint in malloc_error_break to debug
```
