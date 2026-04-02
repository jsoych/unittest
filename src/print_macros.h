#ifndef PRINT_MACROS_H
#define PRINT_MACROS_H

#define PRINT_MODE_VERBOSE 2
#define PRINT_MODE_DEFAULT 1
#define PRINT_MODE_QUIET 0

#ifndef PRINT_MODE
#define PRINT_MODE PRINT_MODE_DEFAULT
#endif

#include "print.h"

/* ---------------- QUIET ---------------- */
#if PRINT_MODE == PRINT_MODE_QUIET

#define PRINT_DEBUG(...) ((void)0)
#define PRINT_INFO(...) ((void)0)
#define PRINT_WARNING(...) ((void)0)
#define PRINT_ERROR(...) print_msg(__func__, PRINT_ERROR, __VA_ARGS__)
#define PRINT_SYSERR(name, err) print_syserr(__func__, name, err)

/* ---------------- VERBOSE ---------------- */
#elif PRINT_MODE == PRINT_MODE_VERBOSE

#define PRINT_DEBUG(...)                                             \
	print_msg_verbose(__FILE__, __LINE__, __func__, PRINT_DEBUG, \
			  __VA_ARGS__)

#define PRINT_INFO(...) \
	print_msg_verbose(__FILE__, __LINE__, __func__, PRINT_INFO, __VA_ARGS__)

#define PRINT_WARNING(...)                                             \
	print_msg_verbose(__FILE__, __LINE__, __func__, PRINT_WARNING, \
			  __VA_ARGS__)

#define PRINT_ERROR(...)                                             \
	print_msg_verbose(__FILE__, __LINE__, __func__, PRINT_ERROR, \
			  __VA_ARGS__)

#define PRINT_SYSERR(name, err) \
	print_syserr_verbose(__FILE__, __LINE__, __func__, name, err)

/* ---------------- DEFAULT ---------------- */
#else

#define PRINT_DEBUG(...) print_msg(__func__, PRINT_DEBUG, __VA_ARGS__)

#define PRINT_INFO(...) print_msg(__func__, PRINT_INFO, __VA_ARGS__)

#define PRINT_WARNING(...) print_msg(__func__, PRINT_WARNING, __VA_ARGS__)

#define PRINT_ERROR(...) print_msg(__func__, PRINT_ERROR, __VA_ARGS__)

#define PRINT_SYSERR(name, err) print_syserr(__func__, name, err);

#endif

#endif
