/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/util.c,v 1.4 2010/04/14 22:04:59 dmh Exp $
 *********************************************************************/

#include "includes.h"

/* Track primitive symbol instances (initialized in ncgen.y) */
Symbol* primsymbols[PRIMNO];

/* Track all known datalist*/
static Datalist* alldatalists = NULL;

char*
append(const char* s1, const char* s2)
{
    int len = (s1?strlen(s1):0)+(s2?strlen(s2):0);
    char* result = (char*)emalloc(len+1);
    result[0] = '\0';
    if(s1) strcat(result,s1);
    if(s2) strcat(result,s2);
    return result;
}


unsigned int
chartohex(char c)
{
    switch (c) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return (c - '0');
        case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F':
            return (c - 'A') + 0x0a;
        case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f':
            return (c - 'a') + 0x0a;
    }
    return 0;
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
 * Remove trailing zeros (after decimal point) but not trailing decimal
 * point from ss, a string representation of a floating-point number that
 * might include an exponent part.
 */
void
tztrim(
    char *ss			/* returned string representing dd */
    )
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

/* Assume bytebuffer contains pointers to char**/
void
reclaimattptrs(void* buf, long count)
{
    int i;
    char** ptrs = (char**)buf;
    for(i=0;i<count;i++) {free((void*)ptrs[i]);}
}

void
freeSymbol(Symbol* sym)
{
#ifdef FIX
    switch (sym->objectclass) {
    case NG_VAR:
	reclaimconstlist(vsym->var.data);
	if(vsym->var.dims != NULL) efree(vsym->var.dims);
	break;
    case NG_ATT:
	if(asym->att.basetype == primsymbols[NC_STRING])
  	    reclaimattptrs(asym->att.data,asym->att.count);
	else
	    efree(asym->att.data);
	break;
    case NG_GRP:
    case NG_DIM:
    case NG_TYP:
    case NG_ENUM:
    case NG_ECONST:
    case NG_VLEN:
    case NG_STRUCT:
    case NG_FIELD:
    case NG_OPAQUE:
    default: break;
    }    
    efree(sym->name);
    efree(sym);
#endif
}

char* nctypenames[17] = {
"NC_NAT",
"NC_BYTE", "NC_CHAR", "NC_SHORT", "NC_INT",
"NC_FLOAT", "NC_DOUBLE",
"NC_UBYTE", "NC_USHORT", "NC_UINT",
"NC_INT64", "NC_UINT64",
"NC_STRING",
"NC_VLEN", "NC_OPAQUE", "NC_ENUM", "NC_COMPOUND"
};

char* nctypenamesextend[9] = {
"NC_GRP", "NC_DIM", "NC_VAR", "NC_ATT", "NC_TYPE",
"NC_ECONST","NC_FIELD", "NC_ARRAY","NC_PRIM"
};

char*
nctypename(nc_type nctype)
{
    char* s;
    if(nctype >= NC_NAT && nctype <= NC_COMPOUND)
	return nctypenames[nctype];
    if(nctype >= NC_GRP && nctype <= NC_PRIM)
	return nctypenamesextend[(nctype - NC_GRP)];
    if(nctype == NC_FILLVALUE) return "NC_FILL";
    s = poolalloc(128);    
    sprintf(s,"NC_<%d>",nctype);
    return s;
}

/* These are the augmented NC_ values (0 based from NC_GRP)*/
char* ncclassnames[9] = { 
"NC_GRP", "NC_DIM", "NC_VAR", "NC_ATT", 
"NC_TYP", "NC_ECONST", "NC_FIELD", "NC_ARRAY", 
"NC_PRIM"
};

char*
ncclassname(nc_class ncc)
{
    char* s;
    if(ncc >= NC_NAT && ncc <= NC_COMPOUND)
	return nctypename((nc_type)ncc);
    if(ncc == NC_FILLVALUE) return "NC_FILL";
    if(ncc >= NC_GRP && ncc <= NC_PRIM)
	return ncclassnames[ncc - NC_GRP];
    s = poolalloc(128);    
    sprintf(s,"NC_<%d>",ncc);
    return s;
}

int ncsizes[17] = {
0,
1,1,2,4,
4,8,
1,2,4,
8,8,
sizeof(char*),
sizeof(nc_vlen_t),
0,0,0
};

int
ncsize(nc_type nctype)
{
    if(nctype >= NC_NAT && nctype <= NC_COMPOUND)
	return ncsizes[nctype];
    return 0;
}

int
hasunlimited(Dimset* dimset)
{
    int i;
    for(i=0;i<dimset->ndims;i++) {
	Symbol* dim = dimset->dimsyms[i];
	if(dim->dim.declsize == NC_UNLIMITED) return 1;
    }
    return 0;
}

/* return 1 if first dimension is unlimited*/
int
isunlimited0(Dimset* dimset)
{
   return (dimset->ndims > 0 && dimset->dimsyms[0]->dim.declsize == NC_UNLIMITED);
}


/* True only if dim[0] is unlimited all rest are bounded*/
/* or all are bounded*/
int
classicunlimited(Dimset* dimset)
{
    int i;
    int last = -1;
    for(i=0;i<dimset->ndims;i++) {
	Symbol* dim = dimset->dimsyms[i];
	if(dim->dim.declsize == NC_UNLIMITED) last = i;
    }
    return (last < 1);
}

/* True only iff no dimension is unlimited*/
int
isbounded(Dimset* dimset)
{
    int i;
    for(i=0;i<dimset->ndims;i++) {
	Symbol* dim = dimset->dimsyms[i];
	if(dim->dim.declsize == NC_UNLIMITED) return 0;
    }
    return 1;
}

int
isclassicprim(nc_type nctype)
{
    return    (nctype >= NC_BYTE && nctype <= NC_DOUBLE)
	;
}

int
isclassicprimplus(nc_type nctype)
{
    return    (nctype >= NC_BYTE && nctype <= NC_DOUBLE)
	   || (nctype == NC_STRING)
	;
}

int
isprim(nc_type nctype)
{
    return    (nctype >= NC_BYTE && nctype <= NC_STRING)
	;
}

int
isprimplus(nc_type nctype)
{
    return    (nctype >= NC_BYTE && nctype <= NC_STRING)
	   || (nctype == NC_ECONST)
	   || (nctype == NC_OPAQUE)
	 ;
}

void
collectpath(Symbol* grp, List* grpstack)
{
    while(grp != NULL) {
        listpush(grpstack,(elem_t)grp);
	grp = grp->container;
    }
}


#ifdef USE_NETCDF4
/* Result is pool'd*/
char*
prefixtostring(List* prefix, char* separator)
{
    int slen=0;
    int plen;
    int i;
    char* result;
    if(prefix == NULL) return pooldup("");
    plen = prefixlen(prefix);
    if(plen == 0) { /* root prefix*/
	slen=0;
        /* slen += strlen(separator);*/
        slen++; /* for null terminator*/
        result = poolalloc(slen);
        result[0] = '\0';
	/*strcat(result,separator);*/
    } else {
        for(i=0;i<plen;i++) {
	    Symbol* sym = (Symbol*)listget(prefix,i);
            slen += (strlen(separator)+strlen(sym->name));
	}
        slen++; /* for null terminator*/
        result = poolalloc(slen);
        result[0] = '\0';
        for(i=0;i<plen;i++) {
	    Symbol* sym = (Symbol*)listget(prefix,i);
            strcat(result,separator);
	    strcat(result,sym->name); /* append "/<prefix[i]>"*/
	}
    }    
    return result;
}
#endif

/* Result is pool'd*/
char*
fullname(Symbol* sym)
{
#ifdef USE_NETCDF4    
    char* s1;
    char* result;
    char* prefix;
    prefix = prefixtostring(sym->prefix,PATHSEPARATOR);
    s1 = poolcat(prefix,PATHSEPARATOR);
    result = poolcat(s1,sym->name);
    return result;
#else
    return nulldup(sym->name);
#endif
}

int
prefixeq(List* x1, List* x2)
{
    Symbol** l1;
    Symbol** l2;    
    int len,i;
    if((len=listlength(x1)) != listlength(x2)) return 0;
    l1=(Symbol**)listcontents(x1);
    l2=(Symbol**)listcontents(x2);
    for(i=0;i<len;i++) {
        if(strcmp(l1[i]->name,l2[i]->name) != 0) return 0;
    }
    return 1;
}

List*
prefixdup(List* prefix)
{
    List* dupseq;
    int i;
    if(prefix == NULL) return listnew();
    dupseq = listnew();
    listsetalloc(dupseq,listlength(prefix));
    for(i=0;i<listlength(prefix);i++) listpush(dupseq,listget(prefix,i));
    return dupseq;    
}

/*
Many of the generate routines need to construct
heap strings for short periods. Remembering to
free such space is error prone, so provide a
pseudo-GC to handle these short term requests.
The idea is to have a fixed size pool
tracking malloc requests and automatically
releasing when the pool gets full.
*/

/* Max number of allocated pool items*/
#define POOLMAX 100

static char* pool[POOLMAX];
static int poolindex = -1;
#define POOL_DEFAULT 256

char*
poolalloc(size_t length)
{
    if(poolindex == -1) { /* initialize*/
	memset((void*)pool,0,sizeof(pool));
	poolindex = 0;
    }
    if(poolindex == POOLMAX) poolindex=0;
    if(length == 0) length = POOL_DEFAULT;
    if(pool[poolindex] != NULL) efree(pool[poolindex]);
    pool[poolindex] = (char*)emalloc(length);
    return pool[poolindex++];
}

char*
pooldup(char* s)
{
    char* sdup = poolalloc(strlen(s)+1);
    strcpy(sdup,s);
    return sdup;
}

char*
poolcat(const char* s1, const char* s2)
{
    int len1, len2;
    char* cat;
    if(s1 == NULL && s2 == NULL) return NULL;
    len1 = (s1?strlen(s1):0);
    len2 = (s2?strlen(s2):0);
    cat = poolalloc(len1+len2+1);
    cat[0] = '\0';
    if(s1 != NULL) strcat(cat,s1);
    if(s2 != NULL) strcat(cat,s2);
    return cat;
}

/* Result is malloc'd*/
unsigned char*
makebytestring(char* s, size_t* lenp)
{
    unsigned char* bytes;
    unsigned char* b;
    size_t slen = strlen(s);
    size_t blen = slen/2;
    int i;

    ASSERT((slen%2) == 0);
    ASSERT(blen > 0);
    bytes = (unsigned char*)emalloc(blen);
    b = bytes;
    for(i=0;i<slen;i+=2) {
	unsigned int digit1 = chartohex(*s++);
	unsigned int digit2 = chartohex(*s++);
	unsigned int byte = (digit1 << 4) | digit2;
	*b++ = byte;				
    }
    if(lenp) *lenp = blen;
    return bytes;
}

int
getpadding(int offset, int alignment)
{
    int rem = (alignment==0?0:(offset % alignment));
    int pad = (rem==0?0:(alignment - rem));
    return pad;
}



void
dlextend(Datalist* dl)
{
    size_t newalloc;
    newalloc = (dl->alloc > 0?2*dl->alloc:1);
    dlsetalloc(dl,newalloc);
}

void
dlsetalloc(Datalist* dl, size_t newalloc)
{
    Constant* newdata;
    if(newalloc <= 0) newalloc = 1;
    if(dl->alloc > 0)
        newdata = (Constant*)erealloc((void*)dl->data,sizeof(Constant)*newalloc);
    else {
        newdata = (Constant*)emalloc(sizeof(Constant)*newalloc);
        memset((void*)newdata,0,sizeof(Constant)*newalloc);
    }
    dl->alloc = newalloc;
    dl->data = newdata;
}

#define DATALISTINIT 256

Datalist*
builddatalist(int initial)
{
    Datalist* ci;
    if(initial <= 0) initial = DATALISTINIT;
    initial++; /* for header*/
    ci = (Datalist*)emalloc(sizeof(Datalist));
    memset((void*)ci,0,sizeof(Datalist)); /* only clear the hdr*/
    ci->data = (Constant*)emalloc(sizeof(Constant)*initial);
    memset((void*)ci->data,0,sizeof(Constant)*initial);
    ci->alloc = initial;
    ci->length = 0;
    return ci;
}

void
dlappend(Datalist* dl, Constant* constant)
{
    if(dl->length >= dl->alloc) dlextend(dl);
    if(constant == NULL) constant = &nullconstant;
    dl->data[dl->length++] = *constant;
}

Constant
builddatasublist(Datalist* dl)
{
    Constant d;
    d.nctype = NC_COMPOUND;
    d.lineno = (dl->length > 0?dl->data[0].lineno:0);
    d.value.compoundv = dl;
    return d;
}

static void
constantFree(Constant* con)
{
    switch(con->nctype) {
    case NC_COMPOUND:
	/* do nothing; ReclaimDatalists below will take care of the datalist	*/
	break;	
    case NC_STRING:
	if(con->value.stringv.len > 0 && con->value.stringv.stringv != NULL)
	    efree(con->value.stringv.stringv);
	break;
    case NC_OPAQUE:
	if(con->value.opaquev.len > 0 && con->value.opaquev.stringv != NULL)
	    efree(con->value.opaquev.stringv);
	break;
    default:
	break;
    }
}

static void
reclaimDatalists(void)
{
    Datalist* list;
    Constant* con;
    /* Step 1: free up the constant content of each datalist*/
    for(list=alldatalists;list != NULL; list = list->next) {
	if(list->data != NULL) { /* avoid multiple free attempts*/
	    int i;
	    for(i=0,con=list->data;i<list->length;i++,con++)
	        constantFree(con);
	    list->data = NULL;
	}	
    }
    /* Step 2: free up the datalist itself*/
    for(list=alldatalists;list != NULL;) {
	Datalist* current = list;
	list = list->next;
	efree(current);
    }
}

static void
reclaimSymbols(void)
{
    Symbol* sym;
    for(sym=symlist;sym;) {
	Symbol* next = sym->next;
        freeSymbol(sym);
	sym = next;
    }
}

void
cleanup()
{
  reclaimDatalists();
  reclaimSymbols();
}

size_t
arraylength(Dimset* dimset)
{
    return subarraylength(dimset,0);
}

/* compute the total n-dimensional size as 1 long array*/
/* stop if we encounter an unlimited dimension */
size_t
subarraylength(Dimset* dimset, int first)
{
    size_t totalsize = 1;
    int i,last;
    last = dimset->ndims;
    for(i=first;i<last;i++) {
	if(dimset->dimsyms[i]->dim.declsize == NC_UNLIMITED) break;
	totalsize = totalsize * MAX(dimset->dimsyms[i]->dim.unlimitedsize,
				     dimset->dimsyms[i]->dim.declsize);
    }
    return totalsize;    
}


/* Do the "complement" of subarray length;
   compute the total n-dimensional size of an array
   starting at 0 thru the 'last' array index.
   stop if we encounter an unlimited dimension
*/
size_t
prefixarraylength(Dimset* dimset, int last)
{
    size_t totalsize = 1;
    int i;
    for(i=0;i<=last;i++) {
	if(dimset->dimsyms[i]->dim.declsize == NC_UNLIMITED) break;
	totalsize = totalsize * MAX(dimset->dimsyms[i]->dim.unlimitedsize,
				     dimset->dimsyms[i]->dim.declsize);
    }
    return totalsize;    
}



#ifdef USE_NETCDF4
extern int H5Eprint1(FILE * stream);
#endif   

void
check_err(const int stat, const int line, const char* file) {
    if (stat != NC_NOERR) {
	fprintf(stderr, "ncgen: %s\n", nc_strerror(stat));
	fprintf(stderr, "\t(%s:%d)\n", file,line);
#ifdef USE_NETCDF4
	H5Eprint1(stderr);
#endif   
	fflush(stderr);
	exit(1);
    }
}

