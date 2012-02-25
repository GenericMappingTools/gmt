/*
This file is part of the netCDF Fortran 77 API.

This file handles the netCDF variable functions.

Copyright 2006, University Corporation for Atmospheric Research. See
the COPYRIGHT file for copying and redistribution conditions.

$Id$
*/

#include <config.h>
#include "netcdf.h"
#include "ncfortran.h"
#include "fort-lib.h"

/*
 * Define a netCDF variable.
 */
FCALLSCFUN6(NF_INT, nc_def_var, NF_DEF_VAR, nf_def_var,
	    NCID, STRING, TYPE, NDIMS, DIMIDS, PVARID)


/*
 * Inquire about a netCDF variable.
 */
FCALLSCFUN7(NF_INT, nc_inq_var, NF_INQ_VAR, nf_inq_var,
	    NCID, VARID, PSTRING, PTYPE, PNDIMS, PDIMIDS, PNATTS)


/*
 * Obtain the ID of a netCDF variable.
 */
FCALLSCFUN3(NF_INT, nc_inq_varid, NF_INQ_VARID, nf_inq_varid,
	    NCID, STRING, PVARID)


/*
 * Obtain the name of a netCDF variable.
 */
FCALLSCFUN3(NF_INT, nc_inq_varname, NF_INQ_VARNAME, nf_inq_varname,
	    NCID, VARID, PSTRING)


/*
 * Obtain the type of a netCDF variable.
 */
FCALLSCFUN3(NF_INT, nc_inq_vartype, NF_INQ_VARTYPE, nf_inq_vartype,
	    NCID, VARID, PTYPE)


/*
 * Obtain the number of dimensions of a netCDF variable.
 */
FCALLSCFUN3(NF_INT, nc_inq_varndims, NF_INQ_VARNDIMS, nf_inq_varndims,
	    NCID, VARID, PNDIMS)


/*
 * Obtain the shape of a netCDF variable.
 */
FCALLSCFUN3(NF_INT, nc_inq_vardimid, NF_INQ_VARDIMID, nf_inq_vardimid,
	    NCID, VARID, PDIMIDS)


/*
 * Obtain the number of attributes of a netCDF variable.
 */
FCALLSCFUN3(NF_INT, nc_inq_varnatts, NF_INQ_VARNATTS, nf_inq_varnatts,
	    NCID, VARID, PNATTS)


/*
 * Rename a netCDF variable.
 */
FCALLSCFUN3(NF_INT, nc_rename_var, NF_RENAME_VAR, nf_rename_var,
	    NCID, VARID, STRING)


/*
 * Copy a netCDF variable.
 */
FCALLSCFUN3(NF_INT, nc_copy_var, NF_COPY_VAR, nf_copy_var,
	    NCID1, VARID, NCID2)
