/*

Copyright 2011 University Corporation for Atmospheric
Research/Unidata. See \ref copyright file for more info.  */

#include <config.h>
#include <stdio.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif	/* HAVE_LOCALE_H */
#include <netcdf.h>
#include "utils.h"
#include "nccomps.h"
#include "nctime0.h"		/* new iso time and calendar stuff */
#include "dumplib.h"
#include "ncdump.h"
#include "vardata.h"
#include "indent.h"
#include "isnan.h"
#include "cdl.h"

#define int64_t long long
#define uint64_t unsigned long long

/* globals */
char *progname;
fspec_t formatting_specs =	/* defaults, overridden by command-line options */
{
    0,			/* construct netcdf name from file name */
    false,		/* print header info only, no data? */
    false,		/* just print coord vars? */
    false,		/* brief  comments in data section? */
    false,		/* full annotations in data section?  */
    false,		/* human-readable output for date-time values? */
    false,		/* use 'T' separator between date and time values as strings? */
    false,		/* output special attributes, eg chunking? */
    LANG_C,		/* language conventions for indices */
    false,	        /* for DAP URLs, client-side cache used */
    0,			/* if -v specified, number of variables in list */
    0,			/* if -v specified, list of variable names */
    0,			/* if -g specified, number of groups names in list */
    0,			/* if -g specified, list of group names */
    0,			/* if -g specified, list of matching grpids */
    0			/* kind of netCDF file */
};

static void
usage(void)
{
#define USAGE   "\
  [-c]             Coordinate variable data and header information\n\
  [-h]             Header information only, no data\n\
  [-v var1[,...]]  Data for variable(s) <var1>,... only\n\
  [-b [c|f]]       Brief annotations for C or Fortran indices in data\n\
  [-f [c|f]]       Full annotations for C or Fortran indices in data\n\
  [-l len]         Line length maximum in data section (default 80)\n\
  [-n name]        Name for netCDF (default derived from file name)\n\
  [-p n[,n]]       Display floating-point values with less precision\n\
  [-k]             Output kind of netCDF file\n\
  [-s]             Output special (virtual) attributes\n\
  [-t]             Output time data as date-time strings\n\
  [-i]             Output time data as date-time strings with ISO-8601 'T' separator\n\
  [-g grp1[,...]]  Data and metadata for group(s) <grp1>,... only\n\
  [-w]             With client-side caching of variables for DAP URLs\n\
  [-x]             Output XML (NcML) instead of CDL\n\
  file             Name of netCDF file (or URL if DAP access enabled)\n"

    (void) fprintf(stderr,
		   "%s [-c|-h] [-v ...] [[-b|-f] [c|f]] [-l len] [-n name] [-p n[,n]] [-k] [-x] [-s] [-t|-i] [-g ...] [-w] file\n%s",
		   progname,
		   USAGE);
    
    (void) fprintf(stderr,
                 "netcdf library version %s\n",
                 nc_inq_libvers());
}


/* 
 * convert pathname of netcdf file into name for cdl unit, by taking 
 * last component of path and stripping off any extension.
 * DMH: add code to handle OPeNDAP url.
 * DMH: I think this also works for UTF8.
 */
static char *
name_path(const char *path)
{
    const char *cp;
    char *new;
    char *sp;

#ifdef vms
#define FILE_DELIMITER ']'
#endif    
#if defined(WIN32) || defined(msdos)
#define FILE_DELIMITER '\\'
#endif    
#ifndef FILE_DELIMITER /* default to unix */
#define FILE_DELIMITER '/'
#endif

#ifdef USE_DAP
    /* See if this is a url */
    {
	char* base;

        extern int nc__testurl(const char*,char**);


 	if(nc__testurl(path,&base)) {
 	    return base; /* Looks like a url */
	}
	/* else fall thru and treat like a file path */
    }
#endif /*USE_DAP*/

    cp = strrchr(path, FILE_DELIMITER);
    if (cp == 0)		/* no delimiter */
      cp = path;
    else			/* skip delimeter */
      cp++;
    new = (char *) emalloc((unsigned) (strlen(cp)+1));
    (void) strncpy(new, cp, strlen(cp) + 1);	/* copy last component of path */
    if ((sp = strrchr(new, '.')) != NULL)
      *sp = '\0';		/* strip off any extension */
    return new;
}

/* Return primitive type name */
static const char *
prim_type_name(nc_type type)
{
    switch (type) {
      case NC_BYTE:
	return "byte";
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
#ifdef USE_NETCDF4
      case NC_UBYTE:
	return "ubyte";
      case NC_USHORT:
	return "ushort";
      case NC_UINT:
	return "uint";
      case NC_INT64:
	return "int64";
      case NC_UINT64:
	return "uint64";
      case NC_STRING:
	return "string";
      case NC_VLEN:
	return "vlen";
      case NC_OPAQUE:
	return "opaque";
      case NC_COMPOUND:
	return "compound";
#endif /* USE_NETCDF4 */
      default:
	error("prim_type_name: bad type %d", type);
	return "bogus";
    }
}


/*
 * Remove trailing zeros (after decimal point) but not trailing decimal
 * point from ss, a string representation of a floating-point number that
 * might include an exponent part.
 */
static void
tztrim(char *ss)
{
    char *cp, *ep;
    
    cp = ss;
    if (*cp == '-')
      cp++;
    while(isdigit((int)*cp) || *cp == '.')
      cp++;
    if (*--cp == '.')
      return;
    ep = cp+1;
    while (*cp == '0')
      cp--;
    cp++;
    if (cp == ep)
      return;
    while (*ep)
      *cp++ = *ep++;
    *cp = '\0';
    return;
}


/* Return file type string */
static const char *
kind_string(int kind)
{
    switch (kind) {
    case NC_FORMAT_CLASSIC:
	return "classic";
    case NC_FORMAT_64BIT:
	return "64-bit offset";
    case NC_FORMAT_NETCDF4:
	return "netCDF-4";
    case NC_FORMAT_NETCDF4_CLASSIC:
	return "netCDF-4 classic model";
    default:
	error("unrecognized file format: %d");
	return "unrecognized";
    }
}


/* 
 * Emit initial line of output for NcML
 */
static void 
pr_initx(int ncid, const char *path)
{
    printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<netcdf xmlns=\"http://www.unidata.ucar.edu/namespaces/netcdf/ncml-2.2\" location=\"%s\">\n", 
	   path);
}


/*
 * Print attribute string, for text attributes.
 */
static void
pr_att_string(
    int kind,
    size_t len,
    const char *string
    )
{
    int iel;
    const char *cp;
    const char *sp;
    unsigned char uc;

    cp = string;
    printf ("\"");
    /* adjust len so trailing nulls don't get printed */
    sp = cp + len - 1;
    while (len != 0 && *sp-- == '\0')
	len--;
    for (iel = 0; iel < len; iel++)
	switch (uc = *cp++ & 0377) {
	case '\b':
	    printf ("\\b");
	    break;
	case '\f':
	    printf ("\\f");
	    break;
	case '\n':		
	    /* Only generate linebreaks after embedded newlines for
	     * classic, 64-bit offset, or classic model files.  For
	     * netCDF-4 files, don't generate linebreaks, because that
	     * would create an extra string in a list of strings.  */
	    if (kind != NC_FORMAT_NETCDF4) {
		printf ("\\n\",\n\t\t\t\"");
	    } else {
		printf("\\n");
	    }
	    break;
	case '\r':
	    printf ("\\r");
	    break;
	case '\t':
	    printf ("\\t");
	    break;
	case '\v':
	    printf ("\\v");
	    break;
	case '\\':
	    printf ("\\\\");
	    break;
	case '\'':
	    printf ("\\\'");
	    break;
	case '\"':
	    printf ("\\\"");
	    break;
	default:
	    if (iscntrl(uc))
	        printf ("\\%03o",uc);
	    else
	        printf ("%c",uc);
	    break;
	}
    printf ("\"");

}


/*
 * Print NcML attribute string, for text attributes.
 */
static void
pr_attx_string(
     size_t len,
     const char *string
     )
{
    int iel;
    const char *cp;
    const char *sp;
    unsigned char uc;

    cp = string;
    printf ("\"");
    /* adjust len so trailing nulls don't get printed */
    sp = cp + len - 1;
    while (len != 0 && *sp-- == '\0')
	len--;
    for (iel = 0; iel < len; iel++)
	switch (uc = *cp++ & 0377) {
	case '\"':
	    printf ("&quot;");
	    break;
	case '<':
	    printf ("&lt;");
	    break;
	case '>':
	    printf ("&gt;");
	    break;
	case '&':
	    printf ("&amp;");
	    break;
	case '\n':
	    printf ("&#xA;");
	    break;
	case '\r':
	    printf ("&#xD;");
	    break;
	case '\t':
	    printf ("&#x9;");
	    break;
	default:
	    if (iscntrl(uc))
	        printf ("&#%d;",uc);
	    else
	        printf ("%c",uc);
	    break;
	}
    printf ("\"");

}


/*
 * Print list of attribute values, for attributes of primitive types.
 * Attribute values must be printed with explicit type tags for
 * netCDF-3 primitive types, because CDL doesn't require explicit
 * syntax to declare such attribute types.  
 */
static void
pr_att_valgs(
    int kind,
    nc_type type,
    size_t len,
    const void *vals
    )
{
    int iel;
    signed char sc;
    short ss;
    int ii;
    char gps[PRIM_LEN];
    float ff;
    double dd;
#ifdef USE_NETCDF4
    unsigned char uc;
    unsigned short us;
    unsigned int ui;
    int64_t i64;
    uint64_t ui64;
    char *stringp;
#endif /* USE_NETCDF4 */
    char *delim = ", ";	/* delimiter between output values */

    if (type == NC_CHAR) {
	char *cp = (char *) vals;
	pr_att_string(kind, len, cp);
	return;
    }
    /* else */
    for (iel = 0; iel < len; iel++) {
	if (iel == len - 1)
	    delim = "";
	switch (type) {
	case NC_BYTE:
	    sc = ((signed char *) vals)[iel];
	    printf ("%db%s", sc, delim);
	    break;
	case NC_SHORT:
	    ss = ((short *) vals)[iel];
	    printf ("%ds%s", ss, delim);
	    break;
	case NC_INT:
	    ii = ((int *) vals)[iel];
	    printf ("%d%s", ii, delim);
	    break;
	case NC_FLOAT:
	    ff = ((float *) vals)[iel];
	    if(isfinite(ff)) {
		int res;
		res = snprintf(gps, PRIM_LEN, float_att_fmt, ff);
		assert(res < PRIM_LEN);
		tztrim(gps);	/* trim trailing 0's after '.' */
		printf ("%s%s", gps, delim);
	    } else {
		if(isnan(ff)) {
		    printf("NaNf%s", delim);
		} else if(isinf(ff)) {
		    if(ff < 0.0f) {
			printf("-");
		    }
		    printf("Infinityf%s", delim);
		}
	    }
	    break;
	case NC_DOUBLE:
	    dd = ((double *) vals)[iel];
	    if(isfinite(dd)) {
		int res;
		res = snprintf(gps, PRIM_LEN, double_att_fmt, dd);
		assert(res < PRIM_LEN);
		tztrim(gps);
		printf ("%s%s", gps, delim);
	    } else {
		if(isnan(dd)) {
		    printf("NaN%s", delim);
		} else if(isinf(dd)) {
		    if(dd < 0.0) {
			printf("-");
		    }
		    printf("Infinity%s", delim);
		}
	    }
	    break;
#ifdef USE_NETCDF4
	case NC_UBYTE:
	    uc = ((unsigned char *) vals)[iel];
	    printf ("%uUB%s", uc, delim);
	    break;
	case NC_USHORT:
	    us = ((unsigned short *) vals)[iel];
	    printf ("%huUS%s", us, delim);
	    break;
	case NC_UINT:
	    ui = ((unsigned int *) vals)[iel];
	    printf ("%uU%s", ui, delim);
	    break;
	case NC_INT64:
	    i64 = ((int64_t *) vals)[iel];
	    printf ("%lldL%s", i64, delim);
	    break;
	case NC_UINT64:
	    ui64 = ((uint64_t *) vals)[iel];
	    printf ("%lluUL%s", ui64, delim);
	    break;
	case NC_STRING:
	    stringp = ((char **) vals)[iel];
	    pr_att_string(kind, strlen(stringp), stringp);
	    printf("%s", delim);
	    break;
#endif /* USE_NETCDF4 */
	default:
	    error("pr_att_vals: bad type");
	}
    }
}


/*
 * Print list of numeric attribute values to string for use in NcML output.
 * Unlike CDL, NcML makes type explicit, so don't need type suffixes.
 */
static void
pr_att_valsx(
     nc_type type,
     size_t len,
     const double *vals,
     char *attvals,		/* returned string */
     size_t attvalslen		/* size of attvals buffer, assumed
				   large enough to hold all len
				   blank-separated values */
     )
{
    int iel;
    float ff;
    double dd;
    int ii;
#ifdef USE_NETCDF4
    unsigned int ui;
    int64_t i64;
    uint64_t ui64;
#endif /* USE_NETCDF4 */

    attvals[0]='\0';
    if (len == 0)
	return;
    for (iel = 0; iel < len; iel++) {
	char gps[PRIM_LEN];
	int res;
	switch (type) {
	case NC_BYTE:
	case NC_SHORT:
	case NC_INT:
	    ii = vals[iel];
	    res = snprintf(gps, PRIM_LEN, "%d", ii);
	    assert(res < PRIM_LEN);
	    (void) strlcat(attvals, gps, attvalslen);
	    (void) strlcat(attvals, iel < len-1 ? " " : "", attvalslen);
	    break;
#ifdef USE_NETCDF4
	case NC_UBYTE:
	case NC_USHORT:
	case NC_UINT:
	    ui = vals[iel];
	    res = snprintf(gps, PRIM_LEN, "%u", ui);
	    assert(res < PRIM_LEN);
	    (void) strlcat(attvals, gps, attvalslen);
	    (void) strlcat(attvals, iel < len-1 ? " " : "", attvalslen);
	    break;
	case NC_INT64:
	    i64 = vals[iel];
	    res = snprintf(gps, PRIM_LEN, "%lld", i64);
	    assert(res < PRIM_LEN);
	    (void) strlcat(attvals, gps, attvalslen);
	    (void) strlcat(attvals, iel < len-1 ? " " : "", attvalslen);
	    break;
	case NC_UINT64:
	    ui64 = vals[iel];
	    res = snprintf(gps, PRIM_LEN, "%llu", ui64);
	    assert(res < PRIM_LEN);
	    (void) strlcat(attvals, gps, attvalslen);
	    (void) strlcat(attvals, iel < len-1 ? " " : "", attvalslen);
	    break;
#endif /* USE_NETCDF4 */
	case NC_FLOAT:
	    ff = vals[iel];
	    res = snprintf(gps, PRIM_LEN, float_attx_fmt, ff);
	    assert(res < PRIM_LEN);
	    tztrim(gps);	/* trim trailing 0's after '.' */
	    (void) strlcat(attvals, gps, attvalslen);
	    (void) strlcat(attvals, iel < len-1 ? " " : "", attvalslen);
	    break;
	case NC_DOUBLE:
	    dd = vals[iel];
	    res = snprintf(gps, PRIM_LEN, double_att_fmt, dd);
	    assert(res < PRIM_LEN);
	    tztrim(gps);	/* trim trailing 0's after '.' */
	    (void) strlcat(attvals, gps, attvalslen);
	    (void) strlcat(attvals, iel < len-1 ? " " : "", attvalslen);
	    break;
	default:
	    error("pr_att_valsx: bad type");
	}
    }
}

/* 
 * Print a variable attribute
 */
static void
pr_att(
    int ncid,
    int kind,
    int varid,
    const char *varname,
    int ia
    )
{
    ncatt_t att;			/* attribute */
	    
    NC_CHECK( nc_inq_attname(ncid, varid, ia, att.name) );
    NC_CHECK( nc_inq_att(ncid, varid, att.name, &att.type, &att.len) );
    att.tinfo = get_typeinfo(att.type);

    indent_out();
    printf ("\t\t");
#ifdef USE_NETCDF4
    if (is_user_defined_type(att.type) || att.type == NC_STRING)
#else
    if (is_user_defined_type(att.type))
#endif
    {
	/* TODO: omit next two lines if att_type_name not needed
	 * because print_type_name() looks it up */
	char att_type_name[NC_MAX_NAME + 1];
	get_type_name(ncid, att.type, att_type_name);

	/* printf ("\t\t%s ", att_type_name); */
	/* ... but handle special characters in CDL names with escapes */
	print_type_name(ncid, att.type);
	printf(" ");
    }
    /* 	printf ("\t\t%s:%s = ", varname, att.name); */
    print_name(varname);
    printf(":");
    print_name(att.name);
    printf(" = ");

    if (att.len == 0) {	/* show 0-length attributes as empty strings */
	att.type = NC_CHAR;
    }

    if (! is_user_defined_type(att.type) ) {
	att.valgp = (void *) emalloc((att.len + 1) * att.tinfo->size );
	NC_CHECK( nc_get_att(ncid, varid, att.name, att.valgp ) );
	if(att.type == NC_CHAR)	/* null-terminate retrieved text att value */
	    ((char *)att.valgp)[att.len] = '\0';
/* (1) Print normal list of attribute values. */
        pr_att_valgs(kind, att.type, att.len, att.valgp);
	printf (" ;");			/* terminator for normal list */
/* (2) If -t option, add list of date/time strings as CDL comments. */
	if(formatting_specs.string_times) {
	    /* Prints text after semicolon and before final newline.
	     * Prints nothing if not qualified for time interpretation.
	     * Will include line breaks for longer lists. */
	    print_att_times(ncid, varid, att);
	    if(is_bounds_att(&att)) {
		insert_bounds_info(ncid, varid, &att);
	    }
	}
#ifdef USE_NETCDF4
	/* If NC_STRING, need to free all the strings also */
	if(att.type == NC_STRING) {
	    nc_free_string(att.len, att.valgp);
	}
#endif /* USE_NETCDF4 */
	free(att.valgp);
    }
#ifdef USE_NETCDF4
    else /* User-defined type. */
    {
       char type_name[NC_MAX_NAME + 1];
       size_t type_size, nfields;
       nc_type base_nc_type;
       int class, i;
       void *data;

       NC_CHECK( nc_inq_user_type(ncid, att.type,  type_name, &type_size, 
				  &base_nc_type, &nfields, &class));
       switch(class)
       {
	  case NC_VLEN:
	      /* because size returned for vlen is base type size, but we
	       * need space to read array of vlen structs into ... */
	      data = emalloc((att.len + 1) * sizeof(nc_vlen_t));
	     break;
	  case NC_OPAQUE:
	      data = emalloc((att.len + 1) * type_size);
	     break;
	  case NC_ENUM:
	      /* a long long is ample for all base types */
	      data = emalloc((att.len + 1) * sizeof(int64_t));
	     break;
	  case NC_COMPOUND:
	      data = emalloc((att.len + 1) * type_size);
	     break;
	  default:
	     error("unrecognized class of user defined type: %d", class);
       }

       NC_CHECK( nc_get_att(ncid, varid, att.name, data));

       switch(class) {
       case NC_VLEN:
	   pr_any_att_vals(&att, data);
	   free(data);
	   break;
       case NC_OPAQUE: {
	   char *sout = emalloc(2 * type_size + strlen("0X") + 1);
	   unsigned char *cp = data;
	   for (i = 0; i < att.len; i++) {
	       (void) ncopaque_val_as_hex(type_size, sout, cp);
	       printf("%s%s", sout, i < att.len-1 ? ", " : "");
	       cp += type_size;
	   }
	   free(sout);
       }
	   break;
       case NC_ENUM: {
	   int64_t value;
	   for (i = 0; i < att.len; i++) {
	       char enum_name[NC_MAX_NAME + 1];
	       switch(base_nc_type)
	       {
	       case NC_BYTE:
		   value = *((char *)data + i);
		   break;
	       case NC_UBYTE:
		   value = *((unsigned char *)data + i);
		   break;
	       case NC_SHORT:
		   value = *((short *)data + i);
		   break;
	       case NC_USHORT:
		   value = *((unsigned short *)data + i);
		   break;
	       case NC_INT:
		   value = *((int *)data + i);
		   break;
	       case NC_UINT:
		   value = *((unsigned int *)data + i);
		   break;
	       case NC_INT64:
		   value = *((int64_t *)data + i);
		   break;
	       case NC_UINT64:
		   value = *((uint64_t *)data + i);
		   break;
	       default:
		   error("enum must have an integer base type: %d", base_nc_type);
	       }
	       NC_CHECK( nc_inq_enum_ident(ncid, att.type, value, 
					   enum_name));
/* 	       printf("%s%s", enum_name, i < att.len-1 ? ", " : ""); */
	       print_name(enum_name);
	       printf("%s", i < att.len-1 ? ", " : "");
	   }
       }
	   break;
       case NC_COMPOUND:
	   pr_any_att_vals(&att, data);
	   free(data);
	   break;
       default:
	   error("unrecognized class of user defined type: %d", class);
       }
       printf (" ;");		/* terminator for user defined types */
    }
#endif /* USE_NETCDF4 */

    printf ("\n");		/* final newline for all attribute types */
}

/* Common code for printing attribute name */
static void
pr_att_name(
    int ncid,
    const char *varname,
    const char *attname
    )
{
    indent_out();
    printf ("\t\t");
    print_name(varname);
    printf(":");
    print_name(attname);
}

/* 
 * Print special _Format global attribute, a virtual attribute not
 * actually stored in the file.
 */
static void
pr_att_global_format(
    int ncid,
    int kind
    )
{
    pr_att_name(ncid, "", NC_ATT_FORMAT);
    printf(" = ");
    printf("\"%s\"", kind_string(kind));
    printf (" ;\n");
}


#ifdef USE_NETCDF4
/* 
 * Print special reserved variable attributes, such as _Chunking,
 * _DeflateLevel, ...  These are virtual, not real, attributes
 * generated from the result of inquire calls.  They are of primitive
 * type to fit into the classic model.  Currently, these only exist
 * for netCDF-4 data.
 */
static void
pr_att_specials(
    int ncid,
    int kind,
    int varid,
    const ncvar_t *varp
    )
{
    /* No special variable attributes for classic or 64-bit offset data */
    if(kind == 1 || kind == 2)
	return;
    /* _Chunking */
    if (varp->ndims > 0) {	/* no chunking for scalar variables */
	int contig = 0;
	NC_CHECK( nc_inq_var_chunking(ncid, varid, &contig, NULL ) );
	if(contig == 1) {
	    pr_att_name(ncid, varp->name, NC_ATT_STORAGE);
	    printf(" = \"contiguous\" ;\n");
	} else {
 	   size_t *chunkp;
	   int i;
	    pr_att_name(ncid, varp->name, NC_ATT_STORAGE);
	    printf(" = \"chunked\" ;\n");
	    chunkp = (size_t *) emalloc(sizeof(size_t) * (varp->ndims + 1) );
	    NC_CHECK( nc_inq_var_chunking(ncid, varid, NULL, chunkp) );
	    /* print chunking, even if it is default */
	    pr_att_name(ncid, varp->name, NC_ATT_CHUNKING);
	    printf(" = ");
	    for(i = 0; i < varp->ndims; i++) {
		printf("%lu%s", (unsigned long)chunkp[i], i+1 < varp->ndims ? ", " : " ;\n");
	    }
	    free(chunkp);
	}
    }

    /*_Deflate, _Shuffle */
    {
	int shuffle=NC_NOSHUFFLE, deflate=0, deflate_level=0;
	NC_CHECK( nc_inq_var_deflate(ncid, varid, &shuffle,
				     &deflate, &deflate_level) );
	if(deflate != 0) {
	    pr_att_name(ncid, varp->name, NC_ATT_DEFLATE);
	    printf(" = %d ;\n", deflate_level);
	}
	if(shuffle != NC_NOSHUFFLE) {
	    pr_att_name(ncid, varp->name, NC_ATT_SHUFFLE);
	    printf(" = \"true\" ;\n");
	}
    }
    /* _Checksum */
    {
	int fletcher32 = 0;
	NC_CHECK( nc_inq_var_fletcher32(ncid, varid, &fletcher32) );
	if(fletcher32 != 0) {
	    pr_att_name(ncid, varp->name, NC_ATT_CHECKSUM);
	    printf(" = \"true\" ;\n");
	}
    }
    /* _Endianness */
    if(varp->tinfo->size > 1) /* Endianness is meaningless for 1-byte types */
    {
	int endianness = 0;
	NC_CHECK( nc_inq_var_endian(ncid, varid, &endianness) );
	if (endianness != NC_ENDIAN_NATIVE) { /* NC_ENDIAN_NATIVE is the default */
	    pr_att_name(ncid, varp->name, NC_ATT_ENDIANNESS);
	    printf(" = ");
	    switch (endianness) {
	    case NC_ENDIAN_LITTLE:
		printf("\"little\"");
		break;
	    case NC_ENDIAN_BIG:
		printf("\"big\"");
		break;
	    default:
		error("pr_att_specials: bad endianness: %d", endianness);
		break;
	    }
	    printf(" ;\n");
	}
    }
    {
	int no_fill = 0;
	/* Don't get the fill_value, it's set explicitly with
	 * _FillValue attribute, because nc_def_var_fill() creates a
	 * _FillValue attribute, if needed, and it's value gets
	 * displayed elsewhere as a normal (not special virtual)
	 * attribute. */
	NC_CHECK( nc_inq_var_fill(ncid, varid, &no_fill, NULL) );
	if(no_fill != 0) {
	    pr_att_name(ncid, varp->name, NC_ATT_NOFILL);
	    printf(" = \"true\" ;\n");
	}
    }
    /* TODO: handle _Nbit when inquire function is available */

    /* TODO: handle _ScaleOffset when inquire is available */

    /* TODO: handle _Szip when szip inquire function is available */
}
#endif /* USE_NETCDF4 */


/* 
 * Print a variable attribute for NcML
 */
static void
pr_attx(
    int ncid,
    int varid,
    int ia
    )
{
    ncatt_t att;			/* attribute */
    char *attvals = NULL;
    int attvalslen = 0;

    NC_CHECK( nc_inq_attname(ncid, varid, ia, att.name) );
    NC_CHECK( nc_inq_att(ncid, varid, att.name, &att.type, &att.len) );
    att.tinfo = get_typeinfo(att.type);

    /* Put attribute values into a single string, with blanks in between */

    switch (att.type) {
    case NC_CHAR:
	attvals = (char *) emalloc(att.len + 1);
	attvalslen = att.len;
	attvals[att.len] = '\0';
	NC_CHECK( nc_get_att_text(ncid, varid, att.name, attvals ) );
	break;
#ifdef USE_NETCDF4
    case NC_STRING:
	/* TODO: this only prints first string value, need to handle
	   multiple strings? */
	attvals = (char *) emalloc(att.len + 1);
	attvals[att.len] = '\0';
	NC_CHECK( nc_get_att_text(ncid, varid, att.name, attvals ) );
	break;
    case NC_VLEN:
	/* TODO */
	break;
    case NC_OPAQUE:
	/* TODO */
	break;
    case NC_COMPOUND:
	/* TODO */
	break;
#endif /* USE_NETCDF4 */
    default:
	att.vals = (double *) emalloc((att.len + 1) * sizeof(double));
	NC_CHECK( nc_get_att_double(ncid, varid, att.name, att.vals ) );
	attvalslen = 20*att.len; /* max 20 chars for each value and blank separator */
	attvals = (char *) emalloc(attvalslen + 1);
	pr_att_valsx(att.type, att.len, att.vals, attvals, attvalslen);
	free(att.vals); 
	break;
    }

    /* Don't output type for string attributes, since that's default type */
    if(att.type == NC_CHAR
#ifdef USE_NETCDF4
                          || att.type == NC_CHAR
#endif /* USE_NETCDF4 */
       ) {
	/* TODO: XML-ish escapes for special chars in names */
	printf ("%s  <attribute name=\"%s\" value=", 
		varid != NC_GLOBAL ? "  " : "", 
		att.name);
	/* print attvals as a string with XML escapes */
	pr_attx_string(attvalslen, attvals);
    } else {			/* non-string attribute */
	char att_type_name[NC_MAX_NAME + 1];
	get_type_name(ncid, att.type, att_type_name);
	/* TODO: print full type name with group prefix, when needed */
	printf ("%s  <attribute name=\"%s\" type=\"%s\" value=\"", 
		varid != NC_GLOBAL ? "  " : "", 
		att.name, 
		att_type_name);
	printf("%s\"",attvals);
    }
    printf (" />\n");
    if(attvals != NULL)
      free (attvals);
}


/* Print optional NcML attribute for a variable's shape */
static void
pr_shape(ncvar_t* varp, ncdim_t *dims)
{
    char *shape;
    int shapelen = 0;
    int id;

    if (varp->ndims == 0)
	return;
    for (id = 0; id < varp->ndims; id++) {
	shapelen += strlen(dims[varp->dims[id]].name) + 1;
    }
    shape = (char *) emalloc(shapelen + 1);
    shape[0] = '\0';
    for (id = 0; id < varp->ndims; id++) {
	/* TODO: XML-ish escapes for special chars in dim names */
	strlcat(shape, dims[varp->dims[id]].name, shapelen);
	strlcat(shape, id < varp->ndims-1 ? " " : "", shapelen);
    }
    printf (" shape=\"%s\"", shape);
    free(shape);
}

#ifdef USE_NETCDF4


/* Print an enum type declaration */
static void
print_enum_type(int ncid, nc_type typeid) {
    char type_name[NC_MAX_NAME + 1];
    size_t type_size;
    nc_type base_nc_type;
    size_t type_nfields;
    int type_class;
    char base_type_name[NC_MAX_NAME + 1];
    int f;
    int64_t memval;
    char memname[NC_MAX_NAME + 1];
 /* extra space for escapes, and punctuation */
#define SAFE_BUF_LEN 4*NC_MAX_NAME+30
    char safe_buf[SAFE_BUF_LEN];
    char *delim;
    int64_t data;	    /* space for data of any primitive type */
    char *esc_btn;
    char *esc_tn;
    char *esc_mn;
    int res;

    NC_CHECK( nc_inq_user_type(ncid, typeid, type_name, &type_size, &base_nc_type, 
			       &type_nfields, &type_class) );

    get_type_name(ncid, base_nc_type, base_type_name); 
    indent_out();
    esc_btn = escaped_name(base_type_name);
    esc_tn = escaped_name(type_name);
    res = snprintf(safe_buf, SAFE_BUF_LEN,"%s enum %s {", esc_btn, esc_tn);
    assert(res < SAFE_BUF_LEN);
    free(esc_btn);
    free(esc_tn);
    lput(safe_buf);
    delim = ", ";
    for (f = 0; f < type_nfields; f++) {
	if (f == type_nfields - 1)
	    delim = "} ;\n";
	NC_CHECK( nc_inq_enum_member(ncid, typeid, f, memname, &data) );
	switch (base_nc_type) {
	case NC_BYTE:
	    memval = *(char *)&data;
	    break;
	case NC_SHORT:
	    memval = *(short *)&data;
	    break;
	case NC_INT:
	    memval = *(int *)&data;
	    break;
#ifdef USE_NETCDF4
	case NC_UBYTE:
	    memval = *(unsigned char *)&data;
	    break;
	case NC_USHORT:
	    memval = *(unsigned short *)&data;
	    break;
	case NC_UINT:
	    memval = *(unsigned int *)&data;
	    break;
	case NC_INT64:
	    memval = *(int64_t *)&data;
	    break;
	case NC_UINT64:
	    memval = *(uint64_t *)&data;
	    break;
#endif /* USE_NETCDF4 */
	default:
	    error("Bad base type for enum!");
	    break;
	}
	esc_mn = escaped_name(memname);
	res = snprintf(safe_buf, SAFE_BUF_LEN, "%s = %lld%s", esc_mn, 
		       memval, delim);
	assert(res < SAFE_BUF_LEN);
	free(esc_mn);
	lput(safe_buf);
    }
}


/* Print a user-defined type declaration */
static void
print_ud_type(int ncid, nc_type typeid) {
    
    char type_name[NC_MAX_NAME + 1];
    char base_type_name[NC_MAX_NAME + 1];
    size_t type_nfields, type_size;
    nc_type base_nc_type;
    int f, type_class;
    
    NC_CHECK( nc_inq_user_type(ncid, typeid, type_name, &type_size, &base_nc_type, 
			       &type_nfields, &type_class) );
    switch(type_class) {
    case NC_VLEN:
	/* TODO: don't bother getting base_type_name if
	 * print_type_name looks it up anyway */
	get_type_name(ncid, base_nc_type, base_type_name);
	indent_out();
/* 	printf("%s(*) %s ;\n", base_type_name, type_name); */
	print_type_name(ncid, base_nc_type);
	printf("(*) ");
	print_type_name(ncid, typeid);
	printf(" ;\n");
	break;
    case NC_OPAQUE:
	indent_out();
/* 	printf("opaque(%d) %s ;\n", (int)type_size, type_name); */
	printf("opaque(%d) ", (int)type_size);
	print_type_name(ncid, typeid);
	printf(" ;\n");
	break;
    case NC_ENUM:
	print_enum_type(ncid, typeid);
	break;
    case NC_COMPOUND:
	{
	    char field_name[NC_MAX_NAME + 1];
	    char field_type_name[NC_MAX_NAME + 1];
	    size_t field_offset;
	    nc_type field_type;
	    int field_ndims;
	    int d;
	    
	    indent_out();
/* 	    printf("compound %s {\n", type_name); */
	    printf("compound ");
	    print_type_name(ncid, typeid);
	    printf(" {\n");
	    for (f = 0; f < type_nfields; f++)
		{
		    NC_CHECK( nc_inq_compound_field(ncid, typeid, f, field_name, 
						    &field_offset, &field_type, 
						    &field_ndims, NULL) );
		    /* TODO: don't bother if field_type_name not needed here */
		    get_type_name(ncid, field_type, field_type_name);
		    indent_out();
/* 		    printf("  %s %s", field_type_name, field_name); */
		    printf("  ");
		    print_type_name(ncid, field_type);
		    printf(" ");
		    print_name(field_name);
		    if (field_ndims > 0) {
			int *field_dim_sizes = (int *) emalloc((field_ndims + 1) * sizeof(int));
			NC_CHECK( nc_inq_compound_field(ncid, typeid, f, NULL, 
							NULL, NULL, NULL, 
							field_dim_sizes) );
			printf("(");
			for (d = 0; d < field_ndims-1; d++)
			    printf("%d, ", field_dim_sizes[d]);
			printf("%d)", field_dim_sizes[field_ndims-1]);
			free(field_dim_sizes);
		    }
		    printf(" ;\n");
		}
            indent_out();
/* 	    printf("}; // %s\n", type_name); */
	    printf("}; // ");
	    print_type_name(ncid, typeid);
	    printf("\n");
	}
	break;
    default:
	error("Unknown class of user-defined type!");
    }
}
#endif /* USE_NETCDF4 */

static void
get_fill_info(int ncid, int varid, ncvar_t *vp) {
    ncatt_t att;			/* attribute */
    int nc_status;			/* return from netcdf calls */
    void *fillvalp = NULL;
    
    vp->has_fillval = 1; /* by default, but turn off for bytes */
	    
    /* get _FillValue attribute */
    nc_status = nc_inq_att(ncid,varid,_FillValue,&att.type,&att.len);
    fillvalp = emalloc(vp->tinfo->size + 1);
    if(nc_status == NC_NOERR &&
       att.type == vp->type && att.len == 1) {
	NC_CHECK(nc_get_att(ncid, varid, _FillValue, fillvalp));
    } else {
	switch (vp->type) {
	case NC_BYTE:
	    /* don't do default fill-values for bytes, too risky */
	    vp->has_fillval = 0;
	    free(fillvalp);
	    fillvalp = 0;
	    break;
	case NC_CHAR:
	    *(char *)fillvalp = NC_FILL_CHAR;
	    break;
	case NC_SHORT:
	    *(short *)fillvalp = NC_FILL_SHORT;
	    break;
	case NC_INT:
	    *(int *)fillvalp = NC_FILL_INT;
	    break;
	case NC_FLOAT:
	    *(float *)fillvalp = NC_FILL_FLOAT;
	    break;
	case NC_DOUBLE:
	    *(double *)fillvalp = NC_FILL_DOUBLE;
	    break;
#ifdef USE_NETCDF4
	case NC_UBYTE:
	    /* don't do default fill-values for bytes, too risky */
	    vp->has_fillval = 0;
	    free(fillvalp);
	    fillvalp = 0;
	    break;
	case NC_USHORT:
	    *(unsigned short *)fillvalp = NC_FILL_USHORT;
	    break;
	case NC_UINT:
	    *(unsigned int *)fillvalp = NC_FILL_UINT;
	    break;
	case NC_INT64:
	    *(int64_t *)fillvalp = NC_FILL_INT64;
	    break;
	case NC_UINT64:
	    *(uint64_t *)fillvalp = NC_FILL_UINT64;
	    break;
	case NC_STRING:
	    *((char **)fillvalp) = NC_FILL_STRING;
	    break;
#endif /* USE_NETCDF4 */
	default:		/* no default fill values for NC_NAT
				   or user-defined types */
	    vp->has_fillval = 0;
	    free(fillvalp);
	    fillvalp = 0;
	    break;
	}
    }
    vp->fillvalp = fillvalp;
}


/* Recursively dump the contents of a group. (Only netcdf-4 format
 * files can have groups, so recursion will not take place for classic
 * format files.)
 *
 * ncid: id of open file (first call) or group (subsequent recursive calls) 
 * path: file path name (first call)
 */
static void
do_ncdump_rec(int ncid, const char *path)
{
   int ndims;			/* number of dimensions */
   int nvars;			/* number of variables */
   int ngatts;			/* number of global attributes */
   int xdimid;			/* id of unlimited dimension */
   int varid;			/* variable id */
   ncdim_t *dims;		/* dimensions */
   size_t *vdims=0;	        /* dimension sizes for a single variable */
   ncvar_t var;			/* variable */
   int id;			/* dimension number per variable */
   int ia;			/* attribute number */
   int iv;			/* variable number */
   idnode_t* vlist = 0;		/* list for vars specified with -v option */
   char type_name[NC_MAX_NAME + 1];
   int kind;		/* strings output differently for nc4 files */
   char dim_name[NC_MAX_NAME + 1];
#ifdef USE_NETCDF4
   int *dimids_grp;	        /* dimids of the dims in this group. */
   int *unlimids;		/* dimids of unlimited dimensions in this group */
   int d_grp, ndims_grp;
   int ntypes, *typeids;
   int nunlim;
#else
   int dimid;			/* dimension id */
#endif /* USE_NETCDF4 */
   int is_root = 1;		/* true if ncid is root group or if netCDF-3 */

#ifdef USE_NETCDF4
   if (nc_inq_grp_parent(ncid, NULL) != NC_ENOGRP)
       is_root = 0;
#endif /* USE_NETCDF4 */

   /*
    * If any vars were specified with -v option, get list of
    * associated variable ids relative to this group.  Assume vars
    * specified with syntax like "grp1/grp2/varname" or
    * "/grp1/grp2/varname" if they are in groups.
    */
   if (formatting_specs.nlvars > 0) {
      vlist = newidlist();	/* list for vars specified with -v option */
      for (iv=0; iv < formatting_specs.nlvars; iv++) {
	  if(nc_inq_gvarid(ncid, formatting_specs.lvars[iv], &varid) == NC_NOERR)
	      idadd(vlist, varid);
      }
   }

#ifdef USE_NETCDF4
   /* Are there any user defined types in this group? */
   NC_CHECK( nc_inq_typeids(ncid, &ntypes, NULL) );
   if (ntypes)
   {
      int t;

      typeids = emalloc((ntypes + 1) * sizeof(int));
      NC_CHECK( nc_inq_typeids(ncid, &ntypes, typeids) );
      indent_out();
      printf("types:\n");
      indent_more();
      for (t = 0; t < ntypes; t++)
      {
	 print_ud_type(ncid, typeids[t]); /* print declaration of user-defined type */
      }
      indent_less();
      free(typeids);
   }
#endif /* USE_NETCDF4 */

   /*
    * get number of dimensions, number of variables, number of global
    * atts, and dimension id of unlimited dimension, if any
    */
   NC_CHECK( nc_inq(ncid, &ndims, &nvars, &ngatts, &xdimid) );
   /* get dimension info */
   dims = (ncdim_t *) emalloc((ndims + 1) * sizeof(ncdim_t));
   if (ndims > 0) {
       indent_out();
       printf ("dimensions:\n");
   }

#ifdef USE_NETCDF4
   /* In netCDF-4 files, dimids will not be sequential because they
    * may be defined in various groups, and we are only looking at one
    * group at a time. */

   /* Find the number of dimids defined in this group. */
   NC_CHECK( nc_inq_ndims(ncid, &ndims_grp) );
   dimids_grp = (int *)emalloc((ndims_grp + 1) * sizeof(int));
   
   /* Find the dimension ids in this group. */
   NC_CHECK( nc_inq_dimids(ncid, 0, dimids_grp, 0) );

   /* Find the number of unlimited dimensions and get their IDs */
   NC_CHECK( nc_inq_unlimdims(ncid, &nunlim, NULL) );
   unlimids = (int *)emalloc((nunlim + 1) * sizeof(int));
   NC_CHECK( nc_inq_unlimdims(ncid, &nunlim, unlimids) );
    
   /* For each dimension defined in this group, get and print out info. */
   for (d_grp = 0; d_grp < ndims_grp; d_grp++)
   {
      int dimid = dimids_grp[d_grp];
      int is_unlimited = 0;
      int uld;
      int stat;

      for (uld = 0; uld < nunlim; uld++) {
	  if(dimid == unlimids[uld]) {
	      is_unlimited = 1;
	      break;
	  }	  
      }
      stat = nc_inq_dim(ncid, dimid, dims[d_grp].name, &dims[d_grp].size);
      if (stat == NC_EDIMSIZE && SIZEOF_SIZE_T < 8) {
	  error("dimension \"%s\" too large for 32-bit platform, try 64-bit version", dims[d_grp].name);
      } else {
	  NC_CHECK (stat);
      }
      indent_out();
      printf ("\t");
      print_name(dims[d_grp].name);
      printf (" = ");
      if(SIZEOF_SIZE_T >= 8) {
	  if (is_unlimited) {
	      printf ("UNLIMITED ; // (%lu currently)\n", 
		      (unsigned long)dims[d_grp].size);
	  } else {
	      printf ("%lu ;\n", (unsigned long)dims[d_grp].size);
	  }
      } else {			/* 32-bit platform */
	  if (is_unlimited) {
	      printf ("UNLIMITED ; // (%u currently)\n", 
		      (unsigned int)dims[d_grp].size);
	  } else {
	      printf ("%u ;\n", (unsigned int)dims[d_grp].size);
	  }
      }
   }
   if(unlimids)
       free(unlimids);
   if(dimids_grp)
       free(dimids_grp);
#else /* not using netCDF-4 */
   for (dimid = 0; dimid < ndims; dimid++) {
      NC_CHECK( nc_inq_dim(ncid, dimid, dims[dimid].name, &dims[dimid].size) );
      indent_out();
      printf ("\t");
      print_name(dims[dimid].name);
      printf (" = ");
      if (dimid == xdimid) {
	  printf ("UNLIMITED ; // (%u currently)\n", 
		  (unsigned int)dims[dimid].size);
      } else {
	  printf ("%u ;\n", (unsigned int)dims[dimid].size);
      }
   }
#endif /* USE_NETCDF4 */

   if (nvars > 0) {
       indent_out();
       printf ("variables:\n");
   }
   /* Because netCDF-4 can have a string attribute with multiple
    * string values, we can't output strings with embedded newlines
    * as what look like multiple strings, as we do for classic and
    * 64-bit offset files.  So we need to know the output file type
    * to know how to print strings with embedded newlines. */
   NC_CHECK( nc_inq_format(ncid, &kind) );
       
   /* For each var, get and print out info. */

   memset((void*)&var,0,sizeof(var));
 
   for (varid = 0; varid < nvars; varid++) {
      NC_CHECK( nc_inq_varndims(ncid, varid, &var.ndims) );
      if(var.dims != NULL) free(var.dims);
      var.dims = (int *) emalloc((var.ndims + 1) * sizeof(int));
      NC_CHECK( nc_inq_var(ncid, varid, var.name, &var.type, 0,
			   var.dims, &var.natts) );
      /* TODO: don't bother if type name not needed here */
      get_type_name(ncid, var.type, type_name);
      var.tinfo = get_typeinfo(var.type);
      indent_out();
/*       printf ("\t%s %s", type_name, var.name); */
      printf ("\t");
      /* TODO: if duplicate type name and not just inherited, print
       * full type name. */
      print_type_name (ncid, var.type);
      printf (" ");
      print_name (var.name);
      if (var.ndims > 0)
	 printf ("(");
      for (id = 0; id < var.ndims; id++) {
	 /* This dim may be in a parent group, so let's look up the
	  * name. */
	 NC_CHECK( nc_inq_dimname(ncid, var.dims[id], dim_name) );
#ifdef USE_NETCDF4
	 /* Subtlety: The following code block is needed because
	  * nc_inq_dimname() currently returns only a simple dimension
	  * name, without a prefix identifying the group it came from.
	  * That's OK unless the dimid identifies a dimension in an
	  * ancestor group that has the same simple name as a
	  * dimension in the current group (or some intermediate
	  * group), in which case the simple name is ambiguous.  This
	  * code tests for that case and provides an absolute dimname
	  * only in the case where a simple name would be
	  * ambiguous. */
	 {
	     int dimid_test;	/* to see if dimname is ambiguous */
	     int locid;		/* group id where dimension is defined */
	     NC_CHECK( nc_inq_dimid(ncid, dim_name, &dimid_test) );
	     locid = ncid;
	     while(var.dims[id] != dimid_test) { /* not in locid, try ancestors */
		 int parent_id;
		 NC_CHECK( nc_inq_grp_parent(locid, &parent_id) );
		 locid = parent_id;
		 NC_CHECK( nc_inq_dimid(locid, dim_name, &dimid_test) );
	     }
	     /* dimid is in group locid, prefix dimname with group name if needed */
	     if(locid != ncid) {
		 size_t len;
		 char *locname;	/* the group name */
		 NC_CHECK( nc_inq_grpname_full(locid, &len, NULL) );
		 locname = emalloc(len + 1);
		 NC_CHECK( nc_inq_grpname_full(locid, &len, locname) );
		 print_name (locname);
		 if(strcmp("/", locname) != 0) { /* not the root group */
		     printf("/");		 /* ensure a trailing slash */
		 }
		 free(locname);
	     }
	 }
#endif	/* USE_NETCDF4 */
	 print_name (dim_name);
	 printf ("%s", id < var.ndims-1 ? ", " : ")");
      }
      printf (" ;\n");

      /* print variable attributes */
      for (ia = 0; ia < var.natts; ia++) { /* print ia-th attribute */
	  pr_att(ncid, kind, varid, var.name, ia);
      }
#ifdef USE_NETCDF4
      /* Print special (virtual) attributes, if option specified */
      if (formatting_specs.special_atts) {
	  pr_att_specials(ncid, kind, varid, &var);
      }
#endif /* USE_NETCDF4 */
   }

   /* get global attributes */
   if (ngatts > 0 || formatting_specs.special_atts) {
      printf ("\n");
      indent_out();
      if (is_root)
	  printf("// global attributes:\n");
      else
	  printf("// group attributes:\n");
   }
   for (ia = 0; ia < ngatts; ia++) { /* print ia-th global attribute */
       pr_att(ncid, kind, NC_GLOBAL, "", ia);
   }
   if (is_root && formatting_specs.special_atts) { /* output special attribute
					   * for format variant */
       pr_att_global_format(ncid, kind);
   }

   /* output variable data, unless "-h" option specified header only
    * or this group is not in list of groups specified by "-g"
    * option  */
   if (! formatting_specs.header_only && group_wanted(ncid) ) {
      if (nvars > 0) {
	  indent_out();
	  printf ("data:\n");
      }
      for (varid = 0; varid < nvars; varid++) {
	 int no_data;
	 /* if var list specified, test for membership */
	 if (formatting_specs.nlvars > 0 && ! idmember(vlist, varid))
	    continue;
	 NC_CHECK( nc_inq_varndims(ncid, varid, &var.ndims) );
	 if(var.dims != NULL) free(var.dims);
	 var.dims = (int *) emalloc((var.ndims + 1) * sizeof(int));
	 NC_CHECK( nc_inq_var(ncid, varid, var.name, &var.type, 0,
			      var.dims, &var.natts) );
	 var.tinfo = get_typeinfo(var.type);
	 /* If coords-only option specified, don't get data for
	  * non-coordinate vars */
	 if (formatting_specs.coord_vals && !iscoordvar(ncid,varid)) {
	    continue;
	 }
	 /* Collect variable's dim sizes */
	 if (vdims) {
	     free(vdims);
	     vdims = 0;
	 }
	 vdims = (size_t *) emalloc((var.ndims + 1) * SIZEOF_SIZE_T);
	 no_data = 0;
	 for (id = 0; id < var.ndims; id++) {
	     size_t len;
	     NC_CHECK( nc_inq_dimlen(ncid, var.dims[id], &len) );
	     if(len == 0) {
		 no_data = 1;
	     }
	     vdims[id] = len;
	 }
	 /* Don't get data for record variables if no records have
	  * been written yet */
	 if (no_data) {
	     free(vdims);
	     vdims = 0;
	     continue;
	 }
	 if(var.fillvalp != NULL) free(var.fillvalp);
	 get_fill_info(ncid, varid, &var); /* sets has_fillval, fillvalp mmbrs */
	 if(var.timeinfo != NULL) {
	     if(var.timeinfo->units) free(var.timeinfo->units);
	     free(var.timeinfo);
	 }
	 get_timeinfo(ncid, varid, &var); /* sets has_timeval, timeinfo mmbrs */
	 /* printf format used to print each value */
	 var.fmt = get_fmt(ncid, varid, var.type);
	 var.locid = ncid;
	 set_tostring_func(&var);
	 if (vardata(&var, vdims, ncid, varid) == -1) {
	    error("can't output data for variable %s", var.name);
	    goto done;
	 }
      }
      if (vdims) {
	  free(vdims);
	  vdims = 0;
      }
   }

#ifdef USE_NETCDF4
   /* For netCDF-4 compiles, check to see if the file has any
    * groups. If it does, this function is called recursively on each
    * of them. */
   {
      int g, numgrps, *ncids;
      char group_name[NC_MAX_NAME + 1];

      /* See how many groups there are. */
      NC_CHECK( nc_inq_grps(ncid, &numgrps, NULL) );
      
      /* Allocate memory to hold the list of group ids. */
      ncids = emalloc((numgrps + 1) * sizeof(int));
      
      /* Get the list of group ids. */
      NC_CHECK( nc_inq_grps(ncid, NULL, ncids) );
      
      /* Call this function for each group. */
      for (g = 0; g < numgrps; g++)
      {
	  NC_CHECK( nc_inq_grpname(ncids[g], group_name) );
	  printf ("\n");
	  indent_out();
/* 	    printf ("group: %s {\n", group_name); */
	  printf ("group: ");
	  print_name (group_name);
	  printf (" {\n");
	  indent_more();
	  do_ncdump_rec(ncids[g], NULL);
	  indent_out();
/* 	    printf ("} // group %s\n", group_name); */
	  printf ("} // group ");
	  print_name (group_name);
	  printf ("\n");
	  indent_less();
      }
      
      free(ncids);
   }
#endif /* USE_NETCDF4 */

done:
   if(var.dims != NULL) free(var.dims);
   if(var.fillvalp != NULL) free(var.fillvalp);
   if(var.timeinfo != NULL) {
      if(var.timeinfo->units) free(var.timeinfo->units);
      free(var.timeinfo);
   }
   if (dims)
      free(dims);
   if (vlist)
      free(vlist);
}


static void
do_ncdump(int ncid, const char *path)
{
   char* esc_specname;
   /* output initial line */
   indent_init();
   indent_out();
   esc_specname=escaped_name(formatting_specs.name);
   printf ("netcdf %s {\n", esc_specname);
   free(esc_specname);
   do_ncdump_rec(ncid, path);
   indent_out();
   printf ("}\n");
}


static void
do_ncdumpx(int ncid, const char *path)
{
    int ndims;			/* number of dimensions */
    int nvars;			/* number of variables */
    int ngatts;			/* number of global attributes */
    int xdimid;			/* id of unlimited dimension */
    int dimid;			/* dimension id */
    int varid;			/* variable id */
    ncdim_t *dims;		/* dimensions */
    ncvar_t var;		/* variable */
    int ia;			/* attribute number */
    int iv;			/* variable number */
    idnode_t* vlist = 0;		/* list for vars specified with -v option */

    /*
     * If any vars were specified with -v option, get list of associated
     * variable ids
     */
    if (formatting_specs.nlvars > 0) {
	vlist = newidlist();	/* list for vars specified with -v option */
	for (iv=0; iv < formatting_specs.nlvars; iv++) {
	    NC_CHECK( nc_inq_varid(ncid, formatting_specs.lvars[iv], &varid) );
	    idadd(vlist, varid);
	}
    }

    /* output initial line */
    pr_initx(ncid, path);

    /*
     * get number of dimensions, number of variables, number of global
     * atts, and dimension id of unlimited dimension, if any
     */
    /* TODO: print names with XML-ish escapes fopr special chars */
    NC_CHECK( nc_inq(ncid, &ndims, &nvars, &ngatts, &xdimid) );
    /* get dimension info */
    dims = (ncdim_t *) emalloc((ndims + 1) * sizeof(ncdim_t));
    for (dimid = 0; dimid < ndims; dimid++) {
	NC_CHECK( nc_inq_dim(ncid, dimid, dims[dimid].name, &dims[dimid].size) );
	if (dimid == xdimid)
  	  printf("  <dimension name=\"%s\" length=\"%d\" isUnlimited=\"true\" />\n", 
		 dims[dimid].name, (int)dims[dimid].size);
	else
	  printf ("  <dimension name=\"%s\" length=\"%d\" />\n", 
		  dims[dimid].name, (int)dims[dimid].size);
    }

    /* get global attributes */
    for (ia = 0; ia < ngatts; ia++)
	pr_attx(ncid, NC_GLOBAL, ia); /* print ia-th global attribute */

    /* get variable info, with variable attributes */
    memset((void*)&var,0,sizeof(var));
    for (varid = 0; varid < nvars; varid++) {
	NC_CHECK( nc_inq_varndims(ncid, varid, &var.ndims) );
	if(var.dims != NULL) free(var.dims);
	var.dims = (int *) emalloc((var.ndims + 1) * sizeof(int));
	NC_CHECK( nc_inq_var(ncid, varid, var.name, &var.type, 0,
			     var.dims, &var.natts) );
	printf ("  <variable name=\"%s\"", var.name);
	pr_shape(&var, dims);

	/* handle one-line variable elements that aren't containers
	   for attributes or data values, since they need to be
	   rendered as <variable ... /> instead of <variable ..>
	   ... </variable> */
	if (var.natts == 0) {
	    if (
		/* header-only specified */
		(formatting_specs.header_only) ||
		/* list of variables specified and this variable not in list */
		(formatting_specs.nlvars > 0 && !idmember(vlist, varid))	||
		/* coordinate vars only and this is not a coordinate variable */
		(formatting_specs.coord_vals && !iscoordvar(ncid, varid)) ||
		/* this is a record variable, but no records have been written */
		(isrecvar(ncid,varid) && dims[xdimid].size == 0)
		) {
		printf (" type=\"%s\" />\n", prim_type_name(var.type));
		continue;
	    }
	}

	/* else nest attributes values, data values in <variable> ... </variable> */
	printf (" type=\"%s\">\n", prim_type_name(var.type));

	/* get variable attributes */
	for (ia = 0; ia < var.natts; ia++) {
	    pr_attx(ncid, varid, ia); /* print ia-th attribute */
	}
	printf ("  </variable>\n");
    }
    
    printf ("</netcdf>\n");
    if (vlist)
	free(vlist);
    if(dims)
	free(dims);
}

static void
make_lvars(char *optarg)
{
    char *cp = optarg;
    int nvars = 1;
    char ** cpp;

    /* compute number of variable names in comma-delimited list */
    formatting_specs.nlvars = 1;
    while (*cp++)
      if (*cp == ',')
 	nvars++;
    formatting_specs.nlvars = nvars;
    formatting_specs.lvars = (char **) emalloc(nvars * sizeof(char*));
    cpp = formatting_specs.lvars;
    /* copy variable names into list */
    for (cp = strtok(optarg, ","); cp != NULL; cp = strtok((char *) NULL, ",")) {
	*cpp = strdup(cp);
	cpp++;
    }
}

static void
make_lgrps(char *optarg)
{
    char *cp = optarg;
    int ngrps = 1;
    char ** cpp;

    /* compute number of group names in comma-delimited list */
    while (*cp++)
      if (*cp == ',')
 	ngrps++;
    formatting_specs.nlgrps = ngrps;
    formatting_specs.lgrps = (char **) emalloc(ngrps * sizeof(char*));
    cpp = formatting_specs.lgrps;
    /* copy group names into list */
    for (cp = strtok(optarg, ","); cp != NULL; cp = strtok((char *) NULL, ",")) {
	*cpp = strdup(cp);
	cpp++;
    }
    /* make empty list of grpids, to be filled in after input file opened */
    formatting_specs.grpids = newidlist();
}


/*
 * Extract the significant-digits specifiers from the (deprecated and
 * undocumented) -d argument on the command-line and update the
 * default data formats appropriately.  This only exists because an
 * old version of ncdump supported the "-d" flag which did not
 * override the C_format attributes (if any).
 */
static void
set_sigdigs(const char *optarg)
{
    char *ptr1 = 0;
    char *ptr2 = 0;
    int flt_digits = FLT_DIGITS; /* default floating-point digits */
    int dbl_digits = DBL_DIGITS; /* default double-precision digits */

    if (optarg != 0 && (int) strlen(optarg) > 0 && optarg[0] != ',')
        flt_digits = (int)strtol(optarg, &ptr1, 10);

    if (flt_digits < 1 || flt_digits > 20) {
	error("unreasonable value for float significant digits: %d",
	      flt_digits);
    }
    if (ptr1 && *ptr1 == ',') {
      dbl_digits = (int)strtol(ptr1+1, &ptr2, 10);
      if (ptr2 == ptr1+1 || dbl_digits < 1 || dbl_digits > 20) {
	  error("unreasonable value for double significant digits: %d",
		dbl_digits);
      }
    }
    set_formats(flt_digits, dbl_digits);
}


/*
 * Extract the significant-digits specifiers from the -p argument on the
 * command-line, set flags so we can override C_format attributes (if any),
 * and update the default data formats appropriately.
 */
static void
set_precision(const char *optarg)
{
    char *ptr1 = 0;
    char *ptr2 = 0;
    int flt_digits = FLT_DIGITS;	/* default floating-point digits */
    int dbl_digits = DBL_DIGITS;	/* default double-precision digits */

    if (optarg != 0 && (int) strlen(optarg) > 0 && optarg[0] != ',') {
        flt_digits = (int)strtol(optarg, &ptr1, 10);
	float_precision_specified = 1;
    }

    if (flt_digits < 1 || flt_digits > 20) {
	error("unreasonable value for float significant digits: %d",
	      flt_digits);
    }
    if (ptr1 && *ptr1 == ',') {
	dbl_digits = (int) strtol(ptr1+1, &ptr2, 10);
	double_precision_specified = 1;
	if (ptr2 == ptr1+1 || dbl_digits < 1 || dbl_digits > 20) {
	    error("unreasonable value for double significant digits: %d",
		  dbl_digits);
	}
    }
    set_formats(flt_digits, dbl_digits);
}

/* Determine whether a variable named varname exists in any group in
   an open netCDF file with id ncid.  If so, return the count of how
   many matching variables were found, else return a count of 0.  The
   variable name can be absolute such as "/foo" or "/GRP1/GRP1A/foo",
   in which case there is only one group to look in, given by the path
   from the root group.  Alternatively, the variable name can be
   relative, such as "foo" or "GRPA/GRPB/foo", in which case every
   group is examined for a variable with that relative name.  */
size_t
nc_inq_varname_count(int ncid, char *varname) {
    /* 
       count = 0;
       status = nc_inq_gvarid(ncid, varname, varid);
       if (status == NC_NOERR)
          count++;
       for each subgroup gid {
          count += nc_inq_varname_count(gid, varname);
       }
       return count;
    */
    size_t count = 0;
    int varid;
    /* look in this group */
    int status = nc_inq_gvarid(ncid, varname, &varid);
#ifdef USE_NETCDF4
    int numgrps;
    int *ncids;
    int g;
#endif

    if (status == NC_NOERR)
	count++;

#ifdef USE_NETCDF4
    /* if this group has subgroups, call recursively on each of them */
    NC_CHECK( nc_inq_grps(ncid, &numgrps, NULL) );
	 
    /* Allocate memory to hold the list of group ids. */
    ncids = emalloc((numgrps + 1) * sizeof(int));
	
    /* Get the list of group ids. */
    NC_CHECK( nc_inq_grps(ncid, NULL, ncids) );
	
    /* Call this function for each group. */
    for (g = 0; g < numgrps; g++) {
	count += nc_inq_varname_count(ncids[g], varname);
    }
    free(ncids);
#endif /* USE_NETCDF4 */
    return count;    
   
}


/* Check if any variable names specified with "-v var1,...,varn" are
 * missing.  Returns 0 if no missing variables detected, otherwise
 * exits. */
static int
missing_vars(int ncid) {
    int iv;
    for (iv=0; iv < formatting_specs.nlvars; iv++) {
	if(nc_inq_varname_count(ncid, formatting_specs.lvars[iv]) == 0) {
	    error("%s: No such variable", formatting_specs.lvars[iv]);
	}
    }
    return 0;
}

/* Determine whether a group named formatting_specs.lgrps[igrp] exists
 * in a netCDF file or group with id ncid.  If so, return the count of
 * how many matching groups were found, else return a count of 0.  If
 * the name begins with "/", it is interpreted as an absolute group
 * name, in which case only 0 or 1 is returned.  Otherwise, interpret
 * it as a relative name, and the total number of occurrences within
 * the file/group identified by ncid is returned.  
 *
 * Also has side effect of updating the ngrpids and the associate
 * grpids array that represent the group list specified by the -g
 * option.  TODO: put this in its own function instead.
 */
static size_t
nc_inq_grpname_count(int ncid, int igrp) {
    size_t count = 0;
#ifdef USE_NETCDF4
    int numgrps;
    int *ncids;
    int g;
    int grpid;
    int status;
#endif
    char *grpname=formatting_specs.lgrps[igrp];

    /* permit empty string to also designate root group */
    if(grpname[0] == '\0' || STREQ(grpname,"/")) { 
	count = 1;
	idadd(formatting_specs.grpids, ncid);
	return count;
    }
#ifdef USE_NETCDF4
    /* Handle absolute group names */
    if(grpname[0] == '/') {
	int grpid;
	status = nc_inq_grp_full_ncid(ncid, grpname, &grpid);
	if(status == NC_NOERR) {
	    count = 1;
	    idadd(formatting_specs.grpids, grpid);
	} else if(status == NC_ENOGRP) {
	    count = 0;
	} else {
	    error("when looking up group %s: %s ", grpname, nc_strerror(status));
	}
	return count;
    }
    
    /* look in this group */
    status = nc_inq_grp_ncid(ncid, grpname, &grpid);
    if (status == NC_NOERR) {
	count++;
	idadd(formatting_specs.grpids, grpid);
    }
    /* if this group has subgroups, call recursively on each of them */
    NC_CHECK( nc_inq_grps(ncid, &numgrps, NULL) );
    if(numgrps > 0) {
	/* Allocate memory to hold the list of group ids. */
	ncids = emalloc(numgrps * sizeof(int));
	/* Get the list of group ids. */
	NC_CHECK( nc_inq_grps(ncid, NULL, ncids) );
	/* Call this function recursively for each group. */
	for (g = 0; g < numgrps; g++) {
	    count += nc_inq_grpname_count(ncids[g], igrp);
	}
	free(ncids);
    }
#endif /* USE_NETCDF4 */
    return count;    
}

/* Check if any group names specified with "-g grp1,...,grpn" are
 * missing.  Returns total number of matching groups if no missing
 * groups detected, otherwise exits. */
static int
grp_matches(int ncid) {
    int ig;
    size_t total = 0;

    for (ig=0; ig < formatting_specs.nlgrps; ig++) {
	size_t count = nc_inq_grpname_count(ncid, ig);
	if(count == 0) {
	    error("%s: No such group", formatting_specs.lgrps[ig]);
	    return 0;
	}
	total += count;
    }
    return total;
}

#ifdef USE_DAP
#define DAP_CLIENT_CACHE_DIRECTIVE	"[cache]"
/* replace path string with same string prefixed by
 * DAP_CLIENT_NCDUMP_DIRECTIVE */
static
void adapt_url_for_cache(char **pathp) {
    char prefix[] = DAP_CLIENT_CACHE_DIRECTIVE;
    char* path = *pathp;
    char *tmp_path = strdup(path);
    path = (char *)emalloc(strlen(prefix) + strlen(tmp_path) + 1);
    path[0] = '\0';
    strncat(path, prefix, strlen(prefix));
    strncat(path, tmp_path, strlen(tmp_path));
    free(tmp_path);
    *pathp = path;
    return;
}
#endif

/** @page ncdump \b ncdump tool - Convert netCDF file to text form (CDL)
@section  SYNOPSIS

\code
ncdump   [-chistxw]  [-v  var1,...]  [-b lang]  [-f lang]
         [-l  len]  [-n  name]  [-p n[,n]]  [-g  grp1,...]  file


ncdump    -k file
\endcode

@section  DESCRIPTION

The \b ncdump utility generates a text representation of a specified
netCDF file on standard output, optionally excluding some or all of
the variable data in the output.  The text representation is in a form
called CDL (network Common Data form Language) that can be viewed,
edited, or serve as input to \b ncgen, a companion program that can
generate a binary netCDF file from a CDL file.  Hence \b ncgen and \b
ncdump can be used as inverses to transform the data representation
between binary and text representations.  See \b ncgen documentation
for a description of CDL and netCDF representations.

\b ncdump may also be used to determine what kind of netCDF file
is used (which variant of the netCDF file format) with the -k
option.

If DAP support was enabled when \b ncdump was built, the file name may
specify a DAP URL. This allows \b ncdump to access data sources from
DAP servers, including data in other formats than netCDF.  When used
with DAP URLs, \b ncdump shows the translation from the DAP data
model to the netCDF data model.

\b ncdump may also be used as a simple browser for netCDF data files,
to display the dimension names and lengths; variable names, types, and
shapes; attribute names and values; and optionally, the values of data
for all variables or selected variables in a netCDF file.  For
netCDF-4 files, groups and user-defined types are also included in \b
ncdump output.

\b ncdump uses '_' to represent data values that are equal to the
'_FillValue' attribute for a variable, intended to represent
data that has not yet been written.  If a variable has no
'_FillValue' attribute, the default fill value for the variable
type is used unless the variable is of byte type.

\b ncdump defines a default display format used for each type of
netCDF data, but this can be changed if a `C_format' attribute
is defined for a netCDF variable.  In this case, \b ncdump will
use the `C_format' attribute to format each value.  For
example, if floating-point data for the netCDF variable `Z' is
known to be accurate to only three significant digits, it would
be appropriate to use the variable attribute

\code
    Z:C_format = "%.3g"
\endcode

@section  OPTIONS

@par -c 
Show the values of \e coordinate \e variables (1D variables with the
same names as dimensions) as well as the declarations of all
dimensions, variables, attribute values, groups, and user-defined
types.  Data values of non-coordinate variables are not included in
the output.  This is usually the most suitable option to use for a
brief look at the structure and contents of a netCDF file.

@par -h
Show only the header information in the output, that is, output only
the declarations for the netCDF dimensions, variables, attributes,
groups, and user-defined types of the input file, but no data values
for any variables. The output is identical to using the '-c' option
except that the values of coordinate variables are not included. (At
most one of '-c' or '-h' options may be present.)

@par -v \a var1,...  

@par 
The output will include data values for the specified variables, in
addition to the declarations of all dimensions, variables, and
attributes. One or more variables must be specified by name in the
comma-delimited list following this option. The list must be a single
argument to the command, hence cannot contain unescaped blanks or
other white space characters. The named variables must be valid netCDF
variables in the input-file. A variable within a group in a netCDF-4
file may be specified with an absolute path name, such as
`/GroupA/GroupA2/var'.  Use of a relative path name such as `var' or
`grp/var' specifies all matching variable names in the file.  The
default, without this option and in the absence of the '-c' or '-h'
options, is to include data values for \e all variables in the output.

@par -b [c|f]
A brief annotation in the form of a CDL comment (text beginning with
the characters '//') will be included in the data section of the
output for each 'row' of data, to help identify data values for
multidimensional variables. If lang begins with 'C' or 'c', then C
language conventions will be used (zero-based indices, last dimension
varying fastest). If lang begins with 'F' or 'f', then FORTRAN
language conventions will be used (one-based indices, first dimension
varying fastest). In either case, the data will be presented in the
same order; only the annotations will differ. This option may be
useful for browsing through large volumes of multidimensional data.

@par -f [c|f]
Full annotations in the form of trailing CDL comments (text beginning
with the characters '//') for every data value (except individual
characters in character arrays) will be included in the data
section. If lang begins with 'C' or 'c', then C language conventions
will be used. If lang begins with 'F' or 'f', then FORTRAN language
conventions will be used. In either case, the data will be presented
in the same order; only the annotations will differ. This option may
be useful for piping data into other filters, since each data value
appears on a separate line, fully identified. (At most one of '-b' or
'-f' options may be present.)

@par -l \e length

@par
Changes the default maximum line length (80) used in formatting lists
of non-character data values.

@par -n \e name

@par
CDL requires a name for a netCDF file, for use by 'ncgen -b' in
generating a default netCDF file name. By default, \b ncdump
constructs this name from the last component of the file name of
the input netCDF file by stripping off any extension it has. Use
the '-n' option to specify a different name. Although the output
file name used by 'ncgen -b' can be specified, it may be wise to
have \b ncdump change the default name to avoid inadvertently
overwriting a valuable netCDF file when using \b ncdump, editing the
resulting CDL file, and using 'ncgen -b' to generate a new netCDF
file from the edited CDL file.

@par -p \e float_digits[, \e double_digits ]

@par
Specifies default precision (number of significant digits) to use in
displaying floating-point or double precision data values for
attributes and variables. If specified, this value overrides the value
of the C_format attribute, if any, for a variable. Floating-point data
will be displayed with \e float_digits significant digits. If \e
double_digits is also specified, double-precision values will be
displayed with that many significant digits. In the absence of any
'-p' specifications, floating-point and double-precision data are
displayed with 7 and 15 significant digits respectively. CDL files can
be made smaller if less precision is required. If both floating-point
and double precisions are specified, the two values must appear
separated by a comma (no blanks) as a single argument to the command.
(To represent every last bit of precision in a CDL file for all
possible floating-point values would requires '-p 9,17'.)

@par -k
Show \e kind of netCDF file, that is which format variant the file uses.
Other options are ignored if this option is specified.  Output will be
one of 'classic'. '64-bit offset', 'netCDF-4', or 'netCDF-4 classic
model'.

@par -s
Specifies that \e special virtual attributes should be output for the
file format variant and for variable properties such as
compression, chunking, and other properties specific to the format
implementation that are primarily related to performance rather
than the logical schema of the data. All the special virtual
attributes begin with '_' followed by an upper-case
letter. Currently they include the global attribute '_Format' and
the variable attributes '_ChunkSizes', '_DeflateLevel',
'_Endianness', '_Fletcher32', '_NoFill', '_Shuffle', and '_Storage'. 
The \b ncgen utility recognizes these attributes and
supports them appropriately.

@par -t
Controls display of time data, if stored in a variable that uses a
udunits compliant time representation such as 'days since 1970-01-01'
or 'seconds since 2009-03-15 12:01:17'.  If this option is specified,
time values are displayed as a human-readable date-time strings rather
than numerical values, interpreted in terms of a 'calendar' variable
attribute, if specified.  For numeric attributes of time variables,
the human-readable time value is displayed after the actual value, in
an associated CDL comment.  Calendar attribute values interpreted with
this option include the CF Conventions values 'gregorian' or
'standard', 'proleptic_gregorian', 'noleap' or '365_day', 'all_leap'
or '366_day', '360_day', and 'julian'.

@par -i
Same as the '-t' option, except output time data as date-time strings
with ISO-8601 standard 'T' separator, instead of a blank.

@par -g \e grp1,...

@par
The output will include data values only for the specified groups.
One or more groups must be specified by name in the comma-delimited
list following this option. The list must be a single argument to the
command. The named groups must be valid netCDF groups in the
input-file. The default, without this option and in the absence of the
'-c' or '-h' options, is to include data values for all groups in the
output.

@par -w
For file names that request remote access using DAP URLs, access data
with client-side caching of entire variables.

@par -x
Output XML (NcML) instead of CDL.  The NcML does not include data values.
The NcML output option currently only works for netCDF classic model data.

@section  EXAMPLES

Look at the structure of the data in the netCDF file foo.nc:

\code
   ncdump -c foo.nc
\endcode

Produce an annotated CDL version of the structure and data in the
netCDF file foo.nc, using C-style indexing for the annotations:

\code
   ncdump -b c foo.nc > foo.cdl
\endcode

Output data for only the variables uwind and vwind from the netCDF
file foo.nc, and show the floating-point data with only three
significant digits of precision:

\code
   ncdump -v uwind,vwind -p 3 foo.nc
\endcode

Produce a fully-annotated (one data value per line) listing of the
data for the variable omega, using FORTRAN conventions for indices,
and changing the netCDF file name in the resulting CDL file to
omega:

\code
   ncdump -v omega -f fortran -n omega foo.nc > Z.cdl
\endcode

Examine the translated DDS for the DAP source from the specified URL:

\code
   ncdump -h http://test.opendap.org:8080/dods/dts/test.01 
\endcode

Without dumping all the data, show the special virtual attributes that indicate
performance-related characterisitics of a netCDF-4 file:

\code
   ncdump -h -s nc4file.nc
\endcode

@section see_also SEE ALSO

ncgen(1), netcdf(3)

@section string_note NOTE ON STRING OUTPUT

For classic, 64-bit offset or netCDF-4 classic model data, \b ncdump
generates line breaks after embedded newlines in displaying character
data.  This is not done for netCDF-4 files, because netCDF-4 supports
arrays of real strings of varying length.
 */
int
main(int argc, char *argv[])
{
    int c;
    int i;
    int max_len = 80;		/* default maximum line length */
    int nameopt = 0;
    boolean xml_out = false;    /* if true, output NcML instead of CDL */
    boolean kind_out = false;	/* if true, just output kind of netCDF file */

#if defined(WIN32) || defined(msdos) || defined(WIN64)
    putenv("PRINTF_EXPONENT_DIGITS=2"); /* Enforce unix/linux style exponent formatting. */
#endif

#ifdef HAVE_LOCALE_H
    setlocale(LC_ALL, "C");     /* CDL may be ambiguous with other locales */
#endif /* HAVE_LOCALE_H */
    opterr = 1;
    progname = argv[0];
    set_formats(FLT_DIGITS, DBL_DIGITS); /* default for float, double data */

    /* If the user called ncdump without arguments, print the usage
     * message and return peacefully. */
    if (argc <= 1)
    {
       usage();
#ifdef vms
    exit(EXIT_SUCCESS);
#else
    return EXIT_SUCCESS;
#endif
    }

    while ((c = getopt(argc, argv, "b:cd:f:g:hikl:n:p:stv:xw")) != EOF)
      switch(c) {
	case 'h':		/* dump header only, no data */
	  formatting_specs.header_only = true;
	  break;
	case 'c':		/* header, data only for coordinate dims */
	  formatting_specs.coord_vals = true;
	  break;
	case 'n':		/*
				 * provide different name than derived from
				 * file name
				 */
	  formatting_specs.name = optarg;
	  nameopt = 1;
	  break;
	case 'b':		/* brief comments in data section */
	  formatting_specs.brief_data_cmnts = true;
	  switch (tolower(optarg[0])) {
	    case 'c':
	      formatting_specs.data_lang = LANG_C;
	      break;
	    case 'f':
	      formatting_specs.data_lang = LANG_F;
	      break;
	    default:
	      error("invalid value for -b option: %s", optarg);
	  }
	  break;
	case 'f':		/* full comments in data section */
	  formatting_specs.full_data_cmnts = true;
	  switch (tolower(optarg[0])) {
	    case 'c':
	      formatting_specs.data_lang = LANG_C;
	      break;
	    case 'f':
	      formatting_specs.data_lang = LANG_F;
	      break;
	    default:
	      error("invalid value for -f option: %s", optarg);
	  }
	  break;
	case 'l':		/* maximum line length */
	  max_len = (int) strtol(optarg, 0, 0);
	  if (max_len < 10) {
	      error("unreasonably small line length specified: %d", max_len);
	  }
	  break;
	case 'v':		/* variable names */
	  /* make list of names of variables specified */
	  make_lvars (optarg);
	  break;
	case 'g':		/* group names */
	  /* make list of names of groups specified */
	  make_lgrps (optarg);
	  break;
	case 'd':		/* specify precision for floats (deprecated, undocumented) */
	  set_sigdigs(optarg);
	  break;
	case 'p':		/* specify precision for floats, overrides attribute specs */
	  set_precision(optarg);
	  break;
        case 'x':		/* XML output (NcML) */
	  xml_out = true;
	  break;
        case 'k':	        /* just output what kind of netCDF file */
	  kind_out = true;
	  break;
	case 't':		/* human-readable strings for date-time values */
	  formatting_specs.string_times = true;
	  formatting_specs.iso_separator = false;
	  break;
	case 'i':		/* human-readable strings for data-time values with 'T' separator */
	  formatting_specs.string_times = true;
	  formatting_specs.iso_separator = true;
	  break;
        case 's':	    /* output special (virtual) attributes for
			     * netCDF-4 files and variables, including
			     * _DeflateLevel, _Chunking, _Endianness,
			     * _Format, _Checksum, _NoFill */
	  formatting_specs.special_atts = true;
	  break;
        case 'w':		/* with client-side cache for DAP URLs */
	  formatting_specs.with_cache = true;
	  break;
        case '?':
	  usage();
	  return EXIT_FAILURE;
      }

    set_max_len(max_len);
    
    argc -= optind;
    argv += optind;

    /* If no file arguments left or more than one, print usage message. */
    if (argc != 1)
    {
       usage();
       return EXIT_FAILURE;
    }

    i = 0;

    init_epsilons();

    {		
	char *path = strdup(argv[i]);
	if(!path)
	    error("out of memory copying argument %s", argv[i]);
        if (!nameopt) 
	    formatting_specs.name = name_path(path);
	if (argc > 0) {
	    int ncid, nc_status;
	    /* If path is a URL, prefix with client-side directive to
	     * make ncdump reasonably efficient */
#ifdef USE_DAP
	    if(formatting_specs.with_cache) /* by default, don't use cache directive */
	    {
		extern int nc__testurl(const char*,char**);
		/* See if this is a url */
		if(nc__testurl(path, NULL)) {
		    adapt_url_for_cache(&path);
		}
		/* else fall thru and treat like a file path */
	    }
#endif /*USE_DAP*/
	    nc_status = nc_open(path, NC_NOWRITE, &ncid);
	    if (nc_status != NC_NOERR) {
		error("%s: %s", path, nc_strerror(nc_status));
	    }
	    NC_CHECK( nc_inq_format(ncid, &formatting_specs.nc_kind) );
	    if (kind_out) {
		printf ("%s\n", kind_string(formatting_specs.nc_kind));
	    } else {
		/* Initialize list of types. */
		init_types(ncid);
		/* Check if any vars in -v don't exist */
		if(missing_vars(ncid))
		    return EXIT_FAILURE;
		if(formatting_specs.nlgrps > 0) {
		    if(formatting_specs.nc_kind != NC_FORMAT_NETCDF4) {
			error("Group list (-g ...) only permitted for netCDF-4 file");
			return EXIT_FAILURE;
		    }
		    /* Check if any grps in -g don't exist */
		    if(grp_matches(ncid) == 0)
			return EXIT_FAILURE;
		}
		if (xml_out) {
		    if(formatting_specs.nc_kind == NC_FORMAT_NETCDF4) {
			error("NcML output (-x) currently only permitted for netCDF classic model");
			return EXIT_FAILURE;
		    }
		    do_ncdumpx(ncid, path);
		} else {
		    do_ncdump(ncid, path);
		}
	    }
	    NC_CHECK( nc_close(ncid) );
	}
	free(path);
    }
#ifdef vms
    exit(EXIT_SUCCESS);
#else
    return EXIT_SUCCESS;
#endif
}
END_OF_MAIN();
