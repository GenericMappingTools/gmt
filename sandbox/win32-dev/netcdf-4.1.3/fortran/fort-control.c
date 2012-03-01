#include <config.h>
#include "netcdf.h"
#include "ncfortran.h"

#ifdef LOGGING
FCALLSCFUN1(NF_INT, nc_set_log_level, NF_SET_LOG_LEVEL, nf_set_log_level,
	    FINT2CINT)
#endif /* LOGGING */

/*
 * Create a netCDF dataset.
 */
FCALLSCFUN3(NF_INT, nc_create, NF_CREATE, nf_create,
	    STRING, FINT2CINT, PNCID)


/*
 * Create a netCDF dataset with I/O attributes.
 */
FCALLSCFUN5(NF_INT, nc__create, NF__CREATE, nf__create,
	    STRING, FINT2CINT, FINT2CSIZET, PCHUNKSIZEHINT, PNCID)


/*
 * Open a netCDF dataset.
 */
FCALLSCFUN3(NF_INT, nc_open, NF_OPEN, nf_open,
	    STRING, FINT2CINT, PNCID)


/*
 * Open a netCDF dataset with I/O attributes.
 */
FCALLSCFUN4(NF_INT, nc__open, NF__OPEN, nf__open,
	    STRING, FINT2CINT, PCHUNKSIZEHINT, PNCID)


/*
 * Set the fill mode of a netCDF dataset.
 */
FCALLSCFUN3(NF_INT, nc_set_fill, NF_SET_FILL, nf_set_fill,
	    NCID, FINT2CINT, PCINT2FINT)


/*
 * Set the fill mode of a netCDF dataset.
 */
FCALLSCFUN2(NF_INT, nc_set_default_format, NF_SET_DEFAULT_FORMAT, 
	    nf_set_default_format, FINT2CINT, PCINT2FINT)


/*
 * Put a netCDF dataset into redefine mode.
 */
FCALLSCFUN1(NF_INT, nc_redef, NF_REDEF, nf_redef,
	    NCID)


/*
 * End definition mode for a netCDF dataset.
 */
FCALLSCFUN1(NF_INT, nc_enddef, NF_ENDDEF, nf_enddef,
	    NCID)


/*
 * End definition mode for a netCDF dataset with I/O attributes.
 */
FCALLSCFUN5(NF_INT, nc__enddef, NF__ENDDEF, nf__enddef,
	    NCID, FINT2CSIZET, FINT2CSIZET, FINT2CSIZET, FINT2CSIZET)


/*
 * Synchronize the external representation of a netCDF dataset with its
 * internal one.
 */
FCALLSCFUN1(NF_INT, nc_sync, NF_SYNC, nf_sync,
	    NCID)


/*
 * Abort changes to a netCDF dataset.
 */
FCALLSCFUN1(NF_INT, nc_abort, NF_ABORT, nf_abort,
	    NCID)


/*
 * Close a netCDF dataset.
 */
FCALLSCFUN1(NF_INT, nc_close, NF_CLOSE, nf_close,
	    NCID)


/*
 * Delete a netCDF dataset by name.
 */
/*EXTERNL int nc_delete(const char * path);*/ /* defined in ../libsrc/nc.c */
FCALLSCFUN1(NF_INT, nc_delete, NF_DELETE, nf_delete,
	    STRING)

/*
 * Create a netCDF dataset with I/O attributes and specific base pe.
 */
FCALLSCFUN6(NF_INT, nc__create_mp, NF__CREATE_MP, nf__create_mp,
	    STRING, FINT2CINT, FINT2CSIZET, FINT2CINT, PCHUNKSIZEHINT, PNCID)

/*
 * Open a netCDF dataset with I/O attributes and specific base pe.
 */
FCALLSCFUN5(NF_INT, nc__open_mp, NF__OPEN_MP, nf__open_mp,
	    STRING, FINT2CINT, FINT2CINT, PCHUNKSIZEHINT, PNCID)

/*
 * Delete a netCDF dataset by name using a specific base pe.
 */
FCALLSCFUN2(NF_INT, nc_delete_mp, NF_DELETE_MP, nf_delete_mp,
	    STRING, FINT2CINT)

/*
 * Set netCDF of base PE
 */
FCALLSCFUN2(NF_INT, nc_set_base_pe, NF_SET_BASE_PE, nf_set_base_pe,
	    NCID, FINT2CINT)

/*
 * Inquire netCDF of base PE
 */
FCALLSCFUN2(NF_INT, nc_inq_base_pe, NF_INQ_BASE_PE, nf_inq_base_pe,
	    NCID, PCINT2FINT)
