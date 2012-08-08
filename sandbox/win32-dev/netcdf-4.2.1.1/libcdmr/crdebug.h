/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapdebug.h,v 1.33 2009/12/03 18:53:16 dmh Exp $
 *********************************************************************/
#ifndef CRDEBUG_H
#define CRDEBUG_H

#undef DEBUG

#include <stdarg.h>
#include <assert.h>

/* Warning: setting CATCHERROR has significant performance impact */
#undef CATCHERROR
#ifdef DEBUG
#undef CATCHERROR
#define CATCHERROR
#endif

#define PANIC(msg) assert(nccrpanic(msg));
#define PANIC1(msg,arg) assert(nccrpanic(msg,arg));
#define PANIC2(msg,arg1,arg2) assert(nccrpanic(msg,arg1,arg2));

#define ASSERT(expr) if(!(expr)) {PANIC(#expr);} else {}

extern int nccrpanic(const char* fmt, ...);

#define MEMCHECK(var,throw) {if((var)==NULL) return (throw);}

#ifdef CATCHERROR
/* Place breakpoint on nccrbreakpoint to catch errors close to where they occur*/
#define THROW(e) nccrthrow(e)
#define THROWCHK(e) (void)nccrthrow(e)

extern int nccrbreakpoint(int err);
extern int nccrthrow(int err);
#else
#define THROW(e) (e)
#define THROWCHK(e)
#endif

#endif /*CRDEBUG_H*/

