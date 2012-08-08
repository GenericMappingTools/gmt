/* test_common.h - utility functions and macros for xdr tests
 * Copyright (c) 2009 Charles S. Wilson
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _XDR_TEST_COMMON_H
#define _XDR_TEST_COMMON_H

#include <stdio.h>
#include <rpc/types.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h> /* needed for CHAR_MIN */
#include <getopt.h>
#if defined(_MSC_VER) || defined(__MINGW32__)
# include <io.h>
# include <process.h>
# include <direct.h>
#else
# include <unistd.h>
#endif

#ifdef __cplusplus
# define XDR_TEST_DECLS_BEGIN extern "C" {
# define XDR_TEST_DECLS_END   }
#else
# define XDR_TEST_DECLS_BEGIN
# define XDR_TEST_DECLS_END
#endif

#ifndef DIR_SEPARATOR
# define DIR_SEPARATOR '/'
# define PATH_SEPARATOR ':'
#endif
#if defined (_WIN32) || defined (__MSDOS__) || defined (__DJGPP__) || defined (__OS2__)
# define HAVE_DOS_BASED_FILE_SYSTEM
# define FOPEN_WB "wb"
# define FOPEN_RB "rb"
# ifndef DIR_SEPARATOR_2
#  define DIR_SEPARATOR_2 '\\'
# endif
# ifndef PATH_SEPARATOR_2
#  define PATH_SEPARATOR_2 ';'
# endif
#endif

#ifndef DIR_SEPARATOR_2
# define IS_DIR_SEPARATOR(ch) ((ch) == DIR_SEPARATOR)
#else
# define IS_DIR_SEPARATOR(ch) \
        (((ch) == DIR_SEPARATOR) || ((ch) == DIR_SEPARATOR_2))
#endif

#ifndef PATH_SEPARATOR_2
# define IS_PATH_SEPARATOR(ch) ((ch) == PATH_SEPARATOR)
#else
# define IS_PATH_SEPARATOR(ch) ((ch) == PATH_SEPARATOR_2)
#endif

#ifdef __CYGWIN__
# define FOPEN_WB "wb"
# define FOPEN_RB "rb"
#endif

#ifndef FOPEN_WB
# define FOPEN_WB "w"
# define FOPEN_RB "r"
#endif
#ifndef _O_BINARY
# define _O_BINARY 0
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
XDR_TEST_DECLS_BEGIN
/* getopt.h doesn't define this because stdlib.h is supposed to.
   But doesn't, on mingw or msvc */
int getopt (int argc, char *const *argv, const char *shortopts);

/* declare our replacement function(s) */
char *mkdtemp (char *);
XDR_TEST_DECLS_END
#endif

#define XDR_LOG_SILENT     -1
#define XDR_LOG_NORMAL      0
#define XDR_LOG_INFO        1
#define XDR_LOG_DETAIL      2
#define XDR_LOG_DEBUG       3
#define XDR_LOG_DEBUG2      4

XDR_TEST_DECLS_BEGIN

extern const char *program_name;
extern void set_program_name (const char *argv0);
extern const char *base_name (const char *s);
extern int mkdir_p (const char *, int);
extern void dumpmem (FILE * f, void *buf, unsigned int len, unsigned int offset);

typedef struct _log_opts
{
  int level;
  FILE *f;
} log_opts;
extern void log_msg (log_opts * o, int level, const char *fmt, ...);

/*********************************************************/
/* definitions for callbacks to manage/debug XDR streams */
/*********************************************************/

/* used to initialize the void* data. called once
 * per test, and must be called prior to xdr_create_cb.
 */
typedef bool_t (*xdr_test_setup_cb)(enum xdr_op, void * /*userdata*/);

/* used to initialize an XDR * from the data given in
 * the void*. For instance, an xdrmem implementation
 * would use a supplied buf and bufsz to call
 * xdrmem_create(). An xdrstdio implementation would
 * open a specified file, then call xdrstdio_create().
 * Should not be called twice on the same XDR * and/or
 * void*, unless xdr_finish_cb is called in between
 * the two occurances.
 */
typedef bool_t (*xdr_create_cb)(XDR *, enum xdr_op, void * /*userdata*/);

/* used to finalize an XDR * created from the data
 * given in the void*. For instance, an xdrstdio
 * implementation might close the specified file.
 * This operation should be callable multiple times
 * on the same object, without harm. Should also
 * call XDR_DESTROY() -- which is not callable in
 * this way, so the xdr_finish_cb should use guard
 * variables in the void*.
 */
typedef bool_t (*xdr_finish_cb)(XDR *, enum xdr_op, void * /*userdata*/);

/* used after an encoding phase to produce debug
 * output. For instance, an xdrmem implementation
 * might produce a hexdump of the the buf in void*.
 */
typedef void   (*xdr_debug_cb)(void * /*userdata*/);

/* used to finalize the void* data. called once
 * per test, and no other callbacks may be accessed
 * (other than xdr_init_test_cb) once this one has
 * been activated.
 */
typedef bool_t (*xdr_test_teardown_cb)(void * /*userdata*/);

typedef struct _xdr_stream_ops {
  xdr_test_setup_cb    setup_cb;    /* may be null */
  xdr_create_cb        create_cb;
  xdr_finish_cb        finish_cb;
  xdr_debug_cb         debug_cb;    /* may be null */
  xdr_test_teardown_cb teardown_cb; /* may be null */
} xdr_stream_ops;

XDR_TEST_DECLS_END
#endif /* _XDR_TEST_COMMON_H */

