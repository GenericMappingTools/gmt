/*********************************************************************
 *   Copyright 2011, University Corporation for Atmospheric Research
 *   See netcdf/README file for copying and redistribution conditions.
 *   $Id$
 *********************************************************************/

#include "config.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>
#include <assert.h>
#include <ctype.h>
#include "utils.h"

/*
 * Print error message to stderr and exit
 */
void
error(const char *fmt, ...)
{
    va_list args ;

    (void) fprintf(stderr,"%s: ", progname);
    va_start(args, fmt) ;
    (void) vfprintf(stderr,fmt,args) ;
    va_end(args) ;

    (void) fprintf(stderr, "\n") ;
    (void) fflush(stderr);	/* to ensure log files are current */
    exit(EXIT_FAILURE);
}

void *
emalloc (			/* check return from malloc */
	size_t size)
{
    void   *p;

    p = (void *) malloc (size==0 ? 1 : size); /* malloc(0) not portable */
    if (p == 0) {
	error ("out of memory\n");
    }
    return p;
}


void
check(int err, const char* file, const int line)
{
    fprintf(stderr,"%s\n",nc_strerror(err));
    fprintf(stderr,"Location: file %s; line %d\n", file,line);
    fflush(stderr); fflush(stdout);
    exit(1);
}


/* 
 * Returns malloced name with chars special to CDL escaped.
 * Caller should free result when done with it.
 */
char*
escaped_name(const char* cp) {
    char *ret;			/* string returned */
    char *sp;
    assert(cp != NULL);

    /* For some reason, and on some machines (e.g. tweety)
       utf8 characters such as \343 are considered control character. */
/*    if(*cp && (isspace(*cp) | iscntrl(*cp)))*/
    if((*cp >= 0x01 && *cp <= 0x20) || (*cp == 0x7f))
    {
	error("name begins with space or control-character: %c",*cp);
    }

    ret = emalloc(4*strlen(cp) + 1); /* max if every char escaped */
    sp = ret;
    *sp = 0;			    /* empty name OK */
    /* Special case: leading number allowed, but we must escape it for CDL */
    if((*cp >= '0' && *cp <= '9'))
    {
	*sp++ = '\\';
    }
    for (; *cp; cp++) {
	if (isascii(*cp)) {
	    if(iscntrl(*cp)) {	/* render control chars as two hex digits, \%xx */
		snprintf(sp, 4,"\\%%%.2x", *cp);
		sp += 4;
	    } else {
		switch (*cp) {
		case ' ':
		case '!':
		case '"':
		case '#':
		case '$':
		case '%':
		case '&':
		case '\'':
		case '(':
		case ')':
		case '*':
		case ',':
		case ':':
		case ';':
		case '<':
		case '=':
		case '>':
		case '?':
		case '[':
		case ']':
		case '\\':
		case '^':
		case '`':
		case '{':
		case '|':
		case '}':
		case '~':
		    *sp++ = '\\';
		    *sp++ = *cp;
		    break;
		default:		/* includes '/' */
		    *sp++ = *cp;
		    break;
		}
	    }
	} else { 		/* not ascii, assume just UTF-8 byte */
	    *sp++ = *cp;
	}
    }
    *sp = 0;
    return ret;
}


/* 
 * Print name with escapes for special characters
 */
void
print_name(const char* name) {
    char *ename = escaped_name(name);
    fputs(ename, stdout);
    free(ename);
}

/* Missing functionality that should be in nc_inq_dimid(), to get
 * dimid from a full dimension path name that may include group
 * names */
int 
nc_inq_dimid2(int ncid, const char *dimname, int *dimidp) {
    int ret = NC_NOERR;
    /* If '/' doesn't occur in dimname, just return id found by
     * nc_inq_dimid() */
    char *sp = strrchr(dimname, '/');
    if(!sp) { /* No '/' in dimname, so return nc_inq_dimid() result */
	ret = nc_inq_dimid(ncid, dimname, dimidp);
    } 
#ifdef USE_NETCDF4
    else {  /* Parse group name out and get dimid using that */
	size_t grp_namelen = sp - dimname;
	char *grpname = emalloc(grp_namelen + 1);
	int grpid;
	strncpy(grpname, dimname, grp_namelen);
	grpname[grp_namelen] = '\0';
	ret = nc_inq_grp_full_ncid(ncid, grpname, &grpid);
	if(ret == NC_NOERR) {
	    ret = nc_inq_dimid(grpid, dimname, dimidp);
	}
	free(grpname);
    }	
#endif	/* USE_NETCDF4 */
    return ret;
}


/*
 * return 1 if varid identifies a record variable
 * else return 0
 */
int
isrecvar(int ncid, int varid)
{
    int ndims;
    int is_recvar = 0;
    int *dimids;

    NC_CHECK( nc_inq_varndims(ncid, varid, &ndims) );
#ifdef USE_NETCDF4
    if (ndims > 0) {
	int nunlimdims;
	int *recdimids;
	int dim, recdim;
	dimids = (int *) emalloc((ndims + 1) * sizeof(int));
	NC_CHECK( nc_inq_vardimid(ncid, varid, dimids) );
	NC_CHECK( nc_inq_unlimdims(ncid, &nunlimdims, NULL) );
	recdimids = (int *) emalloc((nunlimdims + 1) * sizeof(int));
	NC_CHECK( nc_inq_unlimdims(ncid, NULL, recdimids) );
	for (dim = 0; dim < ndims && is_recvar == 0; dim++) {
	    for(recdim = 0; recdim < nunlimdims; recdim++) {
		if(dimids[dim] == recdimids[recdim]) {
		    is_recvar = 1;
		    break;
		}		
	    }
	}
	free(dimids);
	free(recdimids);
    }
#else
    if (ndims > 0) {
	int recdimid;
	dimids = (int *) emalloc((ndims + 1) * sizeof(int));
	NC_CHECK( nc_inq_vardimid(ncid, varid, dimids) );
	NC_CHECK( nc_inq_unlimdim(ncid, &recdimid) );
	if(dimids[0] == recdimid)
	    is_recvar = 1;
	free(dimids);
    }
#endif /* USE_NETCDF4 */
    return is_recvar;
}

