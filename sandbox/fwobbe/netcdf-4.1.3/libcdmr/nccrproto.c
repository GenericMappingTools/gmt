/*********************************************************************
 *   Copyright 2010, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Id$
 *   $Header$
 *********************************************************************/

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "nclist.h"
#include "ncbytes.h"
#include "nclog.h"

#include "netcdf.h"
#include "ast.h"
#include "crdebug.h"
#include "nccrnode.h"
#include "ncStreamx.h"

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
static char* getname(CRnode* node);

static int skiptoheader(bytes_t* packet, size_t* offsetp);

/**************************************************/
/* Define cdmremote magic numbers */

#define MAGIC_START  "\x43\x44\x46\x53"
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
nccr_decodeheader(bytes_t* packet, Header** hdrp)
{
    int ncstat = NC_NOERR;    
    ast_err status = AST_NOERR;
    ast_runtime* rt = NULL;
    Header* protohdr = NULL;
    size_t offset;

    /* Skip to the beginning of header */
    ncstat = skiptoheader(packet,&offset);
    if(ncstat != NC_NOERR) {THROWCHK(ncstat); goto done;}

    /* Now decode the buffer */
    status = ast_byteio_new(AST_READ,packet->bytes+offset,packet->nbytes-offset,&rt);
    if(status != AST_NOERR) goto done;

    status = Header_read(rt,&protohdr);
    if(status != AST_NOERR) goto done;

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
    NCbytes* accum = NULL;
    NClist* path = NULL;
    CRnode* node;

    /**
     * This is a little tricky.
     * In order to produce a pathname
     * that matches what is send by the server,
     * we need to not start the pathname with ".".
     */
    leaf->pathname = NULL;
    path = nclistnew();
    for(node=leaf;;) {
	if(node->flags.isroot) break;
        nclistinsert(path,0,(ncelem)node);
	node = (CRnode*)node->group;
    }

    accum = ncbytesnew();
    ncbytesnull(accum);
    for(i=0;i<nclistlength(path);i++) {
	node = (CRnode*)nclistget(path,i);
	char* name = getname(node);
	if(name == NULL) goto done; /* node has no meaningful name */
	if(i > 0) ncbytescat(accum,".");
	ncbytescat(accum,name);
    }    
    leaf->pathname = ncbytesextract(accum);
    ncbytesfree(accum);
    nclistfree(path);
done:
    return;
}

static char*
getname(CRnode* node)
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
	    dim = (Dimension*)node;
	    for(j=0;j<nclistlength(dimdecls);j++) {
	        Dimension* decl = (Dimension*)nclistget(dimdecls,j);
		if(decl != dim
		   && strcmp(decl->node.pathname,dim->node.pathname)==0) {
		    /* Validate that these are really the same dimension */
		    if(classifydim(decl) == classifydim(dim)
		       && dimsize(decl) == dimsize(dim)) {
		        node->dimdecl = (Dimension*)decl;
		    } else {/* Fail */
			return THROW(NC_EINVALCOORDS);
			goto done;
		    }
		}
	    }
	    ASSERT(node->dimdecl != NULL);
	    break;
	default:
	   break; /* ignore */
	}
    }

done:
    nclistfree(dimdecls);
    return ncstat;
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

	if(count > 0 && dimset != NULL) {
	    for(j=0;j<count;j++)
		nclistpush(replaced,(ncelem)dimset[j]);
	        dimset[j] = dimset[j]->node.dimdecl;
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
skiptoheader(bytes_t* packet, size_t* offsetp)
{
    int status = NC_NOERR;
    unsigned long long vlen;
    size_t size,offset;

    /* Check the structure of the resulting data */
    if(packet->nbytes < (strlen(MAGIC_HEADER) + strlen(MAGIC_HEADER))) {
	nclog(NCLOGERR,"Curl data too short: %d\n",packet->nbytes);
	status = NC_ECURL;
	goto done;
    }
    if(memcmp(packet->bytes,MAGIC_HEADER,strlen(MAGIC_HEADER)) != 0) {
	nclog(NCLOGERR,"MAGIC_HEADER missing\n");
	status = NC_ECURL;
	goto done;
    }
    offset = strlen(MAGIC_HEADER);
    /* Extract the proposed count as a varint */
    vlen = varint_decode(10,packet->bytes+offset,&size);
    offset += size;
    if(vlen != (packet->nbytes-offset)) {
	nclog(NCLOGERR,"Curl data size mismatch\n");
	status = NC_ECURL;
	goto done;
    }
    if(offsetp) *offsetp = offset;

done:
    return status;    
}
