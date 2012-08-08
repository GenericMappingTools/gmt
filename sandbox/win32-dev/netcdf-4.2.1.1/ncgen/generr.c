/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/generr.c,v 1.1 2009/09/25 18:22:22 dmh Exp $
 *********************************************************************/

#include "includes.h"
#include <ctype.h>	/* for isprint() */

int error_count;

#ifndef NO_STDARG
#define vastart(argv,fmt) va_start(argv,fmt)
#else
#define vastart(argv,fmt) va_start(argv)
#endif

/*
 * For logging error conditions.
 * Designed to be called by other vararg procedures
 */
#ifndef NO_STDARG
void
vderror(const char *fmt, va_list argv)
#else
/* Technically illegal; va_alist should be only arg */
void
vderror(fmt,va_alist) const char* fmt; va_dcl
#endif
{
    (void) vfprintf(stderr,fmt,argv) ;
    (void) fputc('\n',stderr) ;
    (void) fflush(stderr);	/* to ensure log files are current */
    error_count++;
}

#ifndef NO_STDARG
void
derror(const char *fmt, ...)
#else
void
derror(fmt,va_alist) const char* fmt; va_dcl
#endif
{
    va_list argv;
    vastart(argv,fmt);
    vderror(fmt,argv);
}

/* Report version errors */
#ifndef NO_STDARG
void
verror(const char *fmt, ...)
#else
void
verror(fmt,va_alist) const char* fmt; va_dcl
#endif
{
    char newfmt[2048];
    va_list argv;
    vastart(argv,fmt);
    strcpy(newfmt,"netCDF classic: not supported: ");
    strncat(newfmt,fmt,2000);
    vderror(newfmt,argv);
}

#ifndef NO_STDARG
void
semwarn(const int lno, const char *fmt, ...)
#else
void
semwarn(lno,fmt,va_alist) const int lno; const char* fmt; va_dcl
#endif
{
    va_list argv;
    vastart(argv,fmt);
    (void)fprintf(stderr,"%s: %s line %d: ", progname, cdlname, lno);
    vderror(fmt,argv);
}

#ifndef NO_STDARG
void
semerror(const int lno, const char *fmt, ...)
#else
void
semerror(lno,fmt,va_alist) const int lno; const char* fmt; va_dcl
#endif
{
    va_list argv;
    vastart(argv,fmt);
    (void)fprintf(stderr,"%s: %s line %d: ", progname, cdlname, lno);
    vderror(fmt,argv);
    exit(1); /* immediately fatal */
}

/* Capture potential version errors */
static char* markcdf4_msg = NULL;
void
markcdf4(const char* msg)
{
    enhanced_flag = 1;
    if(markcdf4_msg == NULL)
        markcdf4_msg = (char*)msg;
}

char* 
getmarkcdf4(void)
{
    return markcdf4_msg;
}

/**************************************************/
/* Provide a version of snprintf that panics if*/
/* the buffer is overrun*/

#ifndef NO_STDARG
void
nprintf(char* buffer, size_t size, const char *fmt, ...)
#else
void
nprintf(buffer,size,fmt)
    char* buffer; size_t size; const char* fmt; va_dcl
#endif
{
    long written;
    va_list args;
    vastart(args,fmt);
    written = vsnprintf(buffer,size,fmt,args);
    if(written < 0 || written >= size) {
	PANIC("snprintf failure");
    }
}
