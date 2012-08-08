#ifndef NC_NCGEN_H
#define NC_NCGEN_H
/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen3/ncgen.h,v 1.8 1997/07/07 18:27:04 russ Exp $
 *********************************************************************/

#define MAX_NC_ATTSIZE    20000	/* max size of attribute (for ncgen) */
#define MAXTRST		  5000	/* max size of string value (for ncgen) */

#include "generic.h"

extern int ncid;		/* handle for netCDF */
extern int ndims;		/* number of dimensions declared for netcdf */
extern int nvars;		/* number of variables declared for netcdf */
extern int natts;		/* number of attributes */
extern int nvdims;		/* number of dimensions for variables */
extern int dimnum;		/* dimension number index for variables */
extern int varnum;		/* variable number index for attributes */
extern int valnum;		/* number of values specified for variable */
extern int rec_dim;		/* number of the unlimited dimension, if any */
extern size_t rec_len;		/* number of elements for a record of data */
extern size_t var_len;		/* variable length (product of dimensions) */
extern size_t var_size;		/* size of each element of variable */

extern struct dims {
    size_t size;
    char *name;
    char *lname;		/* with no "-" characters, for C and Fortran */
} *dims;			/* table of dimensions */

extern struct vars {
    char *name;
    nc_type type;
    int ndims;
    int *dims;			/* array of dimension ids */
    union generic fill_value;	/* set to value of _FillValue attribute */
    int has_data;		/* 1 if data specified, 0 otherwise */
    size_t nrecs;		/* for record variables, number of records
				 * of data in CDL */
    char *data_stmnt;		/* for record variables, to avoid
				 * two passes with -f option */
    char *lname;		/* with no "-" characters, for C and Fortran */
} *vars;			/* table of variables */


extern struct atts {
    int var;			/* number of variable for this attribute */
    char *name;
    nc_type type;
    size_t len;
    void *val;
    char *lname;		/* with no "-" characters, for C and Fortran */
} *atts;			/* table of variable and global attributes */
#endif /*!NC_NCGEN_H*/
