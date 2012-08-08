/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapdebug.c,v 1.9 2009/09/23 22:26:00 dmh Exp $
 *********************************************************************/
#include "config.h"
#include <stdarg.h>
#include <stdio.h>

#include "rpcdebug.h""

#ifdef CATCHERROR

/* Place breakpoint here to catch errors close to where they occur*/
int
ncrpcbreakpoint(int err) {return err;}

int
ncrpcthrow(int err)
{
    if(err == 0) return err;
    return ncrpcbreakpoint(err);
}
#endif

int
ncrpcpanic(const char* fmt, ...)
{
    va_list args;
    if(fmt != NULL) {
      va_start(args, fmt);
      vfprintf(stderr, fmt, args);
      fprintf(stderr, "\n" );
      va_end( args );
    } else {
      fprintf(stderr, "panic" );
    }
    fprintf(stderr, "\n" );
    fflush(stderr);
    return 0;
}

