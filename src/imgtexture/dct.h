/*
 * dct.h
 *
 * Created by Leland Brown on 2010 Oct 31.
 *
 * Copyright (c) 2010-2013 Leland Brown.
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
// This file is intended to be a generic interface for DCT functions.
// The functions prototyped here may be implemented with appropriate
// calls to any DCT or FFT library. These implementations should be
// functionally interchangeable as long as they conform to this interface.
//

#ifndef DCT_H
#define DCT_H

#ifdef __cplusplus
extern "C" {
#endif

struct Dct_Plan {
    // This structure must be filled in by calling setup_dcts() and should
    // not be modified by the caller.
    // Do NOT free these pointers - use cleanup_dcts() instead.
    // Note: input and output buffers may be the same.
    void   * dct_buffer;    // internal buffer for use by perform_dcts()
    double * in_data[2];    // input  data buffers for perform_dcts()
    double * out_data[2];   // output data buffers for perform_dcts()
};

// Specifies a DCT operation to be performed one or more times and
// allocates buffers to be used by perform_dcts().
// On return, plan->dct_buffer will be null if a memory allocation error occurred.
struct Dct_Plan setup_dcts(
    int dct_type,   // 1, 2, or 3 (DCT types I, II, III)
    int nelems      // data length for each DCT
);

// Performs two DCTs, each of size nelems (see setup_dcts()).
// Input arrays are plan->in_data[.] and output arrays are plan->out_data[.].
// Note: input buffers may be overwritten, even if output buffers are different.
// Values in both data buffers must have similar magnitude to avoid roundoff error;
// for a single DCT, fill both buffers with the same values, or one with zeroes.
void perform_dcts(const struct Dct_Plan *plan);     // from setup_dcts()

// Frees memory allocated by setup_dcts().
void cleanup_dcts(struct Dct_Plan *plan);   // from setup_dcts()

#ifdef __cplusplus
}
#endif

#endif
