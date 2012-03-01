/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncdump/vardata.c,v 1.48 2010/05/05 22:15:39 dmh Exp $
 *********************************************************************/

#include <config.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <netcdf.h>
#include "ncdump.h"
#include "dumplib.h"
#include "indent.h"
#include "vardata.h"
#include "utils.h"

/* maximum len of string needed for one value of a primitive type */
#define MAX_OUTPUT_LEN 100
#define	STREQ(a, b)	(*(a) == *(b) && strcmp((a), (b)) == 0)

#define LINEPIND	"    "	/* indent of continued lines */

/* Only read this many values at a time, if last dimension is larger
  than this */
#define VALBUFSIZ 8096

static int linep;		/* line position, not counting global indent */
static int max_line_len;	/* max chars per line, not counting global indent */


/* set position in line before lput() calls */
static void
set_indent(int in) {
    linep = in;
}


void
set_max_len(int len) {
    max_line_len = len-2;
}


/* 
 * Output a string that should not be split across lines.  If it would
 * make current line too long, first output a newline and current
 * (nested group) indentation, then continuation indentation, then
 * output string.
 */
void
lput(const char *cp) {
    size_t nn = strlen(cp);

    if (nn+linep > max_line_len && nn > 2) {
	(void) fputs("\n", stdout);
	indent_out();
	(void) fputs(LINEPIND, stdout);
	linep = (int)strlen(LINEPIND) + indent_get();
    }
    (void) fputs(cp,stdout);
    linep += nn;
}


/*
 * Output a value of an attribute.
 */
static void
print_any_att_val (
    struct safebuf_t *sb,	/* string where output goes */
    const ncatt_t *attp,	/* attrbute */
    const void *valp		/* pointer to the value */
	    ) {
    nctype_t *typ = attp->tinfo;
    (*typ->typ_tostring)(typ, sb, valp);
}


/*
 * Output a value of a variable, except if there is a fill value for
 * the variable and the value is the fill value, print the fill-value string
 * instead.  (Floating-point fill values need only be within machine epsilon of
 * defined fill value.)
 */
static void
print_any_val(
    safebuf_t *sb,		/* string where output goes */
    const ncvar_t *varp,	/* variable */
    const void *valp		/* pointer to the value */
	    )
{
    if (varp->has_fillval && 
	(*(varp->tinfo->val_equals))((const nctype_t *)varp->tinfo, 
				     (const void*)varp->fillvalp, valp) ) {
	sbuf_cpy(sb, FILL_STRING);
    } else {
	(*varp->val_tostring)(varp, sb, valp);
    }
}

/*
 * print last delimiter in each line before annotation (, or ;)
 */
static void
lastdelim (boolean more, boolean lastrow)
{
    if (more) {
	printf(", ");
    } else {
	if(lastrow) {
	    printf(";");
	} else {
	    printf(",");
	}
    }
}

/*
 * print last delimiter in each line before annotation (, or ;)
 */
static void
lastdelim2 (boolean more, boolean lastrow)
{
    if (more) {
	lput(", ");
    } else {
	if(lastrow) {
	    lput(" ;");
	    lput("\n");
	} else {
	    lput(",\n");
	    lput("  ");
	}
    }
}


/*
 * Print a number of attribute values
 */
void
pr_any_att_vals(
     const ncatt_t *ap,		/* attribute */
     const void *vals		/* pointer to block of values */
     )
{
    size_t iel;
    size_t len = ap->len;		/* number of values to print */
    const char *valp = (const char *)vals;
    safebuf_t *sb = sbuf_new();

    for (iel = 0; iel < len - 1; iel++) {
	print_any_att_val(sb, ap, (void *)valp);
	valp += ap->tinfo->size; /* next value according to type */
	sbuf_cat(sb, iel == len - 1 ? "" : ", ");
	lput(sbuf_str(sb));
    }
    print_any_att_val(sb, ap, (void *)valp);
    lput(sbuf_str(sb));
    sbuf_free(sb);
}


/*
 * Annotates a value in data section with var name and indices in comment
 */
static void
annotate(
     const ncvar_t *vp,	/* variable */
     const fspec_t* fsp,	/* formatting specs */
     const size_t *cor,		/* corner coordinates */
     long iel			/* which element in current row */
     )
{
    int vrank = vp->ndims;
    int id;
    
    /* print indices according to data_lang */
/*     printf("  // %s(", vp->name); */
    printf("  // ");
    print_name(vp->name);
    printf("(");
    switch (fsp->data_lang) {
      case LANG_C:
	/* C variable indices */
	for (id = 0; id < vrank-1; id++)
	  printf("%lu,", (unsigned long) cor[id]);
	printf("%lu", (unsigned long) cor[id] + iel);
	break;
      case LANG_F:
	/* Fortran variable indices */
	printf("%lu", (unsigned long) cor[vrank-1] + iel + 1);
	for (id = vrank-2; id >=0 ; id--) {
	    printf(",%lu", 1 + (unsigned long) cor[id]);
	}
	break;
    }
    printf(")\n    ");
}


/*
 * Print a number of variable values, where the optional comments
 * for each value identify the variable, and each dimension index.
 */
static void
pr_any_vals(
     const ncvar_t *vp,		/* variable */
     size_t len,		/* number of values to print */
     boolean more,		/* true if more data for this row will
				 * follow, so add trailing comma */
     boolean lastrow,		/* true if this is the last row for this
				 * variable, so terminate with ";" instead
				 * of "," */
     const void *vals,		/* pointer to block of values */
     const fspec_t* fsp,	/* formatting specs */
     const size_t *cor		/* corner coordinates */
     )
{
    long iel;
    safebuf_t *sb = sbuf_new();
    const char *valp = (const char *)vals;

    for (iel = 0; iel < len-1; iel++) {
	print_any_val(sb, vp, (void *)valp);
	valp += vp->tinfo->size; /* next value according to type */
	if (fsp->full_data_cmnts) {
	    printf("%s, ", sb->buf);
	    annotate (vp, fsp, cor, iel);
	} else {
	    sbuf_cat(sb, ", ");
	    lput(sbuf_str(sb));
	}
    }
    print_any_val(sb, vp, (void *)valp);
    if (fsp->full_data_cmnts) {
	printf("%s", sbuf_str(sb));
	lastdelim (more, lastrow);
	annotate (vp, fsp, cor, iel);
    } else {
	lput(sbuf_str(sb));
	lastdelim2 (more, lastrow);
    }
    sbuf_free(sb);
}


/*
 * Print a number of char variable values as a text string, where the
 * optional comments for each value identify the variable, and each
 * dimension index.
 */
static void
pr_tvals(
     const ncvar_t *vp,		/* variable */
     size_t len,		/* number of values to print */
     boolean more,		/* true if more data for this row will
				 * follow, so add trailing comma */
     boolean lastrow,		/* true if this is the last row for this
				 * variable, so terminate with ";" instead
				 * of "," */
     const char *vals,		/* pointer to block of values */
     const fspec_t* fsp,	/* formatting specs */
     const size_t *cor		/* corner coordinates */
     )
{
    long iel;
    const char *sp;

    printf("\"");
    /* adjust len so trailing nulls don't get printed */
    sp = vals + len;
    while (len != 0 && *--sp == '\0')
	len--;
    for (iel = 0; iel < len; iel++) {
	unsigned char uc;
	switch (uc = *vals++ & 0377) {
	case '\b':
	    printf("\\b");
	    break;
	case '\f':
	    printf("\\f");
	    break;
	case '\n':	/* generate linebreaks after new-lines */
	    printf("\\n\",\n    \"");
	    break;
	case '\r':
	    printf("\\r");
	    break;
	case '\t':
	    printf("\\t");
	    break;
	case '\v':
	    printf("\\v");
	    break;
	case '\\':
	    printf("\\\\");
	    break;
	case '\'':
	    printf("\\\'");
	    break;
	case '\"':
	    printf("\\\"");
	    break;
	default:
	    if (isprint(uc))
		printf("%c",uc);
	    else
		printf("\\%.3o",uc);
	    break;
	}
    }
    printf("\"");
    if (fsp && fsp->full_data_cmnts) {
	lastdelim (more, lastrow);
	annotate (vp, fsp,  (size_t *)cor, 0L);
    } else {
	lastdelim2 (more, lastrow);
    }
}


/*
 * Updates a vector of ints, odometer style.  Returns 0 if odometer
 * overflowed, else 1.
 */
static int
upcorner(
     const size_t *dims,	/* The "odometer" limits for each dimension */
     int ndims,			/* Number of dimensions */
     size_t* odom,		/* The "odometer" vector to be updated */
     const size_t* add		/* A vector to "add" to odom on each update */
     )
{
    int id;
    int ret = 1;

    for (id = ndims-1; id > 0; id--) {
	odom[id] += add[id];
	if(odom[id] >= dims[id]) {
	    odom[id-1]++;
	    odom[id] -= dims[id];
	}
    }
    odom[0] += add[0];
    if (odom[0] >= dims[0])
      ret = 0;
    return ret;
}


/* Output the data for a single variable, in CDL syntax. */
int
vardata(
     const ncvar_t *vp,		/* variable */
     size_t vdims[],		/* variable dimension sizes */
     int ncid,			/* netcdf id */
     int varid,			/* variable id */
     const fspec_t *fsp	        /* formatting specs */
     )
{
    size_t *cor;	     /* corner coordinates */
    size_t *edg;	     /* edges of hypercube */
    size_t *add;	     /* "odometer" increment to next "row"  */
    size_t gulp;
    void *vals;

    int id;
    int ir;
    size_t nels;
    size_t ncols;
    size_t nrows;
    int vrank = vp->ndims;

    cor = (size_t *) emalloc((1 + vrank) * sizeof(size_t));
    edg = (size_t *) emalloc((1 + vrank) * sizeof(size_t));
    add = (size_t *) emalloc((1 + vrank) * sizeof(size_t));

    nels = 1;
    if(vrank == 0) { /*scalar*/
	cor[0] = 0;
	edg[0] = 1;
    } for (id = 0; id < vrank; id++) {
	cor[id] = 0;
	edg[id] = 1;
	nels *= vdims[id];	/* total number of values for variable */
    }

    printf("\n");
    indent_out();
/* 	printf(" %s = ", vp->name); */
/*          or      */
/* 	printf(" %s =\n  ", vp->name); */
	printf(" ");
	print_name(vp->name);
    if (vrank <= 1) {
	printf(" = ");
	set_indent ((int)strlen(vp->name) + 4 + indent_get());
    } else {
	printf(" =\n  ");
	set_indent (2 + indent_get());
    }

    if (vrank < 1) {
	ncols = 1;
    } else {
	ncols = vdims[vrank-1];	/* size of "row" along last dimension */
	edg[vrank-1] = vdims[vrank-1];
	for (id = 0; id < vrank; id++)
	  add[id] = 0;
	if (vrank > 1)
	  add[vrank-2] = 1;
    }
    nrows = nels/ncols;		/* number of "rows" */
    gulp = ncols < VALBUFSIZ ? ncols : VALBUFSIZ;
    vals = emalloc(gulp * vp->tinfo->size);
    
    for (ir = 0; ir < nrows; ir++) {
	/*
	 * rather than just printing a whole row at once (which might
	 * exceed the capacity of some platforms), we break each row
	 * into smaller chunks, if necessary.
	 */
	size_t corsav = 0;
	int left = (int)ncols;
	boolean lastrow;

	if (vrank > 0) {
	    corsav = cor[vrank-1];
	    if (fsp->brief_data_cmnts != false
		&& vrank > 1
		&& left > 0) {	/* print brief comment with indices range */
/* 		printf("// %s(",vp->name); */
		printf("// ");
		print_name(vp->name);
		printf("(");
		switch (fsp->data_lang) {
		  case LANG_C:
		    /* print brief comment with C variable indices */
		    for (id = 0; id < vrank-1; id++)
		      printf("%lu,", (unsigned long)cor[id]);
		    if (vdims[vrank-1] == 1)
		      printf("0");
		    else
		      printf(" 0-%lu", (unsigned long)vdims[vrank-1]-1);
		    break;
		  case LANG_F:
		    /* print brief comment with Fortran variable indices */
		    if (vdims[vrank-1] == 1)
		      printf("1");
		    else
		      printf("1-%lu ", (unsigned long)vdims[vrank-1]);
		    for (id = vrank-2; id >=0 ; id--) {
			printf(",%lu", (unsigned long)(1 + cor[id]));
		    }
		    break;
		}
		printf(")\n");
		indent_out();
		printf("    ");
		set_indent(4 + indent_get());
	    }
	}
	lastrow = (boolean)(ir == nrows-1);
	while (left > 0) {
	    size_t toget = left < gulp ? left : gulp;
	    if (vrank > 0)
	      edg[vrank-1] = toget;
	    NC_CHECK(nc_get_vara(ncid, varid, cor, edg, vals));
	    /* Test if we should treat array of chars as a string  */
	    if(vp->type == NC_CHAR && 
	       (vp->fmt == 0 || STREQ(vp->fmt,"%s") || STREQ(vp->fmt,""))) {
	        pr_tvals(vp, toget, left > toget, lastrow, (char *) vals, 
			 fsp, cor);
	    } else {
	        pr_any_vals(vp, toget, left > toget, lastrow, vals, fsp, cor);
	    }

	    left -= toget;
	    if (vrank > 0)
	      cor[vrank-1] += toget;
	}
	if (vrank > 0)
	  cor[vrank-1] = corsav;
	if (ir < nrows-1)
	  if (!upcorner(vdims,vp->ndims,cor,add))
	    error("vardata: odometer overflowed!");
	set_indent(2);
    }

    free(vals);
    free(cor);
    free(edg);
    free(add);

    return 0;
}


/*
 * print last delimiter in each line before annotation (, or ;)
 */
static void
lastdelim2x (boolean more, boolean lastrow)
{
    if (more) {
	lput(" ");
    } else {
	if(lastrow) {
	    lput("\n   ");
	} else {
	    lput("\n     ");
	}
    }
}


/*
 * Print a number of char variable values as a text string for NcML
 */
static void
pr_tvalsx(
     const ncvar_t *vp,		/* variable */
     size_t len,		/* number of values to print */
     boolean more,		/* true if more data for this row will
				 * follow, so add trailing comma */
     boolean lastrow,		/* true if this is the last row for this
				 * variable, so terminate with ";" instead
				 * of "," */
     const char *vals		/* pointer to block of values */
     )
{
    long iel;
    const char *sp;

    printf("\"");
    /* adjust len so trailing nulls don't get printed */
    sp = vals + len;
    while (len != 0 && *--sp == '\0')
	len--;
    for (iel = 0; iel < len; iel++) {
	unsigned char uc;
	switch (uc = *vals++ & 0377) {
	case '\b':
	    printf("\\b");
	    break;
	case '\f':
	    printf("\\f");
	    break;
	case '\n':	/* generate linebreaks after new-lines */
	    printf("\\n\",\n    \"");
	    break;
	case '\r':
	    printf("\\r");
	    break;
	case '\t':
	    printf("\\t");
	    break;
	case '\v':
	    printf("\\v");
	    break;
	case '\\':
	    printf("\\\\");
	    break;
	case '\'':
	    printf("\\\'");
	    break;
	case '\"':
	    printf("\\\"");
	    break;
	default:
	    if (isprint(uc))
		printf("%c",uc);
	    else
		printf("\\%.3o",uc);
	    break;
	}
    }
    printf("\"");
    lastdelim2x (more, lastrow);
}


/*
 * Print a number of variable values for NcML
 */
static void
pr_any_valsx(
     const ncvar_t *vp,		/* variable */
     size_t len,		/* number of values to print */
     boolean more,		/* true if more data for this row will
				 * follow, so add trailing comma */
     boolean lastrow,		/* true if this is the last row for this
				 * variable, so terminate with ";" instead
				 * of "," */
     const void *vals		/* pointer to block of values */
     )
{
    long iel;
    safebuf_t *sb = sbuf_new();
    const char *valp = (const char *)vals;

    for (iel = 0; iel < len-1; iel++) {
	print_any_val(sb, vp, (void *)valp);
	valp += vp->tinfo->size; /* next value according to type */
	sbuf_cat(sb, " ");
	lput(sbuf_str(sb));
    }
    print_any_val(sb, vp, (void *)valp);
    lput(sbuf_str(sb));
    lastdelim2x (more, lastrow);
    sbuf_free(sb);
}


/* Output the data for a single variable, in NcML syntax.
 *  TODO: currently not called, need option for NcML with values ... */
int
vardatax(
     const ncvar_t *vp,		/* variable */
     size_t vdims[],		/* variable dimension sizes */
     int ncid,			/* netcdf id */
     int varid,			/* variable id */
     const fspec_t *fsp	        /* formatting specs */
     )
{
    size_t *cor;	     /* corner coordinates */
    size_t *edg;	     /* edges of hypercube */
    size_t *add;	     /* "odometer" increment to next "row"  */
    size_t gulp;
    void *vals;

    int id;
    int ir;
    size_t nels;
    size_t ncols;
    size_t nrows;
    int vrank = vp->ndims;

    cor = (size_t *) emalloc((vrank + 1) * sizeof(size_t));
    edg = (size_t *) emalloc((vrank + 1) * sizeof(size_t));
    add = (size_t *) emalloc((vrank + 1) * sizeof(size_t));

    nels = 1;
    for (id = 0; id < vrank; id++) {
	cor[id] = 0;
	edg[id] = 1;
	nels *= vdims[id];	/* total number of values for variable */
    }

    printf("    <values>\n     ");
    set_indent (7);

    if (vrank < 1) {
	ncols = 1;
    } else {
	ncols = vdims[vrank-1];	/* size of "row" along last dimension */
	edg[vrank-1] = vdims[vrank-1];
	for (id = 0; id < vrank; id++)
	  add[id] = 0;
	if (vrank > 1)
	  add[vrank-2] = 1;
    }
    nrows = nels/ncols;		/* number of "rows" */
    gulp = ncols < VALBUFSIZ ? VALBUFSIZ : ncols;
    vals = emalloc(gulp * vp->tinfo->size);
    
    for (ir = 0; ir < nrows; ir++) {
	/*
	 * rather than just printing a whole row at once (which might
	 * exceed the capacity of some platforms), we break each row
	 * into smaller chunks, if necessary.
	 */
	size_t corsav;
	int left = (int)ncols;
	boolean lastrow;

	if (vrank > 0) {
	    corsav = cor[vrank-1];
	}
	lastrow = (boolean)(ir == nrows-1);
	while (left > 0) {
	    size_t toget = left < gulp ? left : gulp;
	    if (vrank > 0)
	      edg[vrank-1] = toget;
	    NC_CHECK(nc_get_vara(ncid, varid, cor, edg, vals) );

	    /* Test if we should treat array of chars as a string  */
	    if(vp->type == NC_CHAR && 
	       (vp->fmt == 0 || STREQ(vp->fmt,"%s") || STREQ(vp->fmt,""))) {
	        pr_tvalsx(vp, toget, left > toget, lastrow, (char *) vals);
	    } else {
	        pr_any_valsx(vp, toget, left > toget, lastrow, vals);
	    }

	    left -= toget;
	    if (vrank > 0)
	      cor[vrank-1] += toget;
	}
	if (vrank > 0)
	  cor[vrank-1] = corsav;
	if (ir < nrows-1)
	  if (!upcorner(vdims,vp->ndims,cor,add))
	    error("vardata: odometer overflowed!");
	set_indent(2);
    }
    printf(" </values>\n");
    free(vals);
    free(cor);
    free(edg);
    free(add);
    return 0;
}
