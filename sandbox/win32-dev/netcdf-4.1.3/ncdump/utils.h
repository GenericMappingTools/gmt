/*********************************************************************
 *   Copyright 2011, University Corporation for Atmospheric Research
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *********************************************************************/

#include "config.h"
#include <netcdf.h>

extern char *progname;		/* for error messages */

#ifndef NO_NETCDF_2
#define NO_NETCDF_2		/* assert we aren't using any netcdf-2 stuff */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define NC_CHECK(fncall) {int statnc=fncall;if(statnc!=NC_NOERR)check(statnc,__FILE__,__LINE__);}

/* Print error message to stderr and exit */
extern void	error ( const char *fmt, ... );

/* Check error on malloc and exit with message if out of memory */
extern void*    emalloc ( size_t size );

/* Check error return.  If bad, print error message and exit. */
extern void check(int err, const char* file, const int line);

/* Return malloced name with chars special to CDL escaped. */
char* escaped_name(const char* cp);

/* Print name of netCDF var, dim, att, group, type, member, or enum
 * symbol with escaped special chars */
void print_name(const char *name);

/* Get dimid from a full dimension path name that may include group
 * names */
extern int 
nc_inq_dimid2(int ncid, const char *dimname, int *dimidp);

#ifdef __cplusplus
}
#endif
