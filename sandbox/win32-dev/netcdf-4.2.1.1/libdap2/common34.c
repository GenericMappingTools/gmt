/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/libncdap3/common34.c,v 1.29 2010/05/25 13:53:02 ed Exp $
 *********************************************************************/

#include "ncdap3.h"

#ifdef HAVE_GETRLIMIT
#  ifdef HAVE_SYS_RESOURCE_H
#    include <sys/time.h>
#  endif
#  ifdef HAVE_SYS_RESOURCE_H
#    include <sys/resource.h>
#  endif
#endif
#include "dapdump.h"

extern CDFnode* v4node;

/* Define the set of protocols known to be constrainable */
static char* constrainableprotocols[] = {"http", "https",NULL};
static NCerror buildcdftree34r(NCDAPCOMMON*,OCddsnode,CDFnode*,CDFtree*,CDFnode**);
static void defdimensions(OCddsnode, CDFnode*, NCDAPCOMMON*, CDFtree*);
static NCerror  attachsubset34r(CDFnode*, CDFnode*);
static void free1cdfnode34(CDFnode* node);

/* Define Procedures that are common to both
   libncdap3 and libncdap4
*/

/* Ensure every node has an initial base name defined and fullname */
/* Exceptions: anonymous dimensions. */
static NCerror
fix1node34(NCDAPCOMMON* nccomm, CDFnode* node)
{
    if(node->nctype == NC_Dimension && node->ocname == NULL) return NC_NOERR;
    ASSERT((node->ocname != NULL));
    nullfree(node->ncbasename);
    node->ncbasename = cdflegalname3(node->ocname);
    if(node->ncbasename == NULL) return NC_ENOMEM;
    nullfree(node->ncfullname);
    node->ncfullname = makecdfpathstring3(node,nccomm->cdf.separator);
    if(node->ncfullname == NULL) return NC_ENOMEM;
    if(node->nctype == NC_Atomic)
        node->externaltype = nctypeconvert(nccomm,node->etype);
    return NC_NOERR;
}

NCerror
fixnodes34(NCDAPCOMMON* nccomm, NClist* cdfnodes)
{
    int i;
    for(i=0;i<nclistlength(cdfnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(cdfnodes,i);
	NCerror err = fix1node34(nccomm,node);
	if(err) return err;
    }
    return NC_NOERR;
}


NCerror
fixgrid34(NCDAPCOMMON* nccomm, CDFnode* grid)
{
    unsigned int i,glen;
    CDFnode* array;

    glen = nclistlength(grid->subnodes);
    array = (CDFnode*)nclistget(grid->subnodes,0);	        
    if(nccomm->controls.flags & (NCF_NC3)) {
        /* Rename grid Array: variable, but leave its oc base name alone */
        nullfree(array->ncbasename);
        array->ncbasename = nulldup(grid->ncbasename);
        if(!array->ncbasename) return NC_ENOMEM;
    }
    /* validate and modify the grid structure */
    if((glen-1) != nclistlength(array->array.dimset0)) goto invalid;
    for(i=1;i<glen;i++) {
	CDFnode* arraydim = (CDFnode*)nclistget(array->array.dimset0,i-1);
	CDFnode* map = (CDFnode*)nclistget(grid->subnodes,i);
	CDFnode* mapdim;
	/* map must have 1 dimension */
	if(nclistlength(map->array.dimset0) != 1) goto invalid;
	/* and the map name must match the ith array dimension */
	if(arraydim->ocname != NULL && map->ocname != NULL
	   && strcmp(arraydim->ocname,map->ocname) != 0)
	    goto invalid;
	/* and the map name must match its dim name (if any) */
	mapdim = (CDFnode*)nclistget(map->array.dimset0,0);
	if(mapdim->ocname != NULL && map->ocname != NULL
	   && strcmp(mapdim->ocname,map->ocname) != 0)
	    goto invalid;
	/* Add appropriate names for the anonymous dimensions */
	/* Do the map name first, so the array dim may inherit */
	if(mapdim->ocname == NULL) {
	    nullfree(mapdim->ncbasename);
	    mapdim->ocname = nulldup(map->ocname);
	    if(!mapdim->ocname) return NC_ENOMEM;
	    mapdim->ncbasename = cdflegalname3(mapdim->ocname);
	    if(!mapdim->ncbasename) return NC_ENOMEM;
	}
	if(arraydim->ocname == NULL) {
	    nullfree(arraydim->ncbasename);
	    arraydim->ocname = nulldup(map->ocname);
	    if(!arraydim->ocname) return NC_ENOMEM;
	    arraydim->ncbasename = cdflegalname3(arraydim->ocname);
	    if(!arraydim->ncbasename) return NC_ENOMEM;
	}
        if(FLAGSET(nccomm->controls,(NCF_NCDAP|NCF_NC3))) {
	    char tmp[3*NC_MAX_NAME];
            /* Add the grid name to the basename of the map */
	    snprintf(tmp,sizeof(tmp),"%s%s%s",map->container->ncbasename,
					  nccomm->cdf.separator,
					  map->ncbasename);
	    nullfree(map->ncbasename);
            map->ncbasename = nulldup(tmp);
	    if(!map->ncbasename) return NC_ENOMEM;
	}
    }
    return NC_NOERR;
invalid:
    return NC_EINVAL; /* mal-formed grid */
}

/**
 *  Given an anonymous dimension, compute the
 *  effective 0-based index wrt to the specified var.
 *  The result should mimic the libnc-dap indices.
 */

static void
computedimindexanon3(CDFnode* dim, CDFnode* var)
{
    int i;
    NClist* dimset = var->array.dimsetall;
    for(i=0;i<nclistlength(dimset);i++) {
	CDFnode* candidate = (CDFnode*)nclistget(dimset,i);
        if(dim == candidate) {
	   dim->dim.index1=i+1;
	   return;
	}
    }
}

/* Replace dims in a list with their corresponding basedim */
static void
replacedims(NClist* dims)
{
    int i;
    for(i=0;i<nclistlength(dims);i++) {
        CDFnode* dim = (CDFnode*)nclistget(dims,i);
	CDFnode* basedim = dim->dim.basedim;
	if(basedim == NULL) continue;
	nclistset(dims,i,(ncelem)basedim);
    }
}

/**
 Two dimensions are equivalent if
 1. they have the same size
 2. neither are anonymous
 3. they ave the same names. 
 */
static int
equivalentdim(CDFnode* basedim, CDFnode* dupdim)
{
    if(dupdim->dim.declsize != basedim->dim.declsize) return 0;
    if(basedim->ocname == NULL && dupdim->ocname == NULL) return 0;
    if(basedim->ocname == NULL || dupdim->ocname == NULL) return 0;
    if(strcmp(dupdim->ocname,basedim->ocname) != 0) return 0;
    return 1;
}

/*
   Provide short and/or unified names for dimensions.
   This must mimic lib-ncdap, which is difficult.
*/
NCerror
computecdfdimnames34(NCDAPCOMMON* nccomm)
{
    int i,j;
    char tmp[NC_MAX_NAME*2];
    NClist* conflicts = nclistnew();
    NClist* varnodes = nccomm->cdf.varnodes;
    NClist* alldims;
    NClist* basedims;
    
    /* Collect all dimension nodes from dimsetall lists */

    alldims = getalldims34(nccomm,0);    

    /* Assign an index to all anonymous dimensions
       vis-a-vis its containing variable
    */
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(varnodes,i);
        for(j=0;j<nclistlength(var->array.dimsetall);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(var->array.dimsetall,j);
	    if(dim->ocname != NULL) continue; /* not anonymous */
 	    computedimindexanon3(dim,var);
	}
    }

    /* Unify dimensions by defining one dimension as the "base"
       dimension, and make all "equivalent" dimensions point to the
       base dimension.
	1. Equivalent means: same size and both have identical non-null names.
	2. Dims with same name but different sizes will be handled separately
    */
    for(i=0;i<nclistlength(alldims);i++) {
	CDFnode* dupdim = NULL;
	CDFnode* basedim = (CDFnode*)nclistget(alldims,i);
	if(basedim == NULL) continue;
	if(basedim->dim.basedim != NULL) continue; /* already processed*/
	for(j=i+1;j<nclistlength(alldims);j++) { /* Sigh, n**2 */
	    dupdim = (CDFnode*)nclistget(alldims,j);
	    if(basedim == dupdim) continue;
	    if(dupdim == NULL) continue;
	    if(dupdim->dim.basedim != NULL) continue; /* already processed */
	    if(!equivalentdim(basedim,dupdim))
		continue;
            dupdim->dim.basedim = basedim; /* equate */
#ifdef DEBUG1
fprintf(stderr,"assign: %s/%s -> %s/%s\n",
basedim->dim.array->ocname,basedim->ocname,
dupdim->dim.array->ocname,dupdim->ocname
);
#endif
	}
    }

    /* Next case: same name and different sizes*/
    /* => rename second dim */

    for(i=0;i<nclistlength(alldims);i++) {
	CDFnode* basedim = (CDFnode*)nclistget(alldims,i);
	if(basedim->dim.basedim != NULL) continue;
	/* Collect all conflicting dimensions */
	nclistclear(conflicts);
        for(j=i+1;j<nclistlength(alldims);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(alldims,j);
	    if(dim->dim.basedim != NULL) continue;
	    if(dim->ocname == NULL && basedim->ocname == NULL) continue;
	    if(dim->ocname == NULL || basedim->ocname == NULL) continue;
	    if(strcmp(dim->ocname,basedim->ocname)!=0) continue;
	    if(dim->dim.declsize == basedim->dim.declsize) continue;
#ifdef DEBUG2
fprintf(stderr,"conflict: %s[%lu] %s[%lu]\n",
			basedim->ncfullname,(unsigned long)basedim->dim.declsize,
			dim->ncfullname,(unsigned long)dim->dim.declsize);
#endif
	    nclistpush(conflicts,(ncelem)dim);
	}
	/* Give  all the conflicting dimensions an index */
	for(j=0;j<nclistlength(conflicts);j++) {
	    CDFnode* dim = (CDFnode*)nclistget(conflicts,j);
	    dim->dim.index1 = j+1;
	}
    }
    nclistfree(conflicts);

    /* Replace all non-base dimensions with their base dimension */
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(varnodes,i);
	replacedims(node->array.dimsetall);
	replacedims(node->array.dimsetplus);
	replacedims(node->array.dimset0);
    }

    /* Collect list of all basedims */
    basedims = nclistnew();
    for(i=0;i<nclistlength(alldims);i++) {
	CDFnode* dim = (CDFnode*)nclistget(alldims,i);
	if(dim->dim.basedim == NULL) {
	    if(!nclistcontains(basedims,(ncelem)dim)) {
		nclistpush(basedims,(ncelem)dim);
	    }
	}
    }

    nccomm->cdf.dimnodes = basedims;

    /* cleanup */
    nclistfree(alldims);

    /* Assign ncbasenames and ncfullnames to base dimensions */
    for(i=0;i<nclistlength(basedims);i++) {
	CDFnode* dim = (CDFnode*)nclistget(basedims,i);
	CDFnode* var = dim->dim.array;
	if(dim->dim.basedim != NULL) PANIC1("nonbase basedim: %s\n",dim->ocname);
	/* stringdim names are already assigned */
	if(dim->ocname == NULL) { /* anonymous: use the index to compute the name */
            snprintf(tmp,sizeof(tmp),"%s_%d",
                            var->ncfullname,dim->dim.index1-1);
            nullfree(dim->ncbasename);
            dim->ncbasename = cdflegalname3(tmp);
            nullfree(dim->ncfullname);
            dim->ncfullname = nulldup(dim->ncbasename);
    	} else { /* !anonymous; use index1 if defined */
   	    char* legalname = cdflegalname3(dim->ocname);
	    nullfree(dim->ncbasename);
	    if(dim->dim.index1 > 0) {/* need to fix conflicting names (see above) */
	        char sindex[64];
		snprintf(sindex,sizeof(sindex),"_%d",dim->dim.index1);
		dim->ncbasename = (char*)malloc(strlen(sindex)+strlen(legalname)+1);
		if(dim->ncbasename == NULL) return NC_ENOMEM;
		strcpy(dim->ncbasename,legalname);
		strcat(dim->ncbasename,sindex);
		nullfree(legalname);
	    } else {/* standard case */
	        dim->ncbasename = legalname;
	    }
    	    nullfree(dim->ncfullname);
	    dim->ncfullname = nulldup(dim->ncbasename);
	}
     }

    /* Verify unique and defined names for dimensions*/
    for(i=0;i<nclistlength(basedims);i++) {
	CDFnode* dim1 = (CDFnode*)nclistget(basedims,i);
	if(dim1->dim.basedim != NULL) PANIC1("nonbase basedim: %s\n",dim1->ncbasename);
	if(dim1->ncbasename == NULL || dim1->ncfullname == NULL)
	    PANIC1("missing dim names: %s",dim1->ocname);
	/* search backward so we can delete duplicates */
	for(j=nclistlength(basedims)-1;j>i;j--) {
	    CDFnode* dim2 = (CDFnode*)nclistget(basedims,j);
	    if(strcmp(dim1->ncfullname,dim2->ncfullname)==0) {
		/* complain and suppress one of them */
		fprintf(stderr,"duplicate dim names: %s[%lu] %s[%lu]\n",
			dim1->ncfullname,(unsigned long)dim1->dim.declsize,
			dim2->ncfullname,(unsigned long)dim2->dim.declsize);
		nclistremove(basedims,j);
	    }
	}
    }

#ifdef DEBUG
for(i=0;i<nclistlength(basedims);i++) {
CDFnode* dim = (CDFnode*)nclistget(basedims,i);
fprintf(stderr,"basedim: %s=%ld\n",dim->ncfullname,(long)dim->dim.declsize);
 }
#endif

    return NC_NOERR;
}

NCerror
makegetvar34(NCDAPCOMMON* nccomm, CDFnode* var, void* data, nc_type dsttype, Getvara** getvarp)
{
    Getvara* getvar;
    NCerror ncstat = NC_NOERR;

    getvar = (Getvara*)calloc(1,sizeof(Getvara));
    MEMCHECK(getvar,NC_ENOMEM);
    if(getvarp) *getvarp = getvar;

    getvar->target = var;
    getvar->memory = data;
    getvar->dsttype = dsttype;
    getvar->target = var;
    if(ncstat) nullfree(getvar);
    return ncstat;
}

int
constrainable34(NC_URI* durl)
{
   char** protocol = constrainableprotocols;
   for(;*protocol;protocol++) {
	if(strcmp(durl->protocol,*protocol)==0)
	    return 1;
   }
   return 0;
}

CDFnode*
makecdfnode34(NCDAPCOMMON* nccomm, char* name, OCtype octype,
             /*optional*/ OCddsnode ocnode, CDFnode* container)
{
    CDFnode* node;
    assert(nccomm != NULL);
    node = (CDFnode*)calloc(1,sizeof(CDFnode));
    if(node == NULL) return (CDFnode*)NULL;

    node->ocname = NULL;
    if(name) {
        size_t len = strlen(name);
        if(len >= NC_MAX_NAME) len = NC_MAX_NAME-1;
        node->ocname = (char*)malloc(len+1);
	if(node->ocname == NULL) return NULL;
	memcpy(node->ocname,name,len);
	node->ocname[len] = '\0';
    }
    node->nctype = octypetonc(octype);
    node->ocnode = ocnode;
    node->subnodes = nclistnew();
    node->container = container;
    if(ocnode != NULL) {
	oc_dds_atomictype(nccomm->oc.conn,ocnode,&octype);
        node->etype = octypetonc(octype);
    }
    return node;
}

/* Given an OCnode tree, mimic it as a CDFnode tree;
   Add DAS attributes if DAS is available. Accumulate set
   of all nodes in preorder.
*/
NCerror
buildcdftree34(NCDAPCOMMON* nccomm, OCddsnode ocroot, OCdxd occlass, CDFnode** cdfrootp)
{
    CDFnode* root = NULL;
    CDFtree* tree = (CDFtree*)calloc(1,sizeof(CDFtree));
    NCerror err = NC_NOERR;
    tree->ocroot = ocroot;
    tree->nodes = nclistnew();
    tree->occlass = occlass;
    tree->owner = nccomm;

    err = buildcdftree34r(nccomm,ocroot,NULL,tree,&root);
    if(!err) {
	if(occlass != OCDAS)
	    fixnodes34(nccomm,tree->nodes);
	if(cdfrootp) *cdfrootp = root;
    }
    return err;
}        

static NCerror
buildcdftree34r(NCDAPCOMMON* nccomm, OCddsnode ocnode, CDFnode* container,
                CDFtree* tree, CDFnode** cdfnodep)
{
    size_t i,ocrank,ocnsubnodes;
    OCtype octype;
    char* ocname = NULL;
    NCerror ncerr = NC_NOERR;
    CDFnode* cdfnode;

    oc_dds_class(nccomm->oc.conn,ocnode,&octype);
    oc_dds_name(nccomm->oc.conn,ocnode,&ocname);
    oc_dds_rank(nccomm->oc.conn,ocnode,&ocrank);
    oc_dds_nsubnodes(nccomm->oc.conn,ocnode,&ocnsubnodes);

    switch (octype) {
    case OC_Dataset:
    case OC_Grid:
    case OC_Structure:
    case OC_Sequence:
    case OC_Atomic:
	cdfnode = makecdfnode34(nccomm,ocname,octype,ocnode,container);
	nclistpush(tree->nodes,(ncelem)cdfnode);
	if(tree->root == NULL) {
	    tree->root = cdfnode;
	    cdfnode->tree = tree;
	}		
	break;

    case OC_Dimension:
    default: PANIC1("buildcdftree: unexpect OC node type: %d",(int)octype);

    }    
    /* cross link */
    cdfnode->root = tree->root;

    if(ocrank > 0) defdimensions(ocnode,cdfnode,nccomm,tree);
    for(i=0;i<ocnsubnodes;i++) {
	OCddsnode ocsubnode;
	CDFnode* subnode;
	oc_dds_ithfield(nccomm->oc.conn,ocnode,i,&ocsubnode);
	ncerr = buildcdftree34r(nccomm,ocsubnode,cdfnode,tree,&subnode);
	if(ncerr) return ncerr;
	nclistpush(cdfnode->subnodes,(ncelem)subnode);
    }
    nullfree(ocname);
    if(cdfnodep) *cdfnodep = cdfnode;
    return ncerr;
}

static void
defdimensions(OCddsnode ocnode, CDFnode* cdfnode, NCDAPCOMMON* nccomm, CDFtree* tree)
{
    size_t i,ocrank;
 
    oc_dds_rank(nccomm->oc.conn,ocnode,&ocrank);
    assert(ocrank > 0);
    for(i=0;i<ocrank;i++) {
	CDFnode* cdfdim;
	OCddsnode ocdim;
	char* ocname;
	size_t declsize;

	oc_dds_ithdimension(nccomm->oc.conn,ocnode,i,&ocdim);
	oc_dimension_properties(nccomm->oc.conn,ocdim,&declsize,&ocname);

	cdfdim = makecdfnode34(nccomm,ocname,OC_Dimension,
                              ocdim,cdfnode->container);
	nullfree(ocname);
	nclistpush(tree->nodes,(ncelem)cdfdim);
	/* Initially, constrained and unconstrained are same */
	cdfdim->dim.declsize = declsize;
	cdfdim->dim.array = cdfnode;
	if(cdfnode->array.dimset0 == NULL) 
	    cdfnode->array.dimset0 = nclistnew();
	nclistpush(cdfnode->array.dimset0,(ncelem)cdfdim);
    }    
}

/* Note: this routine only applies some common
   client parameters, other routines may apply
   specific ones.
*/

NCerror
applyclientparams34(NCDAPCOMMON* nccomm)
{
    int i,len;
    int dfaltstrlen = DEFAULTSTRINGLENGTH;
    int dfaltseqlim = DEFAULTSEQLIMIT;
    const char* value;
    char tmpname[NC_MAX_NAME+32];
    char* pathstr;
    OClink conn = nccomm->oc.conn;
    unsigned long limit;

    ASSERT(nccomm->oc.url != NULL);

    nccomm->cdf.cache->cachelimit = DFALTCACHELIMIT;
    value = oc_clientparam_get(conn,"cachelimit");
    limit = getlimitnumber(value);
    if(limit > 0) nccomm->cdf.cache->cachelimit = limit;

    nccomm->cdf.fetchlimit = DFALTFETCHLIMIT;
    value = oc_clientparam_get(conn,"fetchlimit");
    limit = getlimitnumber(value);
    if(limit > 0) nccomm->cdf.fetchlimit = limit;

    nccomm->cdf.smallsizelimit = DFALTSMALLLIMIT;
    value = oc_clientparam_get(conn,"smallsizelimit");
    limit = getlimitnumber(value);
    if(limit > 0) nccomm->cdf.smallsizelimit = limit;

    nccomm->cdf.cache->cachecount = DFALTCACHECOUNT;
#ifdef HAVE_GETRLIMIT
    { struct rlimit rl;
      if(getrlimit(RLIMIT_NOFILE, &rl) >= 0) {
	nccomm->cdf.cache->cachecount = (size_t)(rl.rlim_cur / 2);
      }
    }
#endif
    value = oc_clientparam_get(conn,"cachecount");
    limit = getlimitnumber(value);
    if(limit > 0) nccomm->cdf.cache->cachecount = limit;
    /* Ignore limit if not caching */
    if(!FLAGSET(nccomm->controls,NCF_CACHE))
        nccomm->cdf.cache->cachecount = 0;

    if(oc_clientparam_get(conn,"nolimit") != NULL)
	dfaltseqlim = 0;
    value = oc_clientparam_get(conn,"limit");
    if(value != NULL && strlen(value) != 0) {
        if(sscanf(value,"%d",&len) && len > 0) dfaltseqlim = len;
    }
    nccomm->cdf.defaultsequencelimit = dfaltseqlim;

    /* allow embedded _ */
    value = oc_clientparam_get(conn,"stringlength");
    if(value != NULL && strlen(value) != 0) {
        if(sscanf(value,"%d",&len) && len > 0) dfaltstrlen = len;
    }
    nccomm->cdf.defaultstringlength = dfaltstrlen;

    /* String dimension limits apply to variables */
    for(i=0;i<nclistlength(nccomm->cdf.varnodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(nccomm->cdf.varnodes,i);
	/* Define the client param stringlength for this variable*/
	var->maxstringlength = 0; /* => use global dfalt */
	strcpy(tmpname,"stringlength_");
	pathstr = makeocpathstring3(conn,var->ocnode,".");
	strcat(tmpname,pathstr);
	nullfree(pathstr);
	value = oc_clientparam_get(conn,tmpname);	
        if(value != NULL && strlen(value) != 0) {
            if(sscanf(value,"%d",&len) && len > 0) var->maxstringlength = len;
	}
    }
    /* Sequence limits apply to sequences */
    for(i=0;i<nclistlength(nccomm->cdf.ddsroot->tree->nodes);i++) {
	CDFnode* var = (CDFnode*)nclistget(nccomm->cdf.ddsroot->tree->nodes,i);
	if(var->nctype != NC_Sequence) continue;
	var->sequencelimit = dfaltseqlim;
	strcpy(tmpname,"nolimit_");
	pathstr = makeocpathstring3(conn,var->ocnode,".");
	strcat(tmpname,pathstr);
	if(oc_clientparam_get(conn,tmpname) != NULL)
	    var->sequencelimit = 0;
	strcpy(tmpname,"limit_");
	strcat(tmpname,pathstr);
	value = oc_clientparam_get(conn,tmpname);
        if(value != NULL && strlen(value) != 0) {
            if(sscanf(value,"%d",&len) && len > 0)
		var->sequencelimit = len;
	}
	nullfree(pathstr);
    }

    /* test for the appropriate fetch flags */
    value = oc_clientparam_get(conn,"fetch");
    if(value != NULL && strlen(value) > 0) {
	if(value[0] == 'd' || value[0] == 'D') {
            SETFLAG(nccomm->controls,NCF_ONDISK);
	}
    }

    /* test for the force-whole-var flag */
    value = oc_clientparam_get(conn,"wholevar");
    if(value != NULL) {
        SETFLAG(nccomm->controls,NCF_WHOLEVAR);
    }

    return NC_NOERR;
}

void
freecdfroot34(CDFnode* root)
{
    int i;
    CDFtree* tree;
    NCDAPCOMMON* nccomm;
    if(root == NULL) return;
    tree = root->tree;
    ASSERT((tree != NULL));
    /* Explicitly FREE the ocroot */
    nccomm = tree->owner;
    oc_root_free(nccomm->oc.conn,tree->ocroot);
    tree->ocroot = NULL;
    for(i=0;i<nclistlength(tree->nodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(tree->nodes,i);
	free1cdfnode34(node);
    }
    nclistfree(tree->nodes);
    nullfree(tree);
}

/* Free up a single node, but not any
   nodes it points to.
*/  
static void
free1cdfnode34(CDFnode* node)
{
    unsigned int j,k;
    if(node == NULL) return;
    nullfree(node->ocname);
    nullfree(node->ncbasename);
    nullfree(node->ncfullname);
    if(node->attributes != NULL) {
	for(j=0;j<nclistlength(node->attributes);j++) {
	    NCattribute* att = (NCattribute*)nclistget(node->attributes,j);
	    nullfree(att->name);
	    for(k=0;k<nclistlength(att->values);k++)
		nullfree((char*)nclistget(att->values,k));
	    nclistfree(att->values);
	    nullfree(att);
	}
    }
    nullfree(node->dodsspecial.dimname);
    nclistfree(node->subnodes);
    nclistfree(node->attributes);
    nclistfree(node->array.dimsetplus);
    nclistfree(node->array.dimsetall);
    nclistfree(node->array.dimset0);

    /* Clean up the ncdap4 fields also */
    nullfree(node->typename);
    nullfree(node->vlenname);
    nullfree(node);
}

/* Return true if node and node1 appear to refer to the same thing;
   takes grid->structure changes into account.
*/
int
nodematch34(CDFnode* node1, CDFnode* node2)
{
    return simplenodematch34(node1,node2);
}

int
simplenodematch34(CDFnode* node1, CDFnode* node2)
{
    if(node1 == NULL) return (node2==NULL);
    if(node2 == NULL) return 0;
    if(node1->nctype != node2->nctype) {
	/* Check for Grid->Structure match */
	if((node1->nctype == NC_Structure && node2->nctype == NC_Grid)
	   || (node2->nctype == NC_Structure && node1->nctype == NC_Grid)){
	   if(node1->ocname == NULL || node2->ocname == NULL
	      || strcmp(node1->ocname,node2->ocname) !=0) return 0;	    	
	} else return 0;
    }
    /* Add hack to address the screwed up Columbia server */
    if(node1->nctype == NC_Dataset) return 1;
    if(node1->nctype == NC_Atomic
       && node1->etype != node2->etype) return 0;
    if(node1->ocname != NULL && node2->ocname != NULL
       && strcmp(node1->ocname,node2->ocname)!=0) return 0;
    if(nclistlength(node1->array.dimset0)
       != nclistlength(node2->array.dimset0)) return 0;
    return 1;
}

/*
Given DDS node, locate the node
in a DATADDS that matches the DDS node.
Return NULL if no node found
*/

void
unattach34(CDFnode* root)
{
    unsigned int i;
    CDFtree* xtree = root->tree;
    for(i=0;i<nclistlength(xtree->nodes);i++) {
	CDFnode* xnode = (CDFnode*)nclistget(xtree->nodes,i);
	/* break bi-directional link */
        xnode->attachment = NULL;
    }
}

static void
setattach(CDFnode* target, CDFnode* template)
{
    target->attachment = template;
    template->attachment = target;
    /* Transfer important information */
    target->externaltype = template->externaltype;
    target->maxstringlength = template->maxstringlength;
    target->sequencelimit = template->sequencelimit;
    target->ncid = template->ncid;
    /* also transfer libncdap4 info */
    target->typeid = template->typeid;
    target->typesize = template->typesize;
}

static NCerror
attachdims34(CDFnode* xnode, CDFnode* template)
{
    unsigned int i;
    for(i=0;i<nclistlength(xnode->array.dimsetall);i++) {
	CDFnode* xdim = (CDFnode*)nclistget(xnode->array.dimsetall,i);
	CDFnode* tdim = (CDFnode*)nclistget(template->array.dimsetall,i);
	setattach(xdim,tdim);
#ifdef DEBUG2
fprintf(stderr,"attachdim: %s->%s\n",xdim->ocname,tdim->ocname);
#endif
    }
    return NC_NOERR;
}

/* 
Match a DATADDS node to a DDS node.
It is assumed that both trees have been regridded if necessary.
*/

static NCerror
attach34r(CDFnode* xnode, NClist* templatepath, int depth)
{
    unsigned int i,plen,lastnode,gridable;
    NCerror ncstat = NC_NOERR;
    CDFnode* templatepathnode;
    CDFnode* templatepathnext;

    plen = nclistlength(templatepath);
    if(depth >= plen) {THROWCHK(ncstat=NC_EINVAL); goto done;}

    lastnode = (depth == (plen-1));
    templatepathnode = (CDFnode*)nclistget(templatepath,depth);
    ASSERT((simplenodematch34(xnode,templatepathnode)));
    setattach(xnode,templatepathnode);    
#ifdef DEBUG2
fprintf(stderr,"attachnode: %s->%s\n",xnode->ocname,templatepathnode->ocname);
#endif

    if(lastnode) goto done; /* We have the match and are done */

    if(nclistlength(xnode->array.dimsetall) > 0) {
	attachdims34(xnode,templatepathnode);
    }

    ASSERT((!lastnode));
    templatepathnext = (CDFnode*)nclistget(templatepath,depth+1);

    gridable = (templatepathnext->nctype == NC_Grid && depth+2 < plen);

    /* Try to find an xnode subnode that matches templatepathnext */
    for(i=0;i<nclistlength(xnode->subnodes);i++) {
        CDFnode* xsubnode = (CDFnode*)nclistget(xnode->subnodes,i);
        if(simplenodematch34(xsubnode,templatepathnext)) {
	    ncstat = attach34r(xsubnode,templatepath,depth+1);
	    if(ncstat) goto done;
        } else if(gridable && xsubnode->nctype == NC_Atomic) {
            /* grids may or may not appear in the datadds;
	       try to match the xnode subnodes against the parts of the grid
	    */
   	    CDFnode* templatepathnext2 = (CDFnode*)nclistget(templatepath,depth+2);
	    if(simplenodematch34(xsubnode,templatepathnext2)) {
	        ncstat = attach34r(xsubnode,templatepath,depth+2);
                if(ncstat) goto done;
	    }
	}
    }
done:
    return THROW(ncstat);
}

NCerror
attach34(CDFnode* xroot, CDFnode* template)
{
    NCerror ncstat = NC_NOERR;
    NClist* templatepath = nclistnew();
    CDFnode* ddsroot = template->root;

    if(xroot->attachment) unattach34(xroot);
    if(ddsroot != NULL && ddsroot->attachment) unattach34(ddsroot);
    if(!simplenodematch34(xroot,ddsroot))
	{THROWCHK(ncstat=NC_EINVAL); goto done;}
    collectnodepath3(template,templatepath,WITHDATASET);
    ncstat = attach34r(xroot,templatepath,0);
done:
    nclistfree(templatepath);
    return ncstat;
}

/* 
Match nodes in template tree to nodes in target tree;
template tree is typically a structural superset of target tree.
WARNING: Dimensions are not attached 
*/

NCerror
attachsubset34(CDFnode* target, CDFnode* template)
{
    NCerror ncstat = NC_NOERR;

    if(template == NULL) {THROWCHK(ncstat=NC_NOERR); goto done;}
    if(!nodematch34(target,template)) {THROWCHK(ncstat=NC_EINVAL); goto done;}
#ifdef DEBUG2
fprintf(stderr,"attachsubset: target=%s\n",dumptree(target));
fprintf(stderr,"attachsubset: template=%s\n",dumptree(template));
#endif
    ncstat = attachsubset34r(target,template);
done:
    return ncstat;
}

static NCerror
attachsubset34r(CDFnode* target, CDFnode* template)
{
    unsigned int i;
    NCerror ncstat = NC_NOERR;
    int fieldindex;

#ifdef DEBUG2
fprintf(stderr,"attachsubsetr: attach: target=%s template=%s\n",
	target->ocname,template->ocname);
#endif

    ASSERT((nodematch34(target,template)));
    setattach(target,template);

    /* Try to match target subnodes against template subnodes */

    fieldindex = 0;
    for(fieldindex=0,i=0;i<nclistlength(template->subnodes) && fieldindex<nclistlength(target->subnodes);i++) {
        CDFnode* templatesubnode = (CDFnode*)nclistget(template->subnodes,i);
        CDFnode* targetsubnode = (CDFnode*)nclistget(target->subnodes,fieldindex);
        if(nodematch34(targetsubnode,templatesubnode)) {
#ifdef DEBUG2
fprintf(stderr,"attachsubsetr: match: %s :: %s\n",targetsubnode->ocname,templatesubnode->ocname);
#endif
            ncstat = attachsubset34r(targetsubnode,templatesubnode);
   	    if(ncstat) goto done;
	    fieldindex++;
	}
    }
done:
    return THROW(ncstat);
}

static void
getalldims34a(NClist* dimset, NClist* alldims)
{
    int i;
    for(i=0;i<nclistlength(dimset);i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimset,i);
	if(!nclistcontains(alldims,(ncelem)dim)) {
#ifdef DEBUG3
fprintf(stderr,"getalldims: %s[%lu]\n",
			dim->ncfullname,(unsigned long)dim->dim.declsize);
#endif
	    nclistpush(alldims,(ncelem)dim);
	}
    }
}

/* Accumulate a set of all the known dimensions
   vis-a-vis defined variables
*/
NClist*
getalldims34(NCDAPCOMMON* nccomm, int visibleonly)
{
    int i;
    NClist* alldims = nclistnew();
    NClist* varnodes = nccomm->cdf.varnodes;

    /* get bag of all dimensions */
    for(i=0;i<nclistlength(varnodes);i++) {
	CDFnode* node = (CDFnode*)nclistget(varnodes,i);
	if(!visibleonly || node->visible) {
	    getalldims34a(node->array.dimsetall,alldims);
	}
    }
    return alldims;
}
