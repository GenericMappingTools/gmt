#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include <ast_runtime.h>
#include <ast_internal.h>
#include <ast_debug.h>

int
ast_catch(int code)
{
    if(code != AST_NOERR && code != AST_EOF) 
	ast_breakpoint(code);
    return code;
}

void
ast_breakpoint(int code)
{
}
