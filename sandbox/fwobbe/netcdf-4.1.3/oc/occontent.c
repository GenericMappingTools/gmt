/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include "ocinternal.h"
#include "occontent.h"
#include "ocdebug.h"

/* Mnemonic*/
#define ISPACKED 1

/* Forward*/
static OCmode modetransition(OCnode* node, OCmode mode);
static size_t maxindexfor(OCnode* node, OCmode srcmode);
static int ocgetmemdata(OCstate*, OCcontent*, void* memory, size_t memsize,
                        size_t start, size_t count);

OCcontent*
ocnewcontent(OCstate* state)
{
    OCcontent* content;
    if(state == NULL) return NULL;
    content = state->contentlist;
    /* Search for an unused content node*/
    while(content != NULL && content->mode != Emptymode) {content = content->next;}
    if(content == NULL) {
	content = (OCcontent*)ocmalloc(sizeof(OCcontent));
	MEMCHECK(content,(OCcontent*)NULL);
	content->magic = OCMAGIC;
        content->next = state->contentlist;
        state->contentlist = content;
    }
    return ocresetcontent(state,content);
}

void
ocfreecontent(OCstate* state, OCcontent* content)
{
    if(content != NULL) {content->mode = Emptymode;}
}

OCcontent*
ocresetcontent(struct OCstate* state, OCcontent* content)
{
    content->state = state;
    content->mode = Nullmode;
    content->node = NULL;
    content->xdrpos.offset = 0;
    content->xdrpos.valid = 0;
    content->index = 0;
    content->packed = 0;
    content->memdata = NULL;
    return content;
}

/* Copy everything except public and private fields*/
OCcontent*
occlonecontent(OCstate* state, OCcontent* content)
{
    OCcontent* clone = ocnewcontent(state);
    clone->mode = content->mode;
    clone->node = content->node;
    clone->xdrpos = content->xdrpos;
    clone->index = content->index;
    clone->maxindex = content->maxindex;
    clone->memdata = content->memdata;
    return clone;
}

int
ocrootcontent(OCstate* state, OCnode* root, OCcontent* content)
{
    OCtree* tree;
    if(state == NULL || root == NULL || content == NULL)
	return OCTHROW(OC_EINVAL);
    if(root->tree == NULL) return OCTHROW(OC_EINVAL);
    tree = root->tree;
    if(tree->dxdclass != OCDATADDS) return OCTHROW(OC_ENODATA);
    if(tree->nodes == NULL) return OCTHROW(OC_EINVAL);
    if(tree->data.memdata == NULL  && tree->data.xdrs == NULL)
	return OCTHROW(OC_EXDR);
    ocresetcontent(state,content);
    content->state = state; /* make sure*/
    content->mode = Fieldmode;
    content->node = root;
    content->tree = tree;
    content->maxindex = oclistlength(content->node->subnodes);
    if(tree->data.memdata == NULL) {
        content->xdrpos.offset = tree->data.bod;
        content->xdrpos.valid = 1;
    } else {
	content->memdata = tree->data.memdata;
        content->mode = tree->data.memdata->mode;
    }
    return OCTHROW(OC_NOERR);
}

/* Remember: we are operating wrt the datadds count, not the dds count */
int
ocarraycontent(OCstate* state, OCcontent* content, OCcontent* newcontent, size_t index)
{
    unsigned int i;
    int stat = OC_NOERR;
    XDR* xdrs;
    unsigned int xdrcount;
    int packed;
    OCtype etype,octype;

    if(state == NULL || content == NULL) return OCTHROW(OC_EINVAL);
    if(content->mode != Dimmode) return OCTHROW(OC_EINVAL);
    if(content->node->array.rank == 0) return OCTHROW(OC_EINVAL);

    etype = content->node->etype;
    octype = content->node->octype;

    if(content->maxindex > 0 && content->maxindex <= index)
	return OCTHROW(OC_EINVALCOORDS);
    content->index = index; /* Track our location in parent content */

    /* check if the data is packed*/
    packed = (octype == OC_Primitive &&
              (etype == OC_Byte || etype == OC_UByte || etype == OC_Char));

    ocresetcontent(state,newcontent);
    /* Set the some of the new content*/
    newcontent->state = state; /* make sure*/
    newcontent->tree = content->tree;
    newcontent->node = content->node; /* keep same node*/
    newcontent->packed = packed;
    newcontent->mode = modetransition(newcontent->node, content->mode);
    newcontent->index = 0;
    newcontent->maxindex = maxindexfor(newcontent->node, content->mode);

    if(content->memdata != NULL) { /* Get data from the compiled version */
        OCASSERT((content->memdata->mode == Dimmode));
        /* Leave the primitive alone to be picked up by oc_getcontent */
	if(octype != OC_Primitive) {
   	    OCmemdata* next;
	    OCASSERT((octype == OC_Structure));
  	    if(content->memdata->count <= index) return OCTHROW(OC_ENODATA);
   	    next = ((OCmemdata**)content->memdata->data.data)[index];
	    newcontent->memdata = next;
	} else
	    newcontent->memdata = content->memdata; /* use same memdata */
	goto done;
    }

    xdrs = content->tree->data.xdrs;
    if(xdrs == NULL) return OCTHROW(OC_EXDR);

    /* checkpoint the beginning of this instance*/
    if(!content->xdrpos.valid) {
	content->xdrpos.offset = xdr_getpos(xdrs);
	content->xdrpos.valid = 1;
    }
    /* move to checkpoint position*/
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) return xdrerror();

    /* Collect the dimension count from the xdr data packet*/
    if(!xdr_u_int(xdrs,&xdrcount)) return xdrerror();
    if(xdrcount < index) return OCTHROW(OC_ENODATA);

    /* pull out redundant second count*/
    /* (note that String/URL do not redundant count)*/
    if(octype == OC_Primitive && etype != OC_String && etype != OC_URL) {
        if(!xdr_u_int(xdrs,&xdrcount)) return xdrerror();
    }

    /* We have to treat OC_Byte/UByte/Char specially*/
    /* because the data is packed in the xdr packet*/
    if(packed) {
        /* In effect, compile where the data is, but wait*/
        /* until a getcontent to retrieve it*/
        OCASSERT((newcontent->mode == Datamode));
        newcontent->index = index; /* record final destination in the packed data*/
        newcontent->packed = 1;
        return OCTHROW(OC_NOERR);
    }

    for(i=0;i<index;i++) {
        stat = ocskipinstance(content->node,xdrs);
        if(stat != OC_NOERR) return OCTHROW(stat);
    }
    /* Record the location of the newcontent */
    newcontent->xdrpos.offset = xdr_getpos(xdrs);
    newcontent->xdrpos.valid = 1;

    /* move back to checkpoint position*/
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) return xdrerror();

done:
    return OCTHROW(stat);
}

int
ocrecordcontent(OCstate* state,OCcontent* content, OCcontent* recordcontent, size_t index)
{
    unsigned int i;
    int stat = OC_NOERR;
    XDR* xdrs;
    char tmp[BYTES_PER_XDR_UNIT];
    OCtype octype,etype;
    int packed;

    if(state == NULL || content == NULL) return OCTHROW(OC_EINVAL);
    if(content->mode != Recordmode) return OCTHROW(OC_EINVAL);

    if(content->maxindex > 0 && content->maxindex <= index)
	return OCTHROW(OC_EINVALCOORDS);
    content->index = index; /* Track our location in parent content */

    octype = content->node->octype;
    etype = content->node->etype;

    /* check if the data is packed*/
    packed = (octype == OC_Primitive &&
              (etype == OC_Byte || etype == OC_UByte || etype == OC_Char));

    ocresetcontent(state,recordcontent);
    /* Set some of the new content*/
    recordcontent->state = state; /* make sure*/
    recordcontent->tree = content->tree;
    recordcontent->node = content->node;
    recordcontent->packed = packed;
    recordcontent->mode = modetransition(recordcontent->node, content->mode);
    recordcontent->index = 0;
    recordcontent->maxindex = maxindexfor(recordcontent->node,content->mode);

    if(content->memdata != NULL) { /* Get data from the compiled version */
	OCmemdata* next;
        OCASSERT((content->memdata->mode == Recordmode));
	if(content->memdata->count <= index)
	    {OCTHROWCHK(stat=OC_ENODATA); goto done;}
	next = ((OCmemdata**)content->memdata->data.data)[index];
	recordcontent->memdata = next;
	goto done;
    }

    xdrs = content->tree->data.xdrs;
    if(xdrs == NULL) return OCTHROW(OC_EXDR);

    /* checkpoint the beginning of this instance*/
    if(!content->xdrpos.valid) {
	content->xdrpos.offset = xdr_getpos(xdrs);
	content->xdrpos.valid = 1;
    }
    /* move to checkpoint position*/
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) return xdrerror();

    switch (content->node->octype) {
    case OC_Sequence:
        for(i=0;i<index;i++) {
            /* pick up the sequence record begin marker*/
            /* extract the tag byte*/
            if(!xdr_opaque(xdrs,tmp,sizeof(tmp))) return xdrerror();
            if(tmp[0] == StartOfoclist) {
                /* skip instance*/
                stat = ocskipinstance(content->node,xdrs);
            } else if(tmp[0] == EndOfoclist) {
                stat = OC_EINVALCOORDS; /* not enough records*/
                break;
            } else {
                oc_log(LOGERR,"missing/invalid begin/end record marker\n");
                stat = OC_EINVAL;
                break;
            }
        }
	if(stat != OC_NOERR) return OCTHROW(stat);

	/* skip the sequence begin marker for the chosen record*/
	/* so we are at its contents*/
        if(!xdr_opaque(xdrs,tmp,sizeof(tmp))) return xdrerror();
        if(tmp[0] != StartOfoclist) return OCTHROW(OC_EINVALCOORDS);

	/* Set contents of the output content*/
        recordcontent->xdrpos.offset = xdr_getpos(xdrs);
	recordcontent->xdrpos.valid = 1;
	break;

    case OC_Dataset:
    case OC_Structure:
    case OC_Grid:
    case OC_Primitive:
    default: return OCTHROW(OC_EINVAL);
    }

    /* move back to checkpoint position*/
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) return xdrerror();

done:
    return OCTHROW(stat);
}

/*
The ocfieldcontent procedure has to deal with the fact
that the dap constraints may have removed some fields
from the datadds and hence some fields may have no
representation in the xdr data (or compiled data).
*/
int
ocfieldcontent(OCstate* state, OCcontent* content, OCcontent* fieldcontent, size_t index)
{
    unsigned int i;
    int stat = OC_NOERR;
    XDR* xdrs;
    OCtype octype,etype;
    int packed;

    if(state == NULL || content == NULL) return OCTHROW(OC_EINVAL);
    if(content->mode != Fieldmode) return OCTHROW(OC_EINVAL);

    if(content->maxindex > 0 && content->maxindex <= index)
	return OCTHROW(OC_EINVALCOORDS);

    content->index = index; /* Track our location in parent content */

    octype = content->node->octype;
    etype = content->node->etype;

    /* check if the data is packed*/
    packed = (octype == OC_Primitive &&
              (etype == OC_Byte || etype == OC_UByte || etype == OC_Char));

    ocresetcontent(state,fieldcontent);
    /* Set the state of the new content*/
    fieldcontent->state = state; /* make sure*/
    fieldcontent->tree = content->tree;
    fieldcontent->node = (OCnode*)oclistget(content->node->subnodes,index);
    fieldcontent->packed = packed;
    fieldcontent->mode = modetransition(fieldcontent->node, content->mode);
    fieldcontent->index = 0; /* record where we want to be */
    fieldcontent->maxindex = maxindexfor(fieldcontent->node,content->mode);

    if(content->memdata != NULL) { /* Get data from the compiled version */
	OCmemdata* md = content->memdata;
	OCmemdata* next;
        OCASSERT((md->mode == Fieldmode));
	if(md->count <= index) {OCTHROWCHK(stat=OC_ENODATA); goto done;}
	next = ((OCmemdata**)md->data.data)[index];
	if(next == NULL) {OCTHROWCHK(stat=OC_ENODATA); goto done;}
	fieldcontent->memdata = next;
	goto done;
    }

    xdrs = content->tree->data.xdrs;
    if(xdrs == NULL) return OCTHROW(OC_EXDR);

    /* checkpoint the beginning of this instance*/
    if(!content->xdrpos.valid) {
	content->xdrpos.offset = xdr_getpos(xdrs);
	content->xdrpos.valid = 1;
    }
    /* move to checkpoint position*/
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) return xdrerror();

    switch (content->node->octype) {
    case OC_Grid:
	/* Note that the Grid array is field 0 and the maps are 1..nsubnodes*/
    case OC_Sequence:
    case OC_Dataset:
    case OC_Structure:
	if(index >= oclistlength(content->node->subnodes)) return OCTHROW(OC_EINVALCOORDS);
        for(i=0;i<index;i++) {
  	    OCnode* field = (OCnode*)oclistget(content->node->subnodes,i);
	    stat = ocskip(field,xdrs);
	    if(stat != OC_NOERR) return OCTHROW(stat);
        }
        fieldcontent->xdrpos.offset = xdr_getpos(xdrs);
	fieldcontent->xdrpos.valid = 1;
	break;

    case OC_Primitive:
    default: return OCTHROW(OC_EINVAL);
    }

    /* move back to checkpoint position*/
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) return xdrerror();

done:
    return OCTHROW(stat);
}

/*
In order to actually extract data,
one must move to the specific primitive typed
field containing the data of interest by using
ocfieldcontent().
Then, oc_getcontent() is invoked to extract
some subsequence of items from the field.
Note that oc_getcontent() will also work for scalars,
but the start must be zero and the count must be one.
*/

int
ocgetcontent(OCstate* state, OCcontent* content, void* memory, size_t memsize,
                 size_t start, size_t count)
{
    int stat = OC_NOERR;
    XDR* xdrs;
    OCtype etype;
    int isscalar;
    size_t elemsize, totalsize;
    int packed;
    unsigned int xdrcount;

    if(state == NULL || content == NULL || memory == NULL)
	{OCTHROWCHK(stat=OC_EINVAL); goto done;}
    if(content->node->octype != OC_Primitive)
	{OCTHROWCHK(stat=OC_EINVAL); goto done;}
    if(content->maxindex > 0 && content->maxindex < start+count)
	return OCTHROW(OC_ENODATA);

    etype = content->node->etype;
    isscalar = (content->node->array.rank == 0);
    if(isscalar && (start != 0 || count != 1))
	{OCTHROWCHK(stat=OC_EINVALCOORDS); goto done;}

    /* validate memory space*/
    elemsize = octypesize(etype);
    totalsize = elemsize*count;
    if(memsize < totalsize) return OCTHROW(OC_EINVAL);

    OCASSERT((occontentmode(state,content)==Dimmode || isscalar));

    if(content->memdata != NULL) { /* Get data from the compiled version */
	stat = ocgetmemdata(state,content,memory,memsize,start,count);
	goto done;
    }
    /* No memdata => use xdr */
    xdrs = content->tree->data.xdrs;
    if(xdrs == NULL) return OCTHROW(OC_EXDR);

    /* check if the data is packed*/
    packed = (!isscalar && (etype == OC_Byte || etype == OC_UByte || etype == OC_Char));

    content->packed = packed;

    /* Make sure we are at the proper offset: ie at count if !scalar */
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) goto shortxdr;

    if(!isscalar) {
        /* Collect the dimension count from the xdr data packet*/
        if(!xdr_u_int(xdrs,&xdrcount)) goto shortxdr;
        if(xdrcount < start) return OCTHROW(OC_EINVALCOORDS);
        if(xdrcount < start+count) return OCTHROW(OC_EINVALCOORDS);
        /* pull out redundant second count*/
        /* (note that String/URL do not have redundant count)*/
        if(etype != OC_String && etype != OC_URL) {
            if(!xdr_u_int(xdrs,&xdrcount)) goto shortxdr;
        }
    }
    /* Extract the data */
#ifdef OCPROGRESS
    oc_log(LOGNOTE,"reading xdr: %lu bytes",(unsigned long)memsize);
#endif
    stat = ocxdrread(xdrs,(char*)memory,memsize,packed,content->node->etype,start,count);
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) return xdrerror(); /* restore location*/
done:
    return OCTHROW(stat);
shortxdr:
    oc_log(LOGERR,"DAP DATADDS appears to be too short");
    return OC_EDATADDS;
}

static int
ocgetmemdata(OCstate* state, OCcontent* content, void* memory, size_t memsize,
             size_t start, size_t count)
{
    OCmemdata* md = content->memdata;
    unsigned short* d16;
    unsigned int* d32;
#ifdef HAVE_LONG_LONG_INT
    unsigned long long* d64;
#endif
    char* dchar;
    char** dstring;
    size_t totalsize;
    size_t elemsize;
    OCtype etype;

    /* Validate the etypes */
    etype = content->node->etype;
    if(etype != md->etype) return OCTHROW(OC_EINVAL);

    elemsize = octypesize(etype);
    totalsize = elemsize*count;

    switch (etype) {
    case OC_Char: case OC_Byte: case OC_UByte:
	dchar = (char*)md->data.data;
	memcpy((void*)memory,(void*)(dchar+start),totalsize);
	break;
    case OC_Int16: case OC_UInt16:
	d16 = (unsigned short*)md->data.data;
	memcpy((void*)memory,(void*)(d16+start),totalsize);	    
	break;
    case OC_Int32: case OC_UInt32: case OC_Float32:
	d32 = (unsigned int*)md->data.data;
	memcpy((void*)memory,(void*)(d32+start),totalsize);
	break;
#ifdef HAVE_LONG_LONG_INT
    case OC_Int64: case OC_UInt64: case OC_Float64:
	d64 = (unsigned long long*)md->data.data;
	memcpy((void*)memory,(void*)(d64+start),totalsize);	    
	break;
#endif
    case OC_String: case OC_URL: {
	unsigned int i;
	char** memstrings = (char**)memory;
	dstring = (char**)md->data.data;
	for(i=0;i<count;i++) {
	    memstrings[i] = nulldup(dstring[start+i]);
	}
	} break;
    default: OCPANIC1("unexpected etype: %d",content->node->etype);
    }
    return OCTHROW(OC_NOERR);
}

size_t
ocfieldcount(OCstate* state, OCcontent* content)
{
    OCnode* node = content->node;
    size_t count;
    OCASSERT((node != NULL));
    count = oclistlength(node->subnodes);
    /* If we are using memdata; then verify against the memdata */
    if(content->memdata != NULL) {
	OCASSERT(content->memdata->count == count);
    }
    return count;
}

/* Extract the actual element count from the xdr packet*/
size_t
ocarraycount(OCstate* state, OCcontent* content)
{
    unsigned int count;
    unsigned int xdrsave;
    OCnode* node = content->node;
    XDR* xdrs;

    OCASSERT((node != NULL));
    OCASSERT((content->mode == Dimmode));

    count = totaldimsize(node);

    /* If we are using memdata; then use that to verify */
    if(content->memdata != NULL) {
	OCASSERT(content->memdata->count == count);
	return (size_t)count;
    }

    /* Otherwise verify against xdr */
    xdrs = content->tree->data.xdrs;

    OCASSERT((xdrs != NULL));

    /* checkpoint current location */
    xdrsave = xdr_getpos(xdrs);

    /* move to content location*/
    OCASSERT(content->xdrpos.valid);
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) return 0;

    /* extract the count*/
    if(!xdr_u_int(xdrs,&count)) count = 0;

    /* return to checkpoint position*/
    if(!xdr_setpos(xdrs,xdrsave)) return 0;

    return (size_t)count;
}

/* Counting records actually requires walking the xdr packet
   so it is not necessarily cheap*/
size_t
ocrecordcount(OCstate* state, OCcontent* content)
{
    int stat = OC_NOERR;
    size_t count;
    OCnode* node = content->node;
    XDR* xdrs;
    char tmp[BYTES_PER_XDR_UNIT];

    OCASSERT((node != NULL));
    OCASSERT((node->octype == OC_Sequence));
    OCASSERT((content->mode == Recordmode));

    /* If we are using memdata; then use that value */
    if(content->memdata != NULL) {
	return content->memdata->count;
    }

    xdrs = content->tree->data.xdrs;
    OCASSERT((xdrs != NULL));

    /* checkpoint the beginning of this instance*/
    if(!content->xdrpos.valid) {
	content->xdrpos.offset = xdr_getpos(xdrs);
	content->xdrpos.valid = 1;
    }
    /* move to checkpoint position*/
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) return 0;

    for(count=0;;count++) {
        /* pick up the sequence record begin marker*/
        /* extract the tag byte*/
        if(!xdr_opaque(xdrs,tmp,sizeof(tmp))) return 0;
        if(tmp[0] == StartOfoclist) {
            /* skip instance*/
	    stat = ocskipinstance(content->node,xdrs);
            if(stat != OC_NOERR) return 0;
        } else if(tmp[0] == EndOfoclist) {
            break; /* done with the count*/
        } else {
            oc_log(LOGERR,"missing/invalid begin/end record marker\n");
	    return 0;
	}
    }

    /* move back to checkpoint position*/
    if(!xdr_setpos(xdrs,content->xdrpos.offset)) return 0;

    return count;
}

static OCmode
modetransition(OCnode* node, OCmode srcmode)
{
    switch (srcmode) {
    case Dimmode:
	if(node->octype == OC_Sequence) return Recordmode;
	if(node->octype == OC_Primitive) return Datamode;
	return Fieldmode;

    case Recordmode:
	if(node->array.rank > 0) return Dimmode;
	if(node->octype == OC_Primitive) return Datamode;
	return Fieldmode;

    case Datamode:
	return Datamode;

    case Nullmode:
    case Fieldmode:
	if(node->array.rank > 0) return Dimmode;
	if(node->octype == OC_Primitive) return Datamode;
	if(node->octype == OC_Sequence) return Recordmode;
	return Fieldmode;

    case Emptymode:
    default:
        OCPANIC1("No defined mode transition: %d",(int)srcmode);
	break;
    }
    return Fieldmode;
}

static size_t
maxindexfor(OCnode* node, OCmode srcmode)
{
    switch (srcmode) {
    case Dimmode:
	if(node->octype == OC_Sequence) return 0;
	if(node->octype == OC_Primitive) return 1;
	return oclistlength(node->subnodes);

    case Recordmode:
	if(node->array.rank > 0) return totaldimsize(node);
	if(node->octype == OC_Primitive) return 1;
	return oclistlength(node->subnodes);

    case Datamode:
	return 1;

    case Nullmode:
    case Fieldmode:
	if(node->array.rank > 0) return totaldimsize(node);
	if(node->octype == OC_Primitive) return 1;
	if(node->octype == OC_Sequence) return 0;
	return oclistlength(node->subnodes);

    case Emptymode:
    default:
        OCPANIC1("No defined mode transition: %d",(int)srcmode);
	break;
    }
    return oclistlength(node->subnodes);
}

OCmode
occontentmode(OCstate* conn, OCcontent* content) { return ((content)->mode);}

OCnode*
occontentnode(OCstate* conn, OCcontent* content) { return ((content)->node);}

size_t
occontentindex(OCstate* conn, OCcontent* content) { return ((content)->index);}

