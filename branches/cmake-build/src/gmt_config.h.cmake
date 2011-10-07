/*
 * $Id$
 */

#include "../config.h"

/* C99 exact-width integer types <inttypes.h>, <stdint.h> */
#cmakedefine HAVE_INTTYPES_H
#cmakedefine HAVE_STDINT_H

/* if <unistd.h> exists */
#cmakedefine HAVE_UNISTD_H

/* which regex library <pcre.h>, <regex.h> */
#cmakedefine HAVE_PCRE
#cmakedefine HAVE_POSIX_ERE

/* compile with GDAL support <gdal.h> */
#cmakedefine HAVE_GDAL

/* file locking */
#cmakedefine FLOCK

/* set triangulation method */
#cmakedefine TRIANGLE_D

/* enable compatibility mode */
#cmakedefine GMT_COMPAT

/* applies only #ifdef _WIN32 */
#cmakedefine USE_MEM_ALIGNED

/* if NetCDF is static; applies only #ifdef _WIN32 */
#cmakedefine NETCDF_STATIC

/* disable VS 'secure' warnings */
#cmakedefine _CRT_SECURE_NO_DEPRECATE
#cmakedefine _CRT_NONSTDC_NO_DEPRECATE
#cmakedefine _SCL_SECURE_NO_DEPRECATE
