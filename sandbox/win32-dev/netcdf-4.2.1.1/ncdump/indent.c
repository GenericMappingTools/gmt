/*********************************************************************
 *   Copyright 2007, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncdump/indent.c,v 1.6 2009/09/28 18:27:04 russ Exp $
 *********************************************************************/

#include <stdio.h>

#include "indent.h"
static int indent = 0;
static int indent_increment = 2; /* blanks for each nesting level */

void 
indent_init() {		/*  initialize output line indent */
    indent = 0;
}

void 
indent_out(){		/*  output current indent */
    /* Just does this, but we avoid looping for small indents:

       int i;
       for (i=0; i < indent; i++)
	 printf(" ");

    */

    int indent_small = 8;
    char* indents[] = 
                     {"", 
		      " ", 
		      "  ", 
		      "   ", 
		      "    ", 
		      "     ", 
		      "      ", 
		      "       ", 
                      "        "
                     };

    int ind = indent;
    while (ind > indent_small) {
	(void) printf("%s", indents[indent_small]);
	ind -= indent_small;
    }
    (void) printf("%s", indents[ind]);
}

void 
indent_more(){		/*  increment current indent */
    indent += indent_increment;
}

void 
indent_less(){		/*  decrement current indent */
    indent -= indent_increment;
}

int
indent_get() {		/* return current indent */
    return indent;
}

