/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include <unistd.h>
#include <fcntl.h>
#include "ocinternal.h"
#include "ocdebug.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

/* Order is important: longest first */
static char* DDSdatamarks[3] = {"Data:\r\n","Data:\n",(char*)NULL};

/* Not all systems have strndup, so provide one*/
char*
ocstrndup(const char* s, size_t len)
{
    char* dup;
    if(s == NULL) return NULL;
    dup = (char*)ocmalloc(len+1);
    MEMCHECK(dup,NULL);
    memcpy((void*)dup,s,len);
    dup[len] = '\0';
    return dup;
}


void
makedimlist(OClist* path, OClist* dims)
{
    unsigned int i,j;
    for(i=0;i<oclistlength(path);i++) {
	OCnode* node = (OCnode*)oclistget(path,i);
        unsigned int rank = node->array.rank;
	for(j=0;j<rank;j++) {
	    OCnode* dim = (OCnode*)oclistget(node->array.dimensions,j);
	    oclistpush(dims,(ocelem)dim);
        }
    }
}

void
ocfreeprojectionclause(OCprojectionclause* clause)
{
    if(clause->target != NULL) free(clause->target);
    while(oclistlength(clause->indexsets) > 0) {
	OClist* slices = (OClist*)oclistpop(clause->indexsets);
        while(oclistlength(slices) > 0) {
	    OCslice* slice = (OCslice*)oclistpop(slices);
	    if(slice != NULL) free(slice);
	}
        oclistfree(slices);
    }
    oclistfree(clause->indexsets);
    free(clause);
}

static void
freeAttributes(OClist* attset)
{
    unsigned int i,j;
    for(i=0;i<oclistlength(attset);i++) {
	OCattribute* att = (OCattribute*)oclistget(attset,i);
	if(att->name != NULL) free(att->name);
	if(att->etype == OC_String || att->etype == OC_URL) {
	    for(j=0;j<att->nvalues;j++) {
		char* s = ((char**)att->values)[j];
		if(s != NULL) free(s);
	    }
	} else {
	    free(att->values);
	}
    }
}

void
freeOCnode(OCnode* cdf, int deep)
{
    unsigned int i;
    if(cdf == NULL) return;
    if(cdf->name != NULL) free(cdf->name);
    if(cdf->fullname != NULL) free(cdf->fullname);
    if(cdf->attributes != NULL) freeAttributes(cdf->attributes);
    if(cdf->subnodes != NULL) {
	if(deep) {
            for(i=0;i<oclistlength(cdf->subnodes);i++) {
	        OCnode* node = (OCnode*)oclistget(cdf->subnodes,i);
		freeOCnode(node,deep);
	    }
	}
        oclistfree(cdf->subnodes);
    }
    free(cdf);
}

int
findbod(OCbytes* buffer, size_t* bodp, size_t* ddslenp)
{
    unsigned int i;
    char* content;
    size_t len = ocbyteslength(buffer);
    char** marks;
    
    content = ocbytescontents(buffer);

    for(marks = DDSdatamarks;*marks;marks++) {
	char* mark = *marks;
        int tlen = strlen(mark);
        for(i=0;i<len;i++) {
	    if((i+tlen) <= len 
	        && (strncmp(content+i,mark,tlen)==0)) {
	       *ddslenp = i;
	        i += tlen;
	        *bodp = i;
	        return 1;
	    }
	}
    }
    *ddslenp = 0;
    *bodp = 0;
    return 0; /* tag not found; not necessarily an error*/
}

/* Compute total # of elements if dimensioned*/
size_t
totaldimsize(OCnode* node)
{
    unsigned int i;
    size_t count = 1;
    for(i=0;i<node->array.rank;i++) {
        OCnode* dim = (OCnode*)oclistget(node->array.dimensions,i);
        count *= (dim->dim.declsize);
    }
    return count;
}

#if 0 /*def IGNORE*/
size_t
totaldimsize(unsigned int rank, size_t* dimsizes)
{
    unsigned int i;
    int unlim = 0;
    unsigned long size = 1;
    for(i=0;i<rank;i++) {
        if(dimsizes[i] != 0) size = (size * dimsizes[i]); else unlim = 1;
    }
    return size;
}
#endif

size_t
octypesize(OCtype etype)
{
    switch (etype) {
    case OC_Char:	return sizeof(char);
    case OC_Byte:	return sizeof(signed char);
    case OC_UByte:	return sizeof(unsigned char);
    case OC_Int16:	return sizeof(short);
    case OC_UInt16:	return sizeof(unsigned short);
    case OC_Int32:	return sizeof(int);
    case OC_UInt32:	return sizeof(unsigned int);
    case OC_Float32:	return sizeof(float);
    case OC_Float64:	return sizeof(double);
#ifdef HAVE_LONG_LONG_INT
    case OC_Int64:	return sizeof(long long);
    case OC_UInt64:	return sizeof(unsigned long long);
#endif
    case OC_String:	return sizeof(char*);
    case OC_URL:	return sizeof(char*);
		  /* Ignore all others */
    default: break;
    }
    return 0;
}

char*
octypetostring(OCtype octype)
{
    switch (octype) {
    case OC_NAT:          return "OC_NAT";
    case OC_Char:         return "OC_Char";
    case OC_Byte:         return "OC_Byte";
    case OC_UByte:	   return "OC_UByte";
    case OC_Int16:        return "OC_Int16";
    case OC_UInt16:       return "OC_UInt16";
    case OC_Int32:        return "OC_Int32";
    case OC_UInt32:       return "OC_UInt32";
    case OC_Int64:        return "OC_Int64";
    case OC_UInt64:       return "OC_UInt64";
    case OC_Float32:      return "OC_Float32";
    case OC_Float64:      return "OC_Float64";
    case OC_String:       return "OC_String";
    case OC_URL:          return "OC_URL";
    /* Non-primitives*/
    case OC_Dataset:      return "OC_Dataset";
    case OC_Sequence:     return "OC_Sequence";
    case OC_Grid:         return "OC_Grid";
    case OC_Structure:    return "OC_Structure";
    case OC_Dimension:    return "OC_Dimension";
    case OC_Attribute:    return "OC_Attribute";
    case OC_Attributeset: return "OC_Attributeset";
    case OC_Primitive:    return "OC_Primitive";
    default: break;
    }
    return NULL;
}

char*
octypetoddsstring(OCtype octype)
{
    switch (octype) {
    case OC_Byte:         return "Byte";
    case OC_Int16:        return "Int16";
    case OC_UInt16:       return "UInt16";
    case OC_Int32:        return "Int32";
    case OC_UInt32:       return "UInt32";
    case OC_Float32:      return "Float32";
    case OC_Float64:      return "Float64";
    case OC_String:       return "String";
    case OC_URL:          return "Url";
    default: break;
    }
    return "<unknown>";
}


OCerror
octypeprint(OCtype etype, char* buf, size_t bufsize, void* value)
{
    if(buf == NULL || bufsize == 0 || value == NULL) return OC_EINVAL;
    buf[0] = '\0';
    switch (etype) {
    case OC_Char:
	snprintf(buf,bufsize,"'%c'",*(char*)value);
	break;
    case OC_Byte:
	snprintf(buf,bufsize,"%d",*(signed char*)value);
	break;
    case OC_UByte:
	snprintf(buf,bufsize,"%u",*(unsigned char*)value);
	break;
    case OC_Int16:
	snprintf(buf,bufsize,"%d",*(short*)value);
	break;
    case OC_UInt16:
	snprintf(buf,bufsize,"%u",*(unsigned short*)value);
	break;
    case OC_Int32:
	snprintf(buf,bufsize,"%d",*(int*)value);
	break;
    case OC_UInt32:
	snprintf(buf,bufsize,"%u",*(unsigned int*)value);
	break;
    case OC_Float32:
	snprintf(buf,bufsize,"%g",*(float*)value);
	break;
    case OC_Float64:
	snprintf(buf,bufsize,"%g",*(double*)value);
	break;
#ifdef HAVE_LONG_LONG_INT
    case OC_Int64:
	snprintf(buf,bufsize,"%lld",*(long long*)value);
	break;
    case OC_UInt64:
	snprintf(buf,bufsize,"%llu",*(unsigned long long*)value);
	break;
#endif
    case OC_String:
    case OC_URL: {
	char* s = *(char**)value;
	snprintf(buf,bufsize,"\"%s\"",s);
	} break;
    default: break;
    }
    return OC_NOERR;
}

size_t
ocxdrsize(OCtype etype)
{
    switch (etype) {
    case OC_Char:
    case OC_Byte:
    case OC_UByte:
    case OC_Int16:
    case OC_UInt16:
    case OC_Int32:
    case OC_UInt32:
	return BYTES_PER_XDR_UNIT;
    case OC_Int64:
    case OC_UInt64:
	return (2*BYTES_PER_XDR_UNIT);
    case OC_Float32:
	return BYTES_PER_XDR_UNIT;
    case OC_Float64:
	return (2*BYTES_PER_XDR_UNIT);
    case OC_String:
    case OC_URL:
    default: break;
    }
    return 0;
}

/***********************************/
/* Skip "len" bytes in the input*/
int
xdr_skip(XDR* xdrs, unsigned int len)
{
    unsigned int pos;
    if(len <= 0) return 1; /* ignore*/
    pos = xdr_getpos(xdrs);
    return xdr_setpos(xdrs,(pos+RNDUP(len)));
}

/* skip "n" string/bytestring instances in the input*/
int
xdr_skip_strings(XDR* xdrs, unsigned int n)
{
    while(n-- > 0) {
        unsigned int slen;
	if(!xdr_u_int(xdrs,&slen)) return xdrerror();
	if(xdr_skip(xdrs,RNDUP(slen))) return xdrerror();
    }
    return OCTHROW(OC_NOERR);
}

unsigned int xdr_roundup(unsigned int n)
{
    unsigned int rem;
    rem = (n % BYTES_PER_XDR_UNIT);
    if(rem > 0) n += (BYTES_PER_XDR_UNIT - rem);
    return n;
}

unsigned int
ocbyteswap(unsigned int i)
{
    unsigned int swap,b0,b1,b2,b3;
    b0 = (i>>24) & 0x000000ff;
    b1 = (i>>16) & 0x000000ff;
    b2 = (i>>8) & 0x000000ff;
    b3 = (i) & 0x000000ff;
    swap = (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
    return swap;
}

/**************************************/

char*
ocerrstring(int err)
{
    if(err == 0) return "no error";
    if(err > 0) return strerror(err);
    switch (err) {
	case OC_EBADID:
	    return "OC_EBADID: Not a valid ID";
	case OC_EINVAL:
	    return "OC_EINVAL: Invalid argument";
	case OC_EPERM:
	    return "OC_EPERM: Write to read only";
	case OC_EINVALCOORDS:
	    return "OC_EINVALCOORDS: Index exceeds dimension bound";
	case OC_ENOTVAR:
	    return "OC_ENOTVAR: Variable not found";
	case OC_ECHAR:
	    return "OC_ECHAR: Attempt to convert between text & numbers";
	case OC_EEDGE:
	    return "OC_EEDGE: Start+count exceeds dimension bound";
	case OC_ESTRIDE:
	    return "OC_ESTRIDE: Illegal stride";
	case OC_ENOMEM:
	    return "OC_ENOMEM: Memory allocation (malloc) failure";
	case OC_EDIMSIZE:
	    return "OC_EDIMSIZE: Invalid dimension size";
	case OC_EDAP:
	    return "OC_EDAP: DAP failure";
	case OC_ECURL:
	    return "OC_ECURL: libcurl failure";
	case OC_EIO:
	    return "OC_EIO: I/O failure";
	case OC_ENODATA:
	    return "OC_ENODATA: Variable has no data in DAP request";
	case OC_EDAPSVC:
	    return "OC_EDAPSVC: DAP Server error";
	case OC_ENAMEINUSE:
	    return "OC_ENAMEINUSE: Duplicate name in DDS";
	case OC_EDAS:
	    return "OC_EDAS: Malformed or unreadable DAS";
	case OC_EDDS:
	    return "OC_EDDS: Malformed or unreadable DDS";
	case OC_EDATADDS:
	    return "OC_EDATADDS: Malformed or unreadable DATADDS";
	case OC_ERCFILE:
	    return "OC_ERCFILE: Malformed or unreadable run-time configuration file";
	case OC_ENOFILE:
	    return "OC_ENOFILE: cannot read content of URL";
	default: break;
    }
    return "<unknown error code>";
}

OCerror
ocsvcerrordata(OCstate* state, char** codep, char** msgp, long* httpp)
{
    if(codep) *codep = state->error.code;
    if(msgp) *msgp = state->error.message;
    if(httpp) *httpp = state->error.httpcode;
    return OC_NOERR;    
}

/* if we get OC_EDATADDS error, then try to capture any
   error message and log it.
*/
void
ocdataddsmsg(OCstate* state, OCtree* tree)
{
#define ERRCHUNK 1024
#define ERRFILL ' '
#define ERRTAG "Error {" 
    int count,len,pos0;
    char* p;
    unsigned int i,j;
    XDR* xdrs;
    char chunk[ERRCHUNK+1];

    if(tree == NULL) return;
    xdrs = tree->data.xdrs;
    len = (int)tree->data.datasize;
    /* obtain the last ERRCHUNK (or less) bytes of xdr data */
    count = (len < ERRCHUNK?len:ERRCHUNK);
    pos0 = (len - count);
    xdr_setpos(xdrs,pos0);
    xdr_opaque(xdrs,(caddr_t)chunk,count);
    chunk[count] = '\0';
    /* Look for error tag */
    for(p=chunk,i=0;i<(count - strlen(ERRTAG));i++,p++) {
        if(strncmp(p,ERRTAG,strlen(ERRTAG))==0) {
	    /* log the error message */
	    /* Do a quick and dirty escape */
	    for(p=chunk+i,j=i;j<len;j++,p++) {
		if(*p > 0 && (*p < ' ' || *p >= '\177')) *p = ERRFILL;
	    }
	    oc_log(LOGERR,"DATADDS failure, possible message: '%s'",
			chunk+i);
	    goto done;
	}
    }
done:
    return;
}
