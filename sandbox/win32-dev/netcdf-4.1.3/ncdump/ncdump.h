/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncdump/ncdump.h,v 1.34 2009/12/02 20:03:43 russ Exp $
 *********************************************************************/
#ifndef _NCDUMP_H_
#define _NCDUMP_H_

typedef int boolean;
enum {false=0, true=1};

typedef struct {			/* dimension */
    char name[NC_MAX_NAME];
    size_t size;
} ncdim_t;

/* forward declarations */
struct nctype_t;
struct ncvar_t;
struct safebuf_t;
struct timeinfo_t;

/*
 * Member function to determine if values for this type are equal, 
 * used to compare with fill value.
 */
typedef boolean (*val_equals_func)(const struct nctype_t *this,
				   const void *v1p, const void *v2p);
/* 
 * Member function to convert value of this type to a string. Returns
 * number of bytes in output string sb (not including trailing null)
 */
typedef int (*typ_tostring_func)(const struct nctype_t *this, 
				 struct safebuf_t *sb, 
				 const void *valp);

/* 
 * Per-variable member function to convert value of this type to a
 * string. Returns number of bytes in output string sb (not
 * including trailing null).  This is needed because a variable
 * can override its type for output, if a variable-specific format
 * is specified in an attribute.
 */
typedef int (*val_tostring_func)(const struct ncvar_t *this, 
				 struct safebuf_t *sb, 
				 const void *valp);

typedef struct nctype_t {	/* type */
    int ncid;		    /* group in which type is defined */
    nc_type tid;	    /* type ID */
    char *name;	       	    /* relative name of type within its group */
    char *grps;	       	    /* full prefix for type name, eg "grp1/grp2" */
    int class;	       	    /* > 0 for netCDF-4 user-defined types */
    size_t size;       	    /* like sizeof, even for user-defined types */
    nc_type base_tid;  	    /* for netCDF-4 enums, vlens */
    size_t nfields;    	    /* for netCDF-4 compound types */
    const char *fmt;   	    /* if non-null, format for printing values */
    nc_type *fids;  	    /* type id for each field of compound type */
    size_t *offsets;   	    /* offsets for each field of compound type */
    int *ranks;		    /* rank for each field of compound type */
    int **sides;	    /* rank sizes for each field where rank > 0 */
    int *nvals;		    /* num of values for each field (prod of sides) */
    /* member functions */
    val_equals_func val_equals; /* function to compare 2 values for equality */
    typ_tostring_func typ_tostring; /* default function to convert
				     * value to string for output (can
				     * be overridden by per-variable
				     * function, if fmt attribute
				     * is specified for a variable) */
} nctype_t;

typedef struct ncvar_t {	/* variable */
    char name[NC_MAX_NAME];
    nc_type type;
    struct nctype_t *tinfo;	/* full type information */
    int ndims;			/* number of dimensions (rank) */
    int *dims;			/* dimension ids */
    int natts;			/* number of attributes */
    boolean has_fillval;	/* has a fill value defined? */
    void* fillvalp;	        /* pointer to the fill value, if any */
    boolean has_timeval;	/* has date-time values, for -t output option */
    struct timeinfo_t *timeinfo; /* if time values, units, calendar, and origin */
    const char *fmt;            /* overriding variable-specific format for
				   printing values or base values, if any */
    int locid;			/* group id */
    /* member functions */
    val_tostring_func val_tostring; /* function to convert value to string for 
				       output */
} ncvar_t;

typedef struct ncatt_t {			/* attribute */
    int var;
    char name[NC_MAX_NAME];
    nc_type type;
    nctype_t *tinfo;
    size_t len;
    char *string;		/* for NcML text attributes (type = NC_CHAR)
				 * TODO: eliminate and just use valgp */
    double *vals;		/* for NcML numeric attributes of all types
				 * TODO: eliminate and just use valgp */
    void *valgp;		/* generic pointer to values of any type */
} ncatt_t;

/* Buffer structure for implementing growable strings, used in
 * preventing buffer overflows when the size needed for a character
 * buffer cannot be easily predicted or limited to any specific
 * maximum, such as when used in recursive function calls for nested
 * vlens and nested compound types. */
typedef struct safebuf_t {
    size_t len;			/* current length of buffer */
    size_t cl;			/* current length of string in buffer, < len-1 */
    char *buf;
} safebuf_t;

typedef
enum {LANG_C, LANG_F} Nclang; 

typedef struct {			/* specification for how to format dump */

    char *name;			/* name specified with -n or derived from
				 * file name */

    boolean header_only;	/* if true, don't print any variable data */

    boolean coord_vals;		/* if true, print header and coordinate
				 * dimension values (values of variables
				 * that are also dimensions), but no other
				 * variable data */

    boolean brief_data_cmnts;	/* if true, put // comments in data section
				 * identifying variable and indices, useful
				 * for navigating through large
				 * multi-dimensional data lists.  */

    boolean full_data_cmnts;	/* if true, put // comments in data section
				 * identifying every value, useful for
				 * navigating through large
				 * multi-dimensional data lists.  */

    boolean iso_times;		/* if true, output date-time values as
				 * human-readable strings.  */

    boolean special_atts;		/* if true, output special attributes
				 * for optimization characteristics:
				 * _Compression, _Chunking,
				 * _Endianness, _Format, _Checksum,
				 * _NoFill */

    Nclang data_lang;		/* Specifies index conventions used in data
				 * comments, either LANG_C (C, 0-based,
				 * column major) or LANG_F (Fortran,
				 * 1-based, row major) */

    int nlvars;			/* Number of variables specified with -v
				 * option on command line */

    boolean with_cache;		/* For DAP URLs, get data with client-side
				 * caching when each variable is first accessed */

    char** lvars;		/* list of variable names specified with -v
				 * option on command line */
} fspec_t;

#endif	/*_NCDUMP_H_ */
