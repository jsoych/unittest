#include <stdio.h>

#include "registry.h"

#if defined(__APPLE__)
#include <mach-o/getsect.h>
#include <mach-o/dyld.h>
#include <stdint.h>

static void print_result(const char *name, int result)
{
    switch (result)
    {
    case TEST_SUCCESS:
        printf("\033[1;32mSUCCESS:\x1b[0m %s\n", name);
        break;
    case TEST_FAILURE:
        printf("\033[1;35mFAILURE:\x1b[0m %s\n", name);
        break;
    case TEST_ERROR:
        printf("\033[1;31mERROR:\x1b[0m %s\n", name);
        break;
    default:
        printf("Unknown test result\n");
        break;
    }
}

// Helper macro to fetch bounds dynamically on macOS at runtime
#define IMPLEMENT_REGISTRY_RUNNER()                                                    \
    extern const struct mach_header_64 _mh_execute_header;                             \
    void registry_runner(void)                                                         \
    {                                                                                  \
        unsigned long size = 0;                                                        \
        uint8_t *start_ptr = getsectiondata(&_mh_execute_header,                       \
                                            "__DATA", "__test_reg", &size);            \
        if (!start_ptr || size == 0)                                                   \
            return;                                                                    \
        const struct registry_entry *start = (const struct registry_entry *)start_ptr; \
        size_t count = size / sizeof(struct registry_entry);                           \
        for (size_t i = 0; i < count; i++)                                             \
            print_result(start[i].name, start[i].func());                              \
    }

#elif defined(__linux__)
// Expose the automatic linker symbols
extern const struct registry_entry __start_test_reg_sec;
extern const struct registry_entry __stop_test_reg_sec;

// Helper macro to loop through bounds on Linux (Updated to match macOS behavior)
#define IMPLEMENT_REGISTRY_RUNNER()                                 \
    void registry_runner(void)                                      \
    {                                                               \
        const struct registry_entry *start = &__start_test_reg_sec; \
        const struct registry_entry *stop = &__stop_test_reg_sec;   \
        for (const struct registry_entry *t = start; t < stop; t++) \
            print_result(t->name, t->func());                       \
    }
#else
#error "Unsupported Operating System"
#endif

IMPLEMENT_REGISTRY_RUNNER()
