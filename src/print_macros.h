/**
 * @file print_macros.h
 * @brief Convenience macros that wrap the low‑level @c print_msg functions.
 *
 * The header defines three compile‑time verbosity modes:
 * - @c PRINT_MODE_QUIET  – Only error and system‑error messages are emitted.
 * - @c PRINT_MODE_DEFAULT – Messages are printed with the standard prefix
 *   (function name and severity level).
 * - @c PRINT_MODE_VERBOSE – Every message is annotated with the source
 *   file name and line number.
 *
 * The macro @c PRINT_MODE can be set externally (e.g., via the compiler’s
 * `-DPRINT_MODE=…` flag).  If it is omitted, the default value
 * (`PRINT_MODE_DEFAULT`) is used.
 */

#ifndef PRINT_MACROS_H
#define PRINT_MACROS_H

/** @def PRINT_MODE_VERBOSE
 *  @brief Enable verbose output (file/line information is included). */
#define PRINT_MODE_VERBOSE 2

/** @def PRINT_MODE_DEFAULT
 *  @brief Enable the normal, non‑quiet output. */
#define PRINT_MODE_DEFAULT 1

/** @def PRINT_MODE_QUIET
 *  @brief Suppress all output except @c PRINT_ERROR and system errors. */
#define PRINT_MODE_QUIET 0

/**
 * @def PRINT_MODE
 * @brief Current printing mode.
 *
 * If the macro @c PRINT_MODE is defined externally, that value is used.
 * Otherwise it defaults to @c PRINT_MODE_DEFAULT.
 */
#ifndef PRINT_MODE
#define PRINT_MODE PRINT_MODE_DEFAULT
#endif

#include "print.h"

/* ----------------------------------------------------------------------
 *  QUIET  – Only error messages are emitted
 * ---------------------------------------------------------------------- */
#if PRINT_MODE == PRINT_MODE_QUIET

/** @def PRINT_DEBUG(...)
 *  @brief No‑op in QUIET mode (debug messages are discarded). */
#define PRINT_DEBUG(...) ((void)0)

/** @def PRINT_INFO(...)
 *  @brief No‑op in QUIET mode (info messages are discarded). */
#define PRINT_INFO(...) ((void)0)

/** @def PRINT_WARNING(...)
 *  @brief No‑op in QUIET mode (warning messages are discarded). */
#define PRINT_WARNING(...) ((void)0)

/** @def PRINT_ERROR(...)
 *  @brief Emit an error message without file/line information.
 *
 *  This expands to:
 *  @code
 *      print_msg(__func__, PRINT_ERROR, __VA_ARGS__);
 *  @endcode
 */
#define PRINT_ERROR(...) print_msg(__func__, PRINT_ERROR, __VA_ARGS__)

/** @def PRINT_SYSERR(name, err)
 *  @brief Emit a system‑error message without file/line information.
 *
 *  Expands to:
 *  @code
 *      print_syserr(__func__, name, err);
 *  @endcode
 */
#define PRINT_SYSERR(name, err) print_syserr(__func__, name, err)

/* ----------------------------------------------------------------------
 *  VERBOSE – Include file name and line number in all messages
 * ---------------------------------------------------------------------- */
#elif PRINT_MODE == PRINT_MODE_VERBOSE

/** @def PRINT_DEBUG(...)
 *  @brief Emit a debug message with file/line/func context. */
#define PRINT_DEBUG(...)                                             \
	print_msg_verbose(__FILE__, __LINE__, __func__, PRINT_DEBUG, \
			  __VA_ARGS__)

/** @def PRINT_INFO(...)
 *  @brief Emit an info message with file/line/func context. */
#define PRINT_INFO(...) \
	print_msg_verbose(__FILE__, __LINE__, __func__, PRINT_INFO, __VA_ARGS__)

/** @def PRINT_WARNING(...)
 *  @brief Emit a warning message with file/line/func context. */
#define PRINT_WARNING(...)                                             \
	print_msg_verbose(__FILE__, __LINE__, __func__, PRINT_WARNING, \
			  __VA_ARGS__)

/** @def PRINT_ERROR(...)
 *  @brief Emit an error message with file/line/func context. */
#define PRINT_ERROR(...)                                             \
	print_msg_verbose(__FILE__, __LINE__, __func__, PRINT_ERROR, \
			  __VA_ARGS__)

/** @def PRINT_SYSERR(name, err)
 *  @brief Emit a system‑error message with file/line/func context. */
#define PRINT_SYSERR(name, err) \
	print_syserr_verbose(__FILE__, __LINE__, __func__, name, err)

/* ----------------------------------------------------------------------
 *  DEFAULT – Normal output, no file/line information
 * ---------------------------------------------------------------------- */
#else

/** @def PRINT_DEBUG(...)
 *  @brief Emit a debug message (no file/line info). */
#define PRINT_DEBUG(...) print_msg(__func__, PRINT_DEBUG, __VA_ARGS__)

/** @def PRINT_INFO(...)
 *  @brief Emit an info message (no file/line info). */
#define PRINT_INFO(...) print_msg(__func__, PRINT_INFO, __VA_ARGS__)

/** @def PRINT_WARNING(...)
 *  @brief Emit a warning message (no file/line info). */
#define PRINT_WARNING(...) print_msg(__func__, PRINT_WARNING, __VA_ARGS__)

/** @def PRINT_ERROR(...)
 *  @brief Emit an error message (no file/line info). */
#define PRINT_ERROR(...) print_msg(__func__, PRINT_ERROR, __VA_ARGS__)

/** @def PRINT_SYSERR(name, err)
 *  @brief Emit a system‑error message (no file/line info). */
#define PRINT_SYSERR(name, err) print_syserr(__func__, name, err);

#endif /* PRINT_MODE */

#endif /* PRINT_MACROS_H */
