/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id $
 *********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netcdf.h>
#include "utils.h"
#include "chunkspec.h"

/* Structure mapping dimension IDs to corresponding chunksizes. */
static struct {
    size_t ndims;		/* number of dimensions in chunkspec string */
    int *dimids;		/* ids for dimensions in chunkspec string */
    size_t *chunksizes;		/* corresponding chunk sizes */
} chunkspecs;

/*
 * Parse chunkspec string and convert into chunkspec_t structure.
 *   ncid: location ID of open netCDF file or group in an open file
 *   spec: string of form 
 *           dim1/n1,dim2/n2,...,dimk/nk
 *
 *         specifying chunk size (ni) to be used for dimension named
 *         dimi.  Dimension names may be absolute,
 *         e.g. "/grp_a/grp_a1/dim".  The "ni" part of the spec may be
 *         omitted, in which case it is assumed to be the entire
 *         dimension size.  That is also the default for dimensions
 *         not mentioned in the string.
 *
 * Returns NC_NOERR if no error, NC_EINVAL if spec has consecutive
 * unescaped commas or no chunksize specified for dimension.
 */
int
chunkspec_parse(int ncid, const char *spec) {
    const char *cp;	   /* character cursor */
    const char *pp = spec; /* previous char cursor for detecting escapes */
    const char *np;	   /* beginning of current dimension name */
    size_t ndims = 0;
    int idim;
    int ret;

    chunkspecs.ndims = 0;
    if (!spec || *spec == '\0')
	return NC_NOERR; 
    /* Count unescaped commas, handle consecutive unescaped commas as error */
    for(cp = spec; *cp; cp++) {
	int comma_seen = 0;
	if(*cp == ',' && *pp != '\\') {
	    if(comma_seen) {	/* consecutive commas detected */
		return(NC_EINVAL);
	    }
	    comma_seen = 1;
	    ndims++;
	} else {
	    comma_seen = 0;
	}
	pp = cp;
    }
    ndims++;
    chunkspecs.ndims = ndims;
    chunkspecs.dimids = (int *) emalloc(ndims * sizeof(int));
    chunkspecs.chunksizes = (size_t *) emalloc(ndims * sizeof(size_t));
    /* Look up dimension ids and assign chunksizes */
    cp = spec;
    pp = spec;
    np = spec;
    idim = 0;
    for(cp = spec; ; cp++) {
	if(*cp == '\0' || (*cp == ',' && *pp != '\\')) { /* found end of "dim/nn" part */
	    char* dimname = 0;
	    char *dp;
	    int dimid;
	    size_t chunksize;
	 
	    for(; pp > np && *pp != '/'; pp--) { /* look backwards for "/" */
		continue;
	    }
	    if(*pp != '/') {	/* no '/' found, no chunksize specified for dimension */
		return(NC_EINVAL);
	    }
	    /* extract dimension name */
	    dimname = (char *) emalloc(pp - np + 1);
	    dp = dimname;
	    while(np < pp) {
		*dp++ = *np++;
	    }
	    *dp = '\0';
	    /* look up dimension id from dimension pathname */
	    ret = nc_inq_dimid2(ncid, dimname, &dimid);
	    if(ret != NC_NOERR)
		break;
	    chunkspecs.dimids[idim] = dimid;
	    /* parse and assign corresponding chunksize */
	    pp++; /* now points to first digit of chunksize, ',', or '\0' */
	    if(*pp == ',' || *pp == '\0') { /* no size specified, use dim len */
		size_t dimlen;
		ret = nc_inq_dimlen(ncid, dimid, &dimlen);
		if(ret != NC_NOERR)
		    return(ret);
		chunksize = dimlen;
	    } else {	      /* convert nnn string to long integer */
		char *ep;
		long val = strtol(pp, &ep, 0);
		if(ep == pp || errno == ERANGE || val < 1) /* allow chunksize bigger than dimlen */
		    return (NC_EINVAL);
		chunksize = val;
	    }
	    chunkspecs.chunksizes[idim] = chunksize;
	    idim++;
	    if(dimname)
		free(dimname);
	    if(*cp == '\0')
		break;
	    /* set np to point to first char after comma */
	    np = cp + 1;
	}
	pp = cp;
    };
    return NC_NOERR;
}

/* Return size in chunkspec string specified for dimension corresponding to dimid, 0 if not found */
size_t
chunkspec_size(int dimid) {
    int idim;
    for(idim = 0; idim < chunkspecs.ndims; idim++) {
	if(dimid == chunkspecs.dimids[idim]) {
	    return chunkspecs.chunksizes[idim];
	}	
    }
    return 0;
}

/* Return number of dimensions for which chunking was specified in
 * chunkspec string on command line, 0 if no chunkspec string was
 * specified. */
int
chunkspec_ndims(void) {
    return chunkspecs.ndims;
}


