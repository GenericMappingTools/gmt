/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen3/genlib.c,v 1.54 2009/11/14 22:33:31 dmh Exp $
 *********************************************************************/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>	/* for isprint() */
#ifndef NO_STDARG
#include	<stdarg.h>
#else
/* try varargs instead */
#include	<varargs.h>
#endif /* !NO_STDARG */
#include <netcdf.h>
#include "generic.h"
#include "ncgen.h"
#include "genlib.h"

extern char *netcdf_name; /* output netCDF filename, if on command line. */
extern int netcdf_flag;
extern int c_flag;
extern int fortran_flag;
extern int cmode_modifier;
extern int nofill_flag;

int	lineno = 1;
int	derror_count = 0;


/* create netCDF from in-memory structure */
static void
gen_netcdf(
     char *filename)		/* name for output netcdf file */
{
    int idim, ivar, iatt;
    int dimid;
    int varid;
    int stat;

    stat = nc_create(filename, cmode_modifier, &ncid);
    check_err(stat);

    /* define dimensions from info in dims array */
    for (idim = 0; idim < ndims; idim++) {
	stat = nc_def_dim(ncid, dims[idim].name, dims[idim].size, &dimid);
	check_err(stat);
    }

    /* define variables from info in vars array */
    for (ivar = 0; ivar < nvars; ivar++) {
	stat = nc_def_var(ncid,
			  vars[ivar].name,
			  vars[ivar].type,
			  vars[ivar].ndims,
			  vars[ivar].dims,
			  &varid);
	check_err(stat);
    }

    /* define attributes from info in atts array */
    for (iatt = 0; iatt < natts; iatt++) {
	varid = (atts[iatt].var == -1) ? NC_GLOBAL : atts[iatt].var;
	switch(atts[iatt].type) {
	case NC_BYTE:
	    stat = nc_put_att_schar(ncid, varid, atts[iatt].name,
				    atts[iatt].type, atts[iatt].len,
				    (signed char *) atts[iatt].val);
	    break;
	case NC_CHAR:
	    stat = nc_put_att_text(ncid, varid, atts[iatt].name,
				   atts[iatt].len,
				   (char *) atts[iatt].val);
	    break;
	case NC_SHORT:
	    stat = nc_put_att_short(ncid, varid, atts[iatt].name,
				    atts[iatt].type, atts[iatt].len,
				    (short *) atts[iatt].val);
	    break;
	case NC_INT:
	    stat = nc_put_att_int(ncid, varid, atts[iatt].name,
				    atts[iatt].type, atts[iatt].len,
				    (int *) atts[iatt].val);
	    break;
	case NC_FLOAT:
	    stat = nc_put_att_float(ncid, varid, atts[iatt].name,
				    atts[iatt].type, atts[iatt].len,
				    (float *) atts[iatt].val);
	    break;
	case NC_DOUBLE:
	    stat = nc_put_att_double(ncid, varid, atts[iatt].name,
				    atts[iatt].type, atts[iatt].len,
				    (double *) atts[iatt].val);
	    break;
	default:
	    stat = NC_EBADTYPE;
	}
	check_err(stat);
    }

    if (nofill_flag) {
	stat = nc_set_fill(ncid, NC_NOFILL, 0);	/* don't initialize with fill values */
	check_err(stat);
    }

    stat = nc_enddef(ncid);
    check_err(stat);
}


/*
 * Given a netcdf type, a pointer to a vector of values of that type,
 * and the index of the vector element desired, returns a pointer to a
 * malloced string representing the value in C.
 */
static char *
cstring(
     nc_type type,		/* netCDF type code */
     void *valp,		/* pointer to vector of values */
     int num)			/* element of vector desired */
{
    static char *cp, *sp, ch;
    signed char *bytep;
    short *shortp;
    int *intp;
    float *floatp;
    double *doublep;

    switch (type) {
      case NC_CHAR:
	sp = cp = (char *) emalloc (7);
	*cp++ = '\'';
	ch = *((char *)valp + num);
	switch (ch) {
	  case '\b': *cp++ = '\\'; *cp++ = 'b'; break;
	  case '\f': *cp++ = '\\'; *cp++ = 'f'; break;
	  case '\n': *cp++ = '\\'; *cp++ = 'n'; break;
	  case '\r': *cp++ = '\\'; *cp++ = 'r'; break;
	  case '\t': *cp++ = '\\'; *cp++ = 't'; break;
	  case '\v': *cp++ = '\\'; *cp++ = 'v'; break;
	  case '\\': *cp++ = '\\'; *cp++ = '\\'; break;
	  case '\'': *cp++ = '\\'; *cp++ = '\''; break;
	  default:
	    if (!isprint((unsigned char)ch)) {
		static char octs[] = "01234567";
		int rem = ((unsigned char)ch)%64;
		*cp++ = '\\';
		*cp++ = octs[((unsigned char)ch)/64]; /* to get, e.g. '\177' */
		*cp++ = octs[rem/8];
		*cp++ = octs[rem%8];
	    } else {
		*cp++ = ch;
	    }
	    break;
	}
	*cp++ = '\'';
	*cp = '\0';
	return sp;
	
      case NC_BYTE:
	cp = (char *) emalloc (7);
	bytep = (signed char *)valp;
	/* Need to convert '\377' to -1, for example, on all platforms */
	(void) sprintf(cp,"%d", (signed char) *(bytep+num));
	return cp;

      case NC_SHORT:
	cp = (char *) emalloc (10);
	shortp = (short *)valp;
	(void) sprintf(cp,"%d",* (shortp + num));
	return cp;

      case NC_INT:
	cp = (char *) emalloc (20);
	intp = (int *)valp;
	(void) sprintf(cp,"%d",* (intp + num));
	return cp;

      case NC_FLOAT:
	cp = (char *) emalloc (20);
	floatp = (float *)valp;
	(void) sprintf(cp,"%.8g",* (floatp + num));
	return cp;

      case NC_DOUBLE:
	cp = (char *) emalloc (20);
	doublep = (double *)valp;
	(void) sprintf(cp,"%.16g",* (doublep + num));
	return cp;

      default:
	derror("cstring: bad type code");
	return 0;
    }
}


/*
 * Generate C code for creating netCDF from in-memory structure.
 */
static void
gen_c(
     const char *filename)
{
    int idim, ivar, iatt, jatt, maxdims;
    int vector_atts;
    char *val_string;
    char stmnt[C_MAX_STMNT];

    /* wrap in main program */
    cline("#include <stdio.h>");
    cline("#include <stdlib.h>");
    cline("#include <netcdf.h>");
    cline("");
    cline("void");
    cline("check_err(const int stat, const int line, const char *file) {");
    cline("    if (stat != NC_NOERR) {");
    cline("	   (void) fprintf(stderr, \"line %d of %s: %s\\n\", line, file, nc_strerror(stat));");
    cline("        exit(1);");
    cline("    }");
    cline("}");
    cline("");
    cline("int");
    sprintf(stmnt, "main() {\t\t\t/* create %s */", filename);
    cline(stmnt);

    /* create necessary declarations */
    cline("");
    cline("   int  stat;\t\t\t/* return status */");
    cline("   int  ncid;\t\t\t/* netCDF id */");

    if (ndims > 0) {
	cline("");
	cline("   /* dimension ids */");
	for (idim = 0; idim < ndims; idim++) {
	    sprintf(stmnt, "   int %s_dim;", dims[idim].lname);
	    cline(stmnt);
	    }

	cline("");
	cline("   /* dimension lengths */");
	for (idim = 0; idim < ndims; idim++) {
	    if (dims[idim].size == NC_UNLIMITED) {
		sprintf(stmnt, "   size_t %s_len = NC_UNLIMITED;",
			dims[idim].lname);
	    } else {
		sprintf(stmnt, "   size_t %s_len = %lu;",
			dims[idim].lname,
			(unsigned long) dims[idim].size);
	    }
	    cline(stmnt);
	}
    }

    maxdims = 0;	/* most dimensions of any variable */
    for (ivar = 0; ivar < nvars; ivar++)
      if (vars[ivar].ndims > maxdims)
	maxdims = vars[ivar].ndims;

    if (nvars > 0) {
	cline("");
	cline("   /* variable ids */");
	for (ivar = 0; ivar < nvars; ivar++) {
	    sprintf(stmnt, "   int %s_id;", vars[ivar].lname);
	    cline(stmnt);
	}

	cline("");
	cline("   /* rank (number of dimensions) for each variable */");
	for (ivar = 0; ivar < nvars; ivar++) {
	    sprintf(stmnt, "#  define RANK_%s %d", vars[ivar].lname,
		    vars[ivar].ndims);
	    cline(stmnt);
	}
	if (maxdims > 0) {	/* we have dimensioned variables */
	    cline("");
	    cline("   /* variable shapes */");
	    for (ivar = 0; ivar < nvars; ivar++) {
		if (vars[ivar].ndims > 0) {
		    sprintf(stmnt, "   int %s_dims[RANK_%s];",
			    vars[ivar].lname, vars[ivar].lname);
		    cline(stmnt);
		}
	    }
	}
    }

    /* determine if we need any attribute vectors */
    vector_atts = 0;
    for (iatt = 0; iatt < natts; iatt++) {
	if (atts[iatt].type != NC_CHAR) {
	    vector_atts = 1;
	    break;
	}
    }
    if (vector_atts) {
	cline("");
	cline("   /* attribute vectors */");
	for (iatt = 0; iatt < natts; iatt++) {
	    if (atts[iatt].type != NC_CHAR) {
		sprintf(stmnt,
		    "   %s %s_%s[%lu];",
		    ncatype(atts[iatt].type),
		    atts[iatt].var == -1 ? "cdf" : vars[atts[iatt].var].lname,
		    atts[iatt].lname,
		    (unsigned long) atts[iatt].len);
		cline(stmnt);
	    }
	}
    }

    /* create netCDF file, uses NC_CLOBBER mode */
    cline("");
    cline("   /* enter define mode */");

    if (!cmode_modifier) {
	sprintf(stmnt,
		"   stat = nc_create(\"%s\", NC_CLOBBER, &ncid);",
		filename);
    } else if (cmode_modifier & NC_64BIT_OFFSET) {
	sprintf(stmnt,
		"   stat = nc_create(\"%s\", NC_CLOBBER|NC_64BIT_OFFSET, &ncid);",
		filename);
#ifdef USE_NETCDF4
    } else if (cmode_modifier & NC_CLASSIC_MODEL) {
	sprintf(stmnt,
		"   stat = nc_create(\"%s\", NC_CLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL, &ncid);",
		filename);
    } else if (cmode_modifier & NC_NETCDF4) {
	sprintf(stmnt,
		"   stat = nc_create(\"%s\", NC_CLOBBER|NC_NETCDF4, &ncid);",
		filename);
#endif
    } else {
       derror("unknown cmode modifier");
    }
    cline(stmnt);
    cline("   check_err(stat,__LINE__,__FILE__);");
    
    /* define dimensions from info in dims array */
    if (ndims > 0) {
	cline("");
	cline("   /* define dimensions */");
    }
    for (idim = 0; idim < ndims; idim++) {
	sprintf(stmnt,
		"   stat = nc_def_dim(ncid, \"%s\", %s_len, &%s_dim);",
		dims[idim].name, dims[idim].lname, dims[idim].lname);
	cline(stmnt);
	cline("   check_err(stat,__LINE__,__FILE__);");
    }

    /* define variables from info in vars array */
    if (nvars > 0) {
	cline("");
	cline("   /* define variables */");
	for (ivar = 0; ivar < nvars; ivar++) {
	    cline("");
	    for (idim = 0; idim < vars[ivar].ndims; idim++) {
		sprintf(stmnt,
			"   %s_dims[%d] = %s_dim;",
			vars[ivar].lname,
			idim,
			dims[vars[ivar].dims[idim]].lname);
		cline(stmnt);
	    }
	    if (vars[ivar].ndims > 0) {	/* a dimensioned variable */
		sprintf(stmnt,
			"   stat = nc_def_var(ncid, \"%s\", %s, RANK_%s, %s_dims, &%s_id);",
			vars[ivar].name,
			nctype(vars[ivar].type),
			vars[ivar].lname,
			vars[ivar].lname,
			vars[ivar].lname);
	    } else {		/* a scalar */
		sprintf(stmnt,
			"   stat = nc_def_var(ncid, \"%s\", %s, RANK_%s, 0, &%s_id);",
			vars[ivar].name,
			nctype(vars[ivar].type),
			vars[ivar].lname,
			vars[ivar].lname);
	    }
	    cline(stmnt);
	    cline("   check_err(stat,__LINE__,__FILE__);");
	}
    }
    
    /* define attributes from info in atts array */
    if (natts > 0) {
	cline("");
	cline("   /* assign attributes */");
	for (iatt = 0; iatt < natts; iatt++) {
	    if (atts[iatt].type == NC_CHAR) { /* string */
		val_string = cstrstr((char *) atts[iatt].val, atts[iatt].len);
		sprintf(stmnt,
			"   stat = nc_put_att_text(ncid, %s%s, \"%s\", %lu, %s);",
			atts[iatt].var == -1 ? "NC_GLOBAL" : vars[atts[iatt].var].lname,
			atts[iatt].var == -1 ? "" : "_id",
			atts[iatt].name,
			(unsigned long) atts[iatt].len,
			val_string);
		cline(stmnt);
		free (val_string);
	    }
	    else {			/* vector attribute */
		for (jatt = 0; jatt < atts[iatt].len ; jatt++) {
		    val_string = cstring(atts[iatt].type,atts[iatt].val,jatt);
		    sprintf(stmnt, "   %s_%s[%d] = %s;",
			    atts[iatt].var == -1 ? "cdf" : vars[atts[iatt].var].lname,
			    atts[iatt].lname,
			    jatt, 
			    val_string);
		    cline(stmnt);
		    free (val_string);
		}
		
		sprintf(stmnt,
			"   stat = nc_put_att_%s(ncid, %s%s, \"%s\", %s, %lu, %s_%s);",
			ncatype(atts[iatt].type),
			atts[iatt].var == -1 ? "NC_GLOBAL" : vars[atts[iatt].var].lname,
			atts[iatt].var == -1 ? "" : "_id",
			atts[iatt].name,
			nctype(atts[iatt].type),
			(unsigned long) atts[iatt].len,
			atts[iatt].var == -1 ? "cdf" : vars[atts[iatt].var].lname,
			atts[iatt].lname);
		cline(stmnt);
	    }
	    cline("   check_err(stat,__LINE__,__FILE__);");
	}
    }

    if (nofill_flag) {
        cline("   /* don't initialize variables with fill values */");
	cline("   stat = nc_set_fill(ncid, NC_NOFILL, 0);");
	cline("   check_err(stat,__LINE__,__FILE__);");
    }

    cline("");
    cline("   /* leave define mode */");
    cline("   stat = nc_enddef (ncid);");
    cline("   check_err(stat,__LINE__,__FILE__);");
}


/* return Fortran type name for netCDF type, given type code */
static const char *
ncftype(
     nc_type type)		/* netCDF type code */
{
    switch (type) {

      case NC_BYTE:
	return "integer";
      case NC_CHAR:
	return "character";
      case NC_SHORT:
	return "integer";
      case NC_INT:
#ifdef MSDOS
	return "integer*4";
#else
	return "integer";
#endif
      case NC_FLOAT:
	return "real";
#if defined(_CRAY) && !defined(__crayx1)
      case NC_DOUBLE:
	return "real";		/* we don't support CRAY 128-bit doubles */
#else
      case NC_DOUBLE:
	return "double precision";
#endif
      default:
	derror("ncftype: bad type code");
	return 0;

    }
}


/* return Fortran type suffix for netCDF type, given type code */
const char *
nfstype(
     nc_type type)		/* netCDF type code */
{
    switch (type) {
      case NC_BYTE:
	return "int1";
      case NC_CHAR:
	return "text";
      case NC_SHORT:
	return "int2";
      case NC_INT:
	return "int";
      case NC_FLOAT:
	return "real";
      case NC_DOUBLE:
	return "double";
      default:
	derror("nfstype: bad type code");
	return 0;

    }
}


/* Return Fortran function suffix for netCDF type, given type code.
 * This should correspond to the Fortran type name in ncftype().
 */
const char *
nfftype(
     nc_type type)		/* netCDF type code */
{
    switch (type) {
      case NC_BYTE:
	return "int";
      case NC_CHAR:
	return "text";
      case NC_SHORT:
	return "int";
      case NC_INT:
	return "int";
      case NC_FLOAT:
	return "real";
#if defined(_CRAY) && !defined(__crayx1)
      case NC_DOUBLE:
	return "real";		/* we don't support CRAY 128-bit doubles */
#else
      case NC_DOUBLE:
	return "double";
#endif
      default:
	derror("nfstype: bad type code");
	return 0;

    }
}


/* return FORTRAN name for netCDF type, given type code */
static const char *
ftypename(
     nc_type type)			/* netCDF type code */
{
    switch (type) {
      case NC_BYTE:
	return "NF_INT1";
      case NC_CHAR:
	return "NF_CHAR";
      case NC_SHORT:
	return "NF_INT2";
      case NC_INT:
	return "NF_INT";
      case NC_FLOAT:
	return "NF_REAL";
      case NC_DOUBLE:
	return "NF_DOUBLE";
      default:
	derror("ftypename: bad type code");
	return 0;
    }
}


/*
 * Generate FORTRAN code for creating netCDF from in-memory structure.
 */
static void
gen_fortran(
     const char *filename)
{
    int idim, ivar, iatt, jatt, itype, maxdims;
    int vector_atts;
    char *val_string;
    char stmnt[FORT_MAX_STMNT];
    char s2[NC_MAX_NAME + 10];
    char *sp;
    /* Need how many netCDF types there are, because we create an array
     * for each type of attribute. */
    int ntypes = 6;		/* number of netCDF types, NC_BYTE, ... */
    nc_type types[6];		/* at least ntypes */
    size_t max_atts[NC_DOUBLE + 1];

    types[0] = NC_BYTE;
    types[1] = NC_CHAR;
    types[2] = NC_SHORT;
    types[3] = NC_INT;
    types[4] = NC_FLOAT;
    types[5] = NC_DOUBLE;

    fline("program fgennc");

    fline("include 'netcdf.inc'");

    /* create necessary declarations */
    fline("* error status return");
    fline("integer  iret");
    fline("* netCDF id");
    fline("integer  ncid");
    if (nofill_flag) {
        fline("* to save old fill mode before changing it temporarily");
	fline("integer  oldmode");
    }

    if (ndims > 0) {
	fline("* dimension ids");
	for (idim = 0; idim < ndims; idim++) {
	    sprintf(stmnt, "integer  %s_dim", dims[idim].lname);
	    fline(stmnt);
	}

	fline("* dimension lengths");
	for (idim = 0; idim < ndims; idim++) {
	    sprintf(stmnt, "integer  %s_len", dims[idim].lname);
	    fline(stmnt);
	}
	for (idim = 0; idim < ndims; idim++) {
	    if (dims[idim].size == NC_UNLIMITED) {
		sprintf(stmnt, "parameter (%s_len = NF_UNLIMITED)",
			dims[idim].lname);
	    } else {
		sprintf(stmnt, "parameter (%s_len = %lu)",
			dims[idim].lname,
			(unsigned long) dims[idim].size);
	    }
	    fline(stmnt);
	}
	
    }

    maxdims = 0;		/* most dimensions of any variable */
    for (ivar = 0; ivar < nvars; ivar++)
      if (vars[ivar].ndims > maxdims)
	maxdims = vars[ivar].ndims;

    if (nvars > 0) {
	fline("* variable ids");
	for (ivar = 0; ivar < nvars; ivar++) {
	    sprintf(stmnt, "integer  %s_id", vars[ivar].lname);
	    fline(stmnt);
	}

	fline("* rank (number of dimensions) for each variable");
	for (ivar = 0; ivar < nvars; ivar++) {
	    sprintf(stmnt, "integer  %s_rank", vars[ivar].lname);
	    fline(stmnt);
	}
	for (ivar = 0; ivar < nvars; ivar++) {
	    sprintf(stmnt, "parameter (%s_rank = %d)", vars[ivar].lname,
		    vars[ivar].ndims);
	    fline(stmnt);
	}
	
	fline("* variable shapes");
	for (ivar = 0; ivar < nvars; ivar++) {
	    if (vars[ivar].ndims > 0) {
		sprintf(stmnt, "integer  %s_dims(%s_rank)",
			vars[ivar].lname, vars[ivar].lname);
		fline(stmnt);
	    }
	}
    }

    /* declarations for variables to be initialized */
    if (nvars > 0) {		/* we have variables */
	fline("* data variables");
	for (ivar = 0; ivar < nvars; ivar++) {
	    struct vars *v = &vars[ivar];
	    /* Generate declarations here for non-record data variables only.
	       Record variables are declared in separate subroutine later,
               when we know how big they are. */
	    if (v->ndims > 0 && v->dims[0] == rec_dim) {
		continue;
	    }
	    /* Make declarations for non-text variables only;
	       for text variables, just include string in nf_put_var call */
	    if (v->type == NC_CHAR) {
                continue;
            }
	    if (v->ndims == 0) { /* scalar */
		sprintf(stmnt, "%s  %s", ncftype(v->type),
			v->lname);
	    } else {
		sprintf(stmnt, "%s  %s(", ncftype(v->type),
			v->lname);
		/* reverse dimensions for FORTRAN */
		for (idim = v->ndims-1; idim >= 0; idim--) {
		    sprintf(s2, "%s_len, ",
			    dims[v->dims[idim]].lname);
		    strcat(stmnt, s2);
		}
		sp = strrchr(stmnt, ',');
		if(sp != NULL) {
		    *sp = '\0';
		}
		strcat(stmnt, ")");
	    }
	    fline(stmnt);
	}
    }

    /* determine what attribute vectors needed */
    for (itype = 0; itype < ntypes; itype++)
        max_atts[(int)types[itype]] = 0;

    vector_atts = 0;
    for (iatt = 0; iatt < natts; iatt++) {
	if (atts[iatt].len > max_atts[(int) atts[iatt].type]) {
	    max_atts[(int)atts[iatt].type] = atts[iatt].len;
	    vector_atts = 1;
	}
    }
    if (vector_atts) {
	fline("* attribute vectors");
	for (itype = 0; itype < ntypes; itype++) {
	    if (types[itype] != NC_CHAR && max_atts[(int)types[itype]] > 0) {
		sprintf(stmnt, "%s  %sval(%lu)", ncftype(types[itype]),
			nfstype(types[itype]),
			(unsigned long) max_atts[(int)types[itype]]);
		fline(stmnt);
	    }
	}
    }

    /* create netCDF file, uses NC_CLOBBER mode */
    fline("* enter define mode");
    if (!cmode_modifier) {
	sprintf(stmnt, "iret = nf_create(\'%s\', NF_CLOBBER, ncid)", filename);
    } else if (cmode_modifier & NC_64BIT_OFFSET) {
	sprintf(stmnt, "iret = nf_create(\'%s\', OR(NF_CLOBBER,NF_64BIT_OFFSET), ncid)", filename);
#ifdef USE_NETCDF4
    } else if (cmode_modifier & NC_CLASSIC_MODEL) {
	sprintf(stmnt, "iret = nf_create(\'%s\', OR(NF_CLOBBER,NC_NETCDF4,NC_CLASSIC_MODEL), ncid)", filename);
    } else if (cmode_modifier & NC_NETCDF4) {
	sprintf(stmnt, "iret = nf_create(\'%s\', OR(NF_CLOBBER,NF_NETCDF4), ncid)", filename);
#endif
    } else {
       derror("unknown cmode modifier");
    }
    fline(stmnt);
    fline("call check_err(iret)");
    
    /* define dimensions from info in dims array */
    if (ndims > 0)
        fline("* define dimensions");
    for (idim = 0; idim < ndims; idim++) {
	if (dims[idim].size == NC_UNLIMITED)
            sprintf(stmnt, "iret = nf_def_dim(ncid, \'%s\', NF_UNLIMITED, %s_dim)",
                    dims[idim].name, dims[idim].lname);
	else
            sprintf(stmnt, "iret = nf_def_dim(ncid, \'%s\', %lu, %s_dim)",
                    dims[idim].name, (unsigned long) dims[idim].size,
			dims[idim].lname);
	fline(stmnt);
	fline("call check_err(iret)");
    }
	  
    /* define variables from info in vars array */
    if (nvars > 0) {
	fline("* define variables");
	for (ivar = 0; ivar < nvars; ivar++) {
	    for (idim = 0; idim < vars[ivar].ndims; idim++) {
		sprintf(stmnt, "%s_dims(%d) = %s_dim",
			vars[ivar].lname,
			vars[ivar].ndims - idim, /* reverse dimensions */
			dims[vars[ivar].dims[idim]].lname);
		fline(stmnt);
	    }
	    if (vars[ivar].ndims > 0) {	/* a dimensioned variable */
		sprintf(stmnt, 
			"iret = nf_def_var(ncid, \'%s\', %s, %s_rank, %s_dims, %s_id)",
			vars[ivar].name,
			ftypename(vars[ivar].type),
			vars[ivar].lname,
			vars[ivar].lname,
			vars[ivar].lname);
	    } else {		/* a scalar */
		sprintf(stmnt, 
			"iret = nf_def_var(ncid, \'%s\', %s, %s_rank, 0, %s_id)",
			vars[ivar].name,
			ftypename(vars[ivar].type),
			vars[ivar].lname,
			vars[ivar].lname);
	    }
	    fline(stmnt);
	    fline("call check_err(iret)");
	}
    }

    /* define attributes from info in atts array */
    if (natts > 0) {
	fline("* assign attributes");
	for (iatt = 0; iatt < natts; iatt++) {
	    if (atts[iatt].type == NC_CHAR) { /* string */
		val_string = fstrstr((char *) atts[iatt].val, atts[iatt].len);
		sprintf(stmnt, 
			"iret = nf_put_att_text(ncid, %s%s, \'%s\', %lu, %s)",
			atts[iatt].var == -1 ? "NF_GLOBAL" : vars[atts[iatt].var].lname,
			atts[iatt].var == -1 ? "" : "_id",
			atts[iatt].name,
			(unsigned long) atts[iatt].len,
			val_string);
		fline(stmnt);
		fline("call check_err(iret)");
		free(val_string);
	    } else {
		for (jatt = 0; jatt < atts[iatt].len ; jatt++) {
		    val_string = fstring(atts[iatt].type,atts[iatt].val,jatt);
		    sprintf(stmnt, "%sval(%d) = %s",
			    nfstype(atts[iatt].type),
			    jatt+1, 
			    val_string);
		    fline(stmnt);
		    free (val_string);
		}
	    
		sprintf(stmnt,
			"iret = nf_put_att_%s(ncid, %s%s, \'%s\', %s, %lu, %sval)",
			nfftype(atts[iatt].type),
			atts[iatt].var == -1 ? "NCGLOBAL" : vars[atts[iatt].var].lname,
			atts[iatt].var == -1 ? "" : "_id",
			atts[iatt].name,
			ftypename(atts[iatt].type),
			(unsigned long) atts[iatt].len,
			nfstype(atts[iatt].type));
		fline(stmnt);
		fline("call check_err(iret)");
	    }
	}
    }

    if (nofill_flag) {
        fline("* don't initialize variables with fill values");
	fline("iret = nf_set_fill(ncid, NF_NOFILL, oldmode)");
	fline("call check_err(iret)");
    }

    fline("* leave define mode");
    fline("iret = nf_enddef(ncid)");
    fline("call check_err(iret)");
}


/*
 * Output a C statement.
 */
void
cline(
     const char *stmnt)
{
    FILE *cout = stdout;
    
    fputs(stmnt, cout);
    fputs("\n", cout);
}

/*
 * From a long line FORTRAN statment, generates the necessary FORTRAN
 * lines with continuation characters in column 6.  If stmnt starts with "*",
 * it is treated as a one-line comment.  Statement labels are *not* handled,
 * but since we don't generate any labels, we don't care.
 */
void
fline(
     const char *stmnt)
{
    FILE *fout = stdout;
    int len = (int) strlen(stmnt);
    int line = 0;
    static char cont[] = {	/* continuation characters */
	' ', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'+', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'+', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    
    if(stmnt[0] == '*') {
	fputs(stmnt, fout);
	fputs("\n", fout);
	return;
    }

    while (len > 0) {
	if (line >= FORT_MAX_LINES)
	  derror("FORTRAN statement too long: %s",stmnt);
	(void) fprintf(fout, "     %c", cont[line++]);
	(void) fprintf(fout, "%.66s\n", stmnt);
	len -= 66;
	if (len > 0)
	  stmnt += 66;
    }
}


/* return C name for netCDF type, given type code */
const char *
nctype(
     nc_type type)			/* netCDF type code */
{
    switch (type) {
      case NC_BYTE:
	return "NC_BYTE";
      case NC_CHAR:
	return "NC_CHAR";
      case NC_SHORT:
	return "NC_SHORT";
      case NC_INT:
	return "NC_INT";
      case NC_FLOAT:
	return "NC_FLOAT";
      case NC_DOUBLE:
	return "NC_DOUBLE";
      default:
	derror("nctype: bad type code");
	return 0;
    }
}


/*
 * Return C type name for netCDF type, given type code.
 */
const char *
ncctype(
     nc_type type)			/* netCDF type code */
{
    switch (type) {
      case NC_BYTE:
	return "signed char";
      case NC_CHAR:
	return "char";
      case NC_SHORT:
	return "short";
      case NC_INT:
	return "int";
      case NC_FLOAT:
	return "float";
      case NC_DOUBLE:
	return "double";
      default:
	derror("ncctype: bad type code");
	return 0;
    }
}



/*
 * Return C type name for netCDF type suffix, given type code.
 */
const char *
ncstype(
     nc_type type)			/* netCDF type code */
{
    switch (type) {
      case NC_BYTE:
	return "schar";
      case NC_CHAR:
	return "text";
      case NC_SHORT:
	return "short";
      case NC_INT:
	return "int";
      case NC_FLOAT:
	return "float";
      case NC_DOUBLE:
	return "double";
      default:
	derror("ncstype: bad type code");
	return 0;
    }
}


/*
 * Return C type name for netCDF attribute container type, given type code.
 */
const char *
ncatype(
     nc_type type)			/* netCDF type code */
{
    switch (type) {
      case NC_BYTE:
	return "int";		/* avoids choosing between uchar and schar */
      case NC_CHAR:
	return "text";
      case NC_SHORT:
	return "short";
      case NC_INT:
	return "int";
      case NC_FLOAT:
	return "float";
      case NC_DOUBLE:
	return "double";
      default:
	derror("ncatype: bad type code");
	return 0;
    }
}


/* return internal size for values of specified netCDF type */
size_t
nctypesize(
     nc_type type)			/* netCDF type code */
{
    switch (type) {
      case NC_BYTE:
	return sizeof(char);
      case NC_CHAR:
	return sizeof(char);
      case NC_SHORT:
	return sizeof(short);
      case NC_INT:
	return sizeof(int);
      case NC_FLOAT:
	return sizeof(float);
      case NC_DOUBLE:
	return sizeof(double);
      default:
	derror("nctypesize: bad type code");
	return 0;
    }
}


/*
 * Given a netcdf numeric type, a pointer to a vector of values of that
 * type, and the index of the vector element desired, returns a pointer
 * to a malloced string representing the value in FORTRAN.  Since this
 * may be used in a DATA statement, it must not include non-constant
 * expressions, such as "char(26)".
 */
char *
fstring(
     nc_type type,		/* netCDF type code */
     void *valp,		/* pointer to vector of values */
     int num)			/* element of vector desired */
{
    static char *cp;
    signed char *schp;
    short *shortp;
    int *intp;
    float *floatp;
    double *doublep;

    switch (type) {
      case NC_BYTE:
	cp = (char *) emalloc (10);
	schp = (signed char *)valp;
        sprintf(cp,"%d", schp[num]);
	return cp;

      case NC_SHORT:
	cp = (char *) emalloc (10);
	shortp = (short *)valp;
	(void) sprintf(cp,"%d",* (shortp + num));
	return cp;

      case NC_INT:
	cp = (char *) emalloc (20);
	intp = (int *)valp;
	(void) sprintf(cp,"%d",* (intp + num));
	return cp;

      case NC_FLOAT:
	cp = (char *) emalloc (20);
	floatp = (float *)valp;
	(void) sprintf(cp,"%.8g",* (floatp + num));
	return cp;

      case NC_DOUBLE:
	cp = (char *) emalloc (25);
	doublep = (double *)valp;
	(void) sprintf(cp,"%.16g",* (doublep + num));
	expe2d(cp);	/* change 'e' to 'd' in exponent */
	return cp;

      default:
	derror("fstring: bad type code");
	return 0;
    }
}


/*
 * Given a pointer to a counted string, returns a pointer to a malloced string
 * representing the string as a C constant.
 */
char *
cstrstr(
     const char *valp,		/* pointer to vector of characters*/
     size_t len)		/* number of characters in valp */
{
    static char *sp;
    char *cp;
    char *istr, *istr0;		/* for null-terminated copy */
    int ii;
    
    if(4*len+3 != (unsigned)(4*len+3)) {
	derror("too much character data!");
	exit(9);
    }
    sp = cp = (char *) emalloc(4*len+3);

    if(len == 1 && *valp == 0) { /* empty string */
	strcpy(sp,"\"\"");
	return sp;
    }

    istr0 = istr = (char *) emalloc(len + 1);
    for(ii = 0; ii < len; ii++) {
	istr[ii] = valp[ii];
    }
    istr[len] = '\0';

    *cp++ = '"';
    for(ii = 0; ii < len; ii++) {
	switch (*istr) {
	case '\0': *cp++ = '\\'; *cp++ = '0'; *cp++ = '0'; *cp++ = '0'; break;
	case '\b': *cp++ = '\\'; *cp++ = 'b'; break;
	case '\f': *cp++ = '\\'; *cp++ = 'f'; break;
	case '\n': *cp++ = '\\'; *cp++ = 'n'; break;
	case '\r': *cp++ = '\\'; *cp++ = 'r'; break;
	case '\t': *cp++ = '\\'; *cp++ = 't'; break;
	case '\v': *cp++ = '\\'; *cp++ = 'v'; break;
	case '\\': *cp++ = '\\'; *cp++ = '\\'; break;
	case '\"': *cp++ = '\\'; *cp++ = '\"'; break;
	default:
	    if (!isprint((unsigned char)*istr)) {
		static char octs[] = "01234567";
		int rem = ((unsigned char)*istr)%64;
		*cp++ = '\\';
		*cp++ = octs[((unsigned char)*istr)/64]; /* to get, e.g. '\177' */
		*cp++ = octs[rem/8];
		*cp++ = octs[rem%8];
	    } else {
		*cp++ = *istr;
	    }
	    break;
	}
	istr++;
    }
    *cp++ = '"';
    *cp = '\0';
    free(istr0);
    return sp;
}


/* Given a pointer to a counted string (not necessarily
 * null-terminated), returns a pointer to a malloced string representing
 * the string as a FORTRAN string expression.  For example, the string
 * "don't" would yield the FORTRAN string "'don''t'", and the string
 * "ab\ncd" would yield "'ab'//char(10)//'cd'".  The common
 * interpretation of "\"-escaped characters is non-standard, so the
 * generated Fortran may require adjustment in compilers that don't
 * recognize "\" as anything special in a character context.  */
char *
fstrstr(
     const char *str,			/* pointer to vector of characters */
     size_t ilen)			/* number of characters in istr */
{
    static char *ostr;
    char *cp, tstr[12];
    int was_print = 0;		/* true if last character was printable */
    char *istr, *istr0;		/* for null-terminated copy */
    int ii;

    if(12*ilen != (size_t)(12*ilen)) {
	derror("too much character data!");
	exit(9);
    }
    istr0 = istr = (char *) emalloc(ilen + 1);
    for(ii = 0; ii < ilen; ii++) {
	istr[ii] = str[ii];
    }
    istr[ilen] = '\0';
    
    if (*istr == '\0') {	/* empty string input, not legal in FORTRAN */
	ostr = (char*) emalloc(strlen("char(0)") + 1);
	strcpy(ostr, "char(0)");
	free(istr0);
	return ostr;
    }
    ostr = cp = (char *) emalloc(12*ilen);
    *ostr = '\0';
    if (isprint((unsigned char)*istr)) { /* handle first character in input */
	*cp++ = '\'';
	switch (*istr) {
	case '\'':
	    *cp++ = '\'';
	    *cp++ = '\'';
	    break;
	case '\\':
	    *cp++ = '\\';
	    *cp++ = '\\';
	    break;
	default:
	    *cp++ = *istr;
	    break;
	}
	*cp = '\0';
	was_print = 1;
    } else {
	sprintf(tstr, "char(%d)", (unsigned char)*istr);
	strcat(cp, tstr);
	cp += strlen(tstr);
	was_print = 0;
    }
    istr++;

    for(ii = 1; ii < ilen; ii++) { /* handle subsequent characters in input */
	if (isprint((unsigned char)*istr)) {
	    if (! was_print) {
		strcat(cp, "//'");
		cp += 3;
	    }
	    switch (*istr) {
	    case '\'':
		*cp++ = '\'';
		*cp++ = '\'';
		break;
	    case '\\':
		*cp++ = '\\';
		*cp++ = '\\';
		break;
	    default:
		*cp++ = *istr;
		break;
	    }
	    *cp = '\0';
	    was_print = 1;
	} else {
	    if (was_print) {
		*cp++ = '\'';
		*cp = '\0';
	    }
	    sprintf(tstr, "//char(%d)", (unsigned char)*istr);
	    strcat(cp, tstr);
	    cp += strlen(tstr);
	    was_print = 0;
	}
	istr++;
    }
    if (was_print)
      *cp++ = '\'';
    *cp = '\0';
    free(istr0);
    return ostr;
}


static void
cl_netcdf(void)
{
    int stat = nc_close(ncid);
    check_err(stat);
}


static void
cl_c(void)
{
    cline("   stat = nc_close(ncid);");
    cline("   check_err(stat,__LINE__,__FILE__);");
#ifndef vms
    cline("   return 0;");
#else
    cline("   return 1;");
#endif
    cline("}");
}

/* Returns true if dimension used in at least one record variable,
  otherwise false.  This is an inefficient algorithm, but we don't call
  it very often ... */
static int
used_in_rec_var(
    int idim			/* id of dimension */
    ) {
    int ivar;
    
    for (ivar = 0; ivar < nvars; ivar++) {
	if (vars[ivar].ndims > 0 && vars[ivar].dims[0] == rec_dim) {
	    int jdim;
	    for (jdim = 0; jdim < vars[ivar].ndims; jdim++) {
		if (vars[ivar].dims[jdim] == idim)
		    return 1;
	    }
	}
    }
    return 0;
}


/* Return name for Fortran fill constant of specified type */
static const char *
f_fill_name(
    nc_type type
    )
{
    switch(type) {
    case NC_BYTE:
	return "NF_FILL_BYTE";
    case NC_CHAR:
	return "NF_FILL_CHAR";
    case NC_SHORT:
	return "NF_FILL_SHORT";
    case NC_INT:
	return "NF_FILL_INT";
    case NC_FLOAT:
	return "NF_FILL_FLOAT";
    case NC_DOUBLE:
	return "NF_FILL_DOUBLE";
    default: break;
    }
    derror("f_fill_name: bad type code");
    return 0;
}


/* Generate Fortran for cleaning up and closing file */
static void
cl_fortran(void)
{
    int ivar;
	    int idim;
    char stmnt[FORT_MAX_STMNT];
    char s2[FORT_MAX_STMNT];
    char*sp;
    int have_rec_var = 0;
    
    /* do we have any record variables? */
    for (ivar = 0; ivar < nvars; ivar++) {
	struct vars *v = &vars[ivar];
        if (v->ndims > 0 && v->dims[0] == rec_dim) {
	    have_rec_var = 1;
            break;
        }
    }        

    if (have_rec_var) {
	fline(" ");
	fline("* Write record variables");
        sprintf(stmnt, "call writerecs(ncid,");
        /* generate parameter list for subroutine to write record vars */
        for (ivar = 0; ivar < nvars; ivar++) {
            struct vars *v = &vars[ivar];
            /* if a record variable, include id in parameter list */
            if (v->ndims > 0 && v->dims[0] == rec_dim) {
                sprintf(s2, "%s_id,", v->lname);
                strcat(stmnt, s2);
            }
        }        
        sp = strrchr(stmnt, ',');
        if(sp != NULL) {
            *sp = '\0';
        }
        strcat(stmnt, ")");
        fline(stmnt);
    }
    
    fline(" ");
    fline("iret = nf_close(ncid)");
    fline("call check_err(iret)");
    fline("end");

    fline(" ");

    if (have_rec_var) {
        sprintf(stmnt, "subroutine writerecs(ncid,");
        for (ivar = 0; ivar < nvars; ivar++) {
            struct vars *v = &vars[ivar];
            if (v->ndims > 0 && v->dims[0] == rec_dim) {
                sprintf(s2, "%s_id,", v->lname);
                strcat(stmnt, s2);
            }
        }        
        sp = strrchr(stmnt, ',');
        if(sp != NULL) {
            *sp = '\0';
        }
        strcat(stmnt, ")");
        fline(stmnt);
	fline(" ");
        fline("* netCDF id");
        fline("integer  ncid");

	fline("* variable ids");
	for (ivar = 0; ivar < nvars; ivar++) {
	    struct vars *v = &vars[ivar];
            if (v->ndims > 0 && v->dims[0] == rec_dim) {
                sprintf(stmnt, "integer  %s_id", v->lname);
                fline(stmnt);
            }
	}

	fline(" ");
        fline("include 'netcdf.inc'");

        /* create necessary declarations */
        fline("* error status return");
        fline("integer  iret");

        /* generate integer/parameter declarations for all dimensions
          used in record variables, except record dimension. */
        fline(" ");
        fline("* netCDF dimension sizes for dimensions used with record variables");
        for (idim = 0; idim < ndims; idim++) {
            /* if used in a record variable and not record dimension */
            if (used_in_rec_var(idim) && dims[idim].size != NC_UNLIMITED) {
                sprintf(stmnt, "integer  %s_len", dims[idim].lname);
                fline(stmnt);
                sprintf(stmnt, "parameter (%s_len = %lu)",
                        dims[idim].lname, (unsigned long) dims[idim].size);
                fline(stmnt);
            }
        }

	fline(" ");
	fline("* rank (number of dimensions) for each variable");
	for (ivar = 0; ivar < nvars; ivar++) {
	    struct vars *v = &vars[ivar];
            if (v->ndims > 0 && v->dims[0] == rec_dim) {
                sprintf(stmnt, "integer  %s_rank", v->lname);
                fline(stmnt);
            }
	}
	for (ivar = 0; ivar < nvars; ivar++) {
	    struct vars *v = &vars[ivar];
            if (v->ndims > 0 && v->dims[0] == rec_dim) {
                sprintf(stmnt, "parameter (%s_rank = %d)", v->lname,
                        v->ndims);
                fline(stmnt);
            }
	}

	fline("* starts and counts for array sections of record variables");
	for (ivar = 0; ivar < nvars; ivar++) {
	    struct vars *v = &vars[ivar];
	    if (v->ndims > 0 && v->dims[0] == rec_dim) {
		sprintf(stmnt,
			"integer  %s_start(%s_rank), %s_count(%s_rank)",
			v->lname, v->lname, v->lname, v->lname);
		fline(stmnt);
	    }
	}
        
	fline(" ");
	fline("* data variables");
        
        for (ivar = 0; ivar < nvars; ivar++) {
            struct vars *v = &vars[ivar];
            if (v->ndims > 0 && v->dims[0] == rec_dim) {
                char *sp;
	    
                fline(" ");
                sprintf(stmnt, "integer  %s_nr", v->lname);
                fline(stmnt);
                if (v->nrecs > 0) {
                    sprintf(stmnt, "parameter (%s_nr = %lu)",
                            v->lname, (unsigned long) v->nrecs);
                } else {
                    sprintf(stmnt, "parameter (%s_nr = 1)",
                            v->lname);
                }
                fline(stmnt);
		if (v->type != NC_CHAR) {
		    sprintf(stmnt, "%s  %s(", ncftype(v->type),
			    v->lname);
		    /* reverse dimensions for FORTRAN */
		    for (idim = v->ndims-1; idim >= 0; idim--) {
			if(v->dims[idim] == rec_dim) {
			    sprintf(s2, "%s_nr, ", v->lname);
			} else {
			    sprintf(s2, "%s_len, ",
				    dims[v->dims[idim]].lname);
			}
			strcat(stmnt, s2);
		    }
		    sp = strrchr(stmnt, ',');
		    if(sp != NULL) {
			*sp = '\0';
		    }
		    strcat(stmnt, ")");
		    fline(stmnt);
		}
            }
        }

        fline(" ");

        /* Emit DATA statements after declarations, because f2c on Linux can't
          handle interspersing them */
        for (ivar = 0; ivar < nvars; ivar++) {
            struct vars *v = &vars[ivar];

            if (v->ndims > 0 && v->dims[0] == rec_dim && v->type != NC_CHAR) {
                if (v->has_data) {
                    fline(v->data_stmnt);
                } else {		/* generate data statement for FILL record */
                    size_t rec_len = 1;
                    for (idim = 1; idim < v->ndims; idim++) {
                        rec_len *= dims[v->dims[idim]].size;
                    }
                    sprintf(stmnt,"data %s /%lu * %s/", v->lname,
			(unsigned long) rec_len,
                            f_fill_name(v->type));		
                    fline(stmnt);
                }
            }
        }
	fline(" ");
	for (ivar = 0; ivar < nvars; ivar++) {
	    struct vars *v = &vars[ivar];
	    /* if a record variable, declare starts and counts */
	    if (v->ndims > 0 && v->dims[0] == rec_dim) {
		if (!v->has_data)
		    continue;
		sprintf(stmnt, "* store %s", v->name);
		fline(stmnt);

		for (idim = 0; idim < v->ndims; idim++) {
		    sprintf(stmnt, "%s_start(%d) = 1", v->lname, idim+1);
		    fline(stmnt);
		}
		for (idim = v->ndims-1; idim > 0; idim--) {
		    sprintf(stmnt, "%s_count(%d) = %s_len", v->lname,
			    v->ndims - idim, dims[v->dims[idim]].lname);
		    fline(stmnt);
		}
                sprintf(stmnt, "%s_count(%d) = %s_nr", v->lname,
                        v->ndims, v->lname);
		fline(stmnt);
		
		if (v->type != NC_CHAR) {
		    sprintf(stmnt,
			    "iret = nf_put_vara_%s(ncid, %s_id, %s_start, %s_count, %s)",
			    nfftype(v->type), v->lname, v->lname, v->lname, v->lname);
		} else {
		    sprintf(stmnt,
			    "iret = nf_put_vara_%s(ncid, %s_id, %s_start, %s_count, %s)",
			    nfftype(v->type), v->lname, v->lname, v->lname,
			    v->data_stmnt);
		}
		
		fline(stmnt);
		fline("call check_err(iret)");
	    }
	}

        fline(" ");

        fline("end");

        fline(" ");
    }

    fline("subroutine check_err(iret)");
    fline("integer iret");
    fline("include 'netcdf.inc'");
    fline("if (iret .ne. NF_NOERR) then");
    fline("print *, nf_strerror(iret)");
    fline("stop");
    fline("endif");
    fline("end");
}


/* invoke netcdf calls (or generate C or Fortran code) to create netcdf
 * from in-memory structure. */
void
define_netcdf(
     const char *netcdfname)
{
    char *filename;		/* output file name */
    
    if (netcdf_name) {		/* name given on command line */
	filename = netcdf_name;
    } else {			/* construct name from CDL name */
	filename = (char *) emalloc(strlen(netcdfname) + 5);
	(void) strcpy(filename,netcdfname);
	if (netcdf_flag == -1)
	  (void) strcat(filename,".cdf"); /* old, deprecated extension */
	else
	  (void) strcat(filename,".nc"); /* new, favored extension */
    }
    if (netcdf_flag)
      gen_netcdf(filename);	/* create netcdf */
    if (c_flag)			/* create C code to create netcdf */
      gen_c(filename);
    if (fortran_flag)		/* create Fortran code to create netcdf */
      gen_fortran(filename);
    free(filename);
}


void
close_netcdf(void)
{
    if (netcdf_flag)
      cl_netcdf();		/* close netcdf */
    if (c_flag)			/* create C code to close netcdf */
      cl_c();
    if (fortran_flag)		/* create Fortran code to close netcdf */
      cl_fortran();
}


void
check_err(int stat) {
    if (stat != NC_NOERR) {
	fprintf(stderr, "ncgen: %s\n", nc_strerror(stat));
	derror_count++;
    }
}

/*
 * For logging error conditions.
 */
#ifndef NO_STDARG
void
derror(const char *fmt, ...)
#else
/*VARARGS1*/
void
derror(fmt, va_alist)
     const char *fmt ;		/* error-message printf-style format */
     va_dcl			/* variable number of error args, if any */
#endif /* !NO_STDARG */
{
    va_list args ;


    if (lineno == 1)
      (void) fprintf(stderr,"%s: %s: ", progname, cdlname);
    else  
      (void) fprintf(stderr,"%s: %s line %d: ", progname, cdlname, lineno);

#ifndef NO_STDARG
    va_start(args ,fmt) ;
#else
    va_start(args) ;
#endif /* !NO_STDARG */

    (void) vfprintf(stderr,fmt,args) ;
    va_end(args) ;

    (void) fputc('\n',stderr) ;
    (void) fflush(stderr);	/* to ensure log files are current */
    derror_count++;
}


void *
emalloc (			/* check return from malloc */
	size_t size)
{
    void   *p;

    p = (void *) malloc (size);
    if (p == 0) {
	derror ("out of memory\n");
	exit(3);
    }
    return p;
}

void *
ecalloc (			/* check return from calloc */
	size_t size)
{
    void   *p;

    p = (void *) calloc (size, 1);
    if (p == 0) {
	derror ("out of memory\n");
	exit(3);
    }
    return p;
}

void *
erealloc (		/* check return from realloc */
     void *ptr,
     size_t size)			/* if 0, this is really a free */
{
    void *p;

    p = (void *) realloc (ptr, size);

    if (p == 0 && size != 0) {
 	derror ("out of memory");
	exit(3);
    }
    return p;
}


/*
 * For generated Fortran, change 'e' to 'd' in exponent of double precision
 * constants.
 */
void
expe2d(
    char *cp)			/* string containing double constant */
{
    char *expchar = strrchr(cp,'e');
    if (expchar) {
	*expchar = 'd';
    }
}



/* Returns non-zero if n is a power of 2, 0 otherwise */
static
int
pow2(
     int n)
{
  int m = n;
  int p = 1;

  while (m > 0) {
    m /= 2;
    p *= 2;
  }
  return p == 2*n;
}


/*
 * Grow an integer array as necessary.
 *
 * Assumption: nar never incremented by more than 1 from last call.
 *
 * Makes sure an array is within a factor of 2 of the size needed.
 *
 * Make sure *arpp points to enough space to hold nar integers.  If not big
 * enough, malloc more space, copy over existing stuff, free old.  When
 * called for first time, *arpp assumed to be uninitialized.
 */
void
grow_iarray(
     int nar,			/* array must be at least this big */
     int **arpp)		/* address of start of int array */
{
  if (nar == 0) {
    *arpp = (int *) emalloc(1 * sizeof(int));
    return;
  }
  if (! pow2(nar))		/* return unless nar is a power of two */
    return;
  *arpp = (int *) erealloc(*arpp, 2 * nar * sizeof(int));
}


/*
 * Grow an array of variables as necessary.
 *
 * Assumption: nar never incremented by more than 1 from last call.
 *
 * Makes sure array is within a factor of 2 of the size needed.
 *
 * Make sure *arpp points to enough space to hold nar variables.  If not big
 * enough, malloc more space, copy over existing stuff, free old.  When
 * called for first time, *arpp assumed to be uninitialized.
 */
void
grow_varray(
     int nar,			/* array must be at least this big */
     struct vars **arpp)	/* address of start of var array */
{
  if (nar == 0) {
    *arpp = (struct vars *) emalloc(1 * sizeof(struct vars));
    return;
  }
  if (! pow2(nar))		/* return unless nar is a power of two */
    return;
  *arpp = (struct vars *) erealloc(*arpp, 2 * nar * sizeof(struct vars));
}


/*
 * Grow an array of dimensions as necessary.
 *
 * Assumption: nar never incremented by more than 1 from last call.
 *
 * Makes sure array is within a factor of 2 of the size needed.
 *
 * Make sure *arpp points to enough space to hold nar dimensions.  If not big
 * enough, malloc more space, copy over existing stuff, free old.  When
 * called for first time, *arpp assumed to be uninitialized.
 */
void
grow_darray(
     int nar,			/* array must be at least this big */
     struct dims **arpp)	/* address of start of var array */
{
  if (nar == 0) {
    *arpp = (struct dims *) emalloc(1 * sizeof(struct dims));
    return;
  }
  if (! pow2(nar))		/* return unless nar is a power of two */
    return;
  *arpp = (struct dims *) erealloc(*arpp, 2 * nar * sizeof(struct dims));
}


/*
 * Grow an array of attributes as necessary.
 *
 * Assumption: nar never incremented by more than 1 from last call.
 *
 * Makes sure array is within a factor of 2 of the size needed.
 *
 * Make sure *arpp points to enough space to hold nar attributes.  If not big
 * enough, malloc more space, copy over existing stuff, free old.  When
 * called for first time, *arpp assumed to be uninitialized.
 */
void
grow_aarray(
     int nar,			/* array must be at least this big */
     struct atts **arpp)	/* address of start of var array */
{
  if (nar == 0) {
    *arpp = (struct atts *) emalloc(1 * sizeof(struct atts));
    return;
  }
  if (! pow2(nar))		/* return unless nar is a power of two */
    return;
  *arpp = (struct atts *) erealloc(*arpp, 2 * nar * sizeof(struct atts));
}

#ifndef HAVE_STRLCAT
/*	$OpenBSD: strlcat.c,v 1.12 2005/03/30 20:13:52 otto Exp $	*/

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}
#endif /* ! HAVE_STRLCAT */


/*
 * Replace special chars in name so it can be used in C and Fortran
 * variable names without causing syntax errors.  Here we just replace
 * each "-" in a name with "_MINUS_", each "." with "_PERIOD_", etc.
 * For bytes with high bit set, from UTF-8 encoding of Unicode, just
 * replace with "_xHH", where each H is the appropriate hex digit.  If
 * a name begins with a number N, such as "4LFTX", replace with
 * "DIGIT_N_", such as "DIGIT_4_LFTX".
 *
 * Returned name is malloc'ed, so caller is responsible for freeing it.
 */
extern char*
decodify (
    const char *name)
{
    int count;		/* number chars in newname */
    char *newname;
    const char *cp;
    char *sp;
    static int init = 0;
    static char* repls[256];	/* replacement string for each char */
    static int lens[256];	/* lengths of replacement strings */
    static struct {
	char c;
	char *s;
    } ctable[] = {
	{' ', "_SPACE_"},
	{'!', "_EXCLAMATION_"},
	{'"', "_QUOTATION_"},
	{'#', "_HASH_"},
	{'$', "_DOLLAR_"},
	{'%', "_PERCENT_"},
	{'&', "_AMPERSAND_"},
	{'\'', "_APOSTROPHE_"},
	{'(', "_LEFTPAREN_"},
	{')', "_RIGHTPAREN_"},
	{'*', "_ASTERISK_"},
	{'+', "_PLUS_"},
	{',', "_COMMA_"},
	{'-', "_MINUS_"},
	{'.', "_PERIOD_"},
	{':', "_COLON_"},
	{';', "_SEMICOLON_"},
	{'<', "_LESSTHAN_"},
	{'=', "_EQUALS_"},
	{'>', "_GREATERTHAN_"},
	{'?', "_QUESTION_"},
	{'@', "_ATSIGN_"},
	{'[', "_LEFTBRACKET_"},
	{'\\', "_BACKSLASH_"},
	{']', "_RIGHTBRACKET_"},
	{'^', "_CIRCUMFLEX_"},
	{'`', "_BACKQUOTE_"},
	{'{', "_LEFTCURLY_"},
	{'|', "_VERTICALBAR_"},
	{'}', "_RIGHTCURLY_"},
	{'~', "_TILDE_"},
 	{'/', "_SLASH_"} 		/* should not occur in names */
/* 	{'_', "_UNDERSCORE_"} */
    };
    static int idtlen;
    static int hexlen;
    int nctable = (sizeof(ctable))/(sizeof(ctable[0]));
    int newlen;

    idtlen = strlen("DIGIT_n_"); /* initial digit template */
    hexlen = 1+strlen("_XHH"); /* template for hex of non-ASCII bytes */
    if(init == 0) {
	int i;
	char *rp;

	for(i = 0; i < 128; i++) {
	    rp = emalloc(2);
	    rp[0] = i;
	    rp[1] = '\0';
	    repls[i] = rp;
	}
	for(i=0; i < nctable; i++) {
	    size_t j = ctable[i].c;
	    free(repls[j]);
	    repls[j] = ctable[i].s;
	}
	for(i = 128; i < 256; i++) {
	    rp = emalloc(hexlen);
	    snprintf(rp, hexlen, "_X%2.2X", i);
	    rp[hexlen - 1] = '\0';
	    repls[i] = rp;
	}
	for(i = 0; i < 256; i++) {
	    lens[i] = strlen(repls[i]);
	}
	init = 1;		/* only do this initialization once */
    }

    count = 0;
    cp = name;
    while(*cp != '\0') {	/* get number of extra bytes for newname */
	size_t j;
        if(*cp < 0) {		/* handle signed or unsigned chars */
	    j = *cp + 256;
	} else {
	    j = *cp;
	}
 	count += lens[j] - 1;
	cp++;
    }

    cp = name;
    if('0' <= *cp && *cp <= '9') { /* names that begin with a digit */
	count += idtlen - 1;
    }
    newlen = strlen(name) + count + 1; /* bytes left to be filled */
    newname = (char *) emalloc(newlen);
    sp = newname;
    if('0' <= *cp && *cp <= '9') { /* handle initial digit, if any */
	snprintf(sp, newlen, "DIGIT_%c_", *cp);
	sp += idtlen;
	newlen -= idtlen;
	cp++;
    }
    *sp = '\0';
    while(*cp != '\0') { /* copy name to newname, replacing special chars */
	size_t j, len;
	/* cp is current position in name, sp is current position in newname */
        if(*cp < 0) {	      /* j is table index for character *cp */
	    j = *cp + 256;
	} else {
	    j = *cp;
	}
	len = strlcat(sp, repls[j], newlen);
	assert(len < newlen);
	sp += lens[j];
	newlen -= lens[j];
	cp++;
    }
    return newname;
}


/*
 * Replace escaped chars in CDL representation of name such as
 * 'abc\:def\ gh\\i' with unescaped version, such as 'abc:def gh\i'.
 */
void
deescapify (char *name)
{
    const char *cp = name;
    char *sp;
    size_t len = strlen(name);
    char *newname;

    if(strchr(name, '\\') == NULL)
	return;

    newname = (char *) emalloc(len + 1);
    cp = name;
    sp = newname;
    while(*cp != '\0') { /* delete '\' chars, except change '\\' to '\' */
	switch (*cp) {
	case '\\':
	    if(*(cp+1) == '\\') {
		*sp++ = '\\';
		cp++;
	    }
	    break;
	default:
	    *sp++ = *cp;
	    break;
	}
	cp++;
    }
    *sp = '\0';
    /* assert(strlen(newname) <= strlen(name)); */
    strncpy(name, newname, len);
    free(newname);
    return;
}
