/*
 * dct_fftpack.c
 *
 * Created by Leland Brown on 2011 Feb 23.
 *
 * Copyright (c) 2011-2013 Leland Brown.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// This file provides a particular implementation of the functions in dct.h,
// in this case using the DCT functions declared in myfftpack.h. Similar
// implementations are possible using other DCT or FFT libraries, if desired.
// These implementations should be functionally interchangeable as long as
// they conform to the interface in dct.h.
//

#include "dct.h"
#include "fftpack.h"

#include <stdlib.h>
#include <assert.h>

static const int max_factors = 30;

struct Dct_Buffer{
    int     dct_type;   // 1 to 3 (DCT types I to III)
    int     nelems;     // length of inout_data0 and inout_data1 buffers
    double *inout_data0;// input/output buffer space
    double *inout_data1;// input/output buffer space
    double *wsave;      // workspace buffer
    int    *ifac;       // info on factorization of nelems
};

struct Dct_Plan setup_dcts(
    int dct_type,   // 1, 2, or 3 (DCT types I, II, III)
    int nelems      // data length for each DCT
) {
// Specifies a DCT operation to be performed one or more times and
// allocates buffers to be used by perform_dcts().
// On return, plan->dct_buffer will be null if a memory allocation error occurred.
    const int max_ifac = (int)( 1.8 * max_factors + 6.9 );
    
    struct Dct_Plan plan;
    struct Dct_Buffer *buf;
    double *data;

    plan.dct_buffer  = NULL;
    plan.in_data[0]  = NULL;
    plan.in_data[1]  = NULL;
    plan.out_data[0] = NULL;
    plan.out_data[1] = NULL;
    
    buf = (struct Dct_Buffer *)malloc( sizeof( struct Dct_Buffer ) );
    if (!buf)
        return plan;

    data = (double *)malloc(30 * nelems * sizeof( double ) + max_ifac * sizeof( int ));
    if (!data) {
        free( buf );
        return plan;
    }
    

    buf->dct_type = dct_type;
    buf->nelems   = nelems;
    
    buf->inout_data0 =  data;
    buf->inout_data1 =  data + nelems;
    buf->wsave =        data + nelems * 2;
    buf->ifac = (int *)(data + nelems * 30);
    
    switch (dct_type) {
    //  case 1:
    //  //  costi( nelems, buf->wsave, buf->ifac );
    //      break;
        case 2: case 3:
            cosqi(nelems, buf->wsave, buf->ifac);
            break;
        default:
            assert(0);    // illegal or unsupported dct_type
    }
    
    assert(buf->ifac[1] <= max_factors);

    plan.dct_buffer  = (void *)buf;
    plan.in_data[0]  = buf->inout_data0;
    plan.in_data[1]  = buf->inout_data1;
    plan.out_data[0] = buf->inout_data0; // in-place transforms
    plan.out_data[1] = buf->inout_data1; // in-place transforms
    return plan;
}

void perform_dcts(const struct Dct_Plan *plan) { // from setup_dcts()
// Performs two DCTs, each of size nelems (see setup_dcts()).
// Input arrays are plan->in_data[.] and output arrays are plan->out_data[.].
// Note: input buffers may be overwritten, even if output buffers are different.
// Values in both data buffers must have similar magnitude to avoid roundoff error;
// for a single DCT, fill both buffers with the same values, or one with zeroes.
    struct Dct_Buffer *buf = (struct Dct_Buffer *)(plan->dct_buffer);

    // verify that caller has not altered these pointers
    assert(plan->in_data[0]  == buf->inout_data0);
    assert(plan->in_data[1]  == buf->inout_data1);
    assert(plan->out_data[0] == buf->inout_data0);  // in-place transform
    assert(plan->out_data[1] == buf->inout_data1);  // in-place transform
    
    switch (buf->dct_type) {
    //  case 1:
    //  //  cost(  buf->nelems, buf->inout_data0, buf->wsave, buf->ifac );
    //  //  cost(  buf->nelems, buf->inout_data1, buf->wsave, buf->ifac );
    //      break;
        case 2:
            cosqb2(buf->nelems, buf->inout_data0, buf->inout_data1, buf->wsave, buf->ifac);
            break;
        case 3:
            cosqf2(buf->nelems, buf->inout_data0, buf->inout_data1, buf->wsave, buf->ifac);
            break;
        default:
            assert( 0 );    // illegal or unsupported dct_type
    }
}

void cleanup_dcts(struct Dct_Plan *plan) {   // from setup_dcts()
// Frees memory allocated by setup_dcts().
    struct Dct_Buffer *buf = (struct Dct_Buffer *)(plan->dct_buffer);
    
    // verify that caller has not altered these pointers
    assert(plan->in_data[0]  == buf->inout_data0);
    assert(plan->in_data[1]  == buf->inout_data1);
    assert(plan->out_data[0] == buf->inout_data0);
    assert(plan->out_data[1] == buf->inout_data1);
    
    free(buf->inout_data0);
    free(buf);
    
    plan->in_data[0]  = NULL;
    plan->in_data[1]  = NULL;
    plan->out_data[1] = NULL;
    plan->out_data[0] = NULL;
    plan->dct_buffer  = NULL;
}
