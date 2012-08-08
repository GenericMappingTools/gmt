/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/cdf3.c,v 1.33 2009/12/03 03:42:37 dmh Exp $
 *********************************************************************/

#include "ncdap3.h"
#include "daputil.h"
#include "dapdump.h"

CDFnode* v4node = NULL;

/* Forward*/
static NCerror sequencecheck3r(CDFnode* node, NClist* vars, CDFnode* topseq);
static NCerror regrid3r(CDFnode*, CDFnode*, NClist*);
static NCerror testregrid3(CDFnode* node, CDFnode* template, NClist*);
static CDFnode* makenewgrid3(CDFnode* node, CDFnode* template);
static NCerror regridinsert(CDFnode* newgrid, CDFnode* node);
static NCerror regridremove(CDFnode* newgrid, CDFnode* node);
static NCerror mapnodes3r(CDFnode*, CDFnode*, int depth);
static NCerror mapfcn(CDFnode* dstnode, CDFnode* srcnode);
static NCerror definedimsetplus3(NCDAPCOMMON* nccomm, CDFnode* node);
static NCerror definedimsetall3(NCDAPCOMMON* nccomm, CDFnode* node);

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
	/* If this node has a bad name, repair it */
	if(dap_badname(node->ocname)) {
	    char* newname = dap_repairname(node->ocname);
	    nullfree(node->ocname);
	    node->ocname = newname;
	}
	if(!node->visible) continue;
	if(node->nctype == NC_Atomic)
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
#ifdef DEBUG2
for(i=0;i<nclistlength(varnodes);i++) {
CDFnode* node = (CDFnode*)nclistget(varnodes,i);
if(node == NULL) continue;
fprintf(stderr,"computevarnodes: var: %s\n",makecdfpathstring3(node,"."));
}
#endif
    return NC_NOERR;
}

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

    /* ensure all variables have an initial full name defined */
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(varnodes,i);
	nullfree(var->ncfullname);
	var->ncfullname = makecdfpathstring3(var,nccomm->cdf.separator);
#ifdef DEBUG2
fprintf(stderr,"var names: %s %s %s\n",
	var->ocname,var->ncbasename,var->ncfullname);
#endif
    }

    /*  unify all variables with same fullname and dimensions
	basevar fields says: "for duplicate grid variables";
        when does this happen?
    */
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
		else if(nclistlength(testnode->array.dimsetall)
			!= nclistlength(var->array.dimsetall))
		    match = 0;
	        else for(d=0;d<nclistlength(testnode->array.dimsetall);d++) {
		    CDFnode* vdim = (CDFnode*)nclistget(var->array.dimsetall,d);
		    CDFnode* tdim = (CDFnode*)nclistget(testnode->array.dimsetall,d);
	            if(vdim->dim.declsize != tdim->dim.declsize) {
		        match = 0;
			break;
		    }
		}
		if(match) {
		    testnode->array.basevar = var;
fprintf(stderr,"basevar invoked: %s\n",var->ncfullname);
		}
	    }
	}
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


/* locate and connect usable sequences and vars.
A sequence is usable iff:
1. it has a path from one of its subnodes to a leaf and that
   path does not contain a sequence.
2. No parent container has dimensions.
*/

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
    if(topseq == NULL && nclistlength(node->array.dimset0) > 0) {
	err = NC_EINVAL; /* This container has dimensions, so no sequence within it
                            can be usable */
    } else if(node->nctype == NC_Sequence) {
	/* Recursively walk the path for each subnode of this sequence node
           looking for a path without any sequence */
	for(i=0;i<nclistlength(node->subnodes);i++) {
	    CDFnode* sub = (CDFnode*)nclistget(node->subnodes,i);
	    err = sequencecheck3r(sub,vars,node);
	    if(err == NC_NOERR) ok = 1; /* there is at least 1 usable var below */
	}
	if(topseq == NULL && ok == 1) {
	    /* this sequence is usable because it has scalar container
               (by construction) and has a path to a leaf without an intermediate
               sequence. */
	    err = NC_NOERR;
	    node->usesequence = 1;
	} else {
	    /* this sequence is unusable because it has no path
               to a leaf without an intermediate sequence. */
	    node->usesequence = 0;
	    err = NC_EINVAL;
	}
    } else if(nclistcontains(vars,(ncelem)node)) {
	/* If we reach a leaf, then topseq is usable, so save it */
	node->array.sequence = topseq;
    } else { /* Some kind of non-sequence container node with no dimensions */
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
OPeNDAP is in the process of changing servers so that
partial grids are converted to structures.  However, not all
servers do this: some elide the grid altogether, which can
lead to ambiguities.  Handle this last case by attempting to
convert the elided case to look like the newer structure
case.  [for some reason, this code has been difficult to get
right; I have rewritten 6 times and it probably is still not
right.]

Input is
(1) the root of the dds that needs to be re-gridded
(2) the full datadds tree that defines where the grids are.
(3) the projections that were used to produce (1) from (2).
*/

NCerror
regrid3(CDFnode* ddsroot, CDFnode* template, NClist* projections)
{
    NCerror ncstat = NC_NOERR;
    NClist* newgrids = nclistnew();

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


#ifdef PROJECTED
    /* turn off the projection tag for all nodes */
    unprojected3(template->tree->nodes);
    /* Set the projection flag for all paths of all nodes
       that are referenced in the projections that produced ddsroot.
       This includes containers and subnodes. If there are no
       projections then mark all nodes 
    */
     projectall3(template->tree->nodes);
#endif

    if(simplenodematch34(ddsroot,template)) {
        ncstat = regrid3r(ddsroot,template,newgrids);
        ddsroot->tree->regridded = 1;
    } else
	ncstat = NC_EINVAL;
    nclistfree(newgrids);
    return ncstat;
}

#ifdef PROJECTED
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
#endif /*PROJECTED*/

/*
Add in virtual structure nodes so that
old style constrainted DDS and DATADDS
look like the new style with structures.
*/
static NCerror
regrid3r(CDFnode* node, CDFnode* template, NClist* gridnodes)
{
    unsigned int inode, itemp;
    NCerror ncstat = NC_NOERR;

    /* Try to match node's subnodes to a subset of the
       template subnodes
    */
#ifdef DEBUG
fprintf(stderr,"regrid: matched: %s -> %s\n",
node->ocname,template->ocname);
#endif
    for(inode=0;inode<nclistlength(node->subnodes);inode++) {
        CDFnode* subnode = (CDFnode*)nclistget(node->subnodes,inode);
	int match = 0;
        for(itemp=0;itemp<nclistlength(template->subnodes);itemp++) {
            CDFnode* subtemp = (CDFnode*)nclistget(template->subnodes,itemp);
	    if(
#ifdef PROJECTED
		subtemp->projected &&
#endif
		simplenodematch34(subnode,subtemp)) {
                ncstat = regrid3r(subnode,subtemp,gridnodes);
                if(ncstat != NC_NOERR) return THROW(ncstat);
		match = 1;
#ifdef PROJECTED
		subtemp->projected = 0; /*make sure we dont reuse this node*/
#endif
		break;
	    }
        }
	if(!match) { /* subnode has no match */
	    /* ok, see if we can regrid */
            for(itemp=0;itemp<nclistlength(template->subnodes);itemp++) {
                CDFnode* subtemp = (CDFnode*)nclistget(template->subnodes,itemp);
#ifdef DEBUG
fprintf(stderr,"regrid: inside: %s.%s :: %s.%s\n",
node->ocname,subnode->ocname,
template->ocname,subtemp->ocname);
#endif
		if(subtemp->nctype != NC_Grid)
		    continue;
#ifdef PROJECTED
		if(!subtemp->projected) continue;
#endif
		ncstat = testregrid3(subnode,subtemp,gridnodes);
                if(ncstat == NC_NOERR) {match=1; break;}
	    }
	    if(!match) {/* really no match */
	        ncstat = THROW(NC_EDDS); /* no match */
	    }
	}
    }
    return THROW(ncstat);
}

/* See if this node can match a subnode of the template
   as a grid, and if so, then rebuild the node graph.
*/
static NCerror
testregrid3(CDFnode* node, CDFnode* template, NClist* gridnodes)
{
    int i,match;
    NCerror ncstat = NC_NOERR;
    ASSERT((template->nctype == NC_Grid));
    { /* try to match inside the grid */
        for(match=0,i=0;i<nclistlength(template->subnodes);i++) {
            CDFnode* gridelem = (CDFnode*)nclistget(template->subnodes,i);
            if(!simplenodematch34(gridelem,node))
		continue;
            ncstat = regrid3r(node,gridelem,gridnodes);
            if(ncstat == NC_NOERR) {
                /* create new grid node if not already created */
                CDFnode* newgrid = NULL;
	        match = 1;
                for(i=0;i<nclistlength(gridnodes);i++) {
                    newgrid = (CDFnode*)nclistget(gridnodes,i);
                    if(newgrid->template == template) break;
                    newgrid = NULL;
                }
                if(newgrid == NULL) {
                    newgrid = makenewgrid3(node,template);
                    if(newgrid == NULL) {ncstat = NC_ENOMEM; goto done;}
                    /* Insert the grid into node's parent */
                    regridinsert(newgrid,node);
                    nclistpush(gridnodes,(ncelem)newgrid);
                    nclistpush(node->root->tree->nodes,(ncelem)newgrid);
                } 
                regridremove(newgrid, node);
                node->container = newgrid;
                nclistpush(newgrid->subnodes,(ncelem)node);
                break; /* done with node */
            }
        }
    }
    if(!match) ncstat = NC_EDDS;
done:
    return ncstat;
}


static CDFnode*
makenewgrid3(CDFnode* node, CDFnode* template)
{
    CDFnode* newgrid;
    newgrid = (CDFnode*)calloc(1,sizeof(CDFnode));
    if(newgrid == NULL) return NULL;
    memset((void*)newgrid,0,sizeof(CDFnode));
    newgrid->virtual = 1;
    newgrid->ocname = nulldup(template->ocname);
    newgrid->ncbasename = nulldup(template->ncbasename);
    newgrid->nctype = NC_Grid;
    newgrid->subnodes = nclistnew();
    newgrid->container = node->container;
    newgrid->template = template;
    return newgrid;
}

static NCerror
regridinsert(CDFnode* newgrid, CDFnode* node)
{
    int i;
    CDFnode* parent;
    /* Locate the index of the node in its current parent */
    parent = node->container;
    for(i=0;i<nclistlength(parent->subnodes);i++) {
	CDFnode* subnode = (CDFnode*)nclistget(parent->subnodes,i);
	if(subnode == node) {
	    /* Insert the grid right before this node */
	    nclistinsert(parent->subnodes,i,(ncelem)newgrid);
	    return NC_NOERR;
	}
    }
    PANIC("regridinsert failure");
    return NC_EINVAL;
}

static NCerror
regridremove(CDFnode* newgrid, CDFnode* node)
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

/**

Make the constrained dds nodes (root)
point to the corresponding unconstrained
dds nodes (fullroot).
 */

NCerror
mapnodes3(CDFnode* root, CDFnode* fullroot)
{
    NCerror ncstat = NC_NOERR;
    ASSERT(root != NULL && fullroot != NULL);
    if(!simplenodematch34(root,fullroot))
	{THROWCHK(ncstat=NC_EINVAL); goto done;}
    /* clear out old associations*/
    unmap3(root);
    ncstat = mapnodes3r(root,fullroot,0);
done:
    return ncstat;
}

static NCerror
mapnodes3r(CDFnode* connode, CDFnode* fullnode, int depth)
{
    unsigned int i,j;
    NCerror ncstat = NC_NOERR;

    ASSERT((simplenodematch34(connode,fullnode)));
    
#ifdef DEBUG
{
char* path1 = makecdfpathstring3(fullnode,".");
char * path2 = makecdfpathstring3(connode,".");
fprintf(stderr,"mapnode: %s->%s\n",path1,path2);
nullfree(path1); nullfree(path2);
}
#endif

    /* Map node */
    mapfcn(connode,fullnode);

    /* Try to match connode subnodes against fullnode subnodes */
    ASSERT(nclistlength(connode->subnodes) <= nclistlength(fullnode->subnodes));

    for(i=0;i<nclistlength(connode->subnodes);i++) {
        CDFnode* consubnode = (CDFnode*)nclistget(connode->subnodes,i);
	/* Search full subnodes for a matching subnode from con */
        for(j=0;j<nclistlength(fullnode->subnodes);j++) {
            CDFnode* fullsubnode = (CDFnode*)nclistget(fullnode->subnodes,j);
            if(simplenodematch34(fullsubnode,consubnode)) {
                ncstat = mapnodes3r(consubnode,fullsubnode,depth+1);
   	        if(ncstat) goto done;
	    }
	}
    }
done:
    return THROW(ncstat);
}


/* The specific actions of a map are defined
   by this function.
*/
static NCerror
mapfcn(CDFnode* dstnode, CDFnode* srcnode)
{
    /* Mark node as having been mapped */
    dstnode->visible = 1;
    dstnode->basenode = srcnode;
    return NC_NOERR;
}

void
unmap3(CDFnode* root)
{
    unsigned int i;
    CDFtree* tree = root->tree;
    for(i=0;i<nclistlength(tree->nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(tree->nodes,i);
	node->basenode = NULL;
        node->visible = 0;
    }
}

/* 
Move dimension data from basenodes to nodes
*/

NCerror
dimimprint3(NCDAPCOMMON* nccomm)
{
    NCerror ncstat = NC_NOERR;
    NClist* allnodes;
    int i,j;
    CDFnode* basenode;

    allnodes = nccomm->cdf.ddsroot->tree->nodes;
    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(allnodes,i);
	int noderank, baserank;
        /* Do dimension imprinting */
	basenode = node->basenode;
	if(basenode == NULL) continue;
	noderank = nclistlength(node->array.dimset0);
	baserank = nclistlength(basenode->array.dimset0);
	if(noderank == 0) continue;
        ASSERT(noderank == baserank);
#ifdef DEBUG
fprintf(stderr,"dimimprint %s/%d -> %s/%d\n",
	makecdfpathstring3(basenode,"."),
	noderank,
	makecdfpathstring3(node,"."),
	baserank);
#endif
        for(j=0;j<noderank;j++) {
	    CDFnode* dim = (CDFnode*)nclistget(node->array.dimset0,j);
	    CDFnode* basedim = (CDFnode*)nclistget(basenode->array.dimset0,j);
	    dim->dim.declsize0 = basedim->dim.declsize;	
#ifdef DEBUG
fprintf(stderr,"dimimprint: %d: %lu -> %lu\n",i,basedim->dim.declsize,dim->dim.declsize0);
#endif
        }
    }
    return ncstat;
}

static CDFnode*
clonedim(NCDAPCOMMON* nccomm, CDFnode* dim, CDFnode* var)
{
    CDFnode* clone;
    clone = makecdfnode34(nccomm,dim->ocname,OC_Dimension,
			  NULL,dim->container);
    /* Record its existence */
    nclistpush(dim->container->root->tree->nodes,(ncelem)clone);
    clone->dim = dim->dim; /* copy most everything */
    clone->dim.dimflags |= CDFDIMCLONE;
    clone->dim.array = var;
    return clone;
}

static NClist*
clonedimset3(NCDAPCOMMON* nccomm, NClist* dimset, CDFnode* var)
{
    NClist* result = nclistnew();
    int i;
    for(i=0;i<nclistlength(dimset);i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
	nclistpush(result,(ncelem)clonedim(nccomm,dim,var));
    }
    return result;
}

/* Define the dimsetplus list for a node */
static NCerror
definedimsetplus3(NCDAPCOMMON* nccomm, CDFnode* node)
{
    int ncstat = NC_NOERR;
    NClist* dimset;
    CDFnode* clone;

    ASSERT(node->array.dimsetplus == NULL);
    if(node->array.dimset0 == NULL)
	dimset = nclistnew();
    else { /* copy the dimset0 into dimset */
        dimset = nclistclone(node->array.dimset0);
    }
    /* Insert the sequence or string dims */
    if(node->array.stringdim != NULL) {
	clone = node->array.stringdim;
        nclistpush(dimset,(ncelem)clone);
    }
    if(node->array.seqdim != NULL) {
	clone = node->array.seqdim;
        nclistpush(dimset,(ncelem)clone);
    }
    node->array.dimsetplus = dimset;
    return ncstat;
}

/* Define the dimsetall list for a node */
static NCerror
definedimsetall3(NCDAPCOMMON* nccomm, CDFnode* node)
{
    int i;
    int ncstat = NC_NOERR;
    NClist* dimsetall;

    ASSERT(node->array.dimsetall == NULL);
    if(node->container != NULL) {
        if(node->container->array.dimsetall == NULL) {
#ifdef DEBUG1
fprintf(stderr,"dimsetall: recurse %s\n",node->container->ocname);
#endif
	    ncstat = definedimsetall3(nccomm,node->container);
	    if(ncstat != NC_NOERR) return ncstat;
        }
	/* We need to clone the parent dimensions because we will be assigning
           indices vis-a-vis this variable */
        dimsetall = clonedimset3(nccomm,node->container->array.dimsetall,node);
    } else
	dimsetall = nclistnew();
    // concat parentall and dimset;
    for(i=0;i<nclistlength(node->array.dimsetplus);i++) {
	CDFnode* clone = (CDFnode*)nclistget(node->array.dimsetplus,i);
	nclistpush(dimsetall,(ncelem)clone);
    }
    node->array.dimsetall = dimsetall;
#ifdef DEBUG1
fprintf(stderr,"dimsetall: |%s|=%d\n",node->ocname,nclistlength(dimsetall));
#endif
    return ncstat;
}

/* Define the dimsetplus and dimsetall lists for
   all nodes with dimensions
*/
NCerror
definedimsets3(NCDAPCOMMON* nccomm)
{
    int i;
    int ncstat = NC_NOERR;
    NClist* allnodes = nccomm->cdf.ddsroot->tree->nodes;

    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* rankednode = (CDFnode*)nclistget(allnodes,i);
	if(rankednode->nctype == NC_Dimension) continue; //ignore
	ASSERT((rankednode->array.dimsetplus == NULL));
	ncstat = definedimsetplus3(nccomm,rankednode);
	if(ncstat != NC_NOERR) return ncstat;
    }
    for(i=0;i<nclistlength(allnodes);i++) {
	CDFnode* rankednode = (CDFnode*)nclistget(allnodes,i);
	if(rankednode->nctype == NC_Dimension) continue; //ignore
	ASSERT((rankednode->array.dimsetall == NULL));
	ASSERT((rankednode->array.dimsetplus != NULL));
	ncstat = definedimsetall3(nccomm,rankednode);
	if(ncstat != NC_NOERR) return ncstat;
    }     
    return NC_NOERR;
}

