/*
 * transpose_inplace.h
 *
 * Created by Leland Brown on 2011 Jan 17.
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

#ifndef TRANSPOSE_INPLACE_H
#define TRANSPOSE_INPLACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "extern_msc.h"

struct Transpose_Progress_Callback {
    // callback function - return nonzero value to cancel operation:
    int (*callback)(
        float portion_complete, // portion of operation completed so far (0.00 to 1.00)
        void *state);           // copy of state information pointer
    // pointer to optional state information for use by callback() function:
    void *state;
};

// Transposes matrix of nrows x ncols elements to ncols x nrows.
// Returns 0 on success, 1 if a memory allocation error occurred (with matrix unchanged),
// or -1 if canceled via progress callback (leaving matrix corrupted).
EXTERN_MSC int transpose_inplace(
    float *a,           // matrix to transpose (row-major order)
    long   nrows,       // number of rows    in matrix a
    long   ncols,       // number of columns in matrix a
    const struct Transpose_Progress_Callback
          *progress     // optional callback functor for status; NULL for none
);

#ifdef __cplusplus
}
#endif

#endif
