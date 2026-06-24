#ifndef REGISTRY_H
#define REGISTRY_H

enum
{
    TEST_SUCCESS = 0,
    TEST_FAILURE,
    TEST_ERROR
};

struct registry_entry
{
    const char *name;
    int (*func)(void);
};

#if defined(__APPLE__)
#define REGISTRY_SECTION "__DATA,__test_reg"
#elif defined(__linux__)
#define REGISTRY_SECTION "test_reg_sec"
#else
#error "Unsupported Operating System"
#endif

#define REGISTER_TEST(test_name, function_ptr)                \
    static const struct registry_entry entries_##function_ptr \
        __attribute__((used, section(REGISTRY_SECTION))) = {  \
            .name = test_name,                                \
            .func = function_ptr}

#endif

void registry_runner(void);
