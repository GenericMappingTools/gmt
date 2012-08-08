/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/generr.h,v 1.2 2010/05/24 19:59:57 dmh Exp $ */

#ifndef GENERR_H
#define GENERR_H

extern int derror_count;

#ifndef NO_STDARG
#define vastart(argv,fmt) va_start(argv,fmt)
#else
#define vastart(argv,fmt) va_start(argv)
#endif

#ifndef NO_STDARG
#include <stdarg.h>
extern void vderror(const char *fmt, va_list argv);
extern void derror(const char *fmt, ...);
extern int panic(const char* fmt, ...);
extern void nprintf(char* buffer, size_t size, const char *fmt, ...);
#else
#include <varargs.h>
/* Technically illegal; va_alist should be only arg */
extern void vderror(fmt,va_alist) const char* fmt; va_dcl;
extern void derror(fmt,va_alist) const char* fmt; va_dcl;
extern void panic(fmt,va_alist) const char* fmt; va_dcl;
extern void nprintf(buffer,size,fmt)
	char* buffer; size_t size; const char* fmt; va_dcl;
#endif

#endif /*GENERR_H*/
