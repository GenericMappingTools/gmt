/*
This file is part of the netCDF Fortran 77 API.

This file handles the netCDF inquiry functions.

Copyright 2006, University Corporation for Atmospheric Research. See
the COPYRIGHT file for copying and redistribution conditions.

$Id$
*/

#include <config.h>
#include "netcdf.h"
#include "ncfortran.h"


/*
 * Generally inquire about a netCDF dataset.
 */
FCALLSCFUN5(NF_INT, nc_inq, NF_INQ, nf_inq,
	    NCID, PNDIMS, PNVARS, PNATTS, PDIMID)


/*
 * Inquire about the number of dimensions in a netCDF dataset.
 */
FCALLSCFUN2(NF_INT, nc_inq_ndims, NF_INQ_NDIMS, nf_inq_ndims,
	    NCID, PNDIMS)


/*
 * Inquire about the number of variables in a netCDF dataset.
 */
FCALLSCFUN2(NF_INT, nc_inq_nvars, NF_INQ_NVARS, nf_inq_nvars,
	    NCID, PNVARS)


/*
 * Inquire about the number of attributes in a netCDF dataset.
 */
FCALLSCFUN2(NF_INT, nc_inq_natts, NF_INQ_NATTS, nf_inq_natts,
	    NCID, PNATTS)


/*
 * Inquire about the index of the unlimited dimension in a netCDF dataset.
 */
FCALLSCFUN2(NF_INT, nc_inq_unlimdim, NF_INQ_UNLIMDIM, nf_inq_unlimdim,
	    NCID, PDIMID)

/*
 * Inquire about the format version of a netCDF dataset.
 */
FCALLSCFUN2(NF_INT, nc_inq_format, NF_INQ_FORMAT, nf_inq_format,
	    NCID, PFORMAT)
