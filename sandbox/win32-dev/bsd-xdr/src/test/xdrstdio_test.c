/* xdrstdio_test.c - test xdr with stdio streams
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#include "test_common.h"
#include "test_data.h"
#include "test_xdrs.h"

const char *program_name;

void
usage (FILE * f, const char *progname)
{
  fprintf (f, "%s [-h] [-v|-s] [-k]\n", progname);
  fputs ("  Tests the stdio functionality of the xdr library\n", f);
  fputs ("  on this specific platform. Does not test data interchange\n", f);
  fputs ("  between different platforms.\n", f);
  fputs ("   -h       : print this help\n", f);
  fputs ("   -v       : verbose mode (may be repeated)\n", f);
  fputs ("   -s       : silent mode\n", f);
  fputs ("   -k       : keep temporary files\n", f);
  fputs ("   -t <DIR> : use temporary dir\n", f);
}

typedef struct _opts
{
  log_opts *log;
  int       keep;
  char     *tmpdir;
} opts;

bool_t test_xdrstdio_int (opts *o);
bool_t test_xdrstdio_u_int (opts *o);
bool_t test_xdrstdio_long (opts *o);
bool_t test_xdrstdio_u_long (opts *o);
bool_t test_xdrstdio_short (opts *o);
bool_t test_xdrstdio_u_short (opts *o);
bool_t test_xdrstdio_char (opts *o);
bool_t test_xdrstdio_u_char (opts *o);
bool_t test_xdrstdio_int8_t (opts *o);
bool_t test_xdrstdio_u_int8_t (opts *o);
bool_t test_xdrstdio_uint8_t (opts *o);
bool_t test_xdrstdio_int16_t (opts *o);
bool_t test_xdrstdio_u_int16_t (opts *o);
bool_t test_xdrstdio_uint16_t (opts *o);
bool_t test_xdrstdio_int32_t (opts *o);
bool_t test_xdrstdio_u_int32_t (opts *o);
bool_t test_xdrstdio_uint32_t (opts *o);
bool_t test_xdrstdio_int64_t (opts *o);
bool_t test_xdrstdio_u_int64_t (opts *o);
bool_t test_xdrstdio_uint64_t (opts *o);
bool_t test_xdrstdio_hyper (opts *o);
bool_t test_xdrstdio_u_hyper (opts *o);
bool_t test_xdrstdio_longlong_t (opts *o);
bool_t test_xdrstdio_u_longlong_t (opts *o);
bool_t test_xdrstdio_float (opts *o);
bool_t test_xdrstdio_double (opts *o);
bool_t test_xdrstdio_bool (opts *o);
bool_t test_xdrstdio_enum (opts *o);
bool_t test_xdrstdio_union (opts *o);
bool_t test_xdrstdio_opaque (opts *o);
bool_t test_xdrstdio_bytes (opts *o);
bool_t test_xdrstdio_string (opts *o);
bool_t test_xdrstdio_wrapstring (opts *o);
bool_t test_xdrstdio_array (opts *o);
bool_t test_xdrstdio_vector (opts *o);
bool_t test_xdrstdio_reference (opts *o);
bool_t test_xdrstdio_pointer (opts *o);
bool_t test_xdrstdio_list (opts *o);
bool_t test_xdrstdio_primitive_struct (opts *o);

/* This is a data struct for the test callback functions */
typedef struct _xdrstdio_creation_data {
  opts  *o;
  int    finish_guard;
  char  *name;
  char  *fullname;
  FILE  *f;
} xdrstdio_creation_data;

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
#endif
/* allocates storage for and computes full name to test file */
bool_t
xdrstdio_test_setup_cb (enum xdr_op op, void * data)
{
  xdrstdio_creation_data* xdrstdio_data = (xdrstdio_creation_data*)data;

  if (xdrstdio_data->fullname)
    {
      free (xdrstdio_data->fullname);
      xdrstdio_data->fullname = NULL;
    }

  xdrstdio_data->fullname = (char *) malloc
      (strlen (xdrstdio_data->name) +
       strlen (xdrstdio_data->o->tmpdir) + 2);
  strcpy (xdrstdio_data->fullname, xdrstdio_data->o->tmpdir);
  strcat (xdrstdio_data->fullname, "/");
  strcat (xdrstdio_data->fullname, xdrstdio_data->name);
  return TRUE;
}
#ifdef _MSC_VER
# pragma warning(pop)
#endif

/* opens test file for read or write */
bool_t
xdrstdio_create_cb (XDR *xdrs, enum xdr_op op, void * data)
{
  bool_t rVal = TRUE;
  xdrstdio_creation_data* xdrstdio_data = (xdrstdio_creation_data*)data;

  switch (op)
    {
      case XDR_DECODE:
      case XDR_FREE:
        xdrstdio_data->f = fopen (xdrstdio_data->fullname, FOPEN_RB);
        break;
      case XDR_ENCODE:
        xdrstdio_data->f = fopen (xdrstdio_data->fullname, FOPEN_WB);
        break;
    }

  if (!xdrstdio_data->f)
    {
      log_msg (xdrstdio_data->o->log, XDR_LOG_NORMAL,
               "could not open data file: %s\n", xdrstdio_data->fullname);
      rVal = FALSE;
    }
  else
    {
      xdrstdio_create (xdrs, xdrstdio_data->f, op);
      xdrstdio_data->finish_guard = 1;
    }
  return rVal;
}

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
# pragma warning(disable:4127)
#endif
/* closes test file for read or write */
bool_t
xdrstdio_finish_cb (XDR * xdrs, enum xdr_op op, void * data)
{
  xdrstdio_creation_data* xdrstdio_data = (xdrstdio_creation_data*)data;
  if (xdrstdio_data->finish_guard)
    {
      xdrstdio_data->finish_guard = 0;
      XDR_DESTROY (xdrs);
      fclose (xdrstdio_data->f);
    }
  return TRUE;
}
#ifdef _MSC_VER
# pragma warning(pop)
#endif

/* deletes test file (if desired) and frees memory for full name */
bool_t
xdrstdio_test_teardown_cb (void * data)
{
  xdrstdio_creation_data* xdrstdio_data = (xdrstdio_creation_data*)data;
  if (!xdrstdio_data->o->keep)
    {
      if (unlink (xdrstdio_data->fullname) != 0)
        {
          log_msg (xdrstdio_data->o->log, XDR_LOG_NORMAL,
                   "Could not delete %s\n", xdrstdio_data->fullname);
        }
    }
  free (xdrstdio_data->fullname);
  return TRUE;
}


static xdr_stream_ops xdrstdio_stream_ops =
{
  xdrstdio_test_setup_cb,
  xdrstdio_create_cb,
  xdrstdio_finish_cb,
  (xdr_debug_cb)(NULL),
  xdrstdio_test_teardown_cb
};

void init_tmpdir (opts *o)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
  static const char * DEFAULT_TMPDIR = "C:/Temp/xdrstdio_test_XXXXXX";
  static const char * FALLBACK_TMPDIR = "C:/Temp";
#else
  static const char * DEFAULT_TMPDIR = "/tmp/xdrstdio_test_XXXXXX";
  static const char * FALLBACK_TMPDIR = "/tmp";
#endif
  char * p;

  if (o->tmpdir)
    {
      char * p = (char *) malloc (strlen(o->tmpdir));
      strcpy (p, o->tmpdir);
      if (!mkdir_p (p, 0700))
        {
          log_msg(o->log, XDR_LOG_NORMAL,
                  "Couldn't create tmpdir %s; trying %s\n",
                  p, DEFAULT_TMPDIR);
          free (p);
          o->tmpdir = NULL;
        }
      else
        o->tmpdir = p;
        return;
    }

  o->tmpdir = (char *) malloc (strlen(DEFAULT_TMPDIR));
  strcpy (o->tmpdir, DEFAULT_TMPDIR);
  p = mkdtemp(o->tmpdir);
  if (!p)
    {
      log_msg(o->log, XDR_LOG_NORMAL,
              "Couldn't create tmpdir; using %s\n", FALLBACK_TMPDIR);
      strcpy (o->tmpdir, FALLBACK_TMPDIR);
      if (!mkdir_p (o->tmpdir, 0700))
        {
          log_msg(o->log, XDR_LOG_NORMAL,
                  "Couldn't even access %s; using '.'\n", FALLBACK_TMPDIR);
          strcpy (o->tmpdir, ".");
        }
    }
}

void cleanup_tmpdir (opts * o)
{
  if (!o->keep)
    {
      if (rmdir (o->tmpdir) != 0)
        {
          log_msg (o->log, XDR_LOG_NORMAL,
                   "Could not remove directory: %s\n", o->tmpdir);
        }
    }
  else
    {
      log_msg (o->log, XDR_LOG_NORMAL,
               "Temporary files saved in %s\n", o->tmpdir);
    }
  free (o->tmpdir);
}

int
main (int argc, char *argv[])
{
  int c;
  log_opts log;
  opts o;
  bool_t rc = TRUE;

  set_program_name (argv[0]);
  log.level = 0;
  log.f = stderr;
  o.log = &log;
  o.keep = 0;
  o.tmpdir = NULL;

  while ((c = getopt (argc, argv, "hvskt:")) != -1)
    switch (c)
      {
      case 'h':
        usage (stdout, program_name);
        return 0;
        break;
      case 'v':
        o.log->level++;
        break;
      case 's':
        o.log->level = XDR_LOG_SILENT;
        break;
      case 'k':
        o.keep = 1;
        break;
      case 't':
        o.tmpdir = optarg;
        break;
      }

  init_tmpdir (&o);
  log_msg (o.log, XDR_LOG_INFO,
           "Using temp directory '%s'\n", o.tmpdir);

  rc &= test_xdrstdio_int (&o);
  rc &= test_xdrstdio_u_int (&o);

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4127)
#endif
  if (sizeof(long) <= BYTES_PER_XDR_UNIT)
    {
      rc &= test_xdrstdio_long (&o);
      rc &= test_xdrstdio_u_long (&o);
    }
  else
    log_msg (o.log, XDR_LOG_NORMAL,
             "Skipping xdr_long and xdr_u_long tests. These are broken on "
             "64 bit platforms.\n");
#ifdef _MSC_VER
# pragma warning(pop)
#endif

  rc &= test_xdrstdio_short (&o);
  rc &= test_xdrstdio_u_short (&o);
  rc &= test_xdrstdio_char (&o);
  rc &= test_xdrstdio_u_char (&o);
  rc &= test_xdrstdio_int8_t (&o);
  rc &= test_xdrstdio_u_int8_t (&o);
  rc &= test_xdrstdio_uint8_t (&o);
  rc &= test_xdrstdio_int16_t (&o);
  rc &= test_xdrstdio_u_int16_t (&o);
  rc &= test_xdrstdio_uint16_t (&o);
  rc &= test_xdrstdio_int32_t (&o);
  rc &= test_xdrstdio_u_int32_t (&o);
  rc &= test_xdrstdio_uint32_t (&o);
  rc &= test_xdrstdio_int64_t (&o);
  rc &= test_xdrstdio_u_int64_t (&o);
  rc &= test_xdrstdio_uint64_t (&o);
  rc &= test_xdrstdio_hyper (&o);
  rc &= test_xdrstdio_u_hyper (&o);
  rc &= test_xdrstdio_longlong_t (&o);
  rc &= test_xdrstdio_u_longlong_t (&o);
  rc &= test_xdrstdio_float (&o);
  rc &= test_xdrstdio_double (&o);
  rc &= test_xdrstdio_bool (&o);
  rc &= test_xdrstdio_enum (&o);
  rc &= test_xdrstdio_union (&o);
  rc &= test_xdrstdio_opaque (&o);
  rc &= test_xdrstdio_bytes (&o);
  rc &= test_xdrstdio_string (&o);
  rc &= test_xdrstdio_wrapstring (&o);
  rc &= test_xdrstdio_array (&o);
  rc &= test_xdrstdio_vector (&o);
  rc &= test_xdrstdio_reference (&o);
  rc &= test_xdrstdio_pointer (&o);
  rc &= test_xdrstdio_list (&o);
  rc &= test_xdrstdio_primitive_struct (&o);

  if (rc == TRUE)
    log_msg (o.log, XDR_LOG_NORMAL, "All tests passed!\n");
  else
    log_msg (o.log, XDR_LOG_NORMAL, "Some tests failed!\n");

  cleanup_tmpdir (&o);
  return (rc == TRUE ? EXIT_SUCCESS : EXIT_FAILURE);
}

bool_t
test_xdrstdio_int (opts * o)
{
  static const char *testid= "test_xdrstdio_int";
  int i;
  bool_t rv;
  int data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_int";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT_DATA[i];
  rv = test_basic_type_core_xdr_int (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_u_int (opts * o)
{
  static const char *testid= "test_xdrstdio_u_int";
  int i;
  bool_t rv;
  unsigned int data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_u_int";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT_DATA[i];
  rv = test_basic_type_core_xdr_u_int (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_long (opts * o)
{
  static const char *testid= "test_xdrstdio_long";
  int i;
  bool_t rv;
  long data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_long";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = LONG_DATA[i];
  rv = test_basic_type_core_xdr_long (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_u_long (opts * o)
{
  static const char *testid= "test_xdrstdio_u_long";
  int i;
  bool_t rv;
  unsigned long data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_u_long";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = ULONG_DATA[i];
  rv = test_basic_type_core_xdr_u_long (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_short (opts * o)
{
  static const char *testid= "test_xdrstdio_short";
  int i;
  bool_t rv;
  short data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_short";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = SHORT_DATA[i];
  rv = test_basic_type_core_xdr_short (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_u_short (opts * o)
{
  static const char *testid= "test_xdrstdio_u_short";
  int i;
  bool_t rv;
  unsigned short data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_u_short";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = USHORT_DATA[i];
  rv = test_basic_type_core_xdr_u_short (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_char (opts * o)
{
  static const char *testid= "test_xdrstdio_char";
  int i;
  bool_t rv;
  char data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_char";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

#if CHAR_MIN < 0
  /* char is signed */
  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (char)SCHAR_DATA[i];
#else
    /* char is unsigned */
  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (char)UCHAR_DATA[i];
#endif
  rv = test_basic_type_core_xdr_char (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_u_char (opts * o)
{
  static const char *testid= "test_xdrstdio_u_char";
  int i;
  bool_t rv;
  unsigned char data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_u_char";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UCHAR_DATA[i];
  rv = test_basic_type_core_xdr_u_char (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_int8_t (opts * o)
{
  static const char *testid= "test_xdrstdio_int8_t";
  int i;
  bool_t rv;
  int8_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_int8_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT8_DATA[i];
  rv = test_basic_type_core_xdr_int8_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_u_int8_t (opts * o)
{
  static const char *testid= "test_xdrstdio_u_int8_t";
  int i;
  bool_t rv;
  u_int8_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_u_int8_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (u_int8_t)UINT8_DATA[i];
  rv = test_basic_type_core_xdr_u_int8_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_uint8_t (opts * o)
{
  static const char *testid= "test_xdrstdio_uint8_t";
  int i;
  bool_t rv;
  uint8_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_uint8_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT8_DATA[i];
  rv = test_basic_type_core_xdr_uint8_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_int16_t (opts * o)
{
  static const char *testid= "test_xdrstdio_int16_t";
  int i;
  bool_t rv;
  int16_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_int16_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT16_DATA[i];
  rv = test_basic_type_core_xdr_int16_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_u_int16_t (opts * o)
{
  static const char *testid= "test_xdrstdio_u_int16_t";
  int i;
  bool_t rv;
  u_int16_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_u_int16_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (u_int16_t)UINT16_DATA[i];
  rv = test_basic_type_core_xdr_u_int16_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_uint16_t (opts * o)
{
  static const char *testid= "test_xdrstdio_uint16_t";
  int i;
  bool_t rv;
  uint16_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_uint16_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT16_DATA[i];
  rv = test_basic_type_core_xdr_uint16_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_int32_t (opts * o)
{
  static const char *testid= "test_xdrstdio_int32_t";
  int i;
  bool_t rv;
  int32_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_int32_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT32_DATA[i];
  rv = test_basic_type_core_xdr_int32_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_u_int32_t (opts * o)
{
  static const char *testid= "test_xdrstdio_u_int32_t";
  int i;
  bool_t rv;
  u_int32_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_u_int32_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (u_int32_t)UINT32_DATA[i];
  rv = test_basic_type_core_xdr_u_int32_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_uint32_t (opts * o)
{
  static const char *testid= "test_xdrstdio_uint32_t";
  int i;
  bool_t rv;
  uint32_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_uint32_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT32_DATA[i];
  rv = test_basic_type_core_xdr_uint32_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_int64_t (opts * o)
{
  static const char *testid= "test_xdrstdio_int64_t";
  int i;
  bool_t rv;
  int64_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_int64_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT64_DATA[i];
  rv = test_basic_type_core_xdr_int64_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_u_int64_t (opts * o)
{
  static const char *testid= "test_xdrstdio_u_int64_t";
  int i;
  bool_t rv;
  u_int64_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_u_int64_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (u_int64_t)UINT64_DATA[i];
  rv = test_basic_type_core_xdr_u_int64_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_uint64_t (opts * o)
{
  static const char *testid= "test_xdrstdio_uint64_t";
  int i;
  bool_t rv;
  uint64_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_uint64_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT64_DATA[i];
  rv = test_basic_type_core_xdr_uint64_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_hyper (opts * o)
{
  static const char *testid= "test_xdrstdio_hyper";
  int i;
  bool_t rv;
  quad_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_hyper";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = HYPER_DATA[i];
  rv = test_basic_type_core_xdr_hyper (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_u_hyper (opts * o)
{
  static const char *testid= "test_xdrstdio_u_hyper";
  int i;
  bool_t rv;
  u_quad_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_u_hyper";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UHYPER_DATA[i];
  rv = test_basic_type_core_xdr_u_hyper (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_longlong_t (opts * o)
{
  static const char *testid= "test_xdrstdio_longlong_t";
  int i;
  bool_t rv;
  quad_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_longlong_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = LONGLONG_DATA[i];
  rv = test_basic_type_core_xdr_longlong_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_u_longlong_t (opts * o)
{
  static const char *testid= "test_xdrstdio_u_longlong_t";
  int i;
  bool_t rv;
  u_quad_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_u_longlong_t";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = ULONGLONG_DATA[i];
  rv = test_basic_type_core_xdr_u_longlong_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_float (opts * o)
{
  static const char *testid= "test_xdrstdio_float";
  bool_t rv;
  float data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_float";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  init_float_data (data);
  rv = test_basic_type_core_xdr_float (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_double (opts * o)
{
  static const char *testid= "test_xdrstdio_double";
  bool_t rv;
  double data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_double";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  init_double_data (data);
  rv = test_basic_type_core_xdr_double (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_bool (opts * o)
{
  static const char *testid= "test_xdrstdio_bool";
  int i;
  bool_t rv;
  bool_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_bool";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = BOOL_DATA[i];
  rv = test_basic_type_core_xdr_bool (o->log, testid, data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_enum (opts * o)
{
  static const char *testid= "test_xdrstdio_enum";
  int i;
  bool_t rv;
  test_enum_t data[TEST_DATA_SZ];
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_enum";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = ENUM_DATA[i];
  rv = test_basic_type_core_xdr_enum (o->log, testid, (enum_t*) data,
      TEST_DATA_SZ, &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_union (opts * o)
{
  static const char *testid= "test_xdrstdio_union";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_union";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_union (o->log, testid,
    &xdrstdio_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_opaque (opts * o)
{
  static const char *testid= "test_xdrstdio_opaque";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_opaque";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_opaque (o->log, testid,
    &xdrstdio_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_bytes (opts * o)
{
  static const char *testid= "test_xdrstdio_bytes";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_bytes";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_bytes (o->log, testid,
    &xdrstdio_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_string (opts * o)
{
  static const char *testid= "test_xdrstdio_string";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_string";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_string (o->log, testid,
    &xdrstdio_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_wrapstring (opts * o)
{
  static const char *testid= "test_xdrstdio_wrapstring";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_wrapstring";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_wrapstring (o->log, testid,
    &xdrstdio_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_array (opts * o)
{
  static const char *testid= "test_xdrstdio_array";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_array";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_array (o->log, testid,
    &xdrstdio_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_vector (opts * o)
{
  static const char *testid= "test_xdrstdio_vector";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_vector";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_vector (o->log, testid,
    &xdrstdio_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_reference (opts * o)
{
  static const char *testid= "test_xdrstdio_reference";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_reference";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_reference (o->log, testid,
    &xdrstdio_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_pointer (opts * o)
{
  static const char *testid= "test_xdrstdio_pointer";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_pointer";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_list (o->log, testid,
    &xdrstdio_stream_ops, "xdr_pgn_list_t_RECURSIVE",
    xdr_pgn_list_t_RECURSIVE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_list (opts * o)
{
  static const char *testid= "test_xdrstdio_list";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_list";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_list (o->log, testid,
    &xdrstdio_stream_ops, "xdr_pgn_list_t",
    xdr_pgn_list_t, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrstdio_primitive_struct (opts * o)
{
  static const char *testid= "test_xdrstdio_primitive_struct";
  bool_t rv;
  xdrstdio_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.name = "xdrstdio_primitive_struct";
  xdr_data.fullname = NULL;
  xdr_data.f = NULL;

  rv = test_core_xdr_primitive_struct (o->log, testid,
    &xdrstdio_stream_ops, (void *)&xdr_data);
  return rv;
}

