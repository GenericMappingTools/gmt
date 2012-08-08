/*********************************************************************
 *   Copyright 2011, University Corporation for Atmospheric Research
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   
 *   Stuff that's common to both ncdump and nccopy
 *
 *********************************************************************/

#include "config.h"

#define	STREQ(a, b)	(*(a) == *(b) && strcmp((a), (b)) == 0)

typedef int boolean;
enum {false=0, true=1};

struct safebuf_t;
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
extern int  nc_inq_dimid2(int ncid, const char *dimname, int *dimidp);

/* Test if variable is a record variable */
extern int  isrecvar ( int ncid, int varid );

#ifdef __cplusplus
}
#endif
