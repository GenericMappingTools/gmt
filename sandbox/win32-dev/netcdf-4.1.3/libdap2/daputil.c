/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/daputil.c,v 1.47 2010/05/21 23:24:15 dmh Exp $
 *********************************************************************/

#include "config.h"

#include <sys/time.h>

#include "oc.h"
extern int oc_dumpnode(OClink, OCobject);

#include "ncdap3.h"
#include "dapalign.h"
#include "dapodom.h"

#define LBRACKET '['
#define RBRACKET ']'

/**************************************************/
/* Provide a hidden interface to allow utilities*/
/* to check if a given path name is really an ncdap3 url.*/
/* If no, return null, else return basename of the url*/
/* minus any extension*/
int
nc__testurl(const char* path, char** basenamep)
{
    OCURI* uri;
    int ok = ocuriparse(path,&uri);
    if(ok) {
	char* slash = strrchr(uri->file, '/');
	char* dot;
	if(slash == NULL) slash = (char*)path; else slash++;
        slash = nulldup(slash);
	dot = strrchr(slash, '.');
        if(dot != NULL &&  dot != slash) *dot = '\0';
	if(basenamep) *basenamep=slash ; else free(slash);
        ocurifree(uri);
    }
    return ok;
}

/**************************************************/

#ifdef UNUSED
static char cvtchars1[] = "0123456789 !#$%&'()*,:;<=>?[\\]^`{|}~\"\\";

static char hexchars[16] = {
'0', '1', '2', '3',
'4', '5', '6', '7',
'8', '9', 'a', 'b',
'c', 'd', 'e', 'f',
};
#endif

/*
Given a legal dap name with arbitrary characters,
convert to equivalent legal cdf name
With the new name policy for netcdf, this procedure
does nothing.
*/

char*
cdflegalname3(char* dapname)
{
#ifndef IGNORE
    return nulldup(dapname);
#else
    int c;
    char* newname;
    char* cvtchars;
    NCbytes* buf;
    if(dapname == NULL) return NULL;
    buf = ncbytesnew();
    cvtchars = cvtchars1;
    while((c=*dapname++)) {
	if(c < 127 && strchr(cvtchars,c) != NULL) {
	    ncbytesappend(buf,'\\');
	    ncbytesappend(buf,c);
	} else if(c < ' ' || c >= 127) {/* non-printable */
	    char tmp[8];
	    int hex1, hex2;
	    hex1 = (c & 0x0F);
	    hex2 = (c & 0xF0) >> 4;
	    tmp[0] = '\\';
	    tmp[1] = 'x';
	    tmp[2] = hexchars[hex2];
            tmp[3] = hexchars[hex1];
	    tmp[4] = '\0';	    
	    ncbytescat(buf,tmp);
	} else
	    ncbytesappend(buf,c);
        cvtchars = cvtcharsn; /* for non-first tests*/
    }
    newname = ncbytesdup(buf);
    ncbytesfree(buf);
    return newname;
#endif
}

#ifdef IGNORE
/* Convert a string, s0, to replace some characters with %XX */
char*
urlescape(char* s0)
{
    int c;
    unsigned int slen;
    char* newname;
    char* p;
    char* q;
    static char urlescapes[] = " %&/:;,=?@'\"<>{}|\\^[]`";


    if(s0 == NULL) return NULL;
    slen = strlen(s0);
    newname = (char*)emalloc(1+(slen*3)); /* if every char goes to %XX */
    p = s0;
    q = newname;
    while((c=*p++)) {
	if(c < ' ' || c >= 127 || strchr(urlescapes,c) != NULL) {
	    int hex1, hex2;
	    hex1 = (c & 0x0F);
	    hex2 = (c & 0xF0) >> 4;
	    *q++ = '%';
	    *q++ = hexchars[hex2];
            *q++ = hexchars[hex1];
	} else
	    *q++ = c;
    }
    *q = '\0';
    return newname;
}
#endif

/* Define the type conversion of the DAP variables
   to the external netCDF variable type.
   The proper way is to, for example, convert unsigned short
   to an int to maintain the values.
   Unfortuneately, libnc-dap does not do this:
   it translates the types directly. For example
   libnc-dap upgrades the DAP byte type, which is unsigned char,
   to NC_BYTE, which signed char.
   Oh well.
   For netcdf-4, we can do proper type conversion.
*/
nc_type
nctypeconvert(NCDAPCOMMON* drno, nc_type nctype)
{
    nc_type upgrade = NC_NAT;
    if(drno->controls.flags & NCF_NC3) {
	/* libnc-dap mimic invariant is to maintain type size */
	switch (nctype) {
	case NC_CHAR:    upgrade = NC_CHAR; break;
	case NC_BYTE:    upgrade = NC_BYTE; break;
	case NC_UBYTE:   upgrade = NC_BYTE; break;
	case NC_SHORT:   upgrade = NC_SHORT; break;
	case NC_USHORT:  upgrade = NC_SHORT; break;
	case NC_INT:     upgrade = NC_INT; break;
	case NC_UINT:    upgrade = NC_INT; break;
	case NC_INT64:   upgrade = NC_INT64; break;
	case NC_UINT64:  upgrade = NC_UINT64; break;
	case NC_FLOAT:   upgrade = NC_FLOAT; break;
	case NC_DOUBLE:  upgrade = NC_DOUBLE; break;
	case NC_URL:
	case NC_STRING:  upgrade = NC_CHAR; break;
	default: break;
	}
    } else if(drno->controls.flags & NCF_NC4) {
	/* netcdf-4 conversion is more correct */
	switch (nctype) {
	case NC_CHAR:    upgrade = NC_CHAR; break;
	case NC_BYTE:    upgrade = NC_BYTE; break;
	case NC_UBYTE:   upgrade = NC_UBYTE; break;
	case NC_SHORT:   upgrade = NC_SHORT; break;
	case NC_USHORT:  upgrade = NC_USHORT; break;
	case NC_INT:     upgrade = NC_INT; break;
	case NC_UINT:    upgrade = NC_UINT; break;
	case NC_INT64:   upgrade = NC_INT64; break;
	case NC_UINT64:  upgrade = NC_UINT64; break;
	case NC_FLOAT:   upgrade = NC_FLOAT; break;
	case NC_DOUBLE:  upgrade = NC_DOUBLE; break;
	case NC_URL:
	case NC_STRING:  upgrade = NC_STRING; break;
	default: break;
	}
    }
    return upgrade;
}

nc_type
octypetonc(OCtype etype)
{
    switch (etype) {
    case OC_Char:	return NC_CHAR;
    case OC_Byte:	return NC_UBYTE;
    case OC_UByte:	return NC_UBYTE;
    case OC_Int16:	return NC_SHORT;
    case OC_UInt16:	return NC_USHORT;
    case OC_Int32:	return NC_INT;
    case OC_UInt32:	return NC_UINT;
    case OC_Int64:	return NC_INT64;
    case OC_UInt64:	return NC_UINT64;
    case OC_Float32:	return NC_FLOAT;
    case OC_Float64:	return NC_DOUBLE;
    case OC_String:	return NC_STRING;
    case OC_URL:	return NC_STRING;
    case OC_Dataset:	return NC_Dataset;
    case OC_Sequence:	return NC_Sequence;
    case OC_Structure:	return NC_Structure;
    case OC_Grid:	return NC_Grid;
    case OC_Dimension:	return NC_Dimension;
    case OC_Primitive:	return NC_Primitive;
    default: break;
    }
    return NC_NAT;
}

OCtype
nctypetodap(nc_type nctype)
{
    switch (nctype) {
    case NC_CHAR:	return OC_Char;
    case NC_BYTE:	return OC_Byte;
    case NC_UBYTE:	return OC_UByte;
    case NC_SHORT:	return OC_Int16;
    case NC_USHORT:	return OC_UInt16;
    case NC_INT:	return OC_Int32;
    case NC_UINT:	return OC_UInt32;
    case NC_INT64:	return OC_Int64;
    case NC_UINT64:	return OC_UInt64;
    case NC_FLOAT:	return OC_Float32;
    case NC_DOUBLE:	return OC_Float64;
    case NC_STRING:	return OC_String;
    default : break;
    }
    return OC_NAT;
}

size_t
nctypesizeof(nc_type nctype)
{
    switch (nctype) {
    case NC_CHAR:	return sizeof(char);
    case NC_BYTE:	return sizeof(signed char);
    case NC_UBYTE:	return sizeof(unsigned char);
    case NC_SHORT:	return sizeof(short);
    case NC_USHORT:	return sizeof(unsigned short);
    case NC_INT:	return sizeof(int);
    case NC_UINT:	return sizeof(unsigned int);
    case NC_INT64:	return sizeof(long long);
    case NC_UINT64:	return sizeof(unsigned long long);
    case NC_FLOAT:	return sizeof(float);
    case NC_DOUBLE:	return sizeof(double);
    case NC_STRING:	return sizeof(char*);
    default: PANIC("nctypesizeof");
    }
    return 0;
}

char*
nctypetostring(nc_type nctype)
{
    switch (nctype) {
    case NC_NAT:	return "NC_NAT";
    case NC_BYTE:	return "NC_BYTE";
    case NC_CHAR:	return "NC_CHAR";
    case NC_SHORT:	return "NC_SHORT";
    case NC_INT:	return "NC_INT";
    case NC_FLOAT:	return "NC_FLOAT";
    case NC_DOUBLE:	return "NC_DOUBLE";
    case NC_UBYTE:	return "NC_UBYTE";
    case NC_USHORT:	return "NC_USHORT";
    case NC_UINT:	return "NC_UINT";
    case NC_INT64:	return "NC_INT64";
    case NC_UINT64:	return "NC_UINT64";
    case NC_STRING:	return "NC_STRING";
    case NC_VLEN:	return "NC_VLEN";
    case NC_OPAQUE:	return "NC_OPAQUE";
    case NC_ENUM:	return "NC_ENUM";
    case NC_COMPOUND:	return "NC_COMPOUND";
    case NC_URL:	return "NC_URL";
    case NC_SET:	return "NC_SET";
    case NC_Dataset:	return "NC_Dataset";
    case NC_Sequence:	return "NC_Sequence";
    case NC_Structure:	return "NC_Structure";
    case NC_Grid:	return "NC_Grid";
    case NC_Dimension:	return "NC_Dimension";
    case NC_Primitive:	return "NC_Primitive";
    default: break;
    }
    return NULL;
}

#ifdef IGNORE
/* 
Assuming node is in the dds or datadds space,
move to the corresponding node in dds0 space
(guaranteed to exist) and collect the set of
the node plus all container nodesin depth first order.
*/
void
collectnode0path3(CDFnode* node, NClist* path, int withdataset)
{
    /* Move to dds0 space */
    if(node->attachment0 == NULL && node->attachment != NULL)
	node = node->attachment;
    if(node->attachment0 != NULL)
	node = node->attachment0;

    collectnodepath3(node,path,withdataset);
}
#endif

/* Collect the set of container nodes ending in "container"*/
void
collectnodepath3(CDFnode* node, NClist* path, int withdataset)
{
    if(node == NULL) return;
    nclistpush(path,(ncelem)node);
    while(node->container != NULL) {
	node = node->container;
	if(!withdataset && node->nctype == NC_Dataset) break;
	nclistinsert(path,0,(ncelem)node);
    }
}



#ifdef IGNORE
/* Compute the 1+deepest occurrence of a sequence in the path*/
int
dividepoint(NClist* path)
{
    /* find divide point*/
    int i,len = nclistlength(path);
    int divide = 0; /* to indicate not found*/
    for(i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	if(node->nctype == NC_Sequence) divide = i+1;
    }
    return divide;
}

/* Divide the set into two parts, those before and including the*/
/* innermost sequence and those below that point*/
void
dividepath(NClist* path, NClist* prefix)
{
    int i,divide;
    divide = dividepoint(path);
    if(divide > 0) { /* move the prefix part if divide >= 0*/
	for(i=0;i<=divide;i++) {
	    ncelem node = nclistget(path,0);
	    nclistpush(prefix,node);
	    nclistremove(path,0);
	}
    }
}
#endif

/* Pad a buffer */
int
alignbuffer3(NCbytes* buf, int alignment)
{
    int pad;
    unsigned long len;
    if(buf == NULL) return 0;
    len = ncbyteslength(buf);
    pad = nccpadding(len,alignment);

#ifdef TEST
    for(;pad > 0;pad--)
        ncbytesappend(buf,0x3a); /* 0x3a was chosen at random */
#else
    ncbytessetlength(buf,len+pad);
#endif
    return 1;
}

size_t
dimproduct3(NClist* dimensions)
{
    size_t size = 1;
    unsigned int i;
    if(dimensions == NULL) return size;
    for(i=0;i<nclistlength(dimensions);i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimensions,i);
	size *= dim->dim.declsize;
    }
    return size;
}

static char* checkseps = "+,:;";

/* Search for substring in value of param. If substring == NULL; then just
   check if param is defined.
*/
int
paramcheck34(NCDAPCOMMON* nccomm, const char* param, const char* substring)
{
    const char* value;
    const char* sh;
    unsigned int splen;
    if(nccomm == NULL || param == NULL) return 0;
    value = oc_clientparam_get(nccomm->oc.conn,param);
    if(value == NULL) return 0;
    if(substring == NULL) return 1;
    splen = strlen(substring);
    for(sh=value;*sh;sh++) {
	if(strncmp(sh,substring,splen)==0) {
	    char cpost = sh[splen];
	    char cpre;
	    int match = 0;
	    cpre = (sh==value?*sh:*(sh-1));
	    /* Check for legal separators */
	    if(sh == value || strchr(checkseps,cpre) != NULL) match++;
	    if(cpost == '\0' || strchr(checkseps,cpost) != NULL) match++;
	    if(match == 2) return 1;
	}
    }
    return 0;
}


/* This is NOT UNION */ 
int
nclistconcat(NClist* l1, NClist* l2)
{
    unsigned int i;
    for(i=0;i<nclistlength(l2);i++) nclistpush(l1,nclistget(l2,i));
    return 1;
}

int
nclistminus(NClist* l1, NClist* l2)
{
    unsigned int i,len,found;
    len = nclistlength(l2);
    found = 0;
    for(i=0;i<len;i++) {
	if(nclistdeleteall(l1,nclistget(l2,i))) found = 1;
    }
    return found;
}

int
nclistdeleteall(NClist* l, ncelem elem)
{
    int i; /* do not make unsigned */
    unsigned int len,found;
    found = 0;
    len = nclistlength(l);
    for(i=len-1;i>=0;i--) {
	ncelem test = nclistget(l,i);
	if(test==elem) {
	    nclistremove(l,i);
            found=1;
        }
    }
    return found;    
}

/* Convert a path to a name string; elide the initial Dataset node*/
/* and elide any node marked as elided.*/
char*
makecdfpathstring3(CDFnode* var, const char* separator)
{
    int slen,i,len,first;
    char* pathname;
    NClist* path = nclistnew();

    collectnodepath3(var,path,WITHDATASET);
    len = nclistlength(path);
    assert(len > 0); /* dataset at least */
    if(len == 1) {pathname = nulldup(""); goto done;} /* Dataset */
    for(slen=0,i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	if(node->nctype == NC_Dataset) continue;
	slen += strlen(node->ncbasename?node->ncbasename:node->name);
    }
    slen += ((len-2)); /* for 1-char separators */
    slen += 1;   /* for null terminator*/
    pathname = (char*)malloc(slen);
    MEMCHECK(pathname,NULL);
    pathname[0] = '\0';    
    for(first=1,i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	char* name = (node->ncbasename?node->ncbasename:node->name);
	if(node->nctype == NC_Dataset) continue;
	if(node->elided) continue;
	if(!first) strcat(pathname,separator);
        strcat(pathname,name);
	first = 0;
    }
done:
    nclistfree(path);
    return pathname;
}

/* Like makecdfpathstring, but using node->name. */
char*
makesimplepathstring3(CDFnode* var)
{
    int slen,i,len,first;
    char* pathname;
    NClist* path = nclistnew();

    collectnodepath3(var,path,!WITHDATASET);
    len = nclistlength(path);
    if(len == 0) {pathname = nulldup(""); goto done;} /* Dataset only */
    for(slen=0,i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	slen += (node->name?strlen(node->name):0);
    }
    slen += (len-1); /* for 1-char separators */
    slen += 1;   /* for null terminator*/
    pathname = (char*)malloc(slen);
    MEMCHECK(pathname,NULL);
    pathname[0] = '\0';    
    for(first=1,i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	char* name = node->name;
	if(!first) strcat(pathname,".");
        strcat(pathname,name?name:"null");
	first = 0;
    }
done:
    nclistfree(path);
    return pathname;
}

char*
makeocpathstring3(OCconnection conn, OCobject var, const char* separator)
{
    char* pathname = NULL;
    NClist* path = nclistnew();
    size_t slen;
    unsigned long len;
    char* name;
    unsigned int i,first;

    if(var == OCNULL) return NULL;
    collectocpath(conn,var,path);
    len = nclistlength(path);
    assert(len > 0); /* var at least */
    for(slen=0,i=0;i<len;i++) {
	OCobject o = (OCobject)nclistget(path,i);
	oc_inq_name(conn,o,&name);
	if(name == NULL) name = nulldup("");
	slen += strlen(name);
	nullfree(name);
    }
    slen += ((len-1)); /* for 1-char separators */
    slen += 1;   /* for null terminator*/
    pathname = (char*)malloc(slen);
    MEMCHECK(pathname,NULL);
    pathname[0] = '\0';    
    for(first=1,i=0;i<len;i++) {
	OCobject o = (OCobject)nclistget(path,i);
	oc_inq_name(conn,o,&name);
	if(name == NULL) name = nulldup("");
	if(!first) strcat(pathname,separator);
        strcat(pathname,name);
	nullfree(name);
	first = 0;
    }

    nclistfree(path);
    return pathname;
}

/* Collect parent ocnodes of node, including node */
NCerror
collectocpath(OCconnection conn, OCobject node, NClist* path)
{
    OCobject container;
    OCtype octype;
    if(node == OCNULL) return NC_NOERR;
    oc_inq_class(conn,node,&octype);
    if(octype == OC_Dataset) return NC_NOERR;
    oc_inq_container(conn,node,&container);
    if(container != OCNULL)
        collectocpath(conn,container,path);
    nclistpush(path,(ncelem)node);
    return NC_NOERR;
}

/* Collect the set names of container nodes ending in "container"*/
void
clonenodenamepath3(CDFnode* node, NClist* path, int withdataset)
{
    if(node == NULL) return;
    /* stop at the dataset container as well*/
    if(node->nctype != NC_Dataset)
        clonenodenamepath3(node->container,path,withdataset);
    if(node->nctype != NC_Dataset || withdataset)
        nclistpush(path,(ncelem)nulldup(node->name));
}

char*
simplepathstring3(NClist* names,  char* separator)
{
    int i;
    size_t len;
    char* result;
    if(nclistlength(names) == 0) return nulldup("");
    for(len=0,i=0;i<nclistlength(names);i++) {
	char* name = (char*)nclistget(names,i);
	len += strlen(name);
	len += strlen(separator);
    }
    len++; /* null terminator */
    result = (char*)malloc(len);
    result[0] = '\0';
    for(i=0;i<nclistlength(names);i++) {
	char* segment = (char*)nclistget(names,i);
	if(i > 0) strcat(result,separator);
	strcat(result,segment);
    }
    return result;
}

#ifdef IGNORE
/* DO NOT FREE RESULT STRING */
char*
getvaraprint(void* arg)
{
    int i;
    static NCbytes* line = NULL;    
    char tmp[64];
    Getvara* gv;

    if(line == NULL) line = ncbytesnew();
    gv = (Getvara*)arg;
    ncbytescat(line,gv->target->name);
    if(gv->walk != NULL) {
        for(i=0;i<nclistlength(gv->walk->segments);i++) {
	    NCsegment* segment = (NCsegment*)nclistget(gv->walk->segments,i);
            ncbytescat(line,segment->segment);
	    if(segment->slicerank == 0)
	        ncbytescat(line,"[]");
	    else {
	        sprintf(tmp,"[%lu:%lu:%lu]",
		    (unsigned long)segment->slices[i].first,
		    (unsigned long)segment->slices[i].stride,
		    (unsigned long)segment->slices[i].length);
	        ncbytescat(line,tmp);
	    }
	}
    }
    return ncbytescontents(line);
}
#endif

/* Define a number of location tests */

/* Is node contained (transitively) in a sequence ? */
BOOL
dapinsequence(CDFnode* node)
{
    if(node == NULL || node->container == NULL) return TRUE;
    for(node=node->container;node->nctype != NC_Dataset;node=node->container) {
       if(node->nctype == NC_Sequence) return TRUE;
    }
    return FALSE;
}

/* Is node a map field of a grid? */
BOOL
dapgridmap(CDFnode* node)
{
    if(node != NULL && node->container != NULL
       && node->container->nctype == NC_Grid) {
	CDFnode* array = (CDFnode*)nclistget(node->container->subnodes,0);
	return (node != array);
    }
    return FALSE;
}

/* Is node an array field of a grid? */
BOOL
dapgridarray(CDFnode* node)
{
    if(node != NULL && node->container != NULL
       && node->container->nctype == NC_Grid) {
	CDFnode* array = (CDFnode*)nclistget(node->container->subnodes,0);
	return (node == array);
    }
    return FALSE;
}

BOOL
dapgridelement(CDFnode* node)
{
    return dapgridarray(node)
           || dapgridmap(node);
}

/* Is node a top-level grid node? */
BOOL
daptopgrid(CDFnode* grid)
{
    if(grid == NULL || grid->nctype != NC_Grid) return FALSE;
    return daptoplevel(grid);
}

/* Is node a top-level sequence node? */
BOOL
daptopseq(CDFnode* seq)
{
    if(seq == NULL || seq->nctype != NC_Sequence) return FALSE;
    return daptoplevel(seq);
}

/* Is node a top-level node? */
BOOL
daptoplevel(CDFnode* node)
{
    if(node->container == NULL 
       || node->container->nctype != NC_Dataset) return FALSE;
    return TRUE;
}

#ifdef IGNORE
/*
Client parameters are assumed to be
one or more instances of bracketed pairs:
e.g "[...][...]...".
The bracket content in turn is assumed to be a
comma separated list of <name>=<value> pairs.
e.g. x=y,z=,a=b.
If the same parameter is specifed more than once,
then the first occurrence is used; this is so that
is possible to forcibly override user specified
parameters by prefixing.
IMPORTANT: client parameter string is assumed to
have blanks compress out.
*/

NClist*
dapparamdecode(char* params0)
{
    char* cp;
    char* cq;
    int c;
    int i;
    int nparams;
    NClist* plist = nclistnew();
    char* params;
    char* params1;

    if(params0 == NULL) return plist;

    /* Kill the leading "[" and trailing "]" */
    if(params0[0] == '[')
      params = nulldup(params0+1);
    else
      params = nulldup(params0);

    params[strlen(params)-1] = '\0';

    params1 = nulldup(params);

    /* Pass 1 to replace "][" pairs with ','*/
    cp=params; cq = params1;
    while((c=*cp++)) {
	if(c == RBRACKET && *cp == LBRACKET) {cp++; c = ',';}
	*cq++ = c;
    }
    *cq = '\0';
    free(params);
    params = params1;

    /* Pass 2 to break string into pieces and count # of pairs */
    nparams=0;
    for(cp=params;(c=*cp);cp++) {
	if(c == ',') {*cp = '\0'; nparams++;}
    }
    nparams++; /* for last one */

    /* Pass 3 to break up each pass into a (name,value) pair*/
    /* and insert into the param list */
    /* parameters of the form name name= are converted to name=""*/
    cp = params;
    for(i=0;i<nparams;i++) {
	char* next = cp+strlen(cp)+1; /* save ptr to next pair*/
	char* vp;
	/*break up the ith param*/
	vp = strchr(cp,'=');
	if(vp != NULL) {*vp = '\0'; vp++;} else {vp = "";}
	if(!nclistcontains(plist,(ncelem)cp)) {
   	    nclistpush(plist,(ncelem)nulldup(cp));
	    nclistpush(plist,(ncelem)nulldup(vp));
	}
	cp = next;
    }
    free(params);
    return plist;
}

const char*
dapparamlookup(NClist* params, const char* clientparam)
{
    int i;
    if(params == NULL || clientparam == NULL) return NULL;
    for(i=0;i<nclistlength(params);i+=2) {
	char* name = (char*)nclistget(params,i);
	if(strcmp(clientparam,name)==0)
	    return (char*)nclistget(params,i+1);
    }
    return NULL;
}

void
dapparamfree(NClist* params)
{
    int i;
    if(params == NULL) return;
    for(i=0;i<nclistlength(params);i++) {
	nullfree((void*)nclistget(params,i));
    }
    nclistfree(params);
}
#endif

unsigned int
modeldecode(int translation, const char* smodel,
            const struct NCTMODEL* models,
            unsigned int dfalt)
{
    for(;models->translation;models++) {
	if(translation != models->translation) continue;
	if(smodel == models->model
	   || (models->model != NULL && strcasecmp(smodel,models->model)==0)) {
	    /* We have a match */
            return models->flags;
	}
    }
    return dfalt;
}

unsigned long
getlimitnumber(const char* limit)
{
    size_t slen;
    unsigned long multiplier = 1;
    unsigned long lu;

    if(limit == NULL) return 0;
    slen = strlen(limit);
    if(slen == 0) return 0;
    switch (limit[slen-1]) {
    case 'G': case 'g': multiplier = GIGBYTE; break;
    case 'M': case 'm': multiplier = MEGBYTE; break;
    case 'K': case 'k': multiplier = KILBYTE; break;
    default: break;
    }
    sscanf(limit,"%lu",&lu);
    return (lu*multiplier);
}

void
dapexpandescapes(char *termstring)
{
    char *s, *t, *endp;
    
    /* expand "\" escapes, e.g. "\t" to tab character  */
    s = termstring;
    t = termstring;
    while(*t) {
	if (*t == '\\') {
	    t++;
	    switch (*t) {
	      case 'a':
		*s++ = '\007'; t++; /* will use '\a' when STDC */
		break;
	      case 'b':
		*s++ = '\b'; t++;
		break;
	      case 'f':
		*s++ = '\f'; t++;
		break;
	      case 'n':
		*s++ = '\n'; t++;
		break;
	      case 'r':
		*s++ = '\r'; t++;
		break;
	      case 't':
		*s++ = '\t'; t++;
		break;
	      case 'v':
		*s++ = '\v'; t++;
		break;
	      case '\\':
		*s++ = '\\'; t++;
		break;
	      case '?':
		*s++ = '\177'; t++;
		break;
	      case 'x':
		t++; /* now t points to one or more hex digits */
		*s++ = (char) strtol(t, &endp, 16);
		t = endp;
		break;
	      case '0':
	      case '1':
	      case '2':
	      case '3':
	      case '4':
	      case '5':
	      case '6':
	      case '7': {
		/* t should now point to 3 octal digits */
		int c;
		c = t[0];
		if(c == 0 || c < '0' || c > '7') goto normal;
		c = t[1];
		if(c == 0 || c < '0' || c > '7') goto normal;
		c = t[2];
		if(c == 0 || c < '0' || c > '7') goto normal;
		c = ((t[0]-'0')<<6)+((t[1]-'0')<<3)+(t[2]-'0');
		*s++ = (char)c;
		t += 3;
		} break;
	      default:
		if(*t == 0)
		    *s++ = '\\';
		else
		    *s++ = *t++;
		break;
	    }
	} else {
normal:	    *s++ = *t++;
	}
    }
    *s = '\0';
    return;
}

#ifdef HAVE_GETTIMEOFDAY
static struct timeval time0;
static struct timeval time1;

static double
deltatime()
{
    double t0, t1;
    t0 = ((double)time0.tv_sec);
    t0 += ((double)time0.tv_usec) / 1000000.0;
    t1 = ((double)time1.tv_sec);
    t1 += ((double)time1.tv_usec) / 1000000.0;
    return (t1 - t0);
}
#endif

/* Provide a wrapper for oc_fetch so we can log what it does */
OCerror
dap_oc_fetch(NCDAPCOMMON* nccomm, OCconnection conn, const char* ce,
             OCdxd dxd, OCobject* rootp)
{
    OCerror ocstat;
    char* ext;
    if(dxd == OCDDS) ext = "dds";
    else if(dxd == OCDAS) ext = "das";
    else ext = "dods";
    if(ce != NULL && strlen(ce) == 0) ce = NULL;
    if(FLAGSET(nccomm->controls,NCF_SHOWFETCH)) {
	if(ce == NULL)
	    nclog(NCLOGNOTE,"fetch: %s.%s",nccomm->oc.uri->uri,ext);
	else
	    nclog(NCLOGNOTE,"fetch: %s.%s?%s",nccomm->oc.uri->uri,ext,ce);
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time0,NULL);
#endif
    }
    ocstat = oc_fetch(conn,ce,dxd,rootp);
    if(FLAGSET(nccomm->controls,NCF_SHOWFETCH)) {
#ifdef HAVE_GETTIMEOFDAY
        double secs;
	gettimeofday(&time1,NULL);
	secs = deltatime();
	nclog(NCLOGNOTE,"fetch complete: %0.3f secs",secs);
#else
	nclog(NCLOGNOTE,"fetch complete.");
#endif
    }
    return ocstat;
}

/* Check a name to see if it contains illegal dap characters */

static char* badchars = "./";

int
dap_badname(char* name)
{
    char* p;
    if(name == NULL) return 0;
    for(p=badchars;*p;p++) {
        if(strchr(name,*p) != NULL) return 1;
    }
    return 0;
}
