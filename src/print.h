#ifndef PRINT_H
#define PRINT_H

#include <stdarg.h>

/* The level of print severity */
typedef enum {
	PRINT_DEBUG = 0,
	PRINT_INFO,
	PRINT_WARNING,
	PRINT_ERROR
} print_level_t;

/*
 * Prints the formatted message at the given level. 
 */
void print_msg(const char *func, print_level_t level, const char *fmt, ...);

/*
 * Prints the formatted message at the given level, and adds the file name
 * and line number to the output.
 */
void print_msg_verbose(const char *file, int line, const char *func,
		       print_level_t level, const char *fmt, ...);

/*
 * Prints the system error message at the given level with the name and
 * err(no).
 */
void print_syserr(const char *func, const char *name, int err);

/*
 * Prints the system error message at the given level with the name and
 * err(no), and add the the file name and line number to the output.
 */
void print_syserr_verbose(const char *file, int line, const char *func,
			  const char *name, int err);

#endif
