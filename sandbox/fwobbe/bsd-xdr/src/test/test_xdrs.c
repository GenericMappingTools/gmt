/* test_xdrs.c - core test routines for xdr tests. Used by
 *               both xdrmem and xdrstdio tests.
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
#include <math.h>
#include <float.h>
#include <string.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#include "test_common.h"
#include "test_data.h"
#include "test_xdrs.h"

#if defined(_MSC_VER)
# define isnan _isnan
#endif

#define TEST_BASIC_TYPE_CORE_FUNCTION_DEF( FUNC, TYPE, TYPEFMT ) \
bool_t                                         \
test_basic_type_core_##FUNC (log_opts * log,   \
  const char     *testid,                      \
  TYPE           *input_data,                  \
  u_int           data_cnt,                    \
  xdr_stream_ops *stream_ops,                  \
  bool_t          check_for_overflow,          \
  void           *xdr_data)                    \
{                                              \
  XDR xdr_enc;                                 \
  XDR xdr_dec;                                 \
  u_int cnt;                                   \
  TYPE val;                                    \
  bool_t pass = TRUE;                          \
                                               \
  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);    \
  if (stream_ops->setup_cb)                                         \
    (*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data);                \
                                                                    \
  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);       \
  for (cnt = 0; cnt < data_cnt; cnt++)                              \
    {                                                               \
      log_msg (log, XDR_LOG_DEBUG2,                                 \
               "%s: about to encode (cnt=%u, val=%"                 \
               TYPEFMT ")\n", testid, cnt, input_data[cnt]);        \
      if (!FUNC (&xdr_enc, (TYPE*)&input_data[cnt]))                \
        {                                                           \
          log_msg (log, XDR_LOG_INFO,                               \
                   "%s: failed " #FUNC " XDR_ENCODE (cnt=%u, val=%" \
                   TYPEFMT ")\n", testid, cnt, input_data[cnt]);    \
          pass = FALSE;                                             \
          goto test_##FUNC##_end;                                   \
        }                                                           \
    }                                                               \
\
  if (check_for_overflow)                                           \
    {                                                               \
      if (FUNC (&xdr_enc, (TYPE*)&input_data[0]))                   \
        {                                                           \
          log_msg (log, XDR_LOG_INFO,                               \
                   "%s: unexpected pass " #FUNC " XDR_ENCODE\n",    \
                    testid);                                        \
          pass = FALSE;                                             \
          goto test_##FUNC##_end;                                   \
        }                                                           \
    }                                                               \
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);       \
\
  if (stream_ops->debug_cb)                                         \
    (*(stream_ops->debug_cb))(xdr_data);                            \
\
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);       \
  for (cnt = 0; cnt < data_cnt; cnt++)                              \
    {                                                               \
      log_msg (log, XDR_LOG_DEBUG2,                                 \
               "%s: about to decode (cnt=%u, exp=%"                 \
               TYPEFMT ")\n", testid, cnt, input_data[cnt]);        \
      if (!FUNC (&xdr_dec, &val) ||                                 \
          (val != input_data[cnt] && !isnan ((double)val)))         \
        {                                                           \
          log_msg (log, XDR_LOG_INFO,                               \
                   "%s: failed " #FUNC " XDR_DECODE (cnt=%u, exp=%" \
                   TYPEFMT ", val=%" TYPEFMT ")\n",                 \
                   testid, cnt, input_data[cnt], val);              \
          pass = FALSE;                                             \
          goto test_##FUNC##_end2;                                  \
        }                                                           \
    }                                                               \
\
  if (FUNC (&xdr_dec, &val))                                        \
    {                                                               \
      log_msg (log, XDR_LOG_INFO,                                   \
               "%s: unexpected pass " #FUNC " XDR_DECODE\n",        \
               testid);                                             \
      pass = FALSE;                                                 \
      goto test_##FUNC##_end2;                                      \
    }                                                               \
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);       \
\
test_##FUNC##_end2:                                                 \
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);       \
test_##FUNC##_end:                                                  \
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);       \
  if (stream_ops->teardown_cb)                                      \
    (*(stream_ops->teardown_cb))(xdr_data);                         \
  if (pass == TRUE)                                                 \
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);            \
  else                                                              \
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);            \
\
  return pass;                                                      \
}

TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_int, int, "d")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_int, u_int, "u")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_long, long, "ld")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_long, unsigned long, "lu")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_short, short, "hd")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_short, unsigned short, "hu")
#if CHAR_MIN < 0
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_char, char, "hhd")
#else
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_char, char, "hhu")
#endif
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_char, u_char, "hhu")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_int8_t, int8_t, PRId8)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_int8_t, u_int8_t, PRIu8)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_uint8_t, uint8_t, PRIu8)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_int16_t, int16_t, PRId16)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_int16_t, u_int16_t, PRIu16)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_uint16_t, uint16_t, PRIu16)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_int32_t, int32_t, PRId32)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_int32_t, u_int32_t, PRIu32)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_uint32_t, uint32_t, PRIu32)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_int64_t, int64_t, PRId64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_int64_t, u_int64_t, PRIu64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_uint64_t, uint64_t, PRIu64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_hyper, quad_t, PRId64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_hyper, u_quad_t, PRIu64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_longlong_t, quad_t, PRId64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_u_longlong_t, u_quad_t, PRIu64)
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_float, float, ".8e")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_double, double, ".15e")
/* TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_quadruple, long double, "%Lf") */
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_bool, bool_t, "d")
TEST_BASIC_TYPE_CORE_FUNCTION_DEF (xdr_enum, enum_t, "d")

bool_t
test_core_xdr_union (log_opts * log,
  const char           *testid,
  xdr_stream_ops       *stream_ops,
  bool_t                check_for_overflow,
  void                 *xdr_data)
{
  XDR xdr_enc;
  XDR xdr_dec;
  u_int cnt;
  test_discrim_union_t data[TEST_DATA_SZ];
  u_int data_cnt = TEST_DATA_SZ;
  bool_t pass = TRUE;

  init_union_data (UNION_DATA);
  init_union_data (data);
  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);

  if (stream_ops->setup_cb)
    {
      if (!(*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data))
        {
          pass = FALSE;
          goto test_core_xdr_union_end1;
        }
    }

  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
  for (cnt = 0; cnt < data_cnt; cnt++)
    pass &= encode_union_data (testid, log, &xdr_enc, cnt, &data[cnt]);

  if (pass != TRUE)
    goto test_core_xdr_union_end2; /* encode_union_data already logged */

  if (check_for_overflow)
    {
      if (xdr_union (&xdr_enc, (enum_t *)&(data[0].type),
                     (char *)&(data[0].value), test_union_dscrim, NULL_xdrproc_t))
        {
          log_msg (log, XDR_LOG_INFO,
                   "%s(xdr_union): unexpected pass XDR_ENCODE\n",
                    testid);
          pass = FALSE;
          goto test_core_xdr_union_end2;
        }
    }
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);

  if (stream_ops->debug_cb)
    (*(stream_ops->debug_cb))(xdr_data);

  /* clear contents of data */
  for (cnt = 0; cnt < TEST_DATA_SZ; cnt++)
    {
      data[cnt].value.u32 = 0;
      data[cnt].type = TEST_UNION_UI32;
    }

  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  for (cnt = 0; cnt < data_cnt; cnt++)
    pass &= decode_union_data (testid, log, &xdr_dec, cnt, &data[cnt]);
  if (pass != TRUE)
    goto test_core_xdr_union_end3; /* decode_union_data already logged */


  if (xdr_union (&xdr_dec, (enum_t *)&(data[0].type),
                 (char *)&(data[0].value), test_union_dscrim, NULL_xdrproc_t))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_union): unexpected pass XDR_DECODE\n", testid);
      pass = FALSE;
      goto test_core_xdr_union_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  pass = compare_union_data (testid, log, UNION_DATA, data);

test_core_xdr_union_end3:
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);
test_core_xdr_union_end2:
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
test_core_xdr_union_end1:

  if (stream_ops->teardown_cb)
    (*(stream_ops->teardown_cb))(xdr_data);
  if (pass == TRUE)
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;
}

bool_t
test_core_xdr_opaque (log_opts * log,
  const char           *testid,
  xdr_stream_ops       *stream_ops,
  void                 *xdr_data)
{
  XDR xdr_enc;
  XDR xdr_dec;
  u_int cnt;
  char data[OPAQUE_DATA_SZ];
  u_int data_cnt = OPAQUE_DATA_SZ;
  bool_t pass = TRUE;

  for (cnt = 0; cnt < OPAQUE_DATA_SZ; cnt++)
    data[cnt] = OPAQUE_DATA[cnt];
  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);

  if (stream_ops->setup_cb)
    {
      if (!(*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data))
        {
          pass = FALSE;
          goto test_core_xdr_opaque_end1;
        }
    }

  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
  if (!xdr_opaque (&xdr_enc, data, data_cnt))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_opaque): failed XDR_ENCODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_opaque_end2;
    }
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);

  if (stream_ops->debug_cb)
    (*(stream_ops->debug_cb))(xdr_data);

  /* clear contents of data */
  for (cnt = 0; cnt < OPAQUE_DATA_SZ; cnt++)
    data[cnt] = 0;

  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
   if (!xdr_opaque (&xdr_dec, data, data_cnt))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_opaque): failed XDR_DECODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_opaque_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  for (cnt = 0; cnt < OPAQUE_DATA_SZ && pass == TRUE; cnt++)
    {
      pass &= (data[cnt] == OPAQUE_DATA[cnt]);
      if (pass != TRUE)
        {
          log_msg (log, XDR_LOG_INFO,
                   "%s(xdr_opaque): failed compare: (cnt=%d, exp=0x%02x, val=0x%02x)\n",
                   testid, cnt, OPAQUE_DATA[cnt], data[cnt]);
          pass = FALSE;
          goto test_core_xdr_opaque_end3;
        }
    }

test_core_xdr_opaque_end3:
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);
test_core_xdr_opaque_end2:
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
test_core_xdr_opaque_end1:

  if (stream_ops->teardown_cb)
    (*(stream_ops->teardown_cb))(xdr_data);
  if (pass == TRUE)
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;
}

bool_t
test_core_xdr_bytes (log_opts * log,
  const char           *testid,
  xdr_stream_ops       *stream_ops,
  void                 *xdr_data)
{
  XDR xdr_enc;
  XDR xdr_dec;
  u_int cnt;
  char *data = NULL;
  char *p = NULL;
  u_int data_cnt = OPAQUE_DATA_SZ;
  bool_t pass = TRUE;

  data = (char *) malloc (data_cnt * sizeof(char));
  for (cnt = 0; cnt < OPAQUE_DATA_SZ; cnt++)
    data[cnt] = OPAQUE_DATA[cnt];

  for (cnt = 0; cnt < OPAQUE_DATA_SZ; cnt++)
    data[cnt] = OPAQUE_DATA[cnt];
  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);

  if (stream_ops->setup_cb)
    {
      if (!(*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data))
        {
          pass = FALSE;
          goto test_core_xdr_bytes_end1;
        }
    }

  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
  if (!xdr_bytes (&xdr_enc, (char **)&data, &data_cnt, MAX_BYTES_SZ))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_bytes): failed XDR_ENCODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_bytes_end2;
    }
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);

  if (stream_ops->debug_cb)
    (*(stream_ops->debug_cb))(xdr_data);

  /* clear contents of data */
  for (cnt = 0; cnt < OPAQUE_DATA_SZ; cnt++)
    data[cnt] = 0;
  data_cnt = 0;

  /* decode into previously allocated buffer */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!xdr_bytes (&xdr_dec, (char **)&data, &data_cnt, MAX_BYTES_SZ))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_bytes): failed XDR_DECODE w/o allocation\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_bytes_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  /* check decoded size */
  if (data_cnt != OPAQUE_DATA_SZ)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_bytes): failed XDR_DECODE size check w/o allocation "
               "(size: exp=%d, val=%d)\n",
               testid, OPAQUE_DATA_SZ, data_cnt);
      pass = FALSE;
      goto test_core_xdr_bytes_end3;
    }

  /* check decoded bytes */
  for (cnt = 0; cnt < OPAQUE_DATA_SZ && pass == TRUE; cnt++)
    {
      pass &= (data[cnt] == OPAQUE_DATA[cnt]);
      if (pass != TRUE)
        {
          log_msg (log, XDR_LOG_INFO,
                   "%s(xdr_bytes): failed XDR_DECODE compare w/o allocation: "
                   "(cnt=%d, exp=0x%02x, val=0x%02x)\n",
                   testid, cnt, OPAQUE_DATA[cnt], data[cnt]);
          pass = FALSE;
          goto test_core_xdr_bytes_end3;
        }
    }

  /* decode into newly allocated buffer */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!xdr_bytes (&xdr_dec, (char **)&p, &data_cnt, MAX_BYTES_SZ))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_bytes): failed XDR_DECODE with allocation\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_bytes_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  /* check decoded size */
  if (data_cnt != OPAQUE_DATA_SZ)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_bytes): failed XDR_DECODE size check with allocation "
               "(size: exp=%d, val=%d)\n",
               testid, OPAQUE_DATA_SZ, data_cnt);
      pass = FALSE;
      goto test_core_xdr_bytes_end3;
    }

  /* check decoded bytes */
  for (cnt = 0; cnt < OPAQUE_DATA_SZ && pass == TRUE; cnt++)
    {
      pass &= (p[cnt] == OPAQUE_DATA[cnt]);
      if (pass != TRUE)
        {
          log_msg (log, XDR_LOG_INFO,
                   "%s(xdr_bytes): failed XDR_DECODE compare with allocation: "
                   "(cnt=%d, exp=0x%02x, val=0x%02x)\n",
                   testid, cnt, OPAQUE_DATA[cnt], p[cnt]);
          pass = FALSE;
          goto test_core_xdr_bytes_end3;
        }
    }


test_core_xdr_bytes_end3:
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);
test_core_xdr_bytes_end2:
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
test_core_xdr_bytes_end1:

  if (stream_ops->teardown_cb)
    (*(stream_ops->teardown_cb))(xdr_data);

  if (p)    { free (p); p = NULL; }
  if (data) { free (data); data = NULL; }

  if (pass == TRUE)
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;
}

bool_t
test_core_xdr_string (log_opts * log,
  const char          *testid,
  xdr_stream_ops      *stream_ops,
  void                *xdr_data)
{
  XDR xdr_enc;
  XDR xdr_dec;
  u_int cnt;
  char *data = NULL;
  char *p = NULL;
  bool_t pass = TRUE;

  data = (char *) malloc ((MAX_STRING_SZ + 1) * sizeof(char));
  strncpy (data, STRING_DATA, MAX_STRING_SZ + 1);
  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);

  if (stream_ops->setup_cb)
    {
      if (!(*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data))
        {
          pass = FALSE;
          goto test_core_xdr_string_end1;
        }
    }

  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
  if (!xdr_string (&xdr_enc, &data, MAX_BYTES_SZ))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_string): failed XDR_ENCODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_string_end2;
    }
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);

  if (stream_ops->debug_cb)
    (*(stream_ops->debug_cb))(xdr_data);

  /* clear contents of data */
  for (cnt = 0; cnt < MAX_STRING_SZ + 1; cnt++)
    data[cnt] = 0;

  /* decode into previously allocated buffer */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!xdr_string (&xdr_dec, &data, MAX_STRING_SZ))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_string): failed XDR_DECODE w/o allocation\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_string_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  /* check decoded string*/
  if (strncmp (data, STRING_DATA, MAX_STRING_SZ + 1) != 0)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_string): failed XDR_DECODE compare w/o allocation: "
               "(exp='%s', val='%s')\n",
               testid, STRING_DATA, data);
      pass = FALSE;
      goto test_core_xdr_string_end3;
    }

  /* decode into newly allocated buffer */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!xdr_string (&xdr_dec, &p, MAX_STRING_SZ))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_string): failed XDR_DECODE with allocation\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_string_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  /* check decoded string*/
  if (strncmp (p, STRING_DATA, MAX_STRING_SZ + 1) != 0)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_string): failed XDR_DECODE compare with allocation: "
               "(exp='%s', val='%s')\n",
               testid, STRING_DATA, p);
      pass = FALSE;
      goto test_core_xdr_string_end3;
    }

test_core_xdr_string_end3:
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);
test_core_xdr_string_end2:
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
test_core_xdr_string_end1:

  if (stream_ops->teardown_cb)
    (*(stream_ops->teardown_cb))(xdr_data);

  if (p)    { free (p); p = NULL; }
  if (data) { free (data); data = NULL; }

  if (pass == TRUE)
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;
}

bool_t
test_core_xdr_wrapstring (log_opts * log,
  const char          *testid,
  xdr_stream_ops      *stream_ops,
  void                *xdr_data)
{
  XDR xdr_enc;
  XDR xdr_dec;
  u_int cnt;
  char *data = NULL;
  char *p = NULL;
  bool_t pass = TRUE;

  data = (char *) malloc ((MAX_STRING_SZ + 1) * sizeof(char));
  strncpy (data, STRING_DATA, MAX_STRING_SZ + 1);
  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);

  if (stream_ops->setup_cb)
    {
      if (!(*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data))
        {
          pass = FALSE;
          goto test_core_xdr_wrapstring_end1;
        }
    }

  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
  if (!xdr_wrapstring (&xdr_enc, &data))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_wrapstring): failed XDR_ENCODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_wrapstring_end2;
    }
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);

  if (stream_ops->debug_cb)
    (*(stream_ops->debug_cb))(xdr_data);

  /* clear contents of data */
  for (cnt = 0; cnt < MAX_STRING_SZ + 1; cnt++)
    data[cnt] = 0;

  /* decode into previously allocated buffer */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!xdr_wrapstring (&xdr_dec, &data))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_wrapstring): failed XDR_DECODE w/o allocation\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_wrapstring_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  /* check decoded wrapstring*/
  if (strncmp (data, STRING_DATA, MAX_STRING_SZ + 1) != 0)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_wrapstring): failed XDR_DECODE compare w/o allocation: "
               "(exp='%s', val='%s')\n",
               testid, STRING_DATA, data);
      pass = FALSE;
      goto test_core_xdr_wrapstring_end3;
    }

  /* decode into newly allocated buffer */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!xdr_wrapstring (&xdr_dec, &p))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_wrapstring): failed XDR_DECODE with allocation\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_wrapstring_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  /* check decoded wrapstring*/
  if (strncmp (p, STRING_DATA, MAX_STRING_SZ + 1) != 0)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_wrapstring): failed XDR_DECODE compare with allocation: "
               "(exp='%s', val='%s')\n",
               testid, STRING_DATA, p);
      pass = FALSE;
      goto test_core_xdr_wrapstring_end3;
    }

test_core_xdr_wrapstring_end3:
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);
test_core_xdr_wrapstring_end2:
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
test_core_xdr_wrapstring_end1:

  if (stream_ops->teardown_cb)
    (*(stream_ops->teardown_cb))(xdr_data);

  if (p)    { free (p); p = NULL; }
  if (data) { free (data); data = NULL; }

  if (pass == TRUE)
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;
}

bool_t
test_core_xdr_array (log_opts * log,
  const char           *testid,
  xdr_stream_ops       *stream_ops,
  void                 *xdr_data)
{
  /* use INT16_DATA as the array data source */
  /* also test XDR_FREE filter */

  XDR xdr_enc;
  XDR xdr_dec;
  XDR xdr_destroy;
  u_int cnt;
  int16_t * data;
  u_int data_cnt = TEST_DATA_SZ;
  int16_t * p = NULL;
  u_int p_cnt = 0;
  bool_t pass = TRUE;

  data = (int16_t *) malloc (data_cnt * sizeof(int16_t));
  for (cnt = 0; cnt < data_cnt; cnt++)
    data[cnt] = INT16_DATA[cnt];
  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);

  if (stream_ops->setup_cb)
    {
      if (!(*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data))
        {
          pass = FALSE;
          goto test_core_xdr_array_end1;
        }
    }

  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
  if (!xdr_array (&xdr_enc, (char **)&data, &data_cnt, TEST_DATA_SZ + 7,
                  sizeof(int16_t), (xdrproc_t)xdr_int16_t))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_array): failed XDR_ENCODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_array_end2;
    }
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);

  if (stream_ops->debug_cb)
    (*(stream_ops->debug_cb))(xdr_data);

  /* decode into allocated buffer */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!xdr_array (&xdr_dec, (char **)&p, &p_cnt, TEST_DATA_SZ + 7,
      sizeof(int16_t), (xdrproc_t)xdr_int16_t))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_array): failed XDR_DECODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_array_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  /* check decoded array */
  if (p_cnt != data_cnt)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_array): failed size compare: (exp=%u, val=%u)\n",
               testid, data_cnt, p_cnt);
      pass = FALSE;
      goto test_core_xdr_array_end3;
    }

  for (cnt = 0; cnt < data_cnt; cnt++)
    {
      if (p[cnt] != data[cnt])
        {
          log_msg (log, XDR_LOG_INFO,
                   "%s(xdr_array): failed compare: (cnt=%u, exp=%"
                   PRId16 ", val=%" PRId16 ")\n",
                   testid, cnt, data[cnt], p[cnt]);
          pass = FALSE;
          goto test_core_xdr_array_end3;
        }
    }

  /* Free allocated buffer; not very efficient in this
   * case, because we deserialize each element from the
   * buffer AGAIN, before freeing the entire array.
   * This makes more sense to do when you have structures
   * that contain pointers. But, here we just verify that
   * it works.
   */
  (*(stream_ops->create_cb))(&xdr_destroy, XDR_FREE, xdr_data);
  if (!xdr_array (&xdr_destroy, (char **)&p, &p_cnt, TEST_DATA_SZ + 7,
                  sizeof(int16_t), (xdrproc_t)xdr_int16_t))
    {
      log_msg (log, XDR_LOG_INFO,
               "%si(xdr_array): failed XDR_FREE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_array_end4;
    }
  if (p != NULL)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_array): XDR_FREE returned success, but did not NULL out ptr\n",
               testid);
      /* assume that data was freed, so null out manually */
      p = NULL;
      pass = FALSE;
      goto test_core_xdr_array_end4;
    }

test_core_xdr_array_end4:
  (*(stream_ops->finish_cb))(&xdr_destroy, XDR_FREE, xdr_data);
test_core_xdr_array_end3:
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);
test_core_xdr_array_end2:
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
test_core_xdr_array_end1:

  if (stream_ops->teardown_cb)
    (*(stream_ops->teardown_cb))(xdr_data);
  if (pass == TRUE)
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;
}

bool_t
test_core_xdr_vector (log_opts * log,
  const char           *testid,
  xdr_stream_ops       *stream_ops,
  void                 *xdr_data)
{
  /* use INT16_DATA as the vector data source */
  /* these have static unfreeable storage, and fixed size */

  XDR xdr_enc;
  XDR xdr_dec;
  u_int cnt;
  int16_t data[TEST_DATA_SZ];
  u_int data_cnt = TEST_DATA_SZ;
  bool_t pass = TRUE;

  for (cnt = 0; cnt < data_cnt; cnt++)
    data[cnt] = INT16_DATA[cnt];
  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);

  if (stream_ops->setup_cb)
    {
      if (!(*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data))
        {
          pass = FALSE;
          goto test_core_xdr_vector_end1;
        }
    }

  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
  if (!xdr_vector (&xdr_enc, (char *)data, data_cnt,
                  sizeof(int16_t), (xdrproc_t)xdr_int16_t))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_vector): failed XDR_ENCODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_vector_end2;
    }
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);

  if (stream_ops->debug_cb)
    (*(stream_ops->debug_cb))(xdr_data);

  /* zero out data */
  for (cnt = 0; cnt < data_cnt; cnt++)
    data[cnt] = 0;

  /* decode into fixed buffer */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!xdr_vector (&xdr_dec, (char *)data, data_cnt,
                   sizeof(int16_t), (xdrproc_t)xdr_int16_t))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_vector): failed XDR_DECODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_vector_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  /* check decoded vector */
  for (cnt = 0; cnt < data_cnt; cnt++)
    {
      if (data[cnt] != INT16_DATA[cnt])
        {
          log_msg (log, XDR_LOG_INFO,
                   "%s(xdr_vector): failed compare: (cnt=%u, exp=%"
                   PRId16 ", val=%" PRId16 ")\n",
                   testid, cnt, INT16_DATA[cnt], data[cnt]);
          pass = FALSE;
          goto test_core_xdr_vector_end3;
        }
    }

test_core_xdr_vector_end3:
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);
test_core_xdr_vector_end2:
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
test_core_xdr_vector_end1:

  if (stream_ops->teardown_cb)
    (*(stream_ops->teardown_cb))(xdr_data);
  if (pass == TRUE)
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;
}

bool_t
test_core_xdr_reference (log_opts * log,
  const char          *testid,
  xdr_stream_ops      *stream_ops,
  void                *xdr_data)
{
  XDR xdr_enc;
  XDR xdr_dec;
  pgn_t *data = NULL;
  pgn_t *p = NULL;
  bool_t pass = TRUE;

  /* initialize input data */
  init_pgn (&data, 0);
  p = (pgn_t *) malloc (sizeof(pgn_t));
  memset (p, 0, sizeof(pgn_t));

  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);

  if (stream_ops->setup_cb)
    {
      if (!(*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data))
        {
          pass = FALSE;
          goto test_core_xdr_reference_end1;
        }
    }

  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
  if (!xdr_pgn_t (&xdr_enc, data))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_pgn_t): failed XDR_ENCODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_reference_end2;
    }
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);

  if (stream_ops->debug_cb)
    (*(stream_ops->debug_cb))(xdr_data);

  /* decode into output variable */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!xdr_pgn_t (&xdr_dec, p))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_pgn_t): failed XDR_DECODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_reference_end3;
    }
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  /* check decoded values */
  if (compare_pgn (p, data) != 0)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_pgn_t): failed data compare.\n",
               testid);
      if (log->level >= XDR_LOG_INFO)
        {
          fprintf (log->f, "Expected: ");
          print_pgn (log->f, data);
          fprintf (log->f, "\nReceived: ");
          print_pgn (log->f, p);
          fprintf (log->f, "\n");
        }
      pass = FALSE;
      goto test_core_xdr_reference_end3;;
    }

test_core_xdr_reference_end3:
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);
test_core_xdr_reference_end2:
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
test_core_xdr_reference_end1:

  if (stream_ops->teardown_cb)
    (*(stream_ops->teardown_cb))(xdr_data);

  if (p)    { free_pgn (&p); p = NULL; }
  if (data) { free_pgn (&data); data = NULL; }

  if (pass == TRUE)
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;
}

bool_t
test_core_xdr_list (log_opts * log,
  const char          *testid,
  xdr_stream_ops      *stream_ops,
  const char          *proc_name,
  bool_t             (*list_proc)(XDR *, pgn_list_t *),
  void                *xdr_data)
{
  /*
   * Same test, called with xdr_pgn_list_t_RECURSIVE() and
   * xdr_pgn_list_t(). The former is relatively inefficient,
   * but explicitly exercises the xdr_pointer primitive.
   * The latter is the "correct" way to serialize a list.
   */
  XDR xdr_enc;
  XDR xdr_dec;
  XDR xdr_destroy;
  u_int cnt;
  u_int pos;
  pgn_list_t data = NULL;
  pgn_list_t p = NULL;
  pgn_node_t *currA;
  pgn_node_t *currB;

  bool_t pass = TRUE;

  /* initialize input data */
  init_pgn_list (&data);


  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
  if (log->level >= XDR_LOG_DEBUG)
    {
      fprintf (log->f, "%s: Linked List Contents (input):\n", testid);
      print_pgn_list (log->f, &data);
    }

  if (stream_ops->setup_cb)
    {
      if (!(*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data))
        {
          pass = FALSE;
          goto test_core_xdr_list_end1;
        }
    }

  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
  if (!(*list_proc)(&xdr_enc, &data))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(%s): failed XDR_ENCODE\n",
               testid, proc_name);
      pass = FALSE;
      goto test_core_xdr_list_end2;
    }
  pos = XDR_GETPOS (&xdr_enc);
  log_msg (log, XDR_LOG_INFO, "%s(%s): used %u bytes\n",
           testid, proc_name, pos);
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);

  if (stream_ops->debug_cb)
    (*(stream_ops->debug_cb))(xdr_data);

  /* decode into output variable */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!(*list_proc)(&xdr_dec, &p))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_pgn_t): failed XDR_DECODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_list_end3;
    }
  pos = XDR_GETPOS (&xdr_dec);
  log_msg (log, XDR_LOG_INFO, "%s(%s): used %u bytes\n",
           testid, proc_name, pos);
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  if (log->level >= XDR_LOG_DEBUG)
    {
      fprintf (log->f, "%s(%s): Linked List Contents (output):\n",
               testid, proc_name);
      print_pgn_list (log->f, &p);
    }

  /* check decoded list */
  currA = data;
  currB = p;
  cnt = 0;
  while (currA && currB)
    {
      if (compare_pgn (&currA->pgn, &currB->pgn) != 0)
        {
          log_msg (log, XDR_LOG_INFO,
                   "%s(%s): failed data compare (element %d).\n",
                   testid, proc_name, cnt);
          if (log->level >= XDR_LOG_INFO)
            {
              fprintf (log->f, "Expected: ");
              print_pgn (log->f, &currA->pgn);
              fprintf (log->f, "\nReceived: ");
              print_pgn (log->f, &currB->pgn);
              fprintf (log->f, "\n");
            }
          pass = FALSE;
          goto test_core_xdr_list_end3;
        }
      currA = currA->pgn_next;
      currB = currB->pgn_next;
      cnt++;
    }
  if ((currA && !currB) || (currB && !currA))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(%s): failed data compare: "
               "# output elements != # input elements).\n",
               testid, proc_name);
      pass = FALSE;
      goto test_core_xdr_list_end3;
    }

  /* Free allocated list. */
  (*(stream_ops->create_cb))(&xdr_destroy, XDR_FREE, xdr_data);
  if (!(*list_proc)(&xdr_destroy, &p))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(%s): failed XDR_FREE\n",
               testid, proc_name);
      pass = FALSE;
      goto test_core_xdr_list_end4;
    }
  pos = XDR_GETPOS (&xdr_destroy);
  log_msg (log, XDR_LOG_INFO, "%s(%s): used %d bytes\n",
           testid, proc_name,pos);
  (*(stream_ops->finish_cb))(&xdr_destroy, XDR_FREE, xdr_data);

  if (p != NULL)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(%s): XDR_FREE returned "
               "success, but did not NULL out ptr\n",
               testid, proc_name);
      /* assume that data was freed, so null out manually */
      p = NULL;
      pass = FALSE;
      goto test_core_xdr_list_end4;
    }

test_core_xdr_list_end4:
  (*(stream_ops->finish_cb))(&xdr_destroy, XDR_FREE, xdr_data);
test_core_xdr_list_end3:
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);
test_core_xdr_list_end2:
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
test_core_xdr_list_end1:

  if (stream_ops->teardown_cb)
    (*(stream_ops->teardown_cb))(xdr_data);

  if (p)    { free_pgn_list (&p); }
  if (data) { free_pgn_list (&data); }

  if (pass == TRUE)
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;
}

bool_t
test_core_xdr_primitive_struct (log_opts * log,
  const char          *testid,
  xdr_stream_ops      *stream_ops,
  void                *xdr_data)
{
  XDR xdr_enc;
  XDR xdr_dec;
  test_struct_of_primitives_t data;
  test_struct_of_primitives_t p;
  u_int pos;
  bool_t pass = TRUE;

  /* initialize input data */
  init_primitive_struct (&data);
  memset (&p, 0, sizeof(test_struct_of_primitives_t));

  log_msg (log, XDR_LOG_DETAIL, "%s: Entering test.\n", testid);
  if (log->level >= XDR_LOG_DEBUG)
    {
      fprintf (log->f, "%s: structure contents (input):\n", testid);
      print_primitive_struct (log->f, &data);
    }

  if (stream_ops->setup_cb)
    {
      if (!(*(stream_ops->setup_cb))(XDR_ENCODE, xdr_data))
        {
          pass = FALSE;
          goto test_core_xdr_primitive_struct_end1;
        }
    }

  (*(stream_ops->create_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
  if (!xdr_primitive_struct_t (&xdr_enc, &data))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_primitive_struct_t): failed XDR_ENCODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_primitive_struct_end2;
    }
  pos = XDR_GETPOS (&xdr_enc);
  log_msg (log, XDR_LOG_INFO,
           "%s(xdr_primitive_struct_t): XDR_ENCODE used %u bytes\n",
           testid, pos);
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);

  if (stream_ops->debug_cb)
    (*(stream_ops->debug_cb))(xdr_data);

  /* decode into output variable */
  (*(stream_ops->create_cb))(&xdr_dec, XDR_DECODE, xdr_data);
  if (!xdr_primitive_struct_t (&xdr_dec, &p))
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_primitive_struct_t): failed XDR_DECODE\n",
               testid);
      pass = FALSE;
      goto test_core_xdr_primitive_struct_end3;
    }
  pos = XDR_GETPOS (&xdr_dec);
  log_msg (log, XDR_LOG_INFO,
           "%s(xdr_primitive_struct_t): XDR_DECODE used %u bytes\n",
           testid, pos);
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);

  if (log->level >= XDR_LOG_DEBUG)
    {
      fprintf (log->f, "%s: structure contents (output):\n", testid);
      print_primitive_struct (log->f, &data);
    }

  /* check decoded values */
  if (compare_primitive_structs (&data, &p) != 0)
    {
      log_msg (log, XDR_LOG_INFO,
               "%s(xdr_primitive_struct_t): failed compare\n",
               testid);
       if (log->level >= XDR_LOG_INFO)
         {
           fprintf (log->f, "Expected:\n");
           print_primitive_struct (log->f, &p);
           fprintf (log->f, "Received:\n");
           print_primitive_struct (log->f, &p);
         }
       pass = FALSE;
       goto test_core_xdr_primitive_struct_end3;
    }

test_core_xdr_primitive_struct_end3:
  (*(stream_ops->finish_cb))(&xdr_dec, XDR_DECODE, xdr_data);
test_core_xdr_primitive_struct_end2:
  (*(stream_ops->finish_cb))(&xdr_enc, XDR_ENCODE, xdr_data);
test_core_xdr_primitive_struct_end1:

  if (stream_ops->teardown_cb)
    (*(stream_ops->teardown_cb))(xdr_data);

  if (pass == TRUE)
    log_msg (log, XDR_LOG_NORMAL, "%s: PASS\n", testid);
  else
    log_msg (log, XDR_LOG_NORMAL, "%s: FAIL\n", testid);

  return pass;
}

