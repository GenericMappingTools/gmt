/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include <unistd.h>
#include <sys/types.h>
#include "ocinternal.h"
#include "ocdata.h"
#include "occontent.h"
#include "ocdebug.h"

/* Mnemonic */
#define COMPILEFIELDS 1

/* Forward */
static int occompile1(OCstate* state, OCnode* node, OCmemdata** memdatap, XDR*);
static int occompilefields(OCstate* state, OCnode* node, OCmemdata** memdatap, XDR*);
static int occompileprim(OCstate* state, OCnode* node, OCmemdata** memdatap,XDR*);
static size_t instancesize(OCtype octype);
static OCmemdata* makememdata(OCtype, OCtype, unsigned long count);

extern int oc_invert_xdr_double;

/*
Provide an option that makes a single pass over 
the data packet and converts it to a completely
in-memory structure. This structure is accessed
using the existing OCcontent mechanism defined
in occontent.c. Note that this leaves the original
tempfile contents. See the procedure octempclear().
*/

/* Assume we are operating on the datadds tree */
int
occompile(OCstate* state, OCnode* xroot)
{
    OCerror ocstat = OC_NOERR;
    OCmemdata* memdata = NULL;
    XDR* xdrs;
    OCtree* xtree;

    if(state == NULL || xroot->tree == NULL)
	return OCTHROW(OC_ENODATA);
    xtree = xroot->tree;
    if(xtree->dxdclass != OCDATADDS) return OCTHROW(OC_EINVAL);
    /* See if the compiler has already been invoked */
    if(xtree->data.memdata != NULL)
	return OC_NOERR;
    if(xtree->data.datasize > OCCOMPILELIMIT) {
	return OCTHROW(OC_ENOMEM);
    }

    xdrs = xtree->data.xdrs;
    if(xdrs == NULL) return OCTHROW(OC_EXDR);

    ocstat = occompile1(state,xtree->root,&memdata,xdrs);
    if(ocstat == OC_NOERR) {
	xtree->data.memdata = memdata;
#ifndef OC_DISK_STORAGE
    freeocmemdata(xtree->data.xdrdata);
    xtree->data.xdrdata = NULL;
    xtree->data.datasize = 0;
    xtree->data.bod = 0;
    xtree->data.ddslen = 0;
#endif
    }
    return OCTHROW(ocstat);
}

static int
occompile1(OCstate* state, OCnode* xnode, OCmemdata** memdatap, XDR* xdrs)
{
    unsigned int i,j,xdrcount;
    int stat = OC_NOERR;
    size_t nelements;
    OCmemdata* memdata = NULL;
    OCmemdata* structdata = NULL;
    OClist* records = NULL;
    OCerror ocstat = OC_NOERR;
    OCmemdata** pmem = NULL;


    /* Allocate the space for this memdata chunk */
    switch (xnode->octype) {

    case OC_Dataset:
    case OC_Grid:
    case OC_Structure: {
	if(xnode->array.rank == 0) {
	    ocstat = occompilefields(state,xnode,&memdata,xdrs);
	    if(ocstat != OC_NOERR) goto fail;
	} else { /* rank > 0 */
	    /* Compute the actual index count after projections */
	    nelements = totaldimsize(xnode);
	    if(nelements == 0) return OCTHROW(OC_ENODATA);
	    memdata = makememdata(xnode->octype,OC_NAT,nelements);
	    MEMCHECK(memdata,OC_ENOMEM);
	    memdata->mode = Dimmode;
	    pmem = (OCmemdata**)&memdata->data;
	    /* Consume the leading count field */
	    if(!xdr_u_int(xdrs,&xdrcount)) {stat = OC_EXDR; goto fail;}
	    /* validate the datadds dimensions */
	    if(xdrcount != nelements) {stat=OC_EINVALCOORDS; goto fail;}
            for(i=0;i<nelements;i++) {
		ocstat = occompilefields(state,xnode,&structdata,xdrs);
		if(ocstat != OC_NOERR) {
		    if(ocstat != OC_ENODATA) goto fail;
		    structdata = NULL; /* Leave a hole for this element */
		}
	        pmem[i] = structdata;
	        structdata = NULL;
	    }
	}
    } break;

    case OC_Sequence:{
	/* Since we do not know the # records beforehand,
           use a oclist to collect the record instances.
           Query: this stores by recor (where e.g. original ocapi
           stores by column). How hard would it be to make the
           storage choice conditional on some user defined flag?
        */
	records = oclistnew();
	for(;;) {
            /* pick up the sequence record begin marker*/
            char tmp[sizeof(unsigned int)];
            /* extract the tag byte*/
	    if(!xdr_opaque(xdrs,tmp,sizeof(tmp))) {stat = OC_EXDR; goto fail;}
            if(tmp[0] == StartOfoclist) { /* Walk each member field*/
		ocstat = occompilefields(state,xnode,&structdata,xdrs);
		if(ocstat != OC_NOERR) goto fail;
		oclistpush(records,(ocelem)structdata);
		structdata = NULL;
            } else if(tmp[0] == EndOfoclist) {
                break; /* we are done with the this sequence instance*/
            } else {
		oc_log(LOGERR,"missing/invalid begin/end record marker\n");
                stat = OC_EINVALCOORDS;
		goto fail;
            }
	}
	/* Convert the list to a proper OCmemdata */
	nelements = oclistlength(records);
	memdata = makememdata(xnode->octype,OC_NAT,nelements);
	MEMCHECK(memdata,OC_ENOMEM);
	memdata->mode = Recordmode;
	pmem = (OCmemdata**)&memdata->data;
	for(j=0;j<nelements;j++) {
	    OCmemdata* record = (OCmemdata*)oclistget(records,j);
	    pmem[j] = record;
	}
	oclistfree(records);	    
	records = NULL;
    } break;

    case OC_Primitive:
	ocstat = occompileprim(state,xnode,&memdata,xdrs);
	if(ocstat != OC_NOERR) goto fail;
	break;

    default: OCPANIC1("ocmap: encountered unexpected node type: %x",xnode->octype);
        break;
    }

/*ok:*/
    if(memdatap) *memdatap = memdata;
    return OCTHROW(ocstat);    

fail:
    if(records != NULL) for(i=0;i<oclistlength(records);i++)
	freeocmemdata((OCmemdata*)oclistget(records,i));
    freeocmemdata(memdata);
    freeocmemdata(structdata);
    return OCTHROW(ocstat);
}


static int
occompilefields(OCstate* state, OCnode* xnode, OCmemdata** memdatap, XDR* xdrs)
{
    OCmemdata* structdata = NULL;
    OCmemdata** qmem;
    OCerror ocstat = OC_NOERR;
    unsigned int i,nfields;

    nfields = oclistlength(xnode->subnodes);

    structdata = makememdata(OC_Structure,OC_NAT,nfields);
    MEMCHECK(structdata,OC_ENOMEM);
    qmem = (OCmemdata**)&structdata->data;
    structdata->mode = Fieldmode;
    for(i=0;i<nfields;i++) {
	OCnode* field = (OCnode*)oclistget(xnode->subnodes,i);
        OCmemdata* fielddata;
        ocstat = occompile1(state,field,&fielddata,xdrs);
	if(ocstat == OC_ENODATA) {
	    fielddata = NULL;
	} else if(ocstat != OC_NOERR) {
	    freeocmemdata(structdata); return OCTHROW(ocstat);
	}
	qmem[i] = fielddata;		
    }
    if(memdatap) *memdatap = structdata;
    return OCTHROW(ocstat);
}

static int
occompileprim(OCstate* state, OCnode* xnode, OCmemdata** memdatap, XDR* xdrs)
{
    unsigned int xdrcount,i;
    size_t nelements = 0;
    OCerror ocstat = OC_NOERR;
    OCmemdata* memdata = NULL;
    
    OCASSERT((xnode->octype == OC_Primitive));

    /* Use the count from the datadds */
    nelements = totaldimsize(xnode);

    if(xnode->array.rank > 0) {
        /* Get first copy of the dimension count */
        if(!xdr_u_int(xdrs,&xdrcount)) {ocstat = OC_EXDR; goto fail;}
        if(xdrcount != nelements) {ocstat=OC_EINVALCOORDS; goto fail;}
        if(xnode->etype != OC_String && xnode->etype != OC_URL) {
            /* Get second copy of the dimension count */
            if(!xdr_u_int(xdrs,&xdrcount)) {ocstat = OC_EXDR; goto fail;}
            if(xdrcount != nelements) {ocstat=OC_EINVALCOORDS; goto fail;}
        }
    } else {
	nelements = 1;
	xdrcount = 1;
    }

    memdata = makememdata(xnode->octype,xnode->etype,nelements);
    MEMCHECK(memdata,OC_ENOMEM);
    memdata->mode = (xnode->array.rank > 0?Dimmode:Datamode);

    switch (xnode->etype) {

    case OC_String: case OC_URL: {/* Get the array of strings and store pointers in buf */
	char** dst = (char**)memdata->data.data;
        for(i=0;i<xdrcount;i++) {
            char* s = NULL;
            if(!xdr_string(xdrs,(void*)&s,OC_INT32_MAX)) {ocstat = OC_EXDR; goto fail;}
	    dst[i] = s;
        }
    } break;

    case OC_Byte:
    case OC_UByte:
    case OC_Char: {
        if(xnode->array.rank == 0) { /* Single unpacked character/byte */
            union {unsigned int i; char c[BYTES_PER_XDR_UNIT];} u;
            if(!xdr_opaque(xdrs,u.c,BYTES_PER_XDR_UNIT)) {ocstat = OC_EXDR; goto fail;}
            u.i = ocntoh(u.i);
	    memdata->data.data[0] = (char)u.i;
        } else { /* Packed characters; count will have already been read */
            char* dst = memdata->data.data;
            if(!xdr_opaque(xdrs,dst,xdrcount)) {ocstat = OC_EXDR; goto fail;}
        }
    } break;        

    case OC_Int16: case OC_UInt16: {
        unsigned short* dst = (unsigned short*)memdata->data.data;
        unsigned int* src;
        size_t xdrsize = xdrcount*BYTES_PER_XDR_UNIT;
        src = (unsigned int*)ocmalloc(xdrsize);
        if(!xdr_opaque(xdrs,(char*)src,xdrsize)) {ocfree(src); ocstat = OC_EXDR; goto fail;}
	if(!oc_network_order) {
            for(i=0;i<xdrcount;i++) { /* Write in place */
                unsigned int hostint = src[i];
		swapinline(dst[i],hostint);
	    }
        }
        ocfree(src);
    } break;

    case OC_Int32: case OC_UInt32:
    case OC_Float32: {
        unsigned int* dst = (unsigned int*)memdata->data.data;
        size_t xdrsize = xdrcount*BYTES_PER_XDR_UNIT;
        if(!xdr_opaque(xdrs,(char*)dst,xdrsize)) {ocstat = OC_EXDR; goto fail;}
	if(!oc_network_order) {
            for(i=0;i<xdrcount;i++) {
                unsigned int hostint = dst[i];
		swapinline(dst[i],hostint);
	    }
	}
    } break;
        
    case OC_Int64: case OC_UInt64:
    case OC_Float64: {
        unsigned int* dst = (unsigned int*)memdata->data.data;
        size_t xdrsize = 2*xdrcount*BYTES_PER_XDR_UNIT;
        if(!xdr_opaque(xdrs,(char*)dst,xdrsize)) {ocstat = OC_EXDR; goto fail;}
	if(!oc_network_order) {
            for(i=0;i<2*xdrcount;i++) {
                unsigned int hostint = dst[i];
		swapinline(dst[i],hostint);
	    }
	}
        if(oc_invert_xdr_double) { /* May need to invert each pair */
            for(i=0;i<2*xdrcount;i+=2) {
                unsigned int tmp = dst[i];
                dst[i] = dst[i+1];
                dst[i+1] = tmp;
            }
        }
    } break;

    default: OCPANIC1("unexpected etype: %d",xnode->etype);
    } /* switch */

/*ok:*/
    if(memdatap) *memdatap = memdata;
    return OCTHROW(ocstat);

fail:
    freeocmemdata(memdata);
    return OCTHROW(ocstat);
}

static size_t
instancesize(OCtype etype)
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
    /* Non-primitives*/
    case OC_Sequence:
    case OC_Grid:    
    case OC_Structure:  return sizeof(char*); /* represented as pointer */
    default: break;
    }
    return 0;
}

static OCmemdata*
makememdata(OCtype octype, OCtype etype, unsigned long count)
{
    unsigned long memsize;
    OCmemdata* memdata;
    if(octype == OC_Primitive)
        memsize = instancesize(etype)*count;
    else
        memsize = instancesize(octype)*count;
    memdata = (OCmemdata*)ocmalloc(sizeof(OCmemdata) + memsize);
    if(memdata == NULL) return NULL;
    memdata->octype = (unsigned long)octype;
    memdata->etype = (unsigned long)etype;
if(memdata->etype > 107) OCPANIC("help");
    memdata->count = count;
    return memdata;
}


void
freeocmemdata(OCmemdata* md)
{
    char** sp;
    OCmemdata** mdp;
    unsigned int i;
    if(md == NULL) return;

    switch ((OCtype)md->octype) {
    case OC_Primitive:
        switch ((OCtype)md->etype) {
	case OC_Char:
	case OC_Byte:
	case OC_UByte:
	case OC_Int16:
	case OC_UInt16:
	case OC_Int32:
	case OC_UInt32:
	case OC_Int64:
	case OC_UInt64:
	case OC_Float32:
	case OC_Float64:
	break;
	case OC_String:
	case OC_URL:
	    sp = (char**)md->data.data;
	    for(i=0;i<md->count;i++) ocfree(sp[i]);
	    break;    
	default: break;
	}
	break;

    case OC_Sequence:
    case OC_Grid:    
    case OC_Structure:
	mdp = (OCmemdata**)md->data.data;
	for(i=0;i<md->count;i++) freeocmemdata(mdp[i]);
	break;    
    default: break;
    }
    ocfree(md);
}
