/*********************************************************************
 *   Copyright 1993, University Corporation for Atmospheric Research
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncdump/vardata.h,v 1.7 2007/10/08 02:47:57 russ Exp $
 *********************************************************************/

extern char *progname;		/* for error messages */

/* Display for user-defined fill values and default floating-point fill
   values; should match what ncgen looks for in ../ncgen/ncgen.l */
#define FILL_STRING "_"

#ifdef __cplusplus
extern "C" {
#endif

/* Output the data for a single variable, in CDL syntax. */
extern int vardata ( const ncvar_t*, /* variable */
		     size_t [], /* variable dimension lengths */
		     int, /* netcdf id */
		     int  /* variable id */
    );

/* Output the data for a single variable, in NcML syntax. */
extern int vardatax ( const ncvar_t*, /* variable */
		     size_t [], /* variable dimension lengths */
		     int, /* netcdf id */
		     int  /* variable id */
    );

/* set maximum line length */
extern void set_max_len ( int len );

/* print string with current indent and splitting long lines, if needed */
extern void lput( const char *string );

/* like lput, but with options to support formatting with appended comments */
extern void lput2( const char *string, boolean first, boolean wrap);

/* print values of an attribute */
extern void pr_any_att_vals( const ncatt_t *attp, const void *vals );

#ifdef __cplusplus
}
#endif
