/* xdrmem_test.c - test xdr with memory buffers
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
  fprintf (f, "%s [-h] [-v]\n", progname);
  fputs ("  Tests the memory buffer functionality of the xdr library\n", f);
  fputs ("  on this specific platform. Does not test data interchange\n", f);
  fputs ("  between different platforms.\n", f);
  fputs ("   -h : print this help\n", f);
  fputs ("   -v : verbose mode (may be repeated)\n", f);
  fputs ("   -s : silent mode\n", f);
}

typedef struct _opts
{
  log_opts *log;
} opts;

bool_t test_xdrmem_int (opts *o);
bool_t test_xdrmem_u_int (opts *o);
bool_t test_xdrmem_long (opts *o);
bool_t test_xdrmem_u_long (opts *o);
bool_t test_xdrmem_short (opts *o);
bool_t test_xdrmem_u_short (opts *o);
bool_t test_xdrmem_char (opts *o);
bool_t test_xdrmem_u_char (opts *o);
bool_t test_xdrmem_int8_t (opts *o);
bool_t test_xdrmem_u_int8_t (opts *o);
bool_t test_xdrmem_uint8_t (opts *o);
bool_t test_xdrmem_int16_t (opts *o);
bool_t test_xdrmem_u_int16_t (opts *o);
bool_t test_xdrmem_uint16_t (opts *o);
bool_t test_xdrmem_int32_t (opts *o);
bool_t test_xdrmem_u_int32_t (opts *o);
bool_t test_xdrmem_uint32_t (opts *o);
bool_t test_xdrmem_int64_t (opts *o);
bool_t test_xdrmem_u_int64_t (opts *o);
bool_t test_xdrmem_uint64_t (opts *o);
bool_t test_xdrmem_hyper (opts *o);
bool_t test_xdrmem_u_hyper (opts *o);
bool_t test_xdrmem_longlong_t (opts *o);
bool_t test_xdrmem_u_longlong_t (opts *o);
bool_t test_xdrmem_float (opts *o);
bool_t test_xdrmem_double (opts *o);
bool_t test_xdrmem_bool (opts *o);
bool_t test_xdrmem_enum (opts *o);
bool_t test_xdrmem_union (opts *o);
bool_t test_xdrmem_opaque (opts *o);
bool_t test_xdrmem_bytes (opts *o);
bool_t test_xdrmem_string (opts *o);
bool_t test_xdrmem_wrapstring (opts *o);
bool_t test_xdrmem_array (opts *o);
bool_t test_xdrmem_vector (opts *o);
bool_t test_xdrmem_reference (opts *o);
bool_t test_xdrmem_pointer (opts *o);
bool_t test_xdrmem_list (opts *o);
bool_t test_xdrmem_primitive_struct (opts *o);

/* This is a data struct for the test callback functions */
typedef struct _xdrmem_creation_data {
  opts     *o;
  int      finish_guard;
  char     *buf;
  u_int     buf_sz;
} xdrmem_creation_data;

bool_t
xdrmem_create_cb (XDR * xdrs, enum xdr_op op, void * data)
{
  xdrmem_creation_data* xdrmem_data = (xdrmem_creation_data*)data;
  xdrmem_create (xdrs, xdrmem_data->buf, xdrmem_data->buf_sz, op);
  xdrmem_data->finish_guard = 1;
  return TRUE;
}

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4100)
# pragma warning(disable:4127)
#endif
bool_t
xdrmem_finish_cb (XDR * xdrs, enum xdr_op op, void * data)
{
  xdrmem_creation_data* xdrmem_data = (xdrmem_creation_data*)data;
  if (xdrmem_data->finish_guard)
    {
      xdrmem_data->finish_guard = 0;
      XDR_DESTROY (xdrs);
    }
  return TRUE;
}
#ifdef _MSC_VER
# pragma warning(pop)
#endif


void
xdrmem_debug_cb (void * data)
{
  xdrmem_creation_data* xdrmem_data = (xdrmem_creation_data*)data;
  if (xdrmem_data->o->log->level >= XDR_LOG_DEBUG)
    dumpmem (xdrmem_data->o->log->f,
             xdrmem_data->buf,
             xdrmem_data->buf_sz,
             0);
}

static xdr_stream_ops xdrmem_stream_ops =
{
  (xdr_test_setup_cb)(NULL),
  xdrmem_create_cb,
  xdrmem_finish_cb,
  xdrmem_debug_cb,
  (xdr_test_teardown_cb)(NULL)
};

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

  while ((c = getopt (argc, argv, "hvs")) != -1)
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
      }

  rc &= test_xdrmem_int (&o);
  rc &= test_xdrmem_u_int (&o);

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4127)
#endif
  if (sizeof(long) <= BYTES_PER_XDR_UNIT)
    {
      rc &= test_xdrmem_long (&o);
      rc &= test_xdrmem_u_long (&o);
    }
  else
    log_msg (o.log, XDR_LOG_NORMAL,
             "Skipping xdr_long and xdr_u_long tests. These are broken on "
             "64 bit platforms.\n");
#ifdef _MSC_VER
# pragma warning(pop)
#endif

  rc &= test_xdrmem_short (&o);
  rc &= test_xdrmem_u_short (&o);
  rc &= test_xdrmem_char (&o);
  rc &= test_xdrmem_u_char (&o);
  rc &= test_xdrmem_int8_t (&o);
  rc &= test_xdrmem_u_int8_t (&o);
  rc &= test_xdrmem_uint8_t (&o);
  rc &= test_xdrmem_int16_t (&o);
  rc &= test_xdrmem_u_int16_t (&o);
  rc &= test_xdrmem_uint16_t (&o);
  rc &= test_xdrmem_int32_t (&o);
  rc &= test_xdrmem_u_int32_t (&o);
  rc &= test_xdrmem_uint32_t (&o);
  rc &= test_xdrmem_int64_t (&o);
  rc &= test_xdrmem_u_int64_t (&o);
  rc &= test_xdrmem_uint64_t (&o);
  rc &= test_xdrmem_hyper (&o);
  rc &= test_xdrmem_u_hyper (&o);
  rc &= test_xdrmem_longlong_t (&o);
  rc &= test_xdrmem_u_longlong_t (&o);
  rc &= test_xdrmem_float (&o);
  rc &= test_xdrmem_double (&o);
  rc &= test_xdrmem_bool (&o);
  rc &= test_xdrmem_enum (&o);
  rc &= test_xdrmem_union (&o);
  rc &= test_xdrmem_opaque (&o);
  rc &= test_xdrmem_bytes (&o);
  rc &= test_xdrmem_string (&o);
  rc &= test_xdrmem_wrapstring (&o);
  rc &= test_xdrmem_array (&o);
  rc &= test_xdrmem_vector (&o);
  rc &= test_xdrmem_reference (&o);
  rc &= test_xdrmem_pointer (&o);
  rc &= test_xdrmem_list (&o);
  rc &= test_xdrmem_primitive_struct (&o);

  if (rc == TRUE)
    log_msg (o.log, XDR_LOG_NORMAL, "All tests passed!\n");
  else
    log_msg (o.log, XDR_LOG_NORMAL, "Some tests failed!\n");

  return (rc == TRUE ? EXIT_SUCCESS : EXIT_FAILURE);
}

bool_t
test_xdrmem_int (opts * o)
{
  static const char *testid= "test_xdrmem_int";
  int i;
  char buf[80]; /* 20*min size */
  int data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT_DATA[i];
  return test_basic_type_core_xdr_int (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_u_int (opts * o)
{
  static const char *testid= "test_xdrmem_u_int";
  int i;
  char buf[80]; /* 20*min size */
  unsigned int data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT_DATA[i];
  return test_basic_type_core_xdr_u_int (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_long (opts * o)
{
  static const char *testid= "test_xdrmem_long";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  long data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = LONG_DATA[i];
  return test_basic_type_core_xdr_long (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_u_long (opts * o)
{
  static const char *testid= "test_xdrmem_u_long";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  unsigned long data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = ULONG_DATA[i];
  return test_basic_type_core_xdr_u_long (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_short (opts * o)
{
  static const char *testid= "test_xdrmem_short";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  short data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = SHORT_DATA[i];
  return test_basic_type_core_xdr_short (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_u_short (opts * o)
{
  static const char *testid= "test_xdrmem_u_short";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  unsigned short data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = USHORT_DATA[i];
  return test_basic_type_core_xdr_u_short (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_char (opts * o)
{
  static const char *testid= "test_xdrmem_char";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  char data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

#if CHAR_MIN < 0
  /* char is signed */
  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (char)SCHAR_DATA[i];
#else
    /* char is unsigned */
  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (char)UCHAR_DATA[i];
#endif
  return test_basic_type_core_xdr_char (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_u_char (opts * o)
{
  static const char *testid= "test_xdrmem_u_char";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  unsigned char data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UCHAR_DATA[i];
  return test_basic_type_core_xdr_u_char (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_int8_t (opts * o)
{
  static const char *testid= "test_xdrmem_int8_t";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  int8_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT8_DATA[i];
  return test_basic_type_core_xdr_int8_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_u_int8_t (opts * o)
{
  static const char *testid= "test_xdrmem_u_int8_t";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  u_int8_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (u_int8_t)UINT8_DATA[i];
  return test_basic_type_core_xdr_u_int8_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_uint8_t (opts * o)
{
  static const char *testid= "test_xdrmem_uint8_t";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  uint8_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT8_DATA[i];
  return test_basic_type_core_xdr_uint8_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_int16_t (opts * o)
{
  static const char *testid= "test_xdrmem_int16_t";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  int16_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT16_DATA[i];
  return test_basic_type_core_xdr_int16_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_u_int16_t (opts * o)
{
  static const char *testid= "test_xdrmem_u_int16_t";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  u_int16_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (u_int16_t)UINT16_DATA[i];
  return test_basic_type_core_xdr_u_int16_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_uint16_t (opts * o)
{
  static const char *testid= "test_xdrmem_uint16_t";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  uint16_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT16_DATA[i];
  return test_basic_type_core_xdr_uint16_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_int32_t (opts * o)
{
  static const char *testid= "test_xdrmem_int32_t";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  int32_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT32_DATA[i];
  return test_basic_type_core_xdr_int32_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_u_int32_t (opts * o)
{
  static const char *testid= "test_xdrmem_u_int32_t";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  u_int32_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (u_int32_t)UINT32_DATA[i];
  return test_basic_type_core_xdr_u_int32_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_uint32_t (opts * o)
{
  static const char *testid= "test_xdrmem_uint32_t";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  uint32_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT32_DATA[i];
  return test_basic_type_core_xdr_uint32_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_int64_t (opts * o)
{
  static const char *testid= "test_xdrmem_int64_t";
  int i;
  char buf[160]; /* TEST_DATA_SZ*8 */
  int64_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 160;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = INT64_DATA[i];
  return test_basic_type_core_xdr_int64_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_u_int64_t (opts * o)
{
  static const char *testid= "test_xdrmem_u_int64_t";
  int i;
  char buf[160]; /* TEST_DATA_SZ*8 */
  u_int64_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 160;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = (u_int64_t)UINT64_DATA[i];
  return test_basic_type_core_xdr_u_int64_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_uint64_t (opts * o)
{
  static const char *testid= "test_xdrmem_uint64_t";
  int i;
  char buf[160]; /* TEST_DATA_SZ*8 */
  uint64_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 160;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UINT64_DATA[i];
  return test_basic_type_core_xdr_uint64_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_hyper (opts * o)
{
  static const char *testid= "test_xdrmem_hyper";
  int i;
  char buf[160]; /* TEST_DATA_SZ*8 */
  quad_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 160;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = HYPER_DATA[i];
  return test_basic_type_core_xdr_hyper (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_u_hyper (opts * o)
{
  static const char *testid= "test_xdrmem_u_hyper";
  int i;
  char buf[160]; /* TEST_DATA_SZ*8 */
  u_quad_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 160;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = UHYPER_DATA[i];
  return test_basic_type_core_xdr_u_hyper (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_longlong_t (opts * o)
{
  static const char *testid= "test_xdrmem_longlong_t";
  int i;
  char buf[160]; /* TEST_DATA_SZ*8 */
  quad_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 160;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = LONGLONG_DATA[i];
  return test_basic_type_core_xdr_longlong_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_u_longlong_t (opts * o)
{
  static const char *testid= "test_xdrmem_u_longlong_t";
  int i;
  char buf[160]; /* TEST_DATA_SZ*8 */
  u_quad_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 160;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = ULONGLONG_DATA[i];
  return test_basic_type_core_xdr_u_longlong_t (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_float (opts * o)
{
  static const char *testid= "test_xdrmem_float";
  char buf[80]; /* TEST_DATA_SZ*4 */
  float data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  init_float_data (data);
  return test_basic_type_core_xdr_float (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_double (opts * o)
{
  static const char *testid= "test_xdrmem_double";
  char buf[160]; /* TEST_DATA_SZ*8 */
  double data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 160;

  init_double_data (data);
  return test_basic_type_core_xdr_double (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_bool (opts * o)
{
  static const char *testid= "test_xdrmem_bool";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  bool_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = BOOL_DATA[i];
  return test_basic_type_core_xdr_bool (o->log, testid, data,
      TEST_DATA_SZ, &xdrmem_stream_ops, TRUE, (void *)&xdr_data);
}

bool_t
test_xdrmem_enum (opts * o)
{
  static const char *testid= "test_xdrmem_enum";
  int i;
  char buf[80]; /* TEST_DATA_SZ*min size */
  test_enum_t data[TEST_DATA_SZ];
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 80;

  for (i=0;i<TEST_DATA_SZ;i++) data[i] = ENUM_DATA[i];
  return test_basic_type_core_xdr_enum (o->log, testid, (enum_t*)(void *)data,
                                        TEST_DATA_SZ, &xdrmem_stream_ops, TRUE,
					(void *)&xdr_data);
}

bool_t
test_xdrmem_union (opts * o)
{
  static const char *testid= "test_xdrmem_union";
  /* size of a xdr'ed union depends on what branch
     is active. Therefore, the size of an array of
     unions...is difficult to determine a priori.
     Perhaps using a separate xdr_sizeof phase?
     However, for this test data set we have exactly:
  */
  char buf[188];
  bool_t rv;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = 188;

  rv = test_core_xdr_union (o->log, testid,
    &xdrmem_stream_ops, FALSE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrmem_opaque (opts * o)
{
  static const char *testid= "test_xdrmem_opaque";
  char buf[OPAQUE_DATA_SZ + (BYTES_PER_XDR_UNIT - (OPAQUE_DATA_SZ % BYTES_PER_XDR_UNIT))];
  int buf_sz = OPAQUE_DATA_SZ + (BYTES_PER_XDR_UNIT - (OPAQUE_DATA_SZ % BYTES_PER_XDR_UNIT));
  bool_t rv;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = buf_sz;

  rv = test_core_xdr_opaque (o->log, testid,
    &xdrmem_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrmem_bytes (opts * o)
{
  static const char *testid= "test_xdrmem_bytes";
  char buf[MAX_BYTES_SZ + BYTES_PER_XDR_UNIT];
  int buf_sz = MAX_BYTES_SZ + BYTES_PER_XDR_UNIT;
  bool_t rv;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = buf_sz;

  rv = test_core_xdr_bytes (o->log, testid,
    &xdrmem_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrmem_string (opts * o)
{
  static const char *testid= "test_xdrmem_string";
  char buf[MAX_STRING_SZ + BYTES_PER_XDR_UNIT];
  int buf_sz = MAX_STRING_SZ + BYTES_PER_XDR_UNIT;
  bool_t rv;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = buf_sz;
  memset (buf, 0, buf_sz);

  rv = test_core_xdr_string (o->log, testid,
    &xdrmem_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrmem_wrapstring (opts * o)
{
  static const char *testid= "test_xdrmem_wrapstring";
  char buf[MAX_STRING_SZ + BYTES_PER_XDR_UNIT];
  int buf_sz = MAX_STRING_SZ + BYTES_PER_XDR_UNIT;
  bool_t rv;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = buf_sz;
  memset (buf, 0, buf_sz);

  rv = test_core_xdr_wrapstring (o->log, testid,
    &xdrmem_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrmem_array (opts * o)
{
  static const char *testid= "test_xdrmem_array";
  char buf[BYTES_PER_XDR_UNIT + TEST_DATA_SZ * BYTES_PER_XDR_UNIT];
  int buf_sz = BYTES_PER_XDR_UNIT + TEST_DATA_SZ * BYTES_PER_XDR_UNIT;
  bool_t rv;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = buf_sz;
  memset (buf, 0, buf_sz);

  rv = test_core_xdr_array (o->log, testid,
    &xdrmem_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrmem_vector (opts * o)
{
  static const char *testid= "test_xdrmem_vector";
  char buf[TEST_DATA_SZ * BYTES_PER_XDR_UNIT];
  int buf_sz = TEST_DATA_SZ * BYTES_PER_XDR_UNIT;
  bool_t rv;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = buf_sz;
  memset (buf, 0, buf_sz);

  rv = test_core_xdr_vector (o->log, testid,
    &xdrmem_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrmem_reference (opts * o)
{
  static const char *testid= "test_xdrmem_reference";
  char buf[100];
  int buf_sz = 100;
  bool_t rv;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = buf_sz;
  memset (buf, 0, buf_sz);

  rv = test_core_xdr_reference (o->log, testid,
    &xdrmem_stream_ops, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrmem_pointer (opts * o)
{
  static const char *testid= "test_xdrmem_pointer";
  bool_t rv;
  char buf[1024];
  int buf_sz = 1024;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = buf_sz;
  memset (buf, 0, buf_sz);

  rv = test_core_xdr_list (o->log, testid,
    &xdrmem_stream_ops, "xdr_pgn_list_t_RECURSIVE",
    xdr_pgn_list_t_RECURSIVE, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrmem_list (opts * o)
{
  static const char *testid= "test_xdrmem_list";
  bool_t rv;
  char buf[1024];
  int buf_sz = 1024;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = buf_sz;
  memset (buf, 0, buf_sz);

  rv = test_core_xdr_list (o->log, testid,
    &xdrmem_stream_ops, "xdr_pgn_list_t",
    xdr_pgn_list_t, (void *)&xdr_data);
  return rv;
}

bool_t
test_xdrmem_primitive_struct (opts * o)
{
  static const char *testid= "test_xdrmem_primitive_struct";
  bool_t rv;
  char buf[144];
  int buf_sz = 144;
  xdrmem_creation_data xdr_data;
  xdr_data.o = o;
  xdr_data.finish_guard = 0;
  xdr_data.buf = &(buf[0]);
  xdr_data.buf_sz = buf_sz;
  memset (buf, 0, buf_sz);

  rv = test_core_xdr_primitive_struct (o->log, testid,
    &xdrmem_stream_ops, (void *)&xdr_data);
  return rv;
}

