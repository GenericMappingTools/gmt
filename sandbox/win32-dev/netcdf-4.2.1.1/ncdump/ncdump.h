/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncdump/ncdump.h,v 1.34 2009/12/02 20:03:43 russ Exp $
 *********************************************************************/
#ifndef _NCDUMP_H_
#define _NCDUMP_H_

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

    boolean string_times;	/* if true, output date-time values as
				 * human-readable strings.  */

    boolean iso_separator;	/* if true, use 'T' separator between
				 * date and time components of
				 * human-readable strings, otherwise
				 * use ' ' */

    boolean special_atts;	/* if true, output special attributes
				 * for optimization characteristics:
				 * _Compression, _Chunking,
				 * _Endianness, _Format, _Checksum,
				 * _NoFill */

    Nclang data_lang;		/* Specifies index conventions used in data
				 * comments, either LANG_C (C, 0-based,
				 * column major) or LANG_F (Fortran,
				 * 1-based, row major) */

    boolean with_cache;		/* For DAP URLs, get data with client-side
				 * caching when each variable is first accessed */

    int nlvars;			/* Number of variables specified with -v
				 * option on command line */

    char** lvars;		/* list of variable names specified with -v
				 * option on command line */

    int nlgrps;			/* Number of groups specified with -g
				 * option on command line */

    char** lgrps;		/* list of group names specified with -g
				 * option on command line */
    
    idnode_t* grpids;		/* list of grpids matching list specified with -g option */

    int nc_kind;		/* kind of netCDF file named on
				 * command line, 1 (classic), 2
				 * (64-bit offset), 3 (netCDF-4), 4
				 * (netCDF-4 classic model) */
} fspec_t;

#endif	/*_NCDUMP_H_ */
