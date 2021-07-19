/*
 * transpose_inplace.c
 *
 * Created by Leland Brown on 2010 Dec 30.
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

#include "transpose_inplace.h"
#include "compatibility.h"

#include <stddef.h> // for ptrdiff_t
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#ifndef _WIN32
#   include <stdint.h>      // intptr_t
#   include <unistd.h>      // sysconf(), getpagesize()
#   include <sys/types.h>
#   include <sys/mman.h>    // posix_madvise()
#endif

// For a 64-bit compile we need LONG to be 64 bits, even if the compiler uses an LLP64 model
#define LONG ptrdiff_t


// Compilation options - algorithm choices:

//#define ALGORITHM_ISOLATE 1
#define ALGORITHM_ISOLATE 2

//#define ALGORITHM_MERGE 1
#define ALGORITHM_MERGE 2

//#define SIMPLE_CASES_SPECIAL 1
#define SIMPLE_CASES_SPECIAL 0


#ifdef _WIN32
//  static INLINE void madv_dontneed( const void *addr, size_t len ) { }
#   define madv_dontneed( addr, len )
#else
	// Tells the virtual memory system that the these pages will not be needed soon and so
	// their physical memory should have highest priority for being swapped out if needed.
	// This prevents other pages, which may be needed sooner, from being swapped out
	// unnecessarily and so can greatly reduce page thrashing in certain situations.
	static INLINE void madv_dontneed( const void *addr, size_t len ) {
	//  const intptr_t pagemask = getpagesize() - 1;
		const intptr_t pagemask = sysconf( _SC_PAGESIZE ) - 1;
		// round start and finish inward to nearest page boundaries
		intptr_t start  = (((intptr_t)addr - 1)   | pagemask) + 1;
		intptr_t finish = (((intptr_t)addr + len) | pagemask) - pagemask;
		intptr_t diff   = finish - start;
		if (diff > 0) { // protect against negative length
			posix_madvise( (void *)start, diff, POSIX_MADV_DONTNEED );
		}
	}
#endif

struct Transpose_Progress_Info {
	const struct Transpose_Progress_Callback
		 *progress;     // progress callback functor
	LONG  moves_done;   // number of block moves done
	float per_move;     // overall portion complete per block move
};

static INLINE int report_progress( const struct Transpose_Progress_Info *info ) {
	return info->progress->callback((float)info->moves_done * info->per_move,
		info->progress->state );
}

static int transpose_inplace_small( float *a, long nrows, long ncols);
static void transpose_outplace(const void *RESTRICT a, void *RESTRICT b, long nrows, long ncols);
static int check_bands_inplace(long nrows, long ncols, int nbands);
static int choose_bands_inplace(long nrows, long ncols);

typedef struct {
	char *addr;
	long  size;
} Band_Info;

static int setup_bands(
	void *a, void *b, long nrows, long ncols, int pbands, int qbands,
	void **bufin, void **bufout, Band_Info *RESTRICT *pinfo, Band_Info *RESTRICT *qinfo );

static int isolate_blocks(
	void *a, const Band_Info *RESTRICT pinfo, const Band_Info *RESTRICT qinfo,
	long nrows, long ncols, int nbands,
	struct Transpose_Progress_Info *progress_info );

static int transpose_blocks(
	const Band_Info *RESTRICT pinfo, const Band_Info *RESTRICT qinfo, int nbands,
	struct Transpose_Progress_Info *progress_info );

static int merge_blocks(
	void *a, const Band_Info *RESTRICT pinfo, const Band_Info *RESTRICT qinfo,
	long nrows, long ncols, int nbands,
	struct Transpose_Progress_Info *progress_info );


int transpose_inplace(
	float *a,           // matrix to transpose (row-major order)
	long   nrows,       // number of rows    in matrix a
	long   ncols,       // number of columns in matrix a
	const struct Transpose_Progress_Callback
		  *progress     // optional callback functor for status; NULL for none
) {
// Transposes matrix of nrows x ncols elements to ncols x nrows.
// Returns 0 on success, 1 if a memory allocation error occurred (with matrix unchanged),
// or -1 if canceled via progress callback (leaving matrix corrupted).
	int error;
	int cancel = 0;

	void               *bufin  = NULL;
	void               *bufout = NULL;
	Band_Info *RESTRICT pinfo  = NULL;
	Band_Info *RESTRICT qinfo  = NULL;

	struct Transpose_Progress_Info  progress_info;
	struct Transpose_Progress_Info *progress_ptr = NULL;

	int nbands = choose_bands_inplace( nrows, ncols );

	if (nbands == 0)
		return 0;   // no work to do (nrows <= 1 or ncols <= 1)

	if (nbands <= 2)
		return transpose_inplace_small( a, nrows, ncols );

	error = setup_bands(a, a, nrows, ncols, nbands, nbands, &bufin, &bufout, &pinfo, &qinfo);
	if (error) return 1;
	
	if (progress) {
		progress_info.progress = progress;
		progress_info.per_move = (float)(2.0 / (float)( 7 * (LONG)nbands * (LONG)nbands - (LONG)nbands));
		progress_info.moves_done = 0;
		progress_ptr = &progress_info;
	}

	cancel = cancel || isolate_blocks( a, pinfo, qinfo, nrows, ncols, nbands, progress_ptr );
	cancel = cancel || transpose_blocks( pinfo, qinfo, nbands, progress_ptr );
	
	free(bufin);
//  pinfo[nbands-1].addr = NULL;

	cancel = cancel || merge_blocks( a, pinfo, qinfo, nrows, ncols, nbands, progress_ptr );
	
	free(bufout);
	free(pinfo);
	free(qinfo);
	
	if (cancel) return -1;

	return 0;
}

static void transpose_outplace(const void *RESTRICT a, void *RESTRICT b, long nrows, long ncols) {
	const float *RESTRICT fromaddr = (const float *)a;
		  float *RESTRICT toaddr   =       (float *)b;
	long i, j;
	// write each new row sequentially, gather from columns
	for (j=0; j<ncols; ++j) {
		const float *RESTRICT fromcol = fromaddr + j;
		for (i=0; i<nrows; ++i) {
			*toaddr++ = *fromcol;
			fromcol  += ncols;
		}
	}
}

static int transpose_inplace_small( float *a, long nrows, long ncols ) {
	LONG totalsize = (LONG)nrows * (LONG)ncols * sizeof( float );
	float *RESTRICT b = (float *)malloc( totalsize );
	
	if (!b) return 1;
	
	transpose_outplace( a, b, nrows, ncols );
	memcpy( a, b, totalsize );
	free( b );
		
	return 0;
}

static int choose_bands_inplace( long nrows, long ncols ) {
// returns 0 if nrows <= 1 or ncols <= 1
	int nbands;
	long maxdim, mindim;

	if (nrows <= ncols) {
		maxdim = ncols;
		mindim = nrows;
	}
	else {
		maxdim = nrows;
		mindim = ncols;
	}
	
	if (mindim <= 1) {
		return 0;
#if SIMPLE_CASES_SPECIAL
	} else if (maxdim <= 6) {   // or 8 or 12 or 16
		return 1;	// simple cases
	} else if (mindim <= 6) {
		return mindim;
//  } else if (mindim * mindim <= 8 * maxdim) {
	} else if (mindim * mindim <= 4 * maxdim) {
		return mindim;
#endif
	}
	
//  double half_harmonic = (double)(nrows * ncols) / (double)(nrows + ncols);
//  return (int)floor( sqrt( 8.0 * half_harmonic ) );

	nbands = (int)floor( 2.0 * sqrt((double)maxdim) );
	for (; nbands>4; --nbands) {    // nbands <= 4 always works
		if (check_bands_inplace( nrows, ncols, nbands ))
			break;
	}
//  assert( nbands * nbands >= maxdim );
	return nbands;
}

static int check_bands_inplace( long nrows, long ncols, int nbands ) {
// This is a sufficient (but not always necessary) condition for transpose_blocks to work correctly.
// Returns 1 if the condition passes, 0 if it fails.
	int  prem0, qrem0, prem1, qrem1;
	LONG limit;

	if (nbands <= 4) return 1;

	prem0 = nrows % nbands;
	qrem0 = ncols % nbands;
	prem1 = nbands - prem0;
	qrem1 = nbands - qrem0;
	
	limit = 2 * (LONG)nrows * (LONG)ncols;
	
	if (prem1 >= qrem0) {   // qrem1 >= prem0
		LONG temp1 = (LONG)nrows * qrem0;
		LONG temp2 = (LONG)ncols * prem0;
		if (temp2 >= temp1)
			return temp1 * prem0 + temp2 * prem1 <= limit;  // (temp1 + temp4) * prem0 <= limit
		else
			return temp1 * qrem1 + temp2 * qrem0 <= limit;  // (temp3 + temp2) * qrem0 <= limit
	}
	else {
		LONG temp3 = (LONG)nrows * qrem1;
		LONG temp4 = (LONG)ncols * prem1;
		if (temp4 >= temp3)
			return temp3 * prem1 + temp4 * prem0 <= limit;  // (temp3 + temp2) * prem1 <= limit
		else
			return temp3 * qrem0 + temp4 * qrem1 <= limit;  // (temp1 + temp4) * qrem1 <= limit
	}

//  LONG temp5 = (LONG)nrows * qrem0 + (LONG)ncols * prem1; // = temp1 + temp4
//  LONG temp6 = (LONG)nrows * qrem1 + (LONG)ncols * prem0; // = temp3 + temp2
//  return
//      ( temp5 * prem0 <= limit && temp6 * qrem0 <= limit ) ||
//      ( temp6 * prem1 <= limit && temp5 * qrem1 <= limit );
}

static int setup_bands(void *a, void *b, long nrows, long ncols, int pbands, int qbands,
	void **bufin, void **bufout, Band_Info *RESTRICT *pinfo, Band_Info *RESTRICT *qinfo) { 
// Allocates *bufin, *bufout, *pinfo, and *qinfo.
// Returns 0 on success, 1 if a memory allocation error occurred.
// Arrays a and b must be aligned on 4-byte boundaries
// Caller MUST free *bufin, *bufout, *pinfo, and *qinfo later!
	const LONG elemsize = sizeof( float );

	int i, j;

	long pmin = (nrows-1) / pbands; // integer division
	long qmin = (ncols-1) / qbands; // integer division

	long pmax = pmin + 1;   // nrows - pbands <= pmin * pbands < nrows <= pmax * pbands < nrows + pbands
	long qmax = qmin + 1;   // ncols - qbands <= qmin * qbands < ncols <= qmax * qbands < ncols + qbands

	int  prem = pmax * pbands - nrows;  // 0 <= prem <  pbands; # of pmin's
	int  qrem = ncols - qmin * qbands;  // 0 <  qrem <= qbands; # of qmax's

	// blocks [0   ..prem-1  ,0   ..qrem-1]   have size pmin*qmax
	// blocks [0   ..prem-1  ,qrem..qbands-1] have size pmin*qmin
	// blocks [prem..pbands-1,0   ..qrem-1]   have size pmax*qmax
	// blocks [prem..pbands-1,qrem..qbands-1] have size pmax*qmin

	LONG colsize = nrows * elemsize;
	LONG rowsize = ncols * elemsize;
	long offset;
	
	if (!*bufin)
		*bufin  = malloc( rowsize * pmax );

	if (!*bufout)
		*bufout = malloc( colsize * qmax );

	if (!*pinfo)
		*pinfo = (Band_Info *)malloc( pbands * sizeof(Band_Info) );

	if (!*qinfo)
		*qinfo = (Band_Info *)malloc( qbands * sizeof(Band_Info) );

	
	if (*bufin && *bufout && *pinfo && *qinfo) {
		Band_Info *RESTRICT ptemp = *pinfo;
		Band_Info *RESTRICT qtemp = *qinfo;

		for (i=0; i<prem; ++i)
			ptemp[i].size = pmin;

		for (; i<pbands; ++i)
			ptemp[i].size = pmax;

		offset = 0;
		for (i=0; i<pbands-1;) {
			ptemp[i].addr = (char *)a + rowsize * (offset + pmax);
			offset += ptemp[i++].size;
		}
		ptemp[pbands-1].addr = *bufin;
		
		for (j=0; j<qrem; ++j)
			qtemp[j].size = qmax;

		for (; j<qbands; ++j)
			qtemp[j].size = qmin;

		offset = 0;
		qtemp[0].addr = *bufout;
		for (j=0; j<qbands-1;) {
			offset += qtemp[j++].size;
			qtemp[j].addr = (char *)b + colsize * (offset - qmax);
		}
		
		return 0;
	}

	free(*bufin);  *bufin  = NULL;
	free(*bufout); *bufout = NULL;
	free(*pinfo);  *pinfo  = NULL;
	free(*qinfo);  *qinfo  = NULL;

	return 1;
}

static int isolate_blocks(void *a, const Band_Info *RESTRICT pinfo, const Band_Info *RESTRICT qinfo,
                          long nrows, long ncols, int nbands, struct Transpose_Progress_Info *progress_info ) {
	const LONG elemsize = sizeof( float );

	int  i, j;
	long k;
	
	LONG rowsize = ncols * elemsize;

	char *fromband = (char *)a + nrows * rowsize;
	for (i=nbands-1; i>=0; --i) {
		char *toaddr   = pinfo[i].addr;
		LONG  bandsize = pinfo[i].size * rowsize;
		fromband -= bandsize;
		if (pinfo[i].size > 1) {
			if (ALGORITHM_ISOLATE == 1) {	// write each block sequentially, gathering from rows
				const char *fromblock = fromband;
				for (j=0; j<nbands; ++j) {
					LONG qsize = qinfo[j].size * elemsize;
					const char *fromaddr = fromblock;
					char       *toblock  = toaddr;
					for (k=0; k<pinfo[i].size; ++k) {
						memcpy( toaddr, fromaddr, qsize );
						toaddr   += qsize;
						fromaddr += rowsize;
					}
					madv_dontneed( toblock, pinfo[i].size * qsize );
					fromblock += qsize;
			
					if (progress_info) {
						++progress_info->moves_done;
						if (report_progress( progress_info ))
							return 1;
					}
				}
				
			}
			else {		// read each row sequentially, distribute to blocks
				char *fromaddr = fromband;
				// CONCURRENCY NOTE: The iterations of this loop (over k)
				// will be independent and can be executed in parallel,
				// if each thread has its own fromaddr initialized
				// as in the comment below.
				for (k=0; k<pinfo[i].size; ++k) {
					//char *fromaddr = fromband + k * rowsize;  // for concurrency
					char *toblock = toaddr;
					for (j=0; j<nbands; ++j) {
						LONG qsize = qinfo[j].size * elemsize;
						memcpy( toblock + qsize * k, fromaddr, qsize );
						toblock  += qsize * pinfo[i].size;
						fromaddr += qsize;
					}
				}
				madv_dontneed( toaddr, bandsize );
			
				if (progress_info) {
					progress_info->moves_done += nbands;
					if (report_progress( progress_info )) return 1;
				}
			}
				
		}
		else if (pinfo[i].size > 0) {
			memcpy( toaddr, fromband, bandsize );
			madv_dontneed( toaddr, bandsize );
		}
	}
	
	return 0;
}

static int merge_blocks(void *a, const Band_Info *RESTRICT pinfo, const Band_Info *RESTRICT qinfo,
                        long nrows, long ncols, int nbands, struct Transpose_Progress_Info *progress_info ) {
	const LONG elemsize = sizeof( float );
	LONG colsize = nrows * elemsize;
	int  i, j;
	long k;
	char *toband = (char *)a + ncols * colsize;

	for (j=nbands-1; j>=0; --j) {
		char *fromaddr = qinfo[j].addr;
		LONG  bandsize = qinfo[j].size * colsize;
		toband -= bandsize;
		if (qinfo[j].size > 1) {
			if (ALGORITHM_MERGE == 1) {	// read each block sequentially, distribute to new rows
				char *toblock = toband;
				for (i=0; i<nbands; ++i) {
					LONG  psize  = pinfo[i].size * elemsize;
					char *toaddr = toblock;
					for (k=0; k<qinfo[j].size; ++k) {
						memcpy( toaddr, fromaddr, psize );
						fromaddr += psize;
						toaddr   += colsize;
					}
					toblock += psize;
			
					if (progress_info) {
						++progress_info->moves_done;
						if (report_progress( progress_info )) return 1;
					}
				}
				madv_dontneed( toband, bandsize );
				
			}
			else {	// write each new row sequentially, gathering from blocks
				char *toaddr = toband;
				// CONCURRENCY NOTE: The iterations of this loop (over k)
				// will be independent and can be executed in parallel,
				// if each thread has its own toaddr initialized
				// as in the comment below.
				for (k=0; k<qinfo[j].size; ++k) {
					//char *toaddr = toband + k * colsize;  // for concurrency
					const char *fromblock = fromaddr;
					char       *torow     = toaddr;
					for (i=0; i<nbands; ++i) {
						LONG psize = pinfo[i].size * elemsize;
						memcpy( toaddr, fromblock + psize * k, psize );
						fromblock += psize * qinfo[j].size;
						toaddr    += psize;
					}
					madv_dontneed( torow, colsize );
				}
			
				if (progress_info) {
					progress_info->moves_done += nbands;
					if (report_progress( progress_info )) return 1;
				}
			}

		}
		else if (qinfo[j].size > 0) {
			memcpy( toband, fromaddr, bandsize );
			madv_dontneed( toband, bandsize );
		}
	}
	
	return 0;
}

static int transpose_blocks(const Band_Info *RESTRICT pinfo, const Band_Info *RESTRICT qinfo,
                            int nbands, struct Transpose_Progress_Info *progress_info ) {
// Matrix of nrows x ncols is being transposed to ncols x nrows.
// Rows and columns are each grouped into nbands groups (bands) of approximately equal size.
// Resulting blocks are each stored contiguously.
// Input  data is in arrays "a" (top band empty) and "bufin" (bottom band of data).
// Output data is in arrays "bufout" (top band of data) and "a" (bottom band empty).
	const LONG elemsize = sizeof( float );
	int i, j;
	LONG poffset = 0;
	LONG qoffset = 0;
	
	for (i=0; i<nbands; ++i) {
		LONG psize   = pinfo[i].size * elemsize;
		LONG qsize   = qinfo[i].size * elemsize;
		LONG newsize = pinfo[i].size * qsize;   // = qinfo[i].size * psize;
		LONG oldsize;

		LONG newoff  = qinfo[i].size * poffset;
		LONG oldoff  = pinfo[i].size * qoffset;

		char *newaddr = qinfo[i].addr;
		char *oldaddr = pinfo[i].addr;

		if (i > 0) {		// move temp blocks [i,0..i-1] to final spaces [i,0..i-1]
			memmove( newaddr, oldaddr, newoff );
			madv_dontneed( newaddr, newoff );

			if (progress_info) {
				progress_info->moves_done += i;
				if (report_progress( progress_info )) return 1;
			}

			newaddr += newoff;  // = qinfo[i].addr + qinfo[i].size * poffset
			oldaddr += oldoff;  // = pinfo[i].addr + pinfo[i].size * qoffset
		}
		
		// transpose and move init block [i,i] to final space [i,i]
		transpose_outplace( oldaddr, newaddr, pinfo[i].size, qinfo[i].size );
		madv_dontneed( newaddr, newsize );

		if (progress_info) {
			++progress_info->moves_done;
			if (report_progress( progress_info )) return 1;
		}
			
		newaddr += newsize; // can be RESTRICT inside for loop below
		oldaddr += newsize; // can be RESTRICT inside for loop below
		
		for (j=i+1; j<nbands; ++j) {
			char *initaddr = pinfo[j].addr + pinfo[j].size * qoffset;
			char *tempaddr = pinfo[j].addr + qinfo[j].size * poffset;
		//  newaddr        = qinfo[i].addr + pinfo[j].offset * qsize;   // can be RESTRICT in this block
		//  oldaddr        = pinfo[i].addr + qinfo[j].offset * psize;   // can be RESTRICT in this block

			newsize = qsize * pinfo[j].size;
			oldsize = psize * qinfo[j].size;

			// transpose and move init block [j,i] to final space [i,j]
			
			transpose_outplace( initaddr, newaddr, pinfo[j].size, qinfo[i].size );
			madv_dontneed( newaddr, newsize );

			if (progress_info) {
				++progress_info->moves_done;
				if (report_progress( progress_info )) return 1;
			}

			// transpose and move init block [i,j] to temp space [j,i]
			transpose_outplace( oldaddr, tempaddr, pinfo[i].size, qinfo[j].size );

			if (progress_info) {
				++progress_info->moves_done;
				if (report_progress( progress_info )) return 1;
			}
			
			newaddr += newsize;
			oldaddr += oldsize;
		}
		
		poffset += psize;
		qoffset += qsize;
	}
	
	return 0;
}
