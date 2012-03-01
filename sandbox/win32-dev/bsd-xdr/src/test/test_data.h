/* test_data.h - data, data structures, and functions to manipulate
 *               them. Used by xdr tests.
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

#ifndef _XDR_TEST_DATA_H
#define _XDR_TEST_DATA_H

#include <stdio.h>
#include <rpc/types.h>
#include <stdint.h>
#include <inttypes.h>
#include "test_common.h"

typedef enum _test_enum {
  EN_A = 0,
  EN_B = 27,
  EN_C = -15,
  EN_D = 92
} test_enum_t;

typedef union _test_union {
  float     flt;
  double    dbl;
  u_int32_t u32;
  char      c;
  int64_t   i64;
} test_union_t;
typedef enum _test_union_enum {
  TEST_UNION_FLOAT = 0,
  TEST_UNION_DOUBLE,
  TEST_UNION_UI32,
  TEST_UNION_CHAR,
  TEST_UNION_I64
} test_union_enum_t;
typedef struct _test_discrim_union {
  test_union_enum_t type;
  test_union_t      value;
} test_discrim_union_t;
extern const struct xdr_discrim test_union_dscrim[6];

/* struct definitions and helper functions for xdr_reference */
typedef struct gnumbers_ {
  int32_t g_assets;
  int32_t g_liabilities;
} gnumbers_t;
extern bool_t xdr_gnumbers_t (XDR *, gnumbers_t *);
typedef struct pgn_ {
  char       *name;
  gnumbers_t *gnp;
} pgn_t;

/* list definitions and helper functions for xdr_pointer */
struct _pgn_node_t;
typedef struct _pgn_node_t  pgn_node_t;
struct _pgn_node_t {
       pgn_t       pgn;
       pgn_node_t *pgn_next;
};
typedef pgn_node_t* pgn_list_t;

typedef struct _test_struct_of_primitives
{
  int            a;
  unsigned int   b;
  long           c;
  unsigned long  d;
  short          e;
  unsigned short f;
  char           g;
  unsigned char  h;
  int8_t         i;
  u_int8_t       j;
  uint8_t        k;
  int16_t        l;
  u_int16_t      m;
  uint16_t       n;
  int32_t        o;
  u_int32_t      p;
  uint32_t       q;
  int64_t        r;
  u_int64_t      s;
  uint64_t       t;
  quad_t         X_hyper;
  u_quad_t       X_u_hyper;
  quad_t         X_ll;
  u_quad_t       X_ull;
  float          u;
  double         v;
  bool_t         w;
  test_enum_t    x;
} test_struct_of_primitives_t;


#define TEST_DATA_SZ 20
#define OPAQUE_DATA_SZ 61

extern const int                    INT_DATA[TEST_DATA_SZ];
extern const unsigned int          UINT_DATA[TEST_DATA_SZ];
extern const long                  LONG_DATA[TEST_DATA_SZ];
extern const unsigned long        ULONG_DATA[TEST_DATA_SZ];
extern const short                SHORT_DATA[TEST_DATA_SZ];
extern const unsigned short      USHORT_DATA[TEST_DATA_SZ];
extern const signed char          SCHAR_DATA[TEST_DATA_SZ];
extern const unsigned char        UCHAR_DATA[TEST_DATA_SZ];
extern const int8_t                INT8_DATA[TEST_DATA_SZ];
extern const uint8_t              UINT8_DATA[TEST_DATA_SZ];
extern const int16_t              INT16_DATA[TEST_DATA_SZ];
extern const uint16_t            UINT16_DATA[TEST_DATA_SZ];
extern const int32_t              INT32_DATA[TEST_DATA_SZ];
extern const uint32_t            UINT32_DATA[TEST_DATA_SZ];
extern const int64_t              INT64_DATA[TEST_DATA_SZ];
extern const uint64_t            UINT64_DATA[TEST_DATA_SZ];
extern const quad_t               HYPER_DATA[TEST_DATA_SZ];
extern const u_quad_t            UHYPER_DATA[TEST_DATA_SZ];
extern const quad_t            LONGLONG_DATA[TEST_DATA_SZ];
extern const u_quad_t         ULONGLONG_DATA[TEST_DATA_SZ];
extern const float                FLOAT_DATA[TEST_DATA_SZ];
extern const double              DOUBLE_DATA[TEST_DATA_SZ];
extern const bool_t                BOOL_DATA[TEST_DATA_SZ];
extern const test_enum_t           ENUM_DATA[TEST_DATA_SZ];
extern const char                OPAQUE_DATA[OPAQUE_DATA_SZ];
extern       test_discrim_union_t UNION_DATA[TEST_DATA_SZ];
extern const char *               NAMES_DATA[TEST_DATA_SZ];
extern const char *               STRING_DATA;

#define FLT_DATA_PINF_INDEX  5
#define FLT_DATA_NINF_INDEX  6
#define FLT_DATA_NAN_INDEX   7
#define FLT_DATA_NZERO_INDEX 8
extern void init_float_data (float *data);
extern void init_double_data (double *data);

/* functions for dealing with test_discrim_union_t objects */
extern void init_union_data (test_discrim_union_t *data);
extern bool_t encode_union_data (const char *testid, log_opts *o, XDR *xdrs,
                                 int cnt, test_discrim_union_t *v);
extern bool_t decode_union_data (const char *testid, log_opts *o, XDR *xdrs,
                                 int cnt, test_discrim_union_t *v);
extern bool_t compare_union_data (const char *testid, log_opts *o,
                                  test_discrim_union_t *s,
                                  test_discrim_union_t *d);

/* functions for dealing with pgn_t objects */
extern bool_t xdr_pgn_t (XDR *, pgn_t *);
extern void init_pgn_contents (pgn_t *, int);
extern void free_pgn_contents (pgn_t *);
extern void init_pgn (pgn_t **, int);
extern void free_pgn (pgn_t **);
extern void print_pgn (FILE *, pgn_t *);
extern int compare_pgn (pgn_t *, pgn_t *);

/* functions for dealing with pgn_list_t objects */
extern bool_t xdr_pgn_node_t_RECURSIVE (XDR *, pgn_node_t *);
extern bool_t xdr_pgn_list_t_RECURSIVE (XDR *, pgn_list_t *);
extern bool_t xdr_pgn_list_t (XDR *, pgn_list_t *);
void init_pgn_node (pgn_node_t **pgn_node, int cnt);
void init_pgn_list (pgn_list_t *pgn_list);
void free_pgn_node (pgn_node_t *pgn_node);
void free_pgn_list (pgn_list_t *pgn_list);
void print_pgn_list (FILE *, pgn_list_t *);

/* functions for dealing with test_struct_of_primitives_t objects */
bool_t xdr_primitive_struct_t (XDR *, test_struct_of_primitives_t *);
void init_primitive_struct (test_struct_of_primitives_t *);
int  compare_primitive_structs (test_struct_of_primitives_t *,
                                test_struct_of_primitives_t *);
void print_primitive_struct (FILE *, test_struct_of_primitives_t *);

XDR_TEST_DECLS_END
#endif /* _XDR_TEST_DATA_H */

