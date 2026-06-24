#include <errno.h>

#include "registry.h"
#include "print.h"
#include "print_macros.h"

#define LEVEL_SIZE (4)

static int level[LEVEL_SIZE] =
    {PRINT_DEBUG, PRINT_INFO, PRINT_WARNING, PRINT_ERROR};

static int test_msg(void)
{
    for (int i = 0; i < LEVEL_SIZE; i++)
        print_msg(__func__, level[i], "message without args");
    return TEST_SUCCESS;
}
REGISTER_TEST("print_msg", test_msg);

static int test_msg_verbose(void)
{
    for (int i = 0; i < LEVEL_SIZE; i++)
        print_msg_verbose(__FILE__, __LINE__, __func__, level[i], "message (%d)", i);
    return TEST_SUCCESS;
}
REGISTER_TEST("print_msg_verbose", test_msg_verbose);

static int test_syserr(void)
{
    print_syserr(__func__, "malloc", ENOMEM);
    print_syserr(__func__, "calloc", ENOMEM);
    return TEST_SUCCESS;
}
REGISTER_TEST("print_syserr", test_syserr);

static int test_syserr_verbose(void)
{
    print_syserr_verbose(__FILE__, __LINE__, __func__, "ioctl", ENOTTY);
    print_syserr_verbose(__FILE__, __LINE__, __func__, "fcntl", EDEADLK);
    return TEST_SUCCESS;
}
REGISTER_TEST("print_syserr_verbose", test_syserr_verbose);

static int test_macros(void)
{
    PRINT_DEBUG("debug message");
    PRINT_INFO("info message");
    PRINT_WARNING("warning message");
    PRINT_ERROR("error message");
    PRINT_SYSERR("malloc", ENOMEM);
    return TEST_SUCCESS;
}
REGISTER_TEST("print_macros", test_macros);
