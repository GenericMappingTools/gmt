/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "config.h"

#include "includes.h"

/*Forward*/
static int nccr_walk_Group(Group*, Group*, NClist*);
static int nccr_walk_Dimension(Group*, Dimension*, NClist*);
static int nccr_walk_Variable(Group*, Variable*, NClist*);
static int nccr_walk_Structure(Group*, Structure*, NClist*);
static int nccr_walk_Attribute(Group*, Attribute*, NClist*);
static int nccr_walk_EnumTypedef(Group*, EnumTypedef*, NClist*);
static int nccr_walk_EnumType(Group*, EnumType*, NClist*);
static void annotate(Group*, CRnode*, Sort, NClist*);

static void computepathname(CRnode* leaf);

static int testmagicnumber(bytes_t* packet, char* magicno, size_t* offsetp);
static int testpacketsize(bytes_t* packet, size_t* offsetp);

/**************************************************/
/* Define cdmremote magic numbers */

#define MAGIC_START  "\x43\x44\x46\x53" /* "CDFS" */
#define MAGIC_END    "\xed\xed\xde\xde"
#define MAGIC_HEADER "\xad\xec\xce\xda" 
#define MAGIC_DATA   "\xab\xec\xce\xba"
#define MAGIC_ERR    "\xab\xad\xba\xda"

int
nccr_cvtasterr(ast_err err)
{
    switch (err) {
    case AST_NOERR: return NC_NOERR;
    case AST_EOF: return NC_EINVAL;
    case AST_ENOMEM: return NC_ENOMEM;
    case AST_EFAIL: return NC_EINVAL /*generic*/;
    case AST_EIO: return NC_EIO;
    case AST_ECURL: return NC_ECURL;
    default: break;
    }
    return NC_EINVAL;
}

int
nccr_decodeheadermessage(bytes_t* packet, Header** hdrp)
{
    int ncstat = NC_NOERR;    
    ast_err status = AST_NOERR;
    ast_runtime* rt = NULL;
    Header* protohdr = NULL;
    size_t offset;
    bytes_t buf = *packet; /* So we can modify it */

    /* Check for optional MAGIC_START */
    offset = 0;
    testmagicnumber(&buf,MAGIC_START,&offset);    
    buf.nbytes -= offset; buf.bytes += offset;
   
    /* Check for MAGIC_HEADER */
    offset = 0;
    if(!testmagicnumber(&buf,MAGIC_HEADER,&offset)) {
	nclog(NCLOGERR,"Curl data too short: %d\n",packet->nbytes);
	ncstat = NC_ECURL;
	goto done;
    }
    buf.nbytes -= offset; buf.bytes += offset;

    /* pull out the packet length and verify */
    offset = 0;
    if(!testpacketsize(&buf,&offset)) {
	nclog(NCLOGERR,"Curl data size mismatch\n");
	THROWCHK((ncstat = NC_EINVAL));
	goto done;
    }
    buf.nbytes -= offset; buf.bytes += offset;

    /* Now decode the buffer */
    status = ast_byteio_new(AST_READ,buf.bytes,buf.nbytes,&rt);
    if(status != AST_NOERR) goto done;

    status = Header_read(rt,&protohdr);
    if(status != AST_NOERR) goto done;

    /* Verify optional MAGIC_END */
    offset = 0;
    testmagicnumber(&buf,MAGIC_END,&offset);
    if(ncstat) {THROWCHK(ncstat); goto done;}

    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto done;

    if(hdrp) *hdrp = protohdr;

done:
    if(status) ncstat =  nccr_cvtasterr(status);
    return ncstat;
}

/* Walk the Header tree to insert uid and sort capture all nodes */

static void
annotate(Group* parent, CRnode* node, Sort sort, NClist* nodes)
{
    if(nclistcontains(nodes,(ncelem)node)) return;
    memset((void*)node,0,sizeof(CRnode));
    node->uid = nclistlength(nodes);
    node->sort = sort;
    node->group = parent;
    nclistpush(nodes,(ncelem)node);
    if(parent == NULL && node->sort == _Group)
	node->flags.isroot = 1;
}

int
nccr_walk_Header(Header* hdr, NClist* nodes)
{
    int ncstat = NC_NOERR;
    annotate(NULL,(CRnode*)hdr,_Header,nodes);
    nccr_walk_Group(NULL,hdr->root,nodes);
    return ncstat;
}

static int
nccr_walk_Group(Group* parent, Group* node, NClist* nodes)
{
    int ncstat = NC_NOERR;
    int i;
    annotate(parent,(CRnode*)node,_Group,nodes);
    for(i=0;i<node->dims.count;i++) {
        ncstat = nccr_walk_Dimension(node,node->dims.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
	node->dims.values[i]->node.flags.isdecl = 1;
    }
    for(i=0;i<node->vars.count;i++) {
        nccr_walk_Variable(node,node->vars.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
    for(i=0;i<node->structs.count;i++) {
        nccr_walk_Structure(node,node->structs.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
    for(i=0;i<node->atts.count;i++) {
        nccr_walk_Attribute(node,node->atts.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
    for(i=0;i<node->groups.count;i++) {
        nccr_walk_Group(node,node->groups.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
    for(i=0;i<node->enumTypes.count;i++) {
        nccr_walk_EnumTypedef(node,node->enumTypes.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }

done:
    return ncstat;
}

static int
nccr_walk_Dimension(Group* parent, Dimension* node, NClist* nodes)
{
    int ncstat = NC_NOERR;
    annotate(parent,(CRnode*)node,_Dimension,nodes);

    return ncstat;
}

static int
nccr_walk_Variable(Group* parent, Variable* node, NClist* nodes)
{
    int ncstat = NC_NOERR;
    int i;
    annotate(parent,(CRnode*)node,_Variable,nodes);
    for(i=0;i<node->shape.count;i++) {
        nccr_walk_Dimension(parent,node->shape.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
    for(i=0;i<node->atts.count;i++) {
        nccr_walk_Attribute(parent,node->atts.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
done:
    return ncstat;
}

static int
nccr_walk_Structure(Group* parent, Structure* node, NClist* nodes)
{
    int ncstat = NC_NOERR;
    int i;
    annotate(parent,(CRnode*)node,_Dimension,nodes);
    for(i=0;i<node->shape.count;i++) {
        nccr_walk_Dimension(parent,node->shape.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
    for(i=0;i<node->atts.count;i++) {
        nccr_walk_Attribute(parent,node->atts.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
    for(i=0;i<node->vars.count;i++) {
        nccr_walk_Variable(parent,node->vars.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
    for(i=0;i<node->structs.count;i++) {
        nccr_walk_Structure(parent,node->structs.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }
done:
    return ncstat;
}

static int
nccr_walk_Attribute(Group* parent, Attribute* node, NClist* nodes)
{
    int ncstat = NC_NOERR;
    annotate(parent,(CRnode*)node,_Attribute,nodes);

    return ncstat;
}

static int
nccr_walk_EnumTypedef(Group* parent, EnumTypedef* node, NClist* nodes)
{
    int ncstat = NC_NOERR;
    int i;
    annotate(parent,(CRnode*)node,_EnumTypedef,nodes);
    for(i=0;i<node->map.count;i++) {
        nccr_walk_EnumType(parent,node->map.values[i],nodes);
        if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    }

done:
    return ncstat;
}

static int
nccr_walk_EnumType(Group* parent, EnumType* node, NClist* nodes)
{
    int ncstat = NC_NOERR;
    annotate(parent,(CRnode*)node,_EnumType,nodes);

    return ncstat;
}

/**************************************************/

int
nccr_compute_pathnames(NClist* nodes)
{
    int ncstat = NC_NOERR;
    int i;
    /* Compute the pathname for selected nodes */
    for(i=0;i<nclistlength(nodes);i++) {
        CRnode* node = (CRnode*)nclistget(nodes,i);
	switch (node->sort) {
	case _Dimension:
	case _Variable:
	case _Structure:
	case _EnumType:
	case _Group:
	    computepathname(node);
	    break;
	default:
	   break; /* ignore */
	}
    }

    return ncstat;
}

static void
computepathname(CRnode* leaf)
{
    int i;
    CRnode* node;
    NClist* path = nclistnew();
    char* name;

    for(node=leaf;;) {
	if(node->flags.isroot) break;
        nclistinsert(path,0,(ncelem)node);
	node = (CRnode*)node->group;
    }

    leaf->pathname = NULL;
    for(i=0;i<nclistlength(path);i++) {
	node = (CRnode*)nclistget(path,i);
	name = nccr_getname(node);
	if(name == NULL) goto done; /* node has no meaningful name */
	leaf->pathname = crpathappend(leaf->pathname,name);
    }    
    nclistfree(path);
done:
    return;
}

char*
nccr_getname(CRnode* node)
{
    switch(node->sort) {
    case _Attribute:
	return ((Attribute*)node)->name;
    case _Dimension:
	return ((Dimension*)node)->name.defined?((Dimension*)node)->name.value:NULL;
    case _Variable:
	return ((Variable*)node)->name;
    case _Structure:
	return ((Structure*)node)->name;
    case _EnumTypedef:
	return ((EnumTypedef*)node)->name;
    case _EnumType:
	return ((EnumType*)node)->value;
    case _Group:
	return ((Group*)node)->name;
    case _Header:
	return "";
    default:
	return NULL;
    }
}

DataType
nccr_gettype(CRnode* node)
{
    switch(node->sort) {
    case _Attribute:
	return ((Attribute*)node)->type;
    case _Variable:
	return ((Variable*)node)->dataType;
    case _Structure:
	return ((Structure*)node)->dataType;
    default:
	return -1;
    }
}

/**************************************************/

/* Map dimension refs to matching decl; if the match sizes
   are different, fail.
*/
int
nccr_map_dimensions(NClist* nodes)
{
    int ncstat = NC_NOERR;
    int i;
    NClist* dimdecls = nclistnew();
    /* Collect dimension decls */
    for(i=0;i<nclistlength(nodes);i++) {
        CRnode* node = (CRnode*)nclistget(nodes,i);
	if(node->sort == _Dimension  && node->flags.isdecl)
	    nclistpush(dimdecls,(ncelem)node);
    }

    /* Map dimension refs to matching decl */
    for(i=0;i<nclistlength(nodes);i++) {
	int j;
	Dimension* dim;
        CRnode* node = (CRnode*)nclistget(nodes,i);
	switch (node->sort) {
	case _Dimension:
	    /* Map dimension references to the corresponding decl */
	    dim = (Dimension*)node;
	    for(j=0;j<nclistlength(dimdecls);j++) {
	        Dimension* decl = (Dimension*)nclistget(dimdecls,j);
		if(decl == dim) continue;
		if(!crpathmatch(decl->node.pathname,dim->node.pathname))
		    continue;
	        /* Validate that these are really the same dimension */
		if(classifydim(decl) == classifydim(dim)) {
		    if(dimsize(decl) == dimsize(dim)) {
		        node->dimdecl = (Dimension*)decl;
		    } else goto fail;
		} else goto fail;
	    }
	    ASSERT(node->flags.isdecl || node->dimdecl != NULL);
	    break;
	default:
	   break; /* ignore */
	}
    }

    nclistfree(dimdecls);
    return ncstat;

fail:
    return THROW(NC_EINVALCOORDS);
}

/**************************************************/

/* Replace all dimension references with their corrsponding
   dimension decl
*/

void
nccr_deref_dimensions(NClist* nodes)
{
    int i,j;
    NClist* replaced = nclistnew();
    for(i=0;i<nclistlength(nodes);i++) {
        CRnode* node = (CRnode*)nclistget(nodes,i);
	size_t count = 0;
	Dimension** dimset = NULL;
	switch (node->sort) {
	case _Variable:
	    count = ((Variable*)node)->shape.count;
	    dimset = ((Variable*)node)->shape.values;
	    break;
	case _Structure:
	    count = ((Structure*)node)->shape.count;
	    dimset = ((Structure*)node)->shape.values;
	    break;
	default:
	    break;
	}

	/* Do the replacement */
	if(count > 0 && dimset != NULL) {
	    for(j=0;j<count;j++) {
		if(!dimset[j]->node.flags.isdecl) {
		    nclistpush(replaced,(ncelem)dimset[j]);
	            dimset[j] = dimset[j]->node.dimdecl;
		}
	    }
	}
    }
    /*Remove the replaced from the nodeset */
    for(i=0;i<nclistlength(replaced);i++) {
        CRnode* repl = (CRnode*)nclistget(replaced,i);
        for(j=0;j<nclistlength(nodes);j++) {
            CRnode* node = (CRnode*)nclistget(nodes,j);
	    if(node == repl) {nclistremove(nodes,j); break;}
	}
    }
    nclistfree(replaced);
}

static int
testmagicnumber(bytes_t* packet, char* magicno, size_t* offsetp)
{
    size_t offset = 0;
    size_t magiclen = strlen(magicno);

    /* Check the structure of the resulting data */
    if(packet->nbytes < magiclen) return 0;

    if(memcmp(packet->bytes,magicno,magiclen) != 0) return 0;

    offset = magiclen;
    if(offsetp) *offsetp = offset;
    return 1;
}

static int
testpacketsize(bytes_t* packet, size_t* offsetp)
{
    unsigned long long vlen;
    size_t offset = 0;
    size_t size;

    /* Extract the proposed count as a varint */
    vlen = varint_decode(packet->nbytes,packet->bytes,&size);
    offset += size;
    if(vlen != (packet->nbytes-offset)) return 0;
    *offsetp = offset;
    return 1;
}


/**************************************************/

/* Extract the Data object and return the offset
   in the buffer where data starts plus its expected
   length.
*/
int
nccr_decodedatamessage(bytes_t* packet, Data** datahdrp, size_t* datastart)
{
    int ncstat = NC_NOERR;    
    ast_err status = AST_NOERR;
    ast_runtime* rt = NULL;
    Data* datahdr = NULL;
    size_t delta, datahdrlen, cumoffset, pos0;
    bytes_t buf = *packet;

    cumoffset = 0;

    /* Check for optional MAGIC_START */
    delta = 0;
    (void)testmagicnumber(&buf,MAGIC_START,&delta);    
    buf.nbytes -= delta; buf.bytes += delta;
    cumoffset += delta;

    /* Check for MAGIC_DATA */
    delta = 0;
    if(!testmagicnumber(&buf,MAGIC_DATA,&delta)) {
	nclog(NCLOGERR,"MAGIC_DATA not found\n");
	THROWCHK((ncstat = NC_EINVAL));
	goto done;
    }
    buf.nbytes -= delta; buf.bytes += delta;
    cumoffset += delta;

    /* pull out the Data Header length */
    delta = 0;
    datahdrlen = 0;
    ncstat = nccr_decodedatacount(&buf,&delta,&datahdrlen);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    buf.nbytes -= delta; buf.bytes += delta;
    cumoffset += delta;

    /* Now decode the buffer; but tracking the buffer position */

    status = ast_byteio_new(AST_READ,buf.bytes,buf.nbytes,&rt);
    if(status != AST_NOERR) goto done;

    /* Capture the position before reading the Data Header */
    status = ast_byteio_count(rt,&pos0);
    if(status != AST_NOERR) goto done;	

    /* Mark the rt with the datahdrlen */
    ast_mark(rt,datahdrlen);

    status = Data_read(rt,&datahdr);
    if(status != AST_NOERR) goto done;

    ast_unmark(rt);

    /* Capture the position after reading the Data Header */
    status = ast_byteio_count(rt,&delta);
    if(status != AST_NOERR) goto done;	

    /* get true delta */
    delta = (delta - pos0);

    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto done;

    buf.nbytes -= delta; buf.bytes += delta;
    cumoffset += delta;
    
#ifdef IGNORE
    /* Read the data vlen at that position */
    delta = 0;
    ncstat = nccr_decodedatacount(&buf,&delta,datalen);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}
    buf.nbytes -= delta; buf.bytes += delta;
    cumoffset += delta;
#endif

    if(datastart) {
	/* save cumulative offset */
	*datastart = cumoffset;
    }

    if(datahdrp) *datahdrp = datahdr;

done:
    if(status) ncstat =  nccr_cvtasterr(status);
    return ncstat;
}

int
nccr_decodedatacount(bytes_t* buf, size_t* offsetp, size_t* countp)
{
    int ncstat = NC_NOERR;    
    ast_err status = AST_NOERR;
    size_t offset = *offsetp;
    size_t count = 0;
    unsigned long long vlen;

    /* Extract a vlen int indicating the length of the vdata */
    vlen = varint_decode(buf->nbytes-offset,buf->bytes+offset,&offset);
    count = (size_t)vlen;
    /* return some values */
    if(offsetp) *offsetp = offset;
    if(countp) *countp = count;

    if(status) ncstat =  nccr_cvtasterr(status);
    return ncstat;
}

