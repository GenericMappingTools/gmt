/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id$ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/genchar.c,v 1.6 2010/05/24 19:59:57 dmh Exp $ */

#include "includes.h"

/******************************************************/
/* Code for generating char variables etc; mostly
   language independent */
/******************************************************/

/*
Matching strings to char variables, attributes, and vlen
constants is challenging because it is desirable to mimic
the original ncgen. The "algorithms" used there have no
simple characterization (such as "abc" == {'a','b','c'}).
So, this rather ugly code is kept in this file
and a variety of heuristics are used to mimic ncgen.
*/

static void gen_chararrayr(Dimset*, Bytebuffer*, int index, Datalist*, int fillchar, size_t);

extern List* vlenconstants;


void
gen_charattr(Symbol* asym, Bytebuffer* databuf)
{
    Datasrc* src;
    Constant* con;

    if(asym->data == NULL) return;
    src = datalist2src(asym->data);
    while((con=srcnext(src))) {
	switch (con->nctype) {
	/* Following list should be consistent with isstringable */
	case NC_CHAR:
	    bbAppend(databuf,con->value.charv);
	    break;
	case NC_BYTE:
	    bbAppend(databuf,con->value.int8v);
	    break;
	case NC_UBYTE:
	    bbAppend(databuf,con->value.uint8v);
	    break;
	case NC_STRING:
	    bbCat(databuf,con->value.stringv.stringv);
	    bbNull(databuf);
	    break;
	case NC_FILL:
	    bbAppend(databuf,NC_FILL_CHAR);
	    break;
	default:
	    semerror(srcline(src),
		     "Encountered non-string constant in attribute: %s",
		     asym->name);
	    return;
	}
    }
}

#ifdef IGNORE
static void
datalistpad(Datalist* data, size_t targetlen)
{
    int i;
    /* pad this datalist to target length */
    for(i=data->length;i<targetlen;i++) {
        Constant ccon,scon;
        Datalist* sublist;
        ccon.nctype = NC_COMPOUND;
        ccon.lineno = 0;
        ccon.filled = 0;
        ccon.value.compoundv = builddatalist(1);
        scon.value.stringv.len = 0;
        scon.value.stringv.stringv = strdup("");
        scon.nctype = NC_STRING;
        scon.lineno = 0;
        scon.filled = 0;
        sublist = ccon.value.compoundv;
        dlappend(sublist,&scon);
    }
}
#endif


/*Note see comment before semantics.c:walkchararray.
  This code finishes off the processing in those comments.
  In particular, at this point, the sizes of all
  unlimited is known. Assuming the same set of 
  cases as in that comment, we do the additional actions.
  Cases:
    1. the variable's dimension set has no unlimiteds
       Action:
       1. do nothing
    2. the dimension set has one or more unlimiteds.
       This means that we have to recursively deal with
       nested compound instances.

       The last (rightmost) unlimited will correspond
       to a sequence of stringables.
       This has two special subcases
       2a. the last dimension IS NOT unlimited
	   Actions:
	   1. pad the concat to the size of the last dimension
       2b. the last dimension IS unlimited
	   Actions:
	   1. pad the concata to the actual unlimited size

    3. For each dimension to the left of the last
       unlimited, there are two cases.
       3a. dimension is NOT unlimited
	   ACTION: 
	   1. pad
       3b. dimension IS unlimited
	   ACTION: 
	   1. pad
*/



void
gen_chararray(Symbol* vsym, Bytebuffer* databuf, Datalist* fillsrc)
{
    int i,fillchar = getfillchar(fillsrc);
    Datalist* data = vsym->data;
    int lastunlimitedindex = lastunlimited(&vsym->typ.dimset);

    /* If there is no unlimited, then treat similarly to a field array */
    if(lastunlimitedindex < 0) {
        /* Semantics.c:walkchararray will have done all the hard work */
	for(i=0;i<data->length;i++) {
	    Constant* con = data->data+i;
            ASSERT(con->nctype == NC_STRING);
            bbAppendn(databuf,con->value.stringv.stringv,con->value.stringv.len);
	}
    } else {/* dimset has at least 1 unlimited */
	/* Compute sub array size for extensions */
	size_t subsize;
        Dimset* dimset = &vsym->typ.dimset;
	for(subsize=1,i=lastunlimitedindex+1;i<dimset->ndims;i++) {
	    Symbol* dim =  dimset->dimsyms[i];
	    size_t declsize = dim->dim.declsize;
	    subsize *= (declsize == NC_UNLIMITED ? dim->dim.unlimitedsize : declsize);
        }
        gen_chararrayr(&vsym->typ.dimset, databuf, 0, data, fillchar, subsize);
    }
    bbNull(databuf);
}

static void
gen_chararrayr(Dimset* dimset, Bytebuffer* databuf, int index, Datalist* data,
               int fillchar, size_t subsize)
{
    int i;
    Symbol* dim = dimset->dimsyms[index];
    int isunlimited = dim->dim.declsize == NC_UNLIMITED;
    int lastunlimitedindex = lastunlimited(dimset);
    
    /* Split on last unlimited */
    if(index == lastunlimitedindex) {
	Constant* con;
	/* pad to the unlimited size of the dimension * subsize */
	ASSERT(data->length == 1);
	con = data->data;
	ASSERT(con->nctype == NC_STRING);
	padstring(con,dim->dim.unlimitedsize*subsize,fillchar);
	bbAppendn(databuf,con->value.stringv.stringv,con->value.stringv.len);
    } else {/* index < lastunlimitedindex*/
	/* data should be a set of compounds */
	size_t expected = (isunlimited ? dim->dim.unlimitedsize : dim->dim.declsize );
	for(i=0;i<expected;i++) {

	    if(i >= data->length) {/* pad buffer */
		int j;
	        for(j=0;j<subsize;j++) bbAppend(databuf,fillchar);
	    } else {
	        Constant* con = data->data+i;
                if(con->nctype != NC_COMPOUND) continue;
                /* recurse */
                gen_chararrayr(dimset,databuf,index+1,con->value.compoundv,
                               fillchar,subsize);
	    }
        }
    }
}

/*
Since the field has fixed
dimensions, we can just
read N elements where N
is the product of the dimensions.
*/

void
gen_charfield(Datasrc* src, Odometer* odom, Bytebuffer* fieldbuf)
{
    Constant* con = srcnext(src);

    /* Semantics.c:walkcharfieldarray will have done all the hard work */
    ASSERT(con->nctype == NC_STRING);
    bbAppendn(fieldbuf,con->value.stringv.stringv,con->value.stringv.len);
}

void
gen_charvlen(Datasrc* vlensrc, Bytebuffer* databuf)
{
    Constant* con = srcnext(vlensrc);

    /* Semantics.c:walkcharfieldarray will have done all the hard work */
    ASSERT(con->nctype == NC_STRING);
    bbAppendn(databuf,con->value.stringv.stringv,con->value.stringv.len);
#ifdef IGNORE
    int count;
    Bytebuffer* vlenbuf = bbNew();
    Constant* con;

    count = 0;
    while((con=srcnext(vlensrc)) != NULL) {
	if(!isstringable(con->nctype)) {
	    semerror(srcline(vlensrc),
		     "Encountered non-string constant in vlen constant");
	    goto done;
        }
	count += collectstring(con,0,vlenbuf);
    }
done:
    bbFree(vlenbuf);
#endif
}

/**************************************************/

#ifdef IGNORE
static Datalist*
dividestringlist(char* s, size_t chunksize, int lineno)
{
    size_t slen,div,rem;
    Datalist* charlist;
    Constant* chars;

    if(s == NULL) s = "";
    slen = strlen(s);
    ASSERT(chunksize > 0);
    div = slen / chunksize;
    rem = slen % chunksize;
    if(rem > 0) div++;

    charlist = builddatalist(div);
    if(!charlist) return NULL;
    charlist->readonly = 0;
    charlist->length = div;
    chars=charlist->data;
    if(slen == 0) {
	/* Special case for null string */
	charlist->length = 1;
	chars->nctype = NC_STRING;
	chars->lineno = lineno;
        chars->value.stringv.len = 0;
        chars->value.stringv.stringv = nulldup("");
    } else
    {
	int i;
	for(i=0;i<(div-1);i++,chars++) {
	    chars->nctype = NC_STRING;
	    chars->lineno = lineno;
            chars->value.stringv.len = chunksize;
	    chars->value.stringv.stringv = (char*)emalloc(chunksize+1);
	    memcpy(chars->value.stringv.stringv,s,chunksize);
	    chars->value.stringv.stringv[chunksize] = '\0';
	    s += chunksize;
	}
	/* Do last chunk */
        chars->nctype = NC_STRING;
	chars->lineno = lineno;
        chars->value.stringv.len = strlen(s);
        chars->value.stringv.stringv = nulldup(s);
    }
    return charlist;
}
#endif

#ifdef IGNORE
static int
stringexplode(Datasrc* src, size_t chunksize)
{
    Constant* con;
    Datalist* charlist;

    if(!isstring(src)) return 0;
    con = srcnext(src);
    charlist = dividestringlist(con->value.stringv.stringv,chunksize,srcline(src));
    srcpushlist(src,charlist);
    return 1;
}
#endif

#ifdef IGNORE
static Datalist*
padstringlist(char* s, size_t chunksize, int lineno)
{
    size_t slen;
    Datalist* charlist;
    Constant* chars;

    if(s == NULL) s = "";
    slen = strlen(s);
    ASSERT(chunksize > 0);
    ASSERT(chunksize >= slen);

    charlist = builddatalist(1);
    if(!charlist) return NULL;
    charlist->readonly = 0;
    charlist->length = 1;
    chars=charlist->data;
    chars->nctype = NC_STRING;
    chars->lineno = lineno;
    chars->value.stringv.len = chunksize;
    chars->value.stringv.stringv = emalloc(chunksize+1);
    if(chars->value.stringv.stringv == NULL) return NULL;
    memset((void*)chars->value.stringv.stringv,0,chunksize+1);
    strcpy(chars->value.stringv.stringv,s);
    return charlist;
}
#endif

#ifdef IGNORE
static int
stringpad(Datasrc* src, size_t chunksize)
{
    Constant* con;
    Datalist* charlist;

    if(!isstring(src)) return 0;
    con = srcnext(src);
    charlist = padstringlist(con->value.stringv.stringv,chunksize,srcline(src));
    srcpushlist(src,charlist);
    return 1;
}
#endif

#ifdef IGNORE
/* Fill */
static int
fillstring(size_t declsize, int len, Bytebuffer* databuf, int fillchar)
{
    for(;len<declsize;len++)
        bbAppend(databuf,fillchar);
    return len;
}
#endif

int
getfillchar(Datalist* fillsrc)
{
    /* Determine the fill char */
    int fillchar = 0;
    if(fillsrc != NULL && fillsrc->length > 0) {
	Constant* ccon = fillsrc->data;
	if(ccon->nctype == NC_CHAR) {
	    fillchar = ccon->value.charv;
	} else if(ccon->nctype == NC_STRING) {	    
	    if(ccon->value.stringv.len > 0) {
	        fillchar = ccon->value.stringv.stringv[0];
	    }
	}
    }
    if(fillchar == 0) fillchar = NC_FILL_CHAR; /* default */
    return fillchar;
}


/*
Take constant, and if stringable, append to databuf as characters.
If the constant, as a string, is not a multiple in length of dimsize,
then pad using fillchar
*/

int
collectstring(Constant* con, size_t dimsize, Bytebuffer* databuf, int fillchar)
{
    size_t padding = 0;

    if(dimsize == 0) dimsize = 1;
    if(con == NULL) con = &fillconstant;
    switch (con->nctype) {
    case NC_STRING: {
        char* s = con->value.stringv.stringv;
        size_t slen = con->value.stringv.len;
        padding = (slen % dimsize == 0 ? 0 : dimsize - (slen % dimsize));
        if(slen > 0) bbAppendn(databuf,s,slen);
        } break;
    case NC_FILLVALUE:
        padding = dimsize;
        break;
    case NC_CHAR:
    case NC_BYTE:
    case NC_UBYTE:
        /* Append */
        bbAppend(databuf,con->value.charv);
        break;
    default:
        semerror(con->lineno,"Non string or character constant encountered");
        return 0;
    }
    while(padding-- > 0) bbAppend(databuf,fillchar);
    bbNull(databuf);
    return 1;
}

/* Given a list of stringables (e.g. strings),
   make each be a multiple in size of size.
   Then concat them all together and create
   and return a new Constant containg that string.
*/   
int
buildcanonicalcharlist(Datalist* list, size_t size, int fillchar, Constant* conp)
{
    int i;
    Bytebuffer* buf = bbNew();
    Constant* con = NULL;
    Constant newcon = nullconstant;

    if(list->length == 0) {
      /* If an empty list, then produce a string of size size using fillchar*/
      for(i=0;i<size;i++) bbAppend(buf,fillchar);
    } else for(i=0;i<list->length;i++) {
	con = list->data+i;
	if(!collectstring(con,size,buf,fillchar)) return 0;
    }
    bbNull(buf);
    /* Construct the new string constant */ 
    newcon.nctype = NC_STRING;
    newcon.lineno = (list->length == 0 ? 0: list->data[0].lineno);
    newcon.value.stringv.len = bbLength(buf);
    newcon.value.stringv.stringv = bbDup(buf);
    bbFree(buf);
    if(conp) *conp = newcon;
    return 1;
}

void
padstring(Constant* con, size_t desiredlength, int fillchar)
{
    size_t len = con->value.stringv.len;
    char* s = con->value.stringv.stringv;
    ASSERT(con->nctype == NC_STRING);
    if(len > desiredlength) {
	semerror(con->lineno,"String constant too long");
	con->value.stringv.len = desiredlength;
    } else if(len < desiredlength) {
	s = realloc(s,desiredlength+1);
	memset(s+len,fillchar,(desiredlength - len));
        s[desiredlength] = '\0';		
        con->value.stringv.stringv = s;
        con->value.stringv.len = desiredlength;
    }
}
#ifdef IGNORE
static size_t
padround(size_t udimsize, Constant* con)
{
    size_t r;
    ASSERT(con->nctype == NC_STRING);
    r = (con->value.stringv.len + (udimsize-1))/udimsize;
    r = r * udimsize;
    return r;
}
#endif
