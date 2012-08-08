/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapdebug.h,v 1.33 2009/12/03 18:53:16 dmh Exp $
 *********************************************************************/
#ifndef RPCDEBUG_H
#define RPCDEBUG_H

#undef DEBUG

#include <stdarg.h>
#include <assert.h>

/* Warning: setting CATCHERROR has significant performance impact */
#undef CATCHERROR
#ifdef DEBUG
#undef CATCHERROR
#define CATCHERROR
#endif

#define PANIC(msg) assert(ncrpcpanic(msg));
#define PANIC1(msg,arg) assert(ncrpcpanic(msg,arg));
#define PANIC2(msg,arg1,arg2) assert(ncrpcpanic(msg,arg1,arg2));

#define ASSERT(expr) if(!(expr)) {PANIC(#expr);} else {}

extern int ncrpcpanic(const char* fmt, ...);

#define MEMCHECK(var,throw) {if((var)==NULL) return (throw);}

#ifdef CATCHERROR
/* Place breakpoint on ncrpcbreakpoint to catch errors close to where they occur*/
#define THROW(e) ncrpcthrow(e)
#define THROWCHK(e) (void)ncrpcthrow(e)

extern int ncrpcbreakpoint(int err);
extern int ncrpcthrow(int err);
#else
#define THROW(e) (e)
#define THROWCHK(e)
#endif

#endif /*CRDEBUG_H*/

