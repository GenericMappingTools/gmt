#ifndef NCGEN_DEBUG_H
#define NCGEN_DEBUG_H

/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/debug.h,v 1.2 2010/03/31 18:18:34 dmh Exp $
 *********************************************************************/

#include <stdarg.h>
#include <assert.h>
#include "generr.h"
#include "bytebuffer.h"

#if 0
#define GENDEBUG 2
#endif

#ifdef GENDEBUG
#  define DEBUG
#  if GENDEBUG > 0
#    define DEBUG1
#  endif
#  if GENDEBUG > 1
#    define DEBUG2
#  endif
#  if GENDEBUG > 2
#    define DEBUG3
#  endif
#endif




extern int ncgdebug;
extern int debug;

extern void fdebug(const char *fmt, ...);

#define PANIC(msg) assert(panic(msg))
#define PANIC1(msg,arg) assert(panic(msg,arg))
#define PANIC2(msg,arg1,arg2) assert(panic(msg,arg1,arg2))
#define ASSERT(expr) {if(!(expr)) {panic("assertion failure: %s",#expr);}}
extern int panic(const char* fmt, ...);

/*
Provide wrapped versions of calloc and malloc.
The wrapped version panics if memory is exhausted.
*/

#define ecalloc(x,y) chkcalloc(x,y)
#define emalloc(x)   chkmalloc(x)
#define erealloc(p,x)   chkrealloc(p,x)
#define efree(x) chkfree(x)
extern void* chkcalloc(size_t, size_t);
extern void* chkmalloc(size_t);
extern void* chkrealloc(void*,size_t);
extern void  chkfree(void*);
#define MEMCHECK(var,throw) {if((var)==NULL) return (throw);}

#endif /*NCGEN_DEBUG_H*/
