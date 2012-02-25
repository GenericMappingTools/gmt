/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/cdf3.c,v 1.33 2009/12/03 03:42:37 dmh Exp $
 *********************************************************************/

#include "ncdap3.h"
#include "daputil.h"
#include "dapdump.h"

CDFnode* v4node = NULL;

/* Define a local structure */
#ifdef IGNORE
struct NCregrid {
    CDFnode* template;
    CDFnode* newgrid;
    int  fieldindex;
};
#endif

/* Forward*/
static NCerror sequencecheck3r(CDFnode* node, NClist* vars, CDFnode* topseq);
static NCerror regrid3r(CDFnode*, CDFnode*, NClist*);
static NCerror testregrid3(CDFnode* node, CDFnode* template, NClist*);
static CDFnode* makenewstruct3(CDFnode* node, CDFnode* template);
static NCerror regridinsert(CDFnode* newgrid, CDFnode* node);
static NCerror regridremove(CDFnode* newgrid, CDFnode* node);
static NCerror imprint3r(CDFnode*, CDFnode*, int depth);
static NCerror imprintdims3(CDFnode*, CDFnode*);
static void projection3r(CDFnode*);
static void unprojected3(NClist* nodes);
static void projectall3(NClist* nodes);

/* Accumulate useful node sets  */
NCerror
computecdfnodesets3(NCDAPCOMMON* nccomm)
{
    unsigned int i;
    NClist* varnodes = nclistnew(); 
    NClist* allnodes = nccomm->cdf.ddsroot->tree->nodes;

    if(nccomm->cdf.seqnodes == NULL) nccomm->cdf.seqnodes = nclistnew();
    if(nccomm->cdf.gridnodes == NULL) nccomm->cdf.gridnodes = nclistnew();
    nclistclear(nccomm->cdf.seqnodes);
    nclistclear(nccomm->cdf.gridnodes);

    computevarnodes3(nccomm,allnodes,varnodes);
    nclistfree(nccomm->cdf.varnodes);
    nccomm->cdf.varnodes = varnodes;

    /* Now compute other sets of interest */
    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(allnodes,i);
	if(!node->visible) continue;
	switch (node->nctype) {
	case NC_Sequence:
	    nclistpush(nccomm->cdf.seqnodes,(ncelem)node);
	    break;
	case NC_Grid:
	    nclistpush(nccomm->cdf.gridnodes,(ncelem)node);
	    break;
	default: break;
	}
    }
    return NC_NOERR;
}

NCerror
computevarnodes3(NCDAPCOMMON* nccomm, NClist* allnodes, NClist* varnodes)
{
    unsigned int i,len;
    NClist* allvarnodes = nclistnew();
    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(allnodes,i);
	/* If this node has a bad name, make it invisible */
	if(dap_badname(node->name)) node->visible=0;
	if(!node->visible) continue;
	if(node->nctype == NC_Primitive)
	    nclistpush(allvarnodes,(ncelem)node);
    }
    /* Further process the variable nodes to get the final set */
    /* Use toplevel vars first */
    len = nclistlength(allvarnodes);
    for(i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(allvarnodes,i);
	if(node == NULL) continue;
        if(daptoplevel(node)) {
	    nclistpush(varnodes,(ncelem)node);
	    nclistset(allvarnodes,i,(ncelem)NULL);
	}
    }
    /*... then grid arrays and maps.
      but exclude the coordinate variables if we are trying to
      exactly mimic nc-dap
    */
    for(i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(allvarnodes,i);
	if(node == NULL) continue;
	if(dapgridarray(node)) {
	    nclistpush(varnodes,(ncelem)node);
	    nclistset(allvarnodes,i,(ncelem)NULL);
        } else if(dapgridmap(node)) {
	    if(!FLAGSET(nccomm->controls,NCF_NCDAP))
		nclistpush(varnodes,(ncelem)node);
	    nclistset(allvarnodes,i,(ncelem)NULL);
	}
    }
    /*... then all others */
    for(i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(allvarnodes,i);
	if(node == NULL) continue;
        nclistpush(varnodes,(ncelem)node);
    }
    nclistfree(allvarnodes);
    return NC_NOERR;
}

#ifdef IGNORE
static void
computecdfattributes(CDFnode* node)
{
    NClist* path = nclistnew();
    if(node->nctype == NC_Primitive) {
        /* Add in copies of the attributes of all parent nodes*/
        collectnodepath(node->container,path,WITHDATASET);
    }
    nclistfree(path);
}
#endif

NCerror
fixgrids3(NCDAPCOMMON* nccomm)
{
    unsigned int i;
    NClist* gridnodes = nccomm->cdf.gridnodes;

    for(i=0;i<nclistlength(gridnodes);i++) {
        CDFnode* grid = (CDFnode*)nclistget(gridnodes,i);
        (void)fixgrid34(nccomm,grid);
	/* Ignore mal-formed grids */
    }
    return NC_NOERR;
}

/*
Figure out the names for variables.
*/
NCerror
computecdfvarnames3(NCDAPCOMMON* nccomm, CDFnode* root, NClist* varnodes)
{
    unsigned int i,j,d;

    /* clear all elided marks; except for dataset and grids */
    for(i=0;i<nclistlength(root->tree->nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(root->tree->nodes,i);
	node->elided = 0;
	if(node->nctype == NC_Grid || node->nctype == NC_Dataset)
	    node->elided = 1;
    }

    /* ensure all variables have an initial full name defined;
    */
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(varnodes,i);
	nullfree(var->ncfullname);
	var->ncfullname = makecdfpathstring3(var,nccomm->cdf.separator);
    }
    /*  unify all variables with same fullname and dimensions. */
    if(FLAGSET(nccomm->controls,NCF_NC3)) {
        for(i=0;i<nclistlength(varnodes);i++) {
	    int match;
	    CDFnode* var = (CDFnode*)nclistget(varnodes,i);
	    for(j=0;j<i;j++) {
	        CDFnode* testnode = (CDFnode*)nclistget(varnodes,j);
		match = 1;
	        if(testnode->array.basevar != NULL)
		    continue; /* already processed */
	        if(strcmp(var->ncfullname,testnode->ncfullname) != 0)
		    match = 0;
		else if(nclistlength(testnode->array.dimensions)
			!= nclistlength(var->array.dimensions))
		    match = 0;
	        else for(d=0;d<nclistlength(testnode->array.dimensions);d++) {
		    CDFnode* vdim = (CDFnode*)nclistget(var->array.dimensions,d);
		    CDFnode* tdim = (CDFnode*)nclistget(testnode->array.dimensions,d);
	            if(vdim->dim.declsize != tdim->dim.declsize) {
		        match = 0;
			break;
		    }
		}
		if(match) {
		    testnode->array.basevar = var;
		}
	    }
	}
    }
    /* Remove elided marks */
    for(i=0;i<nclistlength(root->tree->nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(root->tree->nodes,i);
	node->elided = 0;
    }
    /* Finally, verify unique names */
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var1 = (CDFnode*)nclistget(varnodes,i);
	if(var1->array.basevar != NULL) continue;
	for(j=0;j<i;j++) {
	    CDFnode* var2 = (CDFnode*)nclistget(varnodes,j);
	    if(var2->array.basevar != NULL) continue;
	    if(strcmp(var1->ncfullname,var2->ncfullname)==0) {
		PANIC1("duplicate var names: %s",var1->ncfullname);
	    }
	}
    }
    return NC_NOERR;
}

#ifdef IGNORE
static NClist*
hoist(NCDAPCOMMON* nccomm, CDFnode* container)
{
    int i,j;
    NClist* containers = nclistnew();
    NClist* unique = nclistnew();
    /* Divide the subnodes of this node into containers and unique*/
    for(i=0;i<nclistlength(container->subnodes);i++) {
	CDFnode* sub = (CDFnode*)nclistget(container->subnodes,i);
	switch (sub->nctype) {
	case NC_Structure:
	case NC_Sequence:
	case NC_Grid:
	case NC_Dataset:
	    nclistpush(containers,(ncelem)sub);
	    break;
	case NC_Primitive:
	    nclistpush(unique,(ncelem)sub);
	    break;
	default: PANIC1("computecdfvarnames: bad node type: %d",sub->nctype);
	}
    }

    /* Tentatively hoist each container in turn*/
    while(nclistlength(containers) > 0) {
	CDFnode* subcontainer = (CDFnode*)nclistremove(containers,0);	
	NClist* vars = hoist(nccomm,subcontainer);
	int match;
	/* compute path names without this container*/
	subcontainer->elided = 1;
	if(ncdap3debug > 1) fprintf(stderr,"eliding: %s\n",subcontainer->name);
	makevarnames(nccomm,vars);
	/* look for duplicates in the unique list*/
	match = 0;
        for(i=0;i<nclistlength(unique);i++) {
	    CDFnode* unode = (CDFnode*)nclistget(unique,i);
            for(j=0;j<nclistlength(vars);j++) {
	        CDFnode* var = (CDFnode*)nclistget(vars,j);
		if(strcmp(var->ncfullname,unode->ncfullname)==0) {
		    match = 1;
		    if(ncdap3debug > 1) fprintf(stderr,"match: %s\n",var->ncfullname);
		    break;
		}
	    }
	    if(match) break;
	}
	if(match) {
	    /* Since our goal is to hoist all the vars in a compound type or none,*/
            /* match => we have a collision, so restore the path name of the vars*/
	    /* to include their container*/
	    subcontainer->elided = 0;
	    makevarnames(nccomm,vars);
	}
	/* Add the subcontainer vars to our list of uniquely named vars*/
        for(i=0;i<nclistlength(vars);i++) {
	    CDFnode* var = (CDFnode*)nclistget(vars,i);
	    nclistpush(unique,(ncelem)var);
	}
	nclistfree(vars);
    }
    nclistfree(containers);
    return unique;
}
#endif

#ifdef UNUSED
static void
makevarnames(NCDAPCOMMON* nccomm, NClist* vars)
{
    int i;
    for(i=0;i<nclistlength(vars);i++) {
	CDFnode* var = (CDFnode*)nclistget(vars,i);
	nullfree(var->ncfullname);
        var->ncfullname = makecdfpathstring3(var,nccomm->cdf.separator);
if(var==v4node && var->ncfullname[0] != 'Q')dappanic("");
	if(ncdap3debug > 1)
	    fprintf(stderr,"makevarname: %s->ncfullname=%s\n",var->name,var->ncfullname);
    }
}
#endif

/* locate and connect usable sequences and vars*/
NCerror
sequencecheck3(NCDAPCOMMON* nccomm)
{
    (void)sequencecheck3r(nccomm->cdf.ddsroot,nccomm->cdf.varnodes,NULL);    
    return NC_NOERR;
}


static NCerror
sequencecheck3r(CDFnode* node, NClist* vars, CDFnode* topseq)
{
    unsigned int i;
    NCerror err = NC_NOERR;
    int ok = 0;
    if(topseq == NULL && nclistlength(node->array.dimensions) > 0) {
	err = NC_EINVAL;
    } else if(node->nctype == NC_Sequence) {
	/* recursively compute usability */
	for(i=0;i<nclistlength(node->subnodes);i++) {
	    CDFnode* sub = (CDFnode*)nclistget(node->subnodes,i);
	    err = sequencecheck3r(sub,vars,node);
	    if(err == NC_NOERR) ok = 1; /* there is at least 1 usable var below */
	}
	if(topseq == NULL && ok == 1) {
	    /* this sequence is usable */
	    err = NC_NOERR;
	    node->usesequence = 1;
	} else {
	    /* this sequence is unusable */
	    node->usesequence = 0;
	    err = NC_EINVAL;
	}
    } else if(nclistcontains(vars,(ncelem)node)) {
	node->array.sequence = topseq;
    } else {
	/* recursively compute usability */
	for(i=0;i<nclistlength(node->subnodes);i++) {
	    CDFnode* sub = (CDFnode*)nclistget(node->subnodes,i);
	    err = sequencecheck3r(sub,vars,topseq);
	    if(err == NC_NOERR) ok = 1;
	}
	err = (ok?NC_NOERR:NC_EINVAL);
    }
    return err;
}

/*
OPeNDAP is in the process
of changing servers so that partial
grids are converted to structures.
However, not all servers do this:
some elide the grid altogether,
which can lead to ambiguities.
Handle this last case by attempting
to convert the elided case to look
like the newer structure case.
[for some reason, this code has
 been difficult to get right; I
 have rewritten 6 times and it
 probably is still not right.]

Input is (1) the root of the dds
that needs to be re-gridded
(2) the full datadds tree
that defines where the grids are.
(3) the projections that were used
to produce (1) from (2).
*/

NCerror
regrid3(CDFnode* ddsroot, CDFnode* template, NClist* projections)
{
    int i;
    NCerror ncstat = NC_NOERR;
    NClist* newstructs = nclistnew();

    /* The current regrid assumes that the ddsroot tree
       has missing grids compared to the template.
       It is also assumed that order of the nodes
       in the ddsroot is the same as in the template.
    */
    if(ddsroot->tree->regridded) return NC_NOERR;

#ifdef DEBUG
fprintf(stderr,"regrid: ddsroot=%s\n",dumptree(ddsroot));
fprintf(stderr,"regrid: template=%s\n",dumptree(template));
#endif


    /* turn off the projection tag for all nodes */
    unprojected3(template->tree->nodes);

    /* Set the projection flag for all paths of all nodes
       that are referenced in the projections that produced ddsroot.
       This includes containers and subnodes. If there are no
       projections then mark all nodes 
    */
    if(nclistlength(projections) == 0) {
        projectall3(template->tree->nodes);
    } else for(i=0;i<nclistlength(projections);i++) {
	DCEprojection* proj = (DCEprojection*)nclistget(projections,i);
        ASSERT(proj->discrim == CES_VAR);
        projection3r(proj->var->cdfleaf);
    }

    if(simplenodematch34(ddsroot,template)) {
        ncstat = regrid3r(ddsroot,template,newstructs);
        ddsroot->tree->regridded = 1;
    } else
	ncstat = NC_EINVAL;
    nclistfree(newstructs);
    return ncstat;
}

static void
unprojected3(NClist* nodes)
{
    int i;
    for(i=0;i<nclistlength(nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(nodes,i);
	node->projected = 0;
    }
}

static void
projectall3(NClist* nodes)
{
    int i;
    for(i=0;i<nclistlength(nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(nodes,i);
	node->projected = 1;
    }
}

static void
projection3r(CDFnode* node)
{
    int i;
    NClist* path = nclistnew();
    collectnodepath3(node,path,!WITHDATASET);
    for(i=0;i<nclistlength(path);i++) {
        CDFnode* pathnode = (CDFnode*)nclistget(path,i);
#ifdef DEBUG
if(pathnode->projected == 0)
fprintf(stderr,"projection: %s\n",makesimplepathstring3(pathnode));
#endif
	pathnode->projected = 1;
    }
    /* Now tag everything below me */
    for(i=0;i<nclistlength(node->subnodes);i++) {
	CDFnode* subnode = (CDFnode*)nclistget(node->subnodes,i);
	projection3r(subnode);
    }
    nclistfree(path);
}

/*
Add in virtual structure nodes so that
old style constrainted DDS and DATADDS
look like the new style with structures.
*/
static NCerror
regrid3r(CDFnode* node, CDFnode* template, NClist* structnodes)
{
    unsigned int inode, itemp;
    NCerror ncstat = NC_NOERR;

    /* Try to match node's subnodes to a subset of the
       template subnodes
    */
#ifdef DEBUG
fprintf(stderr,"regrid: matched: %s -> %s\n",
node->ncfullname,template->ncfullname);
#endif
    for(inode=0;inode<nclistlength(node->subnodes);inode++) {
        CDFnode* subnode = (CDFnode*)nclistget(node->subnodes,inode);
	int match = 0;
        for(itemp=0;itemp<nclistlength(template->subnodes);itemp++) {
            CDFnode* subtemp = (CDFnode*)nclistget(template->subnodes,itemp);
	    if(subtemp->projected && simplenodematch34(subnode,subtemp)) {
                ncstat = regrid3r(subnode,subtemp,structnodes);
                if(ncstat != NC_NOERR) return THROW(ncstat);
		match = 1;
		subtemp->projected = 0; /*make sure we dont reuse this node*/
		break;
	    }
        }
	if(!match) { /* subnode has no match */
	    /* ok, see if we can regrid */
            for(itemp=0;itemp<nclistlength(template->subnodes);itemp++) {
                CDFnode* subtemp = (CDFnode*)nclistget(template->subnodes,itemp);
		if(subtemp->nctype != NC_Grid) continue;
		if(!subtemp->projected) continue;
		ncstat = testregrid3(subnode,subtemp,structnodes);
                if(ncstat == NC_NOERR) {match=1; break;}
	    }
	    if(!match) {/* really no match */
	        ncstat = NC_EDDS; /* no match */
	    }
	}
    }
    return THROW(ncstat);
}

/* See if this node can match a subnode of the template
   as a grid, and if so, then rebuild the node graph.
*/
static NCerror
testregrid3(CDFnode* node, CDFnode* template, NClist* structnodes)
{
    int i,match;
    NCerror ncstat = NC_NOERR;
    ASSERT((template->nctype == NC_Grid));
    /* we have a node::template mismatch, first see if this
       is a structure grid? */
    if(node->nctype == NC_Structure) {
       match = 1;
       /* Now, try to match structure subnodes */
       ncstat = regrid3r(node,template,structnodes);
    } else { /* try to match inside the grid */
        for(match=0,i=0;i<nclistlength(template->subnodes);i++) {
            CDFnode* gridelem = (CDFnode*)nclistget(template->subnodes,i);
            if(!simplenodematch34(gridelem,node)) continue;
            ncstat = regrid3r(node,gridelem,structnodes);
            if(ncstat == NC_NOERR) {
                /* create new structure node if not already created */
                CDFnode* newstruct = NULL;
	        match = 1;
                for(i=0;i<nclistlength(structnodes);i++) {
                    newstruct = (CDFnode*)nclistget(structnodes,i);
                    if(newstruct->template == template) break;
                    newstruct = NULL;
                }
                if(newstruct == NULL) {
                    newstruct = makenewstruct3(node,template);
                    if(newstruct == NULL) {ncstat = NC_ENOMEM; goto done;}
                    /* Insert the grid into node's parent */
                    regridinsert(newstruct,node);
                    nclistpush(structnodes,(ncelem)newstruct);
                    nclistpush(node->root->tree->nodes,(ncelem)newstruct);
                } 
                regridremove(newstruct, node);
                node->container = newstruct;
                nclistpush(newstruct->subnodes,(ncelem)node);
                break; /* done with node */
            }
        }
    }
    if(!match) ncstat = NC_EDDS;
done:
    return ncstat;
}


static CDFnode*
makenewstruct3(CDFnode* node, CDFnode* template)
{
    CDFnode* newstruct;
    newstruct = (CDFnode*)calloc(1,sizeof(CDFnode));
    if(newstruct == NULL) return NULL;
    memset((void*)newstruct,0,sizeof(CDFnode));
    newstruct->virtual = 1;
    newstruct->name = nulldup(template->name);
    newstruct->nctype = NC_Structure;
    newstruct->subnodes = nclistnew();
    newstruct->container = node->container;
    newstruct->template = template;
    return newstruct;
}

static NCerror
regridinsert(CDFnode* newstruct, CDFnode* node)
{
    int i;
    CDFnode* parent;
    /* Locate the index of the node in its current parent */
    parent = node->container;
    for(i=0;i<nclistlength(parent->subnodes);i++) {
	CDFnode* subnode = (CDFnode*)nclistget(parent->subnodes,i);
	if(subnode == node) {
	    /* Insert the grid right before this node */
	    nclistinsert(parent->subnodes,i,(ncelem)newstruct);
	    return NC_NOERR;
	}
    }
    PANIC("regridinsert failure");
    return NC_EINVAL;
}

static NCerror
regridremove(CDFnode* newstruct, CDFnode* node)
{
    int i;
    CDFnode* parent;
    /* Locate the index of the node in its current parent and remove */
    parent = node->container;
    for(i=0;i<nclistlength(parent->subnodes);i++) {
	CDFnode* subnode = (CDFnode*)nclistget(parent->subnodes,i);
	if(subnode == node) {
	    nclistremove(parent->subnodes,i);
	    return NC_NOERR;
	}
    }
    PANIC("regridremove failure");
    return NC_EINVAL;
}    

#ifdef IGNORE
static void
regridclean3(NClist* nodes, struct NCregrid* gridnodes)
{
    int i;
    NCregrid* ncregrid;

    for(i=0;i<nclistlength(nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(nodes,i);
        /* See if this node is a newgrid */
        ncregrid = gridnodes;
        while(ncregrid->newgrid != NULL && ncregrid->newgrid != node) ncregrid++;
        if(ncregrid->newgrid == node) continue; /* no need to process this */
        for(i=nclistlength(node->subnodes)-1;i>=0;i--) { /* walk backwards */
	    CDFnode* subnode = (CDFnode*)nclistget(node->subnodes,i);
	    /* Remove any subnodes of this node that now point to a regrid node */
	    ncregrid = gridnodes;
	    while(ncregrid->newgrid != NULL && ncregrid->newgrid != subnode) ncregrid++;
	    /* Found, remove from this list */
	    if(ncregrid->newgrid != NULL) nclistremove(node->subnodes,i);
	}
    }
}
#endif

#ifdef IGNORE
/*
Given a node in some constrained
DDS or DATADDS tree,
return the equivalent node from the
unconstrained DDS tree.
*/

CDFnode*
findddsnode0(CDFnode* node)
{
    if(!node->root->tree->constrained)
	return node; /* node is already
		        from unconstrained tree */
    if(node->attachment0 == NULL
       && node->attachment != NULL)
	node = node->attachment; /* DATADDS node -> DDS node */

    if(node->attachment0 == NULL)
	return NULL; /* cannot reach the unconstrained tree */

    return node->attachment0;
}
#endif

/* 
Move data from nodes in src tree to nodes in dst tree where
the nodes match.  Src tree is typically a structural subset
of dst tree.
*/

NCerror
imprint3(CDFnode* dstroot, CDFnode* srcroot)
{
    NCerror ncstat = NC_NOERR;

    if(srcroot == NULL) {THROWCHK(ncstat=NC_NOERR); goto done;}
    if(!simplenodematch34(dstroot,srcroot))
	{THROWCHK(ncstat=NC_EINVAL); goto done;}
    /* clear out old imprinting */
    unimprint3(dstroot);
    ncstat = imprint3r(dstroot,srcroot,0);
done:
    return ncstat;
}

static NCerror
imprint3r(CDFnode* dstnode, CDFnode* srcnode, int depth)
{
    unsigned int i,j;
    NCerror ncstat = NC_NOERR;

    ASSERT((simplenodematch34(dstnode,srcnode)));
    
    /* Mark node as having been imprinted */
    dstnode->visible = 1;
#ifdef DEBUG
fprintf(stderr,"imprinting: %s\n",makecdfpathstring3(dstnode,"."));
#endif

    /* Do dimension imprinting */
    ASSERT((nclistlength(dstnode->array.dimensions) == nclistlength(srcnode->array.dimensions)));
    if(nclistlength(dstnode->array.dimensions) > 0) {
	ncstat = imprintdims3(dstnode,srcnode);
	if(ncstat) goto done;
    }

    /* Try to match dstnode subnodes against srcnode subnodes */
    ASSERT(nclistlength(dstnode->subnodes) >= nclistlength(srcnode->subnodes));

    for(i=0;i<nclistlength(srcnode->subnodes);i++) {
        CDFnode* srcsubnode = (CDFnode*)nclistget(srcnode->subnodes,i);
	/* Search dst subnodes for a matching subnode from src */
        for(j=0;j<nclistlength(dstnode->subnodes);j++) {
            CDFnode* dstsubnode = (CDFnode*)nclistget(dstnode->subnodes,j);
            if(simplenodematch34(dstsubnode,srcsubnode)) {
                ncstat = imprint3r(dstsubnode,srcsubnode,depth+1);
   	        if(ncstat) goto done;
	    }
	}
    }
done:
    return THROW(ncstat);
}

static NCerror
imprintdims3(CDFnode* dstnode, CDFnode* srcnode)
{
    unsigned int i;
    for(i=0;i<nclistlength(dstnode->array.dimensions);i++) {
	CDFnode* ddim = (CDFnode*)nclistget(dstnode->array.dimensions,i);
	CDFnode* sdim = (CDFnode*)nclistget(srcnode->array.dimensions,i);
	ddim->dim.declsize = sdim->dim.declsize0;	
    }
    return NC_NOERR;
}

void
unimprint3(CDFnode* root)
{
    unsigned int i,j;
    CDFtree* tree = root->tree;
    for(i=0;i<nclistlength(tree->nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(tree->nodes,i);
	node->visible = 0;
        for(j=0;j<nclistlength(node->array.dimensions);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(node->array.dimensions,j);
	    dim->dim.declsize = dim->dim.declsize0;
	}
    }
}

void
setvisible(CDFnode* root, int visible)
{
    unsigned int i;
    CDFtree* tree = root->tree;
    for(i=0;i<nclistlength(tree->nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(tree->nodes,i);
	node->visible = visible;
    }
}

NCerror
imprintself3(CDFnode* root)
{
    setvisible(root,1);
    return NC_NOERR;
}
