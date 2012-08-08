/*
 * Copyright 1993-1996 University Corporation for Atmospheric Research/Unidata
 * 
 * Portions of this software were developed by the Unidata Program at the 
 * University Corporation for Atmospheric Research.
 * 
 * Access and use of this software shall impose the following obligations
 * and understandings on the user. The user is granted the right, without
 * any fee or cost, to use, copy, modify, alter, enhance and distribute
 * this software, and any derivative works thereof, and its supporting
 * documentation for any purpose whatsoever, provided that this entire
 * notice appears in all copies of the software, derivative works and
 * supporting documentation.  Further, UCAR requests that the user credit
 * UCAR/Unidata in any publications that result from the use of this
 * software or in any product that includes this software. The names UCAR
 * and/or Unidata, however, may not be used in any advertising or publicity
 * to endorse or promote any products or commercial entity unless specific
 * written permission is obtained from UCAR/Unidata. The user also
 * understands that UCAR/Unidata is not obligated to provide the user with
 * any support, consulting, training or assistance of any kind with regard
 * to the use, operation and performance of this software nor to provide
 * the user with any updates, revisions, new versions or "bug fixes."
 * 
 * THIS SOFTWARE IS PROVIDED BY UCAR/UNIDATA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL UCAR/UNIDATA BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE ACCESS, USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "config.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>

#include "nc_meta.h"

MetaNode*
NC_meta_alloc(nc_meta nodeclass)
{
    MetaNode* node = (MetaNode*)calloc(1,sizeof(MetaNode));
    if(node != null) {
	node->nodeclass = nodeclass;
    }
    return node;
}

MetaNode**
NC_meta_allocn(nc_meta nodeclass, int count)
{
    int i,j;
    MetaNode** nodelist = (MetaNode**)calloc(count,sizeof(MetaNode*));
    if(nodelist != null) { /* cleanup */
	for(i=0;i<count;i++) {
	    nodelist[i] = nc_meta_alloc(nodeclass);
	    if(nodelist[i] == NULL) {
   	        for(j=0;j<i;j++) nc_meta_free(nodelist[j]);
	    }
	    free(nodelist);
	    nodelist = NULL;
	    break;
	}
    }
    return nodelist;
}

void
NC_meta_free(MetaNode* node)
{
    int i;
    if(node == NULL) return;
    switch(node->nodeclass) {
    case NC_ENUM:
        for(i=0;i<node->nelems;i++) {
	    if(node->econsts[i].name != NULL)
		free(node->econsts[i].name);
	}
	if(node->consts != null) free(node->econsts);
	break;
    case NC_COMPOUND:
	if(node->compound.fields != null) {
            for(i=0;i<node->nelems;i++) {
	        NC_meta_free(node->compound.fields[i]);
	    }
	    free(node->compound.fields);
	}
	break;
    case NC_FIELD:
	break;
    default:
	free((void*)node; break;
    }
}

/**
 Invoke nc_inq functions to fill in a type node
 */

int
NC_meta_create_type(int ncid, nc_type xtype,
		    MetaNode* root, MetaNode** typenodep)
{
    /* Walk the node set to ensure that the type does not already exist */
    int ncstat = NC_NOERR;
    MetaNode* node = locatemetatype(xtype,root);
    if(node == NULL) {
	ncstat = NC_meta_create_type1(ncid,xtype,root,typenodep);
    } else if(typenodep != NULL)
	*typenodep = node;
    return ncstat;
}

static int
NC_meta_create_type1(int ncid, nc_type xtype,
		    MetaNode* root, MetaNode** typenodep)
{
    int ncstat = NC_NOERR;
    int i;
    MetaNode* node;
    nc_meta metatype = NC_NAT;

    /* Determine the top-level type for xtype */
    ncstat = mapnctype(ncid,xtype,&metatype);
    if(ncstat != NC_NOERR) goto done;    

    node = NC_meta_alloc(metatype);
    node->ncid = ncid;
    node->xtype = xtype;

    switch (node->nodeclass) {

    case NC_ATOMIC:
	ncstat = nc_inq_type(ncid,xtype,node->name,&node->size);
	if(ncstat != NC_NOERR) goto fail;
	break;

    case NC_OPAQUE:
	ncstat = nc_inq_opaque(ncid,xtype,node->name,&node->size);
	if(ncstat != NC_NOERR) goto fail;
	break;

    case NC_ENUM:
	ncstat = nc_inq_enum(ncid,xtype,node->name,&node->basetype,
				 &node->size,&node->nelems);
	if(ncstat != NC_NOERR) goto fail;
	/* Now, create and fill in the enum constants */
        node->econsts  = (struct MetaEconst*)calloc(node->nelems,
                                                    sizeof(struct MetaEconst));
	if(node->econsts == NULL) {ncstat = NC_ENOMEM;goto fail;}
	for(i=0;i<node->nelems;i++) {
	    struct MetaEconst* econst = node->consts+i;
	    ncstat = nc_inq_enum_member(ncid,xtype,i,
                                           econst->name,
                                           (void*)econst->value);
  	    if(ncstat != NC_NOERR) goto fail;
        }
	break;

    case NC_COMPOUND:
	ncstat = nc_inq_compound(ncid,xtype,node->name,&node->size,
				 &node->nelems);
	if(ncstat != NC_NOERR) goto fail;
	/* Now, create and fill in the fields */
        node->compound.fields = nc_meta_allocn(NC_FIELD,node->nelems);
	if(node->compound.fields == NULL) {ncstat = NC_ENOMEM;goto fail;}
	for(i=0;i<node->nelems;i++) {
	    MetaNode* field = node->compound.fields+i;
	    nc_type basetype;
	    ncstat = nc_inq_compound_field(ncid,xtype,i,
                                           field->name,
                                           &field->offset,
                                           &basetype,
					   &field->ndims,
					   field->dims);
  	    if(ncstat != NC_NOERR) goto fail;
	    /* create basetype */
	    ncstat = NC_meta_create_type(ncid,basetype,root,&field->basetype);
  	    if(ncstat != NC_NOERR) goto fail;
        }
	break;

    case NC_VLEN: {
        nc_type basetype;
	ncstat = nc_inq_vlen(ncid,xtype,node->name,&node->size,&basetype);
	if(ncstat != NC_NOERR) goto fail;
	/* create basetype */
	ncstat = NC_meta_create_type(ncid,basetype,root,&node->basetype);
  	if(ncstat != NC_NOERR) goto fail;
	} break;

    case NC_GROUP: {
	nc_type* typeids;
	ncstat = nc_inq_typeids(ncid,&node->nelems,NULL);
	if(ncstat != NC_NOERR) goto fail;
	typeids = (nc_type*)calloc(node->nelems,sizeof(nc_type));
	if(typeids == NULL) {ncstat = NC_ENOMEM; goto fail;}
	ncstat = nc_inq_typeids(ncid,&node->nelems,typeids);
	if(ncstat != NC_NOERR) goto fail;
	break;

    default: abort();
    }        
}

static int
mapnctype(int ncid, nc_type xtype, nc_meta* metatypep)
{
    int ncstat = NC_NOERR;
    if(metatypep == NULL) goto done;
    if(xtype <= NC_MAX_ATOMIC_TYPE) goto done;
    ncstat = nc_inq_user_type(ncid,xtype,NULL,NULL,NULL,NULL,metatypep);
done:
    return ncstat;
}

static MetaNode*
locatemetatype(nc_type typeid, MetaNode* root)
{
    int i;
    for(i=0;i<nclistlength(root->root.typeset);i++) {
	MetaNode* node = (MetaNode*)nclistget(root->root.typeset,i);
	if(node->typeid == xtype) {
	    return node;
	}
    }
    return NULL;
}

/**
 * Given
 */

int
NC_meta_computesize(MetaNode* typenode, void* data)
{
    int ncstat = NC_NOERR;
    int i;

    switch (node->nodeclass) {

    case NC_ATOMIC:
	ncstat = nc_inq_type(ncid,xtype,node->name,&node->size);
	if(ncstat != NC_NOERR) goto fail;
	break;

    case NC_OPAQUE:
	ncstat = nc_inq_opaque(ncid,xtype,node->name,&node->size);
	if(ncstat != NC_NOERR) goto fail;
	break;

    case NC_ENUM:
	ncstat = nc_inq_enum(ncid,xtype,node->name,&node->basetype,
				 &node->size,&node->nelems);
	if(ncstat != NC_NOERR) goto fail;
	/* Now, create and fill in the enum constants */
        node->econsts  = (struct MetaEconst*)calloc(node->nelems,
                                                    sizeof(struct MetaEconst));
	if(node->econsts == NULL) {ncstat = NC_ENOMEM;goto fail;}
	for(i=0;i<node->nelems;i++) {
	    struct MetaEconst* econst = node->consts+i;
	    ncstat = nc_inq_enum_member(ncid,xtype,i,
                                           econst->name,
                                           (void*)econst->value);
  	    if(ncstat != NC_NOERR) goto fail;
        }
	break;

    case NC_COMPOUND:
	ncstat = nc_inq_compound(ncid,xtype,node->name,&node->size,
				 &node->nelems);
	if(ncstat != NC_NOERR) goto fail;
	/* Now, create and fill in the fields */
        node->compound.fields = nc_meta_allocn(NC_FIELD,node->nelems);
	if(node->compound.fields == NULL) {ncstat = NC_ENOMEM;goto fail;}
	for(i=0;i<node->nelems;i++) {
	    MetaNode* field = node->compound.fields+i;
	    nc_type basetype;
	    ncstat = nc_inq_compound_field(ncid,xtype,i,
                                           field->name,
                                           &field->offset,
                                           &basetype,
					   &field->ndims,
					   field->dims);
  	    if(ncstat != NC_NOERR) goto fail;
	    /* create basetype */
	    ncstat = NC_meta_create_type(ncid,basetype,root,&field->basetype);
  	    if(ncstat != NC_NOERR) goto fail;
        }
	break;

    case NC_VLEN: {
        nc_type basetype;
	ncstat = nc_inq_vlen(ncid,xtype,node->name,&node->size,&basetype);
	if(ncstat != NC_NOERR) goto fail;
	/* create basetype */
	ncstat = NC_meta_create_type(ncid,basetype,root,&node->basetype);
  	if(ncstat != NC_NOERR) goto fail;
	} break;

    default: break;
    }        
    return ncstat;
}
