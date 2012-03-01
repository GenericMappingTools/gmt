/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/dapdebug.h,v 1.33 2009/12/03 18:53:16 dmh Exp $
 *********************************************************************/
#ifndef DEBUG_H
#define DEBUG_H

#undef DEBUG
#undef PARSEDEBUG

#include <stdarg.h>
#include <assert.h>

/* Warning: setting CATCHERROR has significant performance impact */
#undef CATCHERROR
#ifdef DEBUG
#undef CATCHERROR
#define CATCHERROR
#endif

#define PANIC(msg) assert(dappanic(msg));
#define PANIC1(msg,arg) assert(dappanic(msg,arg));
#define PANIC2(msg,arg1,arg2) assert(dappanic(msg,arg1,arg2));

#define ASSERT(expr) if(!(expr)) {PANIC(#expr);} else {}

extern int ncdap3debug;

extern int dappanic(const char* fmt, ...);

#define MEMCHECK(var,throw) {if((var)==NULL) return (throw);}

#ifdef CATCHERROR
/* Place breakpoint on dapbreakpoint to catch errors close to where they occur*/
#define THROW(e) dapthrow(e)
#define THROWCHK(e) (void)dapthrow(e)

extern int dapbreakpoint(int err);
extern int dapthrow(int err);
#else
#define THROW(e) (e)
#define THROWCHK(e)
#endif

#endif /*DEBUG_H*/

