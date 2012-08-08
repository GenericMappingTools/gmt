/*********************************************************************
 *   Copyright 2009, University Corporation for Atmospheric Research
 *   See netcdf/README file for copying and redistribution conditions.
 *   "$Id $"
 *********************************************************************/

#ifndef _NCITER_
#define _NCITER_

#include <netcdf.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* 
 * The opaque structure to hold per-variable state of iteration
 */
typedef struct {
    int first;	     /* false after associated next function invoked */
    int right_dim;   /* rightmost dimension for start of variable pieces */
    size_t rows;     /* how many subpiece rows in bufsiz */
    size_t numrows;  /* how many row pieces in right_dim dimension */
    size_t cur;	     /* current "row" in loop over row pieces */
    size_t leftover; /* how many rows left over after partitioning
		      * bufsiz into subpiece blocks */
    int more;	     /* whether there is more data still to get */
    size_t to_get;   /* number of values to get on this access */
    int rank;	     /* number of dimensions */
    size_t inc;	     /* increment for right_dim element of start vector */
    int chunked;     /* 1 if chunked, 0 if contiguous */
    size_t dimsizes[NC_MAX_VAR_DIMS];
    size_t chunksizes[NC_MAX_VAR_DIMS]; /* ignored if not chunked */
} nciter_t;

/*
 * The Interface
 */

/* Get iterator for a variable. */
extern int
nc_get_iter(Symbol*, size_t bufsize, nciter_t *iterp);

/* Iterate over blocks of variable values, using start and count
 * vectors.  Returns number of values to access (product of counts),
 * or 0 if done. */
extern size_t
nc_next_iter(nciter_t *iterp, size_t *start, size_t *count);

#if defined(__cplusplus)
}
#endif

#endif /* _NCITER_ */
