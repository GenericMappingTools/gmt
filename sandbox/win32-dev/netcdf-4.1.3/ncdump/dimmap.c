/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id $
 *********************************************************************/

#include <netcdf.h>		/* just for error codes */
#include "utils.h"

static struct {
    size_t ndims;
    int *odimids;
    int *idimids;
    int *ounlims;
    int *iunlims;
} dimmap;

/* initialize a dimmap for n dimension ids */
int
dimmap_init(size_t n) {
    int stat = NC_NOERR;
    int i;
    dimmap.ndims = n;
    dimmap.odimids = emalloc(n * sizeof(int));
    dimmap.idimids = emalloc(n * sizeof(int));
    dimmap.ounlims = emalloc(n * sizeof(int));
    dimmap.iunlims = emalloc(n * sizeof(int));
    for(i = 0; i < n; i++) {
	dimmap.odimids[i] = -1;
	dimmap.idimids[i] = -1;
	dimmap.ounlims[i] = 0;
	dimmap.iunlims[i] = 0;
    }
    return stat;
}

/* store association between an input dimid and an output dimid, which
 * should be ints between 0 and ndims-1, inclusive */
int
dimmap_store(int idimid, int odimid, int iunlim, int ounlim) {
    int stat = NC_NOERR;
    dimmap.odimids[idimid] = odimid; /* used to map input dimids to output dimids */
    dimmap.idimids[odimid] = idimid; /* used to map output dimids to input dimids */
    dimmap.ounlims[odimid] = ounlim;
    dimmap.iunlims[idimid] = iunlim;
    return stat;
}

/* return odimid associated with specified idimid, or -1 if not found */
int
dimmap_odimid(int idimid) {
    return dimmap.odimids[idimid];
}

/* return idimid associated with specified odimid, or -1 if not found */
int
dimmap_idimid(int odimid) {
    return dimmap.idimids[odimid];
}

/* return whether odimid dimension is unlimited */
int
dimmap_ounlim(int odimid) {
    return dimmap.ounlims[odimid];
}

/* return whether idimid dimension is unlimited */
int
dimmap_iunlim(int idimid) {
    return dimmap.iunlims[idimid];
   
}

