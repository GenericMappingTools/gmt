/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/debug.c,v 1.2 2010/05/24 19:59:57 dmh Exp $ */

#include "includes.h"

extern char* ncclassname(nc_class);

#ifdef DEBUG
int debug = 1;
#else
int debug = 0;
#endif

void fdebug(const char *fmt, ...)
{
    va_list argv;
    if(debug == 0) return;
    va_start(argv,fmt);
    (void)vfprintf(stderr,fmt,argv) ;
}

/**************************************************/

/* Support debugging of memory*/
/* Also guarantee that calloc zeros memory*/
void*
chkcalloc(size_t size, size_t nelems)
{
    return chkmalloc(size*nelems);
}

void*
chkmalloc(size_t size)
{
    void* memory = calloc(size,1); /* use calloc to zero memory*/
    if(memory == NULL) {
	panic("malloc:out of memory");
    }
    memset(memory,0,size);
    return memory;
}

void*
chkrealloc(void* ptr, size_t size)
{
    void* memory = realloc(ptr,size);
    if(memory == NULL) {
	panic("realloc:out of memory");
    }
    return memory;
}

void
chkfree(void* mem)
{
    if(mem != NULL) free(mem);
}

int
panic(const char* fmt, ...)
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
    abort();
    return 0;
}
