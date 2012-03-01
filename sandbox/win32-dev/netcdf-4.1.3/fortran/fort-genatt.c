/*
This file is part of the netCDF Fortran 77 API.

This file handles the netCDF attribute functions.

Copyright 2006, University Corporation for Atmospheric Research. See
the COPYRIGHT file for copying and redistribution conditions.

$Id$
*/

#include <config.h>
#include "netcdf.h"
#include "ncfortran.h"


/*
 * Inquire about a netCDF attribute.
 */
FCALLSCFUN5(NF_INT, nc_inq_att, NF_INQ_ATT, nf_inq_att,
	    NCID, VARID, STRING, PTYPE, PCOUNT)


/*
 * Obtain the index of a netCDF attribute.
 */
FCALLSCFUN4(NF_INT, nc_inq_attid, NF_INQ_ATTID, nf_inq_attid,
	    NCID, VARID, STRING, PATTID)


/*
 * Obtain the type of a netCDF attribute.
 */
FCALLSCFUN4(NF_INT, nc_inq_atttype, NF_INQ_ATTTYPE, nf_inq_atttype,
	    NCID, VARID, STRING, PTYPE)


/*
 * Obtain the length of a netCDF attribute.
 */
FCALLSCFUN4(NF_INT, nc_inq_attlen, NF_INQ_ATTLEN, nf_inq_attlen,
	    NCID, VARID, STRING, PCOUNT)


/*
 * Obtain the name of a netCDF attribute.
 */
FCALLSCFUN4(NF_INT, nc_inq_attname, NF_INQ_ATTNAME, nf_inq_attname,
	    NCID, VARID, ATTID, PSTRING)


/*
 * Copy an attribute from one netCDF dataset to another.
 */
FCALLSCFUN5(NF_INT, nc_copy_att, NF_COPY_ATT, nf_copy_att,
	    NCID1, VARID1, STRING, NCID2, VARID2)


/*
 * Rename a netCDF attribute.
 */
FCALLSCFUN4(NF_INT, nc_rename_att, NF_RENAME_ATT, nf_rename_att,
	    NCID, VARID, STRING, STRING)


/*
 * Remove a netCDF attribute.
 */
FCALLSCFUN3(NF_INT, nc_del_att, NF_DEL_ATT, nf_del_att,
	    NCID, VARID, STRING)
