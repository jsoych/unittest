/**
 * @file print.h
 * @brief Lightweight, severity‑aware printing utilities.
 *
 * The header declares a small logging façade that supports four
 * verbosity levels and optional file/line annotation.  The implementation
 * is intentionally minimal so that it can be embedded in a variety of
 * projects without pulling in external logging frameworks.
 */

#ifndef PRINT_H
#define PRINT_H

#include <stdarg.h>

/**
 * @enum print_level_t
 * @brief Severity level for messages.
 *
 * The enumeration values are ordered from least to most severe.  The
 * numeric values can be used directly in comparisons or switches.
 */
typedef enum {
	PRINT_DEBUG = 0, /**< Debug‑level (least severe). */
	PRINT_INFO, /**< Informational messages. */
	PRINT_WARNING, /**< Warning conditions. */
	PRINT_ERROR /**< Error conditions. */
} print_level_t;

/**
 * @brief Print a formatted message at a given severity level.
 *
 * The function behaves similarly to `vprintf()` but prefixes the message
 * with a severity label and the calling function name.
 *
 * @param func  Name of the function from which `print_msg()` is invoked.
 * @param level Severity level of the message.
 * @param fmt   `printf`‑style format string.
 * @param ...   Variable arguments that correspond to @p fmt.
 */
void print_msg(const char *func, print_level_t level, const char *fmt, ...);

/**
 * @brief Print a formatted message with file and line information.
 *
 * The function prepends the file name and line number to the output in
 * addition to the severity level and function name.
 *
 * @param file  Name of the source file (typically `__FILE__`).
 * @param line  Line number in @p file (typically `__LINE__`).
 * @param func  Name of the function from which the message originates.
 * @param level Severity level of the message.
 * @param fmt   `printf`‑style format string.
 * @param ...   Variable arguments that correspond to @p fmt.
 */
void print_msg_verbose(const char *file, int line, const char *func,
		       print_level_t level, const char *fmt, ...);

/**
 * @brief Print a system error message at a given severity level.
 *
 * The function formats the string as:
 * @code
 *   "<func>: <name> error <err> (<error string>)"
 * @endcode
 * where `<error string>` is obtained from `strerror(err)`.
 *
 * @param func Name of the calling function.
 * @param name A user‑supplied label that identifies the context
 *             (e.g., the name of a file that failed to open).
 * @param err  System error number (e.g., from `errno`).
 */
void print_syserr(const char *func, const char *name, int err);

/**
 * @brief Print a system error message with file and line annotation.
 *
 * The output includes the file name and line number in addition to the
 * information printed by `print_syserr()`.
 *
 * @param file  Name of the source file (typically `__FILE__`).
 * @param line  Line number in @p file (typically `__LINE__`).
 * @param func  Name of the calling function.
 * @param name  User‑supplied label that identifies the context.
 * @param err   System error number.
 */
void print_syserr_verbose(const char *file, int line, const char *func,
			  const char *name, int err);

#endif /* PRINT_H */
