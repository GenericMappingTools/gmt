/* test_xdrs.h - core test routines for xdr tests. Used by
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

#ifndef _XDR_TEST_XDRS_H
#define _XDR_TEST_XDRS_H

#include <stdio.h>
#include <rpc/types.h>
#include <stdint.h>
#include <inttypes.h>

#include "test_common.h"

XDR_TEST_DECLS_BEGIN

#define TEST_BASIC_TYPE_CORE_FUNCTION_DECL( FUNC, TYPE ) \
bool_t                                         \
test_basic_type_core_##FUNC (log_opts * log,   \
  const char     *testid,                      \
  TYPE           *input_data,                  \
  u_int           data_cnt,                    \
  xdr_stream_ops *stream_ops,                  \
  bool_t          check_for_overflow,          \
  void           *xdr_data)

TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_int, int);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_int, u_int);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_long, long);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_long, unsigned long);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_short, short);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_short, unsigned short);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_char, char);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_char, u_char);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_int8_t, int8_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_int8_t, u_int8_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_uint8_t, uint8_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_int16_t, int16_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_int16_t, u_int16_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_uint16_t, uint16_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_int32_t, int32_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_int32_t, u_int32_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_uint32_t, uint32_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_int64_t, int64_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_int64_t, u_int64_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_uint64_t, uint64_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_hyper, quad_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_hyper, u_quad_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_longlong_t, quad_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_u_longlong_t, u_quad_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_float, float);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_double, double);
/* TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_quadruple, long double) */
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_bool, bool_t);
TEST_BASIC_TYPE_CORE_FUNCTION_DECL (xdr_enum, enum_t);

bool_t
test_core_xdr_union
   (log_opts * log,
    const char           *testid,
    xdr_stream_ops       *stream_ops,
    bool_t                check_for_overflow,
    void                 *xdr_data);

bool_t
test_core_xdr_opaque
   (log_opts * log,
    const char           *testid,
    xdr_stream_ops       *stream_ops,
    void                 *xdr_data);

#define MAX_BYTES_SZ 128u
bool_t
test_core_xdr_bytes
   (log_opts * log,
    const char           *testid,
    xdr_stream_ops       *stream_ops,
    void                 *xdr_data);

#define MAX_STRING_SZ 128u
bool_t
test_core_xdr_string
   (log_opts * log,
    const char           *testid,
    xdr_stream_ops       *stream_ops,
    void                 *xdr_data);

bool_t
test_core_xdr_wrapstring
   (log_opts * log,
    const char           *testid,
    xdr_stream_ops       *stream_ops,
    void                 *xdr_data);

bool_t
test_core_xdr_array
   (log_opts * log,
    const char           *testid,
    xdr_stream_ops       *stream_ops,
    void                 *xdr_data);

bool_t
test_core_xdr_vector
   (log_opts * log,
    const char           *testid,
    xdr_stream_ops       *stream_ops,
    void                 *xdr_data);

bool_t
test_core_xdr_reference
   (log_opts * log,
    const char           *testid,
    xdr_stream_ops       *stream_ops,
    void                 *xdr_data);

bool_t
test_core_xdr_list (log_opts * log,
  const char          *testid,
  xdr_stream_ops      *stream_ops,
  const char          *proc_name,
  bool_t             (*list_proc)(XDR *, pgn_list_t *),
  void                *xdr_data);

bool_t
test_core_xdr_primitive_struct (log_opts * log,
  const char          *testid,
  xdr_stream_ops      *stream_ops,
  void                *xdr_data);

XDR_TEST_DECLS_END
#endif /* _XDR_TEST_COMMON_H */

