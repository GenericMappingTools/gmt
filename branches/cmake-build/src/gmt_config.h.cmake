/*
 * $Id$
 */

#include "../config.h"

/* which regex library */
#cmakedefine HAVE_PCRE
#cmakedefine HAVE_POSIX_ERE

/* compile with GDAL support */
#cmakedefine HAVE_GDAL

/* file locking */
#cmakedefine FLOCK

/* set triangulation method */
#cmakedefine TRIANGLE_D

/* enable compatibility mode */
#cmakedefine GMT_COMPAT

/* applies only if defined(WIN32) */
#cmakedefine USE_MEM_ALIGNED

