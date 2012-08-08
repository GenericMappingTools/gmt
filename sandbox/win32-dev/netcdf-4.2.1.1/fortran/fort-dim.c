/*
This file is part of the netCDF Fortran 77 API.

This file handles the netCDF dimension functions.

Copyright 2006, University Corporation for Atmospheric Research. See
the COPYRIGHT file for copying and redistribution conditions.

$Id$
*/

#include <config.h>
#include "netcdf.h"
#include "ncfortran.h"


/*
 * Define a netCDF dimension.
 */
FCALLSCFUN4(NF_INT, nc_def_dim, NF_DEF_DIM, nf_def_dim,
	    NCID, STRING, COUNT, PDIMID)


/*
 * Obtain a netCDF dimension's index.
 */
FCALLSCFUN3(NF_INT, nc_inq_dimid, NF_INQ_DIMID, nf_inq_dimid,
	    NCID, STRING, PDIMID)


/*
 * Inquire about a netCDF dimension.
 */
FCALLSCFUN4(NF_INT, nc_inq_dim, NF_INQ_DIM, nf_inq_dim,
	    NCID, DIMID, PSTRING, PCOUNT)


/*
 * Obtain a netCDF dimension's name.
 */
FCALLSCFUN3(NF_INT, nc_inq_dimname, NF_INQ_DIMNAME, nf_inq_dimname,
	    NCID, DIMID, PSTRING)


/*
 * Obtain a netCDF dimension's length.
 */
FCALLSCFUN3(NF_INT, nc_inq_dimlen, NF_INQ_DIMLEN, nf_inq_dimlen,
	    NCID, DIMID, PCOUNT)


/*
 * Rename a netCDF dimension.
 */
FCALLSCFUN3(NF_INT, nc_rename_dim, NF_RENAME_DIM, nf_rename_dim,
	    NCID, DIMID, STRING)
