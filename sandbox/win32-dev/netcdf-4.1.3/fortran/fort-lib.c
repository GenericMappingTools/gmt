/*
This file is part of the netCDF Fortran 77 API.

This file handles the the conversion of vecors from fortran to C.

Copyright 2006, University Corporation for Atmospheric Research. See
the COPYRIGHT file for copying and redistribution conditions.

$Id$
*/

#include <config.h>
#include <stddef.h>	/* for NULL */
#include <errno.h>

#include "netcdf.h"
#include "ncfortran.h"
#include "fort-lib.h"


/*
 * Convert a C dimension-ID vector into a FORTRAN dimension-ID vector.
 */
EXTERNL NF_INTEGER *
c2f_dimids(int ncid, int varid, const int* cdimids, NF_INTEGER* fdimids)
{
    int	i;
    int	ndims;

    if (nc_inq_varndims(ncid, varid, &ndims) != 0)
	return NULL;

    for (i = 0; i < ndims; ++i)
	fdimids[ndims - 1 - i] = cdimids[i] + 1;

    return fdimids;
}


/*
 * Convert a FORTRAN dimension-ID vector into a C dimension-ID vector.
 */
EXTERNL int *
f2c_dimids(int ndims, const NF_INTEGER* fdimids, int* cdimids)
{
    int	i;

    for (i = 0; i < ndims; ++i)
	cdimids[i] = fdimids[ndims - 1 - i] - 1;

    return cdimids;
}

/* Convert a C dimension-ID vector into a FORTRAN dimension-ID vector. */
EXTERNL NF_INTEGER *
c2f_chunksizes(int ncid, int varid, const int *cchunksizes, NF_INTEGER *fchunksizes)
{
    int	i;
    int	ndims;

    if (nc_inq_varndims(ncid, varid, &ndims))
	return NULL;

    for (i = 0; i < ndims; ++i)
	fchunksizes[ndims - 1 - i] = cchunksizes[i];

    return fchunksizes;
}

/* Convert a FORTRAN dimension-ID vector into a C dimension-ID vector. */
EXTERNL int *
f2c_chunksizes(int ncid, int varid, const NF_INTEGER *fchunksizes, int *cchunksizes)
{
    int	i;
    int	ndims;

    if (nc_inq_varndims(ncid, varid, &ndims))
	return NULL;

    for (i = 0; i < ndims; ++i)
	cchunksizes[i] = fchunksizes[ndims - 1 - i];

    return cchunksizes;
}

/*
 * Convert FORTRAN co-ordinates into C co-ordinates.
 */
EXTERNL size_t *
f2c_coords(int ncid, int varid, const NF_INTEGER* fcoords,
	   size_t *ccoords)
{
    int	i;
    int	ndims;

    if (nc_inq_varndims(ncid, varid, &ndims) != 0)
	return NULL;

    for (i = 0; i < ndims; ++i)
	ccoords[i] = fcoords[ndims - 1 - i] - 1;

    return ccoords;
}


/*
 * Convert FORTRAN counts into C counts.
 */
EXTERNL size_t *
f2c_counts(int ncid, int varid, const NF_INTEGER* fcounts,
    size_t* ccounts)
{
    int	i;
    int	ndims;

    if (nc_inq_varndims(ncid, varid, &ndims) != 0)
	return NULL;

    for (i = 0; i < ndims; ++i)
	ccounts[i] = fcounts[ndims - 1 - i];

    return ccounts;
}


/*
 * Convert FORTRAN strides into C strides.
 *
 * Helper function.
 */
EXTERNL ptrdiff_t *
f2c_strides(int ncid, int varid, const NF_INTEGER* fstrides,
    ptrdiff_t* cstrides)
{
    int	i;
    int	ndims;

    if (nc_inq_varndims(ncid, varid, &ndims) != 0)
	return NULL;

    for (i = 0; i < ndims; ++i)
	cstrides[i] = fstrides[ndims - 1 - i];

    return cstrides;
}


/*
 * Convert a FORTRAN mapping vector into a C mapping vector.
 */
EXTERNL ptrdiff_t *
f2c_maps(int ncid, int varid, const NF_INTEGER* fmaps, ptrdiff_t* cmaps)
{
    return f2c_strides(ncid, varid, fmaps, cmaps);
}

#ifdef USE_NETCDF4
/* These appear to only be defined in netcdf-4*/

/* Get the varids for a fortran function (i.e. add 1 to each
 * varid.) */
EXTERNL int
nc_inq_varids_f(int ncid, int *nvars, int *fvarids)
{
   int *varids, nvars1;
   int i, ret = NC_NOERR;

   /* Get the information from the C library. */
   if ((ret = nc_inq_varids(ncid, &nvars1, NULL)))
      return ret;
   if (!(varids = malloc(nvars1 * sizeof(int))))
      return NC_ENOMEM;
   if ((ret = nc_inq_varids(ncid, NULL, varids)))
      goto exit;

   /* Add one to each, for fortran. */
   for (i = 0; i < nvars1; i++)
      fvarids[i] = varids[i] + 1;

   /* Tell the user how many there are. */
   if (nvars)
      *nvars = nvars1;

  exit:
   free(varids);
   return ret;
}
/* Get the dimids for a fortran function (i.e. add 1 to each
 * dimid.) */
EXTERNL int
nc_inq_dimids_f(int ncid, int *ndims, int *fdimids, int parent)
{
   int *dimids, ndims1;
   int i, ret = NC_NOERR;

   /* Get the information from the C library. */
   if ((ret = nc_inq_dimids(ncid, &ndims1, NULL, parent)))
      return ret;
   if (!(dimids = malloc(ndims1 * sizeof(int))))
      return NC_ENOMEM;
   if ((ret = nc_inq_dimids(ncid, NULL, dimids, parent)))
      goto exit;

   /* Add one to each, for fortran. */
   for (i = 0; i < ndims1; i++)
      fdimids[i] = dimids[i] + 1;

   /* Tell the user how many there are. */
   if (ndims)
      *ndims = ndims1;

  exit:
   free(dimids);
   return ret;
}

/* Swap the dim sizes for fortran. */
EXTERNL int
nc_insert_array_compound_f(int ncid, int typeid, char *name, 
			 size_t offset, nc_type field_typeid,
			 int ndims, int *dim_sizesp)
{
   int *dim_sizes_f;
   int i, ret;

   if (ndims <= 0)
      return NC_EINVAL;

   /* Allocate some storage to hold ids. */
   if (!(dim_sizes_f = malloc(ndims * sizeof(int))))
      return NC_ENOMEM;

   /* Create a backwards list of dimension sizes. */
   for (i = 0; i < ndims; i++)
      dim_sizes_f[i] = dim_sizesp[ndims - i - 1];

   /* Call with backwards list. */
   ret = nc_insert_array_compound(ncid, typeid, name, offset, field_typeid, 
				  ndims, dim_sizes_f);

   /* Clean up. */
   free(dim_sizes_f);
   return ret;
}

EXTERNL int
nc_inq_compound_field_f(int ncid, nc_type xtype, int fieldid, char *name, 
			size_t *offsetp, nc_type *field_typeidp, int *ndimsp, 
			int *dim_sizesp)
{
   int ndims;
   int ret;

   /* Find out how many dims. */
   if ((ret = nc_inq_compound_field(ncid, xtype, fieldid, NULL, NULL, 
				    NULL, &ndims, NULL)))
      return ret;

   /* Call the function. */
   if ((ret = nc_inq_compound_field(ncid, xtype, fieldid, name, offsetp, 
				    field_typeidp, ndimsp, dim_sizesp)))
      return ret;

   /* Swap the order of the dimsizes. */
   if (ndims)
   {
      int *f, *b, temp;
      for (f = dim_sizesp, b = &dim_sizesp[ndims - 1]; f < b; f++, b--)
      {
	 temp = *f;
	 *f = *b;
	 *b = temp;
      }
   }  

   return NC_NOERR;
}

#endif /*USE_NETCDF4*/
