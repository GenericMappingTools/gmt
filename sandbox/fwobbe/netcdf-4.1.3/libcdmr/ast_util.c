#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <ast_runtime.h>
#include <ast_internal.h>
#include <ast_util.h>

static int logstate = 1;

void ast_logset(int tf) {logstate = tf;}


void
ast_log(const char* fmt, ...)
{
    va_list ap;

    if(logstate == 1) {
        va_start(ap, fmt); 
	vfprintf(stderr,fmt,ap);
    }
}

/* Given an error number, return an error message. */
const char *
ast_strerror(ast_err err)
{
   /* System error? */
   if(err > 0) {
      const char *cp = (const char *) strerror(err);
      if(cp == NULL)
	 return "Unknown Error";
      return cp;
   }

   /* If we're here, this is a netcdf error code. */
    switch(err) {
    case AST_NOERR: return "No error";
    case AST_ENOMEM: return "Out of memory";
    case AST_EFAIL: return "AST Failure";
    case AST_EOF: return "End of file";
    case AST_EIO: return "IO error";
    case AST_ECURL: return "Libcurl error";
    default:
        return "Unknown Error";
   }
}


