/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include "ocinternal.h"
#include "ocdata.h"
#include "ocdebug.h"

const char StartOfoclist = '\x5A';
const char EndOfoclist = '\xA5';

extern int oc_invert_xdr_double;

#define LOCALMEMMAX 1024

/* Skip arbitrary dimensioned instance; handles dimensioning.*/
int
ocskip(OCnode* node, XDR* xdrs)
{
    unsigned int i,j,rank;
    int stat = OC_NOERR;
    unsigned int xdrcount;
    unsigned int len;

    switch (node->octype) {
        case OC_Primitive:
            /* handle non-uniform types separately*/
            if(node->etype == OC_String || node->etype == OC_URL) {
                rank = node->array.rank;
        	xdrcount = 1;
                if(rank > 0 && !xdr_u_int(xdrs,&xdrcount)) return xdrerror();
                len = xdrcount;
        	for(i=0;i<xdrcount;i++) {
                    if(!xdr_u_int(xdrs,&len)) return xdrerror();
        	    if(!xdr_skip(xdrs,len)) return xdrerror();
        	}
            } else { /* uniform => do a direct skip*/
        	OCASSERT((node->dap.arraysize > 0
                          && node->dap.instancesize > 0));
		if(node->array.rank > 0) {
        	    if(!xdr_skip(xdrs,node->dap.arraysize)) return xdrerror();
		} else {
        	    if(!xdr_skip(xdrs,node->dap.instancesize)) return xdrerror();
		}
            }
	    break;

        case OC_Grid:
	    OCASSERT((node->array.rank == 0));
            if(node->dap.instancesize > 0) { /* do a direct skip*/
                if(!xdr_skip(xdrs,node->dap.arraysize)) return xdrerror();
		break;
            } else { /* Non-uniform size*/
                /* Walk array and maps*/
                for(j=0;j<oclistlength(node->subnodes);j++) {
                    OCnode* field = (OCnode*)oclistget(node->subnodes,j);
                    stat = ocskip(field,xdrs);
                    if(stat != OC_NOERR) {OCTHROWCHK(stat); break;}
                }
                if(stat != OC_NOERR) {OCTHROWCHK(stat); break;}
	    }
	    break;

	case OC_Dataset:
            OCASSERT((node->array.rank == 0));
	    /* fall-thru*/
        case OC_Structure:
            if(node->dap.instancesize > 0) { /* do a direct skip*/
                if(!xdr_skip(xdrs,node->dap.arraysize)) return xdrerror();
            } else {
                /* Non-uniform size, we have to skip element by element*/
                rank = node->array.rank;
                xdrcount = 1;
                if(rank > 0 && !xdr_u_int(xdrs,&xdrcount)) return xdrerror();
                for(i=0;i<xdrcount;i++) { /* skip element by element*/
                    /* Walk each structure field*/
                    for(j=0;j<oclistlength(node->subnodes);j++) {
                        OCnode* field = (OCnode*)oclistget(node->subnodes,j);
                        stat = ocskip(field,xdrs);
                        if(stat != OC_NOERR) {OCTHROWCHK(stat); break;}
                    }
                    if(stat != OC_NOERR) {OCTHROWCHK(stat); break;}
                }
	    }
	    break;

        case OC_Sequence: /* not uniform, so walk record by record*/
	    OCASSERT((node->array.rank == 0));
            for(;;) {
                /* pick up the sequence record begin marker*/
                char tmp[sizeof(unsigned int)];
                /* extract the tag byte*/
                if(!xdr_opaque(xdrs,tmp,sizeof(tmp))) return xdrerror();
                if(tmp[0] == StartOfoclist) {
                    /* Walk each member field*/
                    for(j=0;j<oclistlength(node->subnodes);j++) {
                        OCnode* member = (OCnode*)oclistget(node->subnodes,j);
                        stat = ocskip(member,xdrs);
                        if(stat != OC_NOERR) {OCTHROWCHK(stat); break;}
                    }
                } else if(tmp[0] == EndOfoclist) {
                    break; /* we are done with the this sequence instance*/
                } else {
                    oc_log(LOGERR,"missing/invalid begin/end record marker\n");
                    stat = OC_EINVALCOORDS;
                    {OCTHROWCHK(stat); break;}
                }
                if(stat != OC_NOERR) {OCTHROWCHK(stat); break;}
            }
            break;

        default:
	    OCPANIC1("oc_move: encountered unexpected node type: %x",node->octype);
	    break;
    }
    return OCTHROW(stat);
}

/* Skip arbitrary single instance; except for primitives
   Assumes that parent will handle arrays of compound instances
   or records of compound instances of this node type
   Specifically, all array counts have been absorbed by some parent caller.*/
int
ocskipinstance(OCnode* node, XDR* xdrs)
{
    unsigned int i;
    int stat = OC_NOERR;
    unsigned int xdrcount;

#if 0
    unsigned int j,rank;

    switch (node->octype) {
	case OC_Dataset:
        case OC_Grid:
	    OCASSERT((node->array.rank == 0));
	    stat = ocskip(node,xdrs);
	    break;

        case OC_Sequence: /* instance is essentially same a structure */
        case OC_Structure:
            if(node->dap.instancesize > 0) { /* do a direct skip*/
                if(!xdr_skip(xdrs,node->dap.instancesize)) return xdrerror();
            } else {
                /* Non-uniform size, we have to skip field by field*/
                /* Walk each structure/sequence field*/
                for(j=0;j<oclistlength(node->subnodes);j++) {
                    OCnode* field = (OCnode*)oclistget(node->subnodes,j);
                    stat = ocskip(field,xdrs);
                    if(stat != OC_NOERR) break;
                }
                if(stat != OC_NOERR) break;
	    }
	    break;
	case OC_Primitive:
	    if(node->etype == OC_String || node->etype == OC_URL) {
                if(!xdr_u_int(xdrs,&xdrcount)) return xdrerror();
		if(!xdr_skip(xdrs,xdrcount)) return xdrerror();
	    } else {
	        OCASSERT((node->dap.instancesize > 0));
                if(!xdr_skip(xdrs,node->dap.instancesize)) return xdrerror();
	    }
	    break;

        default:
	    OCPANIC1("oc_move: encountered unexpected node type: %x",node->octype);
	    break;
    }
#else
    if(node->dap.instancesize > 0) { /* do a direct skip*/
        if(!xdr_skip(xdrs,node->dap.instancesize)) return xdrerror();
    } else if(node->octype == OC_Primitive) {
	OCASSERT((node->etype == OC_String || node->etype == OC_URL));
        if(!xdr_u_int(xdrs,&xdrcount)) return xdrerror();
	if(!xdr_skip(xdrs,xdrcount)) return xdrerror();
    } else {
        /* Non-uniform size Grid/Sequence/Structure/Dataset;*/
        /* we have to skip field by field*/
        for(i=0;i<oclistlength(node->subnodes);i++) {
            OCnode* field = (OCnode*)oclistget(node->subnodes,i);
            stat = ocskip(field,xdrs);
            if(stat != OC_NOERR) break;
        }
    }
#endif
    return OCTHROW(stat);
}

/*
Extract data from the xdr packet into a chunk of memory.
Normally, it is assumed that we are (at least virtually)
"at" a single instance in the xdr packet; which we read.
Virtually because for packed data, we need to point to
the beginning of the packed data and use the index to indicate
which packed element to get.
*/
int
ocxdrread(XDR* xdrs, char* memory, size_t memsize, int packed, OCtype octype, unsigned int start, size_t count)
{
    int stat = OC_NOERR;
    unsigned int i;
    size_t elemsize = octypesize(octype);
    char* localmem = NULL;
    char* startmem = NULL;
    size_t totalsize;
    size_t xdrsize;
    unsigned int xdrckp = xdr_getpos(xdrs);

    /* validate memory space*/
    totalsize = elemsize*count;
    if(memsize < totalsize) return OCTHROW(OC_EINVAL);

    /* Handle packed data specially*/
    /* WARNING: assumes that the initial count has been read*/
    if(packed) {
        char tmp[LOCALMEMMAX];
	unsigned int readsize = start+count;
	if(readsize <= LOCALMEMMAX) /* avoid malloc/free for common case*/
	    localmem = tmp;
	else {
            localmem = (char*)ocmalloc(readsize);
	    MEMCHECK(localmem,OC_ENOMEM);
	}
	if(!xdr_opaque(xdrs,(char*)localmem,readsize)) goto shortxdr;
	memcpy((void*)memory,(void*)(localmem+start),count);
	if(readsize > LOCALMEMMAX) ocfree(localmem);
	if(!xdr_setpos(xdrs,xdrckp)) return xdrerror(); /* revert to beginning*/
	return OCTHROW(OC_NOERR);
    }

    /* Not packed: extract count items; use xdr_opaque to speed up*/
    if(octype == OC_String || octype == OC_URL) {
	/* do nothing here; handle below*/
    } else if(octype == OC_Float64
              || octype == OC_UInt64
              || octype == OC_Int64) {
	unsigned int* p;
        xdrsize = 2*(start+count)*BYTES_PER_XDR_UNIT;
        localmem = (char*)ocmalloc(xdrsize);
	startmem = localmem+(2*start*BYTES_PER_XDR_UNIT);
	MEMCHECK(localmem,OC_ENOMEM);
	if(!xdr_opaque(xdrs,(char*)localmem,xdrsize)) goto shortxdr;
	if(!oc_network_order) {
	    for(p=(unsigned int*)startmem,i=0;i<2*count;i++,p++) {
		unsigned int swap = *p;
		swapinline(*p,swap);
	    }
	}
    } else {
	unsigned int* p;
        xdrsize = (start+count)*BYTES_PER_XDR_UNIT;
        localmem = (char*)ocmalloc(xdrsize);
	MEMCHECK(localmem,OC_ENOMEM);
	startmem = localmem+(start*BYTES_PER_XDR_UNIT);
	if(!xdr_opaque(xdrs,(char*)localmem,xdrsize)) goto shortxdr;
	if(!oc_network_order) {
	    for(p=(unsigned int*)startmem,i=0;i<count;i++,p++) {
		unsigned int swap = *p;
		swapinline(*p,swap);
	    }
	}
    }

    switch (octype) {

    case OC_Char: case OC_Byte: case OC_UByte: {
	char* pmem = (char*)memory;
	unsigned int* p = (unsigned int*)startmem;
	for(i=0;i<count;i++) {*pmem++ = (char)(*p++);}
	} break;

    case OC_Int16: case OC_UInt16: {
	unsigned short* pmem = (unsigned short*)memory;
	unsigned int* p = (unsigned int*)startmem;
	for(i=0;i<count;i++) {*pmem++ = (unsigned short)(*p++);}
	} break;

    case OC_Int32: case OC_UInt32: {
	memcpy((void*)memory,(void*)startmem,count*sizeof(unsigned int));
	} break;

    case OC_Float32: {
	memcpy((void*)memory,(void*)startmem,count*sizeof(float));
	} break;

    case OC_Int64: case OC_UInt64: case OC_Float64: {
	unsigned int* p;
	unsigned int* pmem = (unsigned int*)memory;
	/* Sometimes need to invert order*/
	for(p=(unsigned int*)startmem,i=0;i<count;i++) {
	    if(oc_invert_xdr_double) {
	        pmem[1] = (unsigned int)(*p++);
	        pmem[0] = (unsigned int)(*p++);
	    } else {
	        pmem[0] = (unsigned int)(*p++);
	        pmem[1] = (unsigned int)(*p++);
	    }
	    pmem += 2;
	}
	} break;

    case OC_String: case OC_URL: {
        char* s = NULL;
	char** pmem = (char**)memory;
	/* First skip to the starting string */
	for(i=0;i<start;i++) {
	    s = NULL; /* make xdr_string alloc the space */
            if(!xdr_string(xdrs,&s,OC_INT32_MAX)) goto shortxdr;
	    ocfree(s);
        }
	/* Read count strings */
	for(i=0;i<count;i++) {
	    s = NULL; /* make xdr_string alloc the space */	
            if(!xdr_string(xdrs,&s,OC_INT32_MAX)) goto shortxdr;
	    pmem[i] = s;
	}
	} break;

    default: return OCTHROW(OC_EINVAL);
    }

done:
    ocfree(localmem);
    if(!xdr_setpos(xdrs,xdrckp)) return xdrerror(); /* revert to beginning*/
    return OCTHROW(stat);

shortxdr:
    oc_log(LOGERR,"DAP DATADDS packet is apparently too short");
    stat = OC_EDATADDS;
    goto done;    
}

int
countrecords(OCnode* node, XDR* xdrs, size_t* nrecordsp)
{
    int stat = OC_NOERR;
    size_t nrecords = 0;
    unsigned int xdroffset;
    if(node->octype != OC_Sequence) return OCTHROW(OC_EINVAL);
    /* checkpoint the xdr position*/
    xdroffset = xdr_getpos(xdrs);
    for(;;) { unsigned int i;
        /* pick up the sequence record begin marker*/
        char tmp[sizeof(unsigned int)];
        /* extract the tag byte*/
        if(!xdr_opaque(xdrs,tmp,sizeof(tmp))) return xdrerror();
        if(tmp[0] == StartOfoclist) {
            /* Walk each member field*/
            for(i=0;i<oclistlength(node->subnodes);i++) {
                OCnode* member = (OCnode*)oclistget(node->subnodes,i);
                stat = ocskip(member,xdrs);
                if(stat != OC_NOERR) break;
            }
	    nrecords++;
        } else if(tmp[0] == EndOfoclist) {
            break; /* we are done with the this sequence instance*/
        } else {
            oc_log(LOGERR,"missing/invalid begin/end record marker\n");
            stat = OC_EINVALCOORDS;
            break;
        }
        if(stat != OC_NOERR) break;
    }
    /* move to checkpoint position*/
    if(!xdr_setpos(xdrs,xdroffset)) return xdrerror();
    if(nrecordsp != NULL) *nrecordsp = nrecords;
    return OCTHROW(stat);
}
