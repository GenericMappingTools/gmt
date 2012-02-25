/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include <stdarg.h>
#include "ocinternal.h"
#include "ocdebug.h"

int ocdebug;

#ifdef OCCATCHERROR
/* Place breakpoint here to catch errors close to where they occur*/
int
ocbreakpoint(int err) {return err;}

int
octhrow(int err)
{
    if(err == 0) return err;
    return ocbreakpoint(err);
}
#endif

int
xdrerror(void)
{
    oc_log(LOGERR,"xdr failure");
    return OCTHROW(OC_EDATADDS);
}


void*
occalloc(size_t size, size_t nelems)
{
    return ocmalloc(size*nelems);
}

void*
ocmalloc(size_t size)
{
    void* memory = calloc(size,1); /* use calloc to zero memory*/
    if(memory == NULL) oc_log(LOGERR,"ocmalloc: out of memory");
    return memory;
}

void
ocfree(void* mem)
{
    if(mem != NULL) free(mem);
}

int
ocpanic(const char* fmt, ...)
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
