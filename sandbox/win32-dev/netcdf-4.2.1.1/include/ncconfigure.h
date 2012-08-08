/*
 * Copyright 2010 University Corporation for Atmospheric
 * Research/Unidata. See COPYRIGHT file for more info.
 *
 * This header file is for the parallel I/O functions of netCDF.
 * 
 */
/* "$Id$" */

#ifndef NCCONFIGURE_H
#define NCCONFIGURE_H 1

/*
This is included in bottom
of config.h. It is where,
typically, alternatives to
missing functions should be
defined.
*/

#ifndef HAVE_STRDUP
extern char* strdup(const char*);
#endif

/* handle null arguments */
#ifndef nulldup
#ifdef HAVE_STRDUP
#define nulldup(s) ((s)==NULL?NULL:strdup(s))
#else
char *nulldup(const char* s);
#endif
#endif 


#ifndef nulldup
#define nulldup(s) ((s)==NULL?NULL:strdup(s))
#endif
#ifndef nulllen
#define nulllen(s) ((s)==NULL?0:strlen(s))
#endif
#ifndef nullfree
#define nullfree(s) {if((s)!=NULL) {free(s);} else {}}
#endif

#ifndef HAVE_UCHAR
typedef unsigned char uchar;
#endif

#ifndef HAVE_LONGLONG
typedef long long longlong;
typedef unsigned long long ulonglong;
#endif

#ifndef HAVE_UINT
typedef unsigned int uint;
#endif

#endif /* NCCONFIGURE_H */
