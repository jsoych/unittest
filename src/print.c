#include "print.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define COLOUR_CYAN "\033[1;36m"
#define COLOUR_MAGENTA "\x1b[35m"
#define COLOUR_RED "\x1b[31m"
#define COLOUR_RESET "\x1b[0m"
#define COLOUR_YELLOW "\033[1;33m"

static const char *level_to_string(print_level_t level)
{
	switch (level)
	{
	case PRINT_DEBUG:
		return "debug";
	case PRINT_INFO:
		return "info";
	case PRINT_WARNING:
		return "warning";
	case PRINT_ERROR:
		return "error";
	default:
		return "unknown";
	}
}

static const char *level_to_colour(print_level_t level)
{
	switch (level)
	{
	case PRINT_DEBUG:
		return COLOUR_MAGENTA;
	case PRINT_INFO:
		return COLOUR_CYAN;
	case PRINT_WARNING:
		return COLOUR_YELLOW;
	case PRINT_ERROR:
		return COLOUR_RED;
	default:
		return "";
	}
}

static void vprint_common(const char *prefix, print_level_t level,
						  const char *fmt, va_list args)
{
	const char *lvl = level_to_string(level);
	const char *col = level_to_colour(level);

	if (prefix)
	{
		fprintf(stderr, "%s: ", prefix);
	}

	fprintf(stderr, "%s%s:%s ", col, lvl, COLOUR_RESET);

	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
}

void print_msg(const char *func, print_level_t level, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprint_common(func, level, fmt, args);
	va_end(args);
}

void print_msg_verbose(const char *file, int line, const char *func,
					   print_level_t level, const char *fmt, ...)
{
	char prefix[256];
	snprintf(prefix, sizeof(prefix), "%s:%d: %s", file, line, func);

	va_list args;
	va_start(args, fmt);
	vprint_common(prefix, level, fmt, args);
	va_end(args);
}

static void psyserr(const char *prefix, const char *name, int err)
{
	const char *lvl = level_to_string(PRINT_ERROR);
	const char *col = level_to_colour(PRINT_ERROR);
	if (prefix)
	{
		fprintf(stderr, "%s: ", prefix);
	}
	fprintf(stderr, "%s%s:%s %s: %s\n", col, lvl, COLOUR_RESET, name,
			strerror(err));
}

void print_syserr(const char *func, const char *name, int err)
{
	psyserr(func, name, err);
}

void print_syserr_verbose(const char *file, int line, const char *func,
						  const char *name, int err)
{
	char prefix[256];
	snprintf(prefix, sizeof(prefix), "%s:%d: %s", file, line, func);
	psyserr(prefix, name, err);
}
