#include <stdio.h>

#include "registry.h"
#include "unittest.h"

static void test_ok(UnittestResult *result)
{
    unittest_result_ok(result);
}

static void test_fail(UnittestResult *result)
{
    unittest_result_fail(result, "failed");
}

static void test_error(UnittestResult *result)
{
    unittest_result_error(result, "error");
}

static int setup(void)
{
    unittest_setup();
    return TEST_SUCCESS;
}
REGISTER_TEST("unittest_setup", setup);

static int tear_down(void)
{
    unittest_tear_down();
    unittest_setup();
    unittest_tear_down();
    return TEST_SUCCESS;
}
REGISTER_TEST("unittest_tear_down", tear_down);

static int add_dependency(void)
{
    unittest_setup();

    // Create tests
    Unittest *ok = unittest_create_test("test_ok", test_ok);
    Unittest *fail = unittest_create_test("test_fail", test_fail);
    Unittest *err = unittest_create_test("test_error", test_error);

    // ok -> fail -> err, ok -> err
    if (unittest_add_dependency(ok, fail) != 0)
    {
        unittest_tear_down();
        return TEST_FAILURE;
    }

    if (unittest_add_dependency(fail, err) != 0)
    {
        unittest_tear_down();
        return TEST_FAILURE;
    }

    if (unittest_add_dependency(ok, err) != 0)
    {
        unittest_tear_down();
        return TEST_FAILURE;
    }

    unittest_tear_down();
    return TEST_SUCCESS;
}
REGISTER_TEST("unittest_add_dependency", add_dependency);

static int run_all(void)
{
    const char *msg;
    unittest_setup();
    if (!unittest_create_test("test", test_ok))
    {
        msg = "unable to create test";
        goto error;
    }

    if (unittest_run_all(NULL) != 0)
    {
        msg = "failed to run all";
        goto failure;
    }

    unittest_tear_down();

    unittest_setup();

    Unittest *suite = unittest_create_suite("suite");
    if (!suite)
    {
        msg = "unable to create suite";
        goto failure;
    }

    for (int i = 0; i < 10; i++)
    {
        if (unittest_add_test(suite, "test", test_ok) != 0)
        {
            msg = "unable to add test";
            goto failure;
        }
    }

    struct unittest_opts opts = {
        .level = UNITTEST_VERBOSE,
        .timeout_ms = 1};

    if (unittest_run_all(&opts) != 0)
    {
        msg = "failed to run all";
        goto failure;
    }

    unittest_print_all(opts.level);

    unittest_setup();

    Unittest *ok = unittest_create_test("test_ok", test_ok);
    Unittest *fail = unittest_create_test("test_fail", test_fail);
    Unittest *err = unittest_create_test("test_error", test_error);

    suite = unittest_create_suite("suite_ok");
    for (int i = 0; i < 5; i++)
        unittest_add_test(suite, "test_suite_ok", test_ok);

    // err -> fail -> ok, fail -> suite
    unittest_add_dependency(fail, ok);
    unittest_add_dependency(fail, suite);
    unittest_add_dependency(err, fail);

    if (unittest_run_all(&opts) == 0)
    {
        msg = "failed to taint connected component";
        goto failure;
    }
    unittest_print_all(opts.level);

    unittest_tear_down();
    return TEST_SUCCESS;

failure:
    printf("%s\n", msg);
    unittest_tear_down();
    return TEST_FAILURE;

error:
    printf("%s\n", msg);
    unittest_tear_down();
    return TEST_ERROR;
}
REGISTER_TEST("unittest_run_all", run_all);
