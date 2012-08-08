/* This is part of the netCDF package.
   Copyright 2011 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.

   Includes prototypes for some functions used to translate parameters
   between C and Fortran.
  */

#ifndef UD_FORT_LIB_H
#define UD_FORT_LIB_H

#include <stddef.h>	/* for ptrdiff_t, size_t */
#include "ncfortran.h"
#include <netcdf_f.h>

/*
 * PURPOSE: Convert a C dimension-ID vector into a FORTRAN dimension-ID
 *	    vector
 * REQUIRE: <ncid> is valid && <varid> is valid && <cdimids> != NULL &&
 *	    <fdimids> != NULL && <cdimids> != <fdimids>
 * PROMISE: The order of the dimensions will be reversed and 1 will be
 *	    added to each element.  RESULT == <fdimids>
 */
EXTERNL NF_INTEGER *
c2f_dimids(int		ncid,		/* netCDF dataset ID */
	   int		varid,		/* netCDF variable ID */
	   const int *cdimids,	/* C dim IDs */
	   NF_INTEGER *fdimids);	/* FORTRAN dim IDs */

/*
 * PURPOSE: Convert a FORTRAN dimension-ID vector into a C dimension-ID
 *	    vector
 * REQUIRE: <ndims> == 0 || (<ndims> >= 0 && <fdimids> != NULL &&
 *	    <cdimids> != NULL && <fdimids> != <cdimids>)
 * PROMISE: The order of the dimensions will be reversed and 1 will be
 *	    subtracted from each element.  RESULT == <cdimids>
 */
EXTERNL  int *
f2c_dimids(int ndims, 		/* number of dims  */
	   const NF_INTEGER *fdimids,	/* FORTRAN dim IDs */
	   int *cdimids);	/* C dim IDs */

/* These two are the same as the dimids, but for chunksizes, so that 1
 * is not added/subtracted. */
EXTERNL  NF_INTEGER * 
c2f_chunksizes(int ncid, int varid, const int* cchunksizes, 
	       NF_INTEGER *fchunksizes);

EXTERNL int * 
f2c_chunksizes(int ncid, int varid, const NF_INTEGER *fchunksizes, 
	       int *cchunksizes);

/*
 * PURPOSE: Convert a FORTRAN co-ordinate vector into a C co-ordinate vector
 * REQUIRE: <ncid> refers to an open dataset && <varid> refers to an
 *	    existing variable && <fcoords> != NULL && <ccoords> != NULL &&
 *	    <fcoords> != <ccoords>
 * PROMISE: The order of the co-ordinates will be reversed and 1 will be
 *	    subtracted from each element.  RESULT == <ccoords>.
 */
EXTERNL size_t *
f2c_coords(int ncid,		/* dataset ID */
	   int varid,		/* variable ID */
	   const NF_INTEGER *fcoords,	/* FORTRAN coords */
	   size_t *ccoords);	/* C coords */

/*
 * PURPOSE: Convert a FORTRAN edge-count vector into a C edge-count vector
 * REQUIRE: <ncid> refers to an open dataset && <varid> refers to an
 *	    existing variable && <fcounts> != NULL && <ccounts> != NULL &&
 *	    <fcounts> != <ccounts> && <fcounts> != <ccounts>
 * PROMISE: The order of the edge-counts will be reversed.  
 *	    RESULT == <ccounts>.
 */
EXTERNL size_t *
f2c_counts(int		ncid,		/* dataset ID */
	   int		varid,		/* variable ID */
	   const NF_INTEGER*	fcounts,	/* FORTRAN counts */
	   size_t*	ccounts);	/* C counts */

/*
 * PURPOSE: Convert a FORTRAN stride vector into a C stride vector
 * REQUIRE: <ncid> refers to an open dataset && <varid> refers to an
 *	    existing variable && <fstrides> != NULL && <cstrides> != NULL &&
 *	    <fstrides> != <cstrides>
 * PROMISE: The order of the strides will be reversed.  RESULT == <cstrides>.
 */
EXTERNL ptrdiff_t *
f2c_strides(int		ncid,		/* dataset ID */
	    int		varid,		/* variable ID */
	    const NF_INTEGER *fstrides,	/* FORTRAN strides */
	    ptrdiff_t *cstrides);	/* C strides */

/*
 * PURPOSE: Convert a FORTRAN mapping vector into a C mapping vector
 * REQUIRE: <ncid> refers to an open dataset && <varid> refers to an
 *	    existing variable && <fmaps> != NULL && <cmaps> != NULL &&
 *	    <fmaps> != <cmaps>
 * PROMISE: The order of the mapping vector will be reversed.
 *	    RESULT == <cmaps>.
 */
EXTERNL ptrdiff_t *
f2c_maps(int		ncid,		/* dataset ID */
	 int		varid,		/* variable ID */
	 const NF_INTEGER *fmaps,		/* FORTRAN mapping */
	 ptrdiff_t *cmaps);		/* C mapping */


#endif	/* header-file lockout */
