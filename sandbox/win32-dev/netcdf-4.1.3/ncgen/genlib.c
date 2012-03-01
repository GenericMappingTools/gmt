/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/genlib.c,v 1.57 2010/04/04 19:39:47 dmh Exp $
 *********************************************************************/

#include "includes.h"

/* invoke netcdf calls (or generate C or Fortran code) to create netcdf
 * from in-memory structure.
The output file name is chosen by using the following in priority order:
1. -o flag name
2. command line input file with .cdl changed to .nc
3. dataset name as specified in netcdf <name> {...} 
*/
void
define_netcdf(void)
{
    char *filename;		/* output file name */
    filename = NULL;
    /* Rule for specifying the dataset name:
	1. use -o name
	2. use input cdl file name
	3. use the datasetname
	It would be better if there was some way
	to specify the datasetname independently of the
	file name, but oh well.
    */
    if(netcdf_name) { /* -o flag name */
	filename = nulldup(netcdf_name);
    }    
    if(filename == NULL &&
	cdlname != NULL && strcmp(cdlname,"-") != 0) {/* cmd line name */
	char base[1024];
	char* p;
	strcpy(base,cdlname);
	/* remove any suffix and prefix*/
	p = strrchr(base,'.');
	if(p != NULL) {*p= '\0';}
	p = strrchr(base,'/');
	if(p != NULL) {strcpy(base,p+1);}
	if(strlen(base) > 0) {
	    strcat(base,
		(binary_flag == -1?".cdf" /* old, deprecated extension */
				  :".nc")); /* preferred*/
	    filename = nulldup(base);
	}
    }
    if(filename == NULL) {/* construct name from dataset name */
	char base[1024];
	strcpy(base,datasetname);
	strcat(base,
		(binary_flag == -1?".cdf" /* old, deprecated extension */
				  :".nc")); /* preferred*/
	filename = nulldup(base);
    }

    /* Execute exactly one of these */
#ifdef ENABLE_C
    if (c_flag) gen_ncc(filename); else /* create C code to create netcdf */
#endif
#ifdef ENABLE_F77
    if (f77_flag) gen_ncf77(filename); else /* create Fortran code */
#endif
#ifdef ENABLE_JAVA
    if(java_flag) {
	gen_ncjava(filename);
    } else
#endif
/* Binary is the default */
#ifdef ENABLE_BINARY
    gen_netcdf(filename); /* create netcdf */
#else
    derror("No language specified");
#endif
    close_netcdf();
    cleanup();
}

void
close_netcdf(void)
{
#ifdef ENABLE_C
    if (c_flag) cl_c(); else /* create C code to close netcdf */
#endif
#ifdef ENABLE_F77
    if (f77_flag) cl_f77(); else
#endif
#ifdef ENABLE_JAVA
    if (java_flag) cl_java(); else
#endif
#ifdef ENABLE_BINARY
    if (binary_flag) cl_netcdf();
#endif
}

/* Compute the C name for a given symbol*/
/* Cache in symbol->lname*/
const char*
cname(Symbol* sym)
{
    if(sym->lname == NULL) {
	char* name = pooldup(sym->name);
#ifdef USE_NETCDF4
	if(sym->subclass == NC_FIELD || sym->subclass == NC_ECONST) {
	     sym->lname = nulldup(decodify(name));
	} else
#endif
	if(sym->objectclass == NC_ATT && sym->att.var != NULL) {
	    /* Attribute name must be prefixed with the cname of the*/
	    /* associated variable*/
	    const char* vname = cname(sym->att.var);
	    const char* aname = decodify(name);
	    sym->lname = (char*)emalloc(strlen(vname)
					+strlen(aname)
					+1+1);
	    sym->lname[0] = '\0';
            strcpy(sym->lname,vname);
	    strcat(sym->lname,"_");
	    strcat(sym->lname,aname);
	} else {
            /* convert to language form*/
#ifdef USE_NETCDF4
            sym->lname = nulldup(decodify(cprefixed(sym->prefix,name,"_")));
#else
            sym->lname = nulldup(decodify(name)); /* convert to usable form*/
#endif
	}
    }
    return sym->lname;
}

#ifdef USE_NETCDF4
/* Result is pool alloc'd*/
char*
cprefixed(List* prefix, char* suffix, char* separator)
{
    int slen;
    int plen;
    int i;
    char* result;

    ASSERT(suffix != NULL);
    plen = prefixlen(prefix);
    if(prefix == NULL || plen == 0) return decodify(suffix);
    /* plen > 0*/
    slen = 0;
    for(i=0;i<plen;i++) {
	Symbol* sym = (Symbol*)listget(prefix,i);
	slen += (strlen(sym->name)+strlen(separator));
    }
    slen += strlen(suffix);
    slen++; /* for null terminator*/
    result = poolalloc(slen);
    result[0] = '\0';
    /* Leave off the root*/
    i = (rootgroup == (Symbol*)listget(prefix,0))?1:0;
    for(;i<plen;i++) {
	Symbol* sym = (Symbol*)listget(prefix,i);
        strcat(result,sym->name); /* append "<prefix[i]/>"*/
	strcat(result,separator);
    }    
    strcat(result,suffix); /* append "<suffix>"*/
    return result;
}
#endif
